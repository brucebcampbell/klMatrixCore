/*******************************
* Copyright (c) <2007>, <Bruce Campbell> All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  *  
* Bruce B Campbell 07 08 2014  *
********************************/
#ifndef __kl_matlab_iface__
#define __kl_matlab_iface__
#include <string>
#include <stdio.h>
#include <malloc.h>
#include <fstream>

#include "mat.h"  //Matlab Iface include file
#include "engine.h"  //Matlab Iface include file
#include "kl_matrix.h"

typedef enum klHoldOnStatus{  NoHold=0, FirstPlot=1, HoldOn=2, LastPlot=3};

class klMatlabEngineThreadMap 
{
public:

	void SetGlobalEngine (Engine* theEngine)
	{	klGuard<klMutex> lg(lock);//bbcrevisit -we need to justify/ verify nothing bad [ie race deadlock ]can happen
	klThreadId thisThread=klThread<klMutex>::getCurrentThreadId();
	cout<<"Setting Global Matlabl Engine From TID: "<<thisThread<<endl;
	theStaticEngine=theEngine;
	}

	void EvalMatlabString(const char* evalString)
	{
		klGuard<klMutex> lg(lock);
		klThreadId thisThread=klThread<klMutex>::getCurrentThreadId();
		cout<<"Locking and Using  Global Matlab Engine Frtom TID: "<<thisThread<<endl;
		//Thread blocking until Matlab allows more than one engine per process.
		if(this->theStaticEngine!=0)
		{
			engEvalString(theStaticEngine, evalString);
		}
	}

	void insert (klThreadId id, Engine * matlabEngine)
	{
		klGuard<klMutex> guard(lock);
		engineMap[id] = matlabEngine;
	}
	//This removes the engine from the map and closes the connection to Matlab.
	void remove (klThreadId id)
	{
		klGuard<klMutex> guard(lock);

		map<klThreadId, Engine * >::iterator i = engineMap.find(id);
		if (i != engineMap.end())
		{
			engClose((*i).second);
			engineMap.erase (i);
		}
	}
	static Engine* find (klThreadId id)
	{
		klGuard<klMutex> guard(lock);

		map<klThreadId, Engine*>::iterator i = engineMap.find(id);
		if (i == engineMap.end())
			return 0;

		return (*i).second;

	}
public :
	static klMutex lock;

	//This is for multiple engines in a single process
	static map<klThreadId, Engine*> engineMap;

	//static Engine* engine;//bbcrevisit remove

	static Engine* theStaticEngine;
};

//Pass in Tex for title, xAxix, and yAxis.
//useExtents directs kl to set the scale of the axis according to the 
//domain and range set in klVector<TYPE>
//setting start & finish directs the output to be windowed to that range
//start and finish are 0-based indices to the elements of c - bbc in case we need to index a subset?  Could do better.  Remove or improve this.
//hold on indicates multiple plots will be made to one gcf and the user will save the file.
//use the color attribute to specify a matlab color; r,g,b,c,m,y,k or [r,g,b] - use a ' before and after the spec
template<class TYPE> void klPlot1D(klVector<TYPE>&  c,const char* filename,
	const char* title=NULL,const char* xAxis=NULL,const char* yAxis=NULL,
	bool useExtents=true, unsigned int start=0,unsigned int finish=0,klHoldOnStatus holdOn= klHoldOnStatus::NoHold,const char* markerType=NULL)

{
	klMatlabEngineThreadMap klmtm;

	Engine* matlabEngine=klmtm.find(klThread<klMutex>::getCurrentThreadId() );

	//How to get the error message from matlab engine
	char errmsg[1024];
	errmsg[1023] = '\0';
	engOutputBuffer(matlabEngine, errmsg, 512);

	mxArray *T = NULL, *a = NULL, *d = NULL;

	unsigned int range=c.getRowSize();

	TYPE x0 =c.x0;
	TYPE x1 =c.x1;

	if(start==0 && finish ==0)
		T = mxCreateDoubleMatrix(1, c.getRowSize(), mxREAL);
	else
	{
		if(start>finish || finish>=c.getRowSize())
		{
			ANSI_INFO; throw klError(err + "kllot1D: ERROR bad domain for window.");
		}
		range=finish-start;


		T = mxCreateDoubleMatrix(1, range+1, mxREAL);
	}
	unsigned int i;
	double* pMx=mxGetPr(T);
	if(start==0 && finish ==0)
	{
		for(i=0;i<c.getRowSize();i++)
		{
			*(pMx+i)=(double)c[i];
		}

	}
	else
	{
		for(i=start;i<=finish;i++)
		{
			*( pMx+ (i-start)  )=(double)c[i];
		}

	}
	engPutVariable(matlabEngine, "T", T);

	char* evalString=new char[2048];

	char* colorSpec = new char[256];
	if( markerType==NULL)
	{
		sprintf(colorSpec,"'b'");
	}
	else
	{
		sprintf(colorSpec,"%s",markerType);
	}

	if(c.x1>c.x0)
	{
		mxArray *Tx = NULL;
		Tx = mxCreateDoubleMatrix(1, c.getRowSize(), mxREAL);
		double* pTx=mxGetPr(Tx);
		double dx = (c.x1-c.x0)/c.getRowSize();
		for(i=0;i<c.getRowSize();i++)
		{
			*(pTx+i)=(double)c.x0 + i*dx;
		}
		engPutVariable(matlabEngine, "Tx", Tx);

		if(holdOn == klHoldOnStatus::NoHold)
		{
			sprintf(evalString,"figure('Visible','off');plot(Tx,T,%s);",colorSpec);

		}
		if(holdOn==klHoldOnStatus::FirstPlot )
		{
			sprintf(evalString,"figure('Visible','off');plot(Tx,T,%s);hold on;",colorSpec);

		}
		if(holdOn==klHoldOnStatus::HoldOn || holdOn == klHoldOnStatus::LastPlot)
		{
			sprintf(evalString,"plot(Tx,T,%s)",colorSpec);
		}
	}
	else
	{
		if(holdOn == klHoldOnStatus::NoHold)
		{
			sprintf(evalString,"figure('Visible','off');plot(T,%s);",colorSpec);

		}
		if(holdOn==klHoldOnStatus::FirstPlot )
		{
			sprintf(evalString,"figure('Visible','off');plot(T,%s);hold on;",colorSpec);

		}
		if(holdOn==klHoldOnStatus::HoldOn || holdOn == klHoldOnStatus::LastPlot)
		{
			sprintf(evalString,"plot(T,%s)",colorSpec);
		}
		

	}
	engEvalString(matlabEngine, evalString);

	if(holdOn ==klHoldOnStatus::LastPlot || holdOn==klHoldOnStatus::NoHold)
	{
		if(title!=NULL)
		{
			sprintf(evalString,"title('%s');",title);//title({'This title','has 2 lines'}) % 
			engEvalString(matlabEngine, evalString);
		}	

		if(xAxis!=NULL)
		{
			sprintf(evalString,"xlabel('%s');",xAxis);
			engEvalString(matlabEngine, evalString);
		}
		if(yAxis!=NULL)
		{
			sprintf(evalString,"ylabel('%s');",yAxis);
			engEvalString(matlabEngine, evalString);
		}
		if(c.y0<c.y1)
		{
			sprintf(evalString,"set(gca,'YTick',%f:%f:%f);",c.y0,(c.y1-c.y0)/20,c.y1);
			//axis([xmin,xmax,ymin,ymax])
			//add 10% to the top and bottom
			sprintf(evalString,"axis([%f %f %f %f])",c.x0-(c.x1-c.x0)*.1 ,c.x1+(c.x1-c.x0)*.1,c.y0-(c.y1-c.y0)*.1,c.y1+(c.y1-c.y0)*.1);
			engEvalString(matlabEngine, evalString);
		}
		sprintf(evalString,"saveas(gcf,'%s');",filename);
		engEvalString(matlabEngine, evalString);
	}
	mxDestroyArray(T);
	delete evalString;
}

//New scatterplot 050314 - I find this interface hard to interpret.  Agreed with the code comments above that
//this could be improved.  The start and finish parameters are misleading - and I am not sure if they work.
//bbcrevisit - make a git task to document and improve this. For the scatter plot we don't need the range  
template<class TYPE> void klScatterPlot2D(klVector<TYPE>&  x,klVector<TYPE>&  y,const char* filename,
	const char* title=NULL,const char* xAxis=NULL,const char* yAxis=NULL,
	bool useExtents=true, klHoldOnStatus holdOn= klHoldOnStatus::NoHold,const char* markerType=NULL)
{
	klMatlabEngineThreadMap klmtm;

	if(x.getRowSize() !=y.getRowSize())
	{
		ANSI_INFO; throw klError(err + "x.getRowSize() !=y.getRowSize() in template<class TYPE> void klScatterPlot2D(klVector<TYPE>&  x,klVector<TYPE>&  y,const char* filename...");
	}

	Engine* matlabEngine=klmtm.find(klThread<klMutex>::getCurrentThreadId() );

	//How to get the error message from matlab engine
	char errmsg[1024];
	errmsg[1023] = '\0';
	engOutputBuffer(matlabEngine, errmsg, 512);

	mxArray *Tx = NULL, *Ty=NULL ,*a = NULL, *d = NULL;

	Tx = mxCreateDoubleMatrix(1, x.getRowSize(), mxREAL);

	Ty = mxCreateDoubleMatrix(1, y.getRowSize(), mxREAL);

	unsigned int i;
	double* pMx=mxGetPr(Tx);

	double* pMy=mxGetPr(Ty);

	for(i=0;i<x.getRowSize();i++)
	{
		*(pMx+i)=(double)x[i];
		*(pMy+i)=(double)y[i];		
	}
	engPutVariable(matlabEngine, "Tx", Tx);

	engPutVariable(matlabEngine, "Ty", Ty);

	char* evalString=new char[2048];

	char* colorSpec = new char[256];
	if( markerType==NULL)
	{
		sprintf(colorSpec,"'b.'");
	}
	else
	{
		sprintf(colorSpec,"%s",markerType);
	}
	if(holdOn == klHoldOnStatus::NoHold)
	{
		sprintf(evalString,"figure('Visible','off');scatter(Tx,Ty,%s);",colorSpec);

	}
	if(holdOn==klHoldOnStatus::FirstPlot )
	{
		sprintf(evalString,"figure('Visible','off');scatter(Tx,Ty,%s);hold on;",colorSpec);

	}
	if(holdOn==klHoldOnStatus::HoldOn || holdOn == klHoldOnStatus::LastPlot)
	{
		sprintf(evalString,"scatter(Tx,Ty,%s)",colorSpec);
	}

	engEvalString(matlabEngine, evalString);

	engOutputBuffer(matlabEngine, errmsg, 512);

	if(holdOn ==klHoldOnStatus::LastPlot || holdOn==klHoldOnStatus::NoHold)
	{
		if(title!=NULL)
		{
			sprintf(evalString,"title('%s');",title);//title({'This title','has 2 lines'}) % 
			engEvalString(matlabEngine, evalString);
		}	

		if(xAxis!=NULL)
		{
			sprintf(evalString,"xlabel('%s');",xAxis);
			engEvalString(matlabEngine, evalString);
		}
		if(yAxis!=NULL)
		{
			sprintf(evalString,"ylabel('%s');",yAxis);
			engEvalString(matlabEngine, evalString);
		}

		sprintf(evalString,"saveas(gcf,'%s');",filename);
		engEvalString(matlabEngine, evalString);
	}
	mxDestroyArray(Tx);
	mxDestroyArray(Ty);
	delete evalString;
	delete colorSpec;
}

//Meshplot of 3-D Data
template<class TYPE> void klScatterPlot3D(klMatrix<TYPE>&  c,const char* filename,
	const char* title=NULL,const char* xAxis=NULL,const char* yAxis=NULL,const char* zAxis=NULL,
	bool useExtents=true,klHoldOnStatus holdOn= klHoldOnStatus::NoHold,const char* markerType=NULL)
{
	klMatlabEngineThreadMap klmtm;

	Engine* matlabEngine=klmtm.find(klThread<klMutex>::getCurrentThreadId() );

	mxArray *T = NULL, *a = NULL, *d = NULL;

	if(c.getColumns() !=3)
	{
		ANSI_INFO; throw klError(err + " template<class TYPE> void klMatlabPlot3D(klMatrix<TYPE>  - ERROR :  needs 3 dimensional data");
	}

	T = mxCreateDoubleMatrix(c.getRows(),c.getColumns(), mxREAL);
	klMatrix<TYPE>  ctr=c.transpose();
	double* pd=mxGetPr(T);
	memcpy((char *) mxGetPr(T), (char *) ctr.getMemory(), ctr.getColumns()*ctr.getRows()*sizeof(TYPE));
	engPutVariable(matlabEngine, "T", T);

	char* evalString=new char[2048];
	d = engGetVariable(matlabEngine, "T");
	//How to get the error message from matlab engine
	char errmsg[1024];
	errmsg[1023] = '\0';
	engOutputBuffer(matlabEngine, errmsg, 512);

	char* colorSpec = new char[256];
	if( markerType==NULL)
	{
		sprintf(colorSpec,"'b.'");
	}
	else
	{
		sprintf(colorSpec,"%s",markerType);
	}
	if(holdOn == klHoldOnStatus::NoHold)
	{
		sprintf(evalString,"figure('Visible','off');scatter3(T(:,1),T(:,2),T(:,3),%s);",colorSpec);

	}
	if(holdOn==klHoldOnStatus::FirstPlot )
	{
		sprintf(evalString,"figure('Visible','off');scatter3(T(:,1),T(:,2),T(:,3),%s);hold on;",colorSpec);

	}
	if(holdOn==klHoldOnStatus::HoldOn || holdOn == klHoldOnStatus::LastPlot)
	{
		sprintf(evalString,"scatter3(T(:,1),T(:,2),T(:,3),%s)",colorSpec);
	}

	engEvalString(matlabEngine, evalString);

	if(holdOn ==klHoldOnStatus::LastPlot || holdOn==klHoldOnStatus::NoHold)
	{
		if(title!=NULL)
		{
			sprintf(evalString,"title('%s');",title);//title({'This title','has 2 lines'}) % 
			engEvalString(matlabEngine, evalString);
		}	

		if(xAxis!=NULL)
		{
			sprintf(evalString,"xlabel('%s');",xAxis);
			engEvalString(matlabEngine, evalString);
		}


		if(yAxis!=NULL)
		{
			sprintf(evalString,"ylabel('%s');",yAxis);
			engEvalString(matlabEngine, evalString);
		}

		if(zAxis!=NULL)
		{
			sprintf(evalString,"zlabel('%s');",zAxis);
			engEvalString(matlabEngine, evalString);
		}
		sprintf(evalString,"h=kp; saveas(h,'%s');",filename);
		engEvalString(matlabEngine, evalString);

	}
	mxDestroyArray(T);
	delete evalString;
	delete colorSpec;
}


template<class TYPE> void klHeatMapPlot(klMatrix<TYPE>&  c,const char* filename,
	const char* title=NULL,const char* xAxis=NULL,const char* yAxis=NULL,const char* zAxis=NULL,
	bool useExtents=true)
{
	klMatlabEngineThreadMap klmtm;

	Engine* matlabEngine=klmtm.find(klThread<klMutex>::getCurrentThreadId() );

	mxArray *T = NULL, *a = NULL, *d = NULL;

	T = mxCreateDoubleMatrix(c.getRows(),c.getColumns(), mxREAL);
	klMatrix<TYPE>  ctr=c.transpose();
	double* pd=mxGetPr(T);
	memcpy((char *) mxGetPr(T), (char *) c.getMemory(), ctr.getColumns()*ctr.getRows()*sizeof(TYPE));
	engPutVariable(matlabEngine, "T", T);

	char* evalString=new char[2048];
	d = engGetVariable(matlabEngine, "T");
	//How to get the error message from matlab engine
	char errmsg[1024];
	errmsg[1023] = '\0';
	engOutputBuffer(matlabEngine, errmsg, 512);

	sprintf(evalString,"figure('Visible','off');kp=imagesc(T);");

	engEvalString(matlabEngine, evalString);

	if(title!=NULL)
	{
		sprintf(evalString,"title('%s');",title);
		engEvalString(matlabEngine, evalString);
	}	

	if(xAxis!=NULL)
	{
		sprintf(evalString,"xlabel('%s');",xAxis);
		engEvalString(matlabEngine, evalString);
	}

	if(yAxis!=NULL)
	{
		sprintf(evalString,"ylabel('%s');",yAxis);
		engEvalString(matlabEngine, evalString);
	}

	if(zAxis!=NULL)
	{
		sprintf(evalString,"zlabel('%s');",zAxis);
		engEvalString(matlabEngine, evalString);
	}

	sprintf(evalString,"h=kp; saveas(h,'%s');",filename);
	engEvalString(matlabEngine, evalString);

	mxDestroyArray(T);

	delete evalString;
}

template<class TYPE> void klPlotHistogram(klVector<TYPE>&  c,const char* filename,
	const char* title=NULL,const char* xAxis=NULL,const char* yAxis=NULL,
	klHoldOnStatus holdOn= klHoldOnStatus::NoHold,const char* markerType=NULL)
{
	klMatlabEngineThreadMap klmtm;

	Engine* matlabEngine=klmtm.find(klThread<klMutex>::getCurrentThreadId() );

	//How to get the error message from matlab engine
	char errmsg[1024];
	errmsg[1023] = '\0';
	engOutputBuffer(matlabEngine, errmsg, 512);

	mxArray *T = NULL, *a = NULL, *d = NULL;

	TYPE x0 =c.x0;
	TYPE x1 =c.x1;

	char* evalString=new char[2048];

	char* colorSpec = new char[256];
	if( markerType==NULL)
	{
		sprintf(colorSpec,"'b'");
	}
	else
	{
		sprintf(colorSpec,"%s",markerType);
	}

	//Put the variable in Matlab
	T = mxCreateDoubleMatrix(1, c.getRowSize(), mxREAL);
	unsigned int i;
	double* pMx=mxGetPr(T);
	for(i=0;i<c.getRowSize();i++)
	{
		*(pMx+i)=(double)c[i];
	}
	engPutVariable(matlabEngine, "T", T);

	unsigned int numBins=std::floor(c.getRows()/10.0);
	if (numBins > 200 )
		numBins = 200;
	if (numBins <10)
		numBins =10;

	sprintf(evalString,"figure('Visible','off');hist(T,%d)",numBins);

	engEvalString(matlabEngine, evalString);

	if(holdOn !=klHoldOnStatus::HoldOn)
	{	
		if(title!=NULL)
		{
			//title({'This title','has 2 lines'}) %
			sprintf(evalString,"title('%s');",title); 
			engEvalString(matlabEngine, evalString);
		}	

		if(xAxis!=NULL)
		{
			sprintf(evalString,"xlabel('%s');",xAxis);
			engEvalString(matlabEngine, evalString);
		}
		if(yAxis!=NULL)
		{
			sprintf(evalString,"ylabel('%s');",yAxis);
			engEvalString(matlabEngine, evalString);
		}
		if(c.y0<c.y1)
		{
			sprintf(evalString,"set(gca,'YTick',%f:%f:%f);",c.y0,(c.y1-c.y0)/20,c.y1);
			sprintf(evalString,"axis([%f %f %f %f])",c.x0-(c.x1-c.x0)*.1 ,c.x1+(c.x1-c.x0)*.1,c.y0-(c.y1-c.y0)*.1,c.y1+(c.y1-c.y0)*.1);
			engEvalString(matlabEngine, evalString);
		}
		sprintf(evalString,"saveas(gcf,'%s');",filename);
		engEvalString(matlabEngine, evalString);
	}
	mxDestroyArray(T);
	delete evalString;
}


template<class TYPE> klMatrix<TYPE> klMatlabImportMatrix(char* filen,TYPE dummy);
template<class TYPE> klMatrix<TYPE> klMatlabImportMatrix(char* file,TYPE dummy)
{
	MATFile *pmat;
	mxArray *pa;
	const char **dir;
	const char *name;
	int	  ndir;
	int i;
	unsigned int j;

	pmat = matOpen(file, "r");
	if (pmat == NULL)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix :Error opening file");
	}
	dir = (const char **)matGetDir(pmat, &ndir);
	if (dir == NULL)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix :Error reading directory of file");
	} 
	mxFree(dir);

	// In order to use matGetNextXXX correctly, reopen file to read in headers. 
	if (matClose(pmat) != 0) 
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix: Error closing file");
	}
	pmat = matOpen(file, "r");
	if (pmat == NULL) 
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix :Error reopening file");

	}
	for (i=0; i < ndir; i++) 
	{
		pa = matGetNextVariableInfo(pmat, &name);
		if (pa == NULL) 
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix :Error reading in file");
		}
		//Verify all dimensions are 2
		int sz=mxGetNumberOfDimensions(pa);
		if (sz!=2)
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix :only two dimensional arrays supported");
		}
		mxDestroyArray(pa);
	}

	// Reopen file to read in actual arrays. 
	if (matClose(pmat) != 0)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix: Error closing file");
	}
	pmat = matOpen(file, "r");
	if (pmat == NULL)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix: Error opening file");

	}

	// Read in each array.
	for (i=0; i<ndir; i++)
	{
		pa = matGetNextVariable(pmat, &name);
		if (pa == NULL)
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix :Error reading in file");
		} 
	}

	if (matClose(pmat) != 0)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix: Error closing file");
	}

	unsigned long rows, columns;
	rows =mxGetM(pa);
	columns = mxGetN(pa);
	int bytes =mxGetElementSize(pa);
	if(bytes==0)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatlabImportMatrix: unrecognized data type - possible cell array");
	}

	klMatrix<TYPE> data(rows,columns);

	// void* pData=mxGetData(pa);
	double* pData =mxGetPr(pa);

	for(i=0;i<rows;i++)
	{
		for(j=0;j<columns;j++)
		{
			double* l=(pData + j*rows +i);
			//data[i][j]=static_cast<TYPE>( *l );//(TYPE)*(pData + j*bytes*rows +i);
			data[i][j]=(TYPE)(*l);

		}
	}
	return data;
}

template<class TYPE>void klMatlabExportVector(klVector<TYPE>& vector, char* filename_prefix,TYPE dummy);
template<class TYPE> void klMatlabExportVector(klVector<TYPE>& vector, char* filename_prefix,TYPE dummy)
{
	char* filename=new char[256];
	sprintf(filename,"%s.mat",filename_prefix);

	MATFile *pmat;
	mxArray *pa1;
	pmat = matOpen(filename, "w");
	unsigned long dataSize=vector.getRowSize();
	if (pmat == NULL) {printf("Error creating file %s\n", filename);    printf("(Do you have write permission in this directory?)\n");  }
	pa1 = mxCreateDoubleMatrix(1,dataSize,mxREAL);
	if (pa1 == NULL ) {printf("%s : Out of memory on line %d\n", __FILE__, __LINE__);printf("Unable to create mxArray.\n");}
	char* name1=new char[256];
	//Copy Vector to double array...
	double* data1;
	if(sizeof(TYPE)!=sizeof(double))
	{
		int j;
		data1= new double[dataSize];
		for(j=0;j<dataSize;j++)
		{
			*(data1+j)=vector[j];
		}
	}
	else
		data1=vector.getMemory();
	memcpy((void *)(mxGetPr(pa1)), (void *)data1, sizeof(double)*dataSize);  
	sprintf(name1,"%s",filename_prefix);
	int status=0;
	status |= matPutVariable(pmat,name1, pa1);
	if (status != 0) {printf("%s :  Error using matPutArray on line %d\n", __FILE__, __LINE__); }  
	mxDestroyArray(pa1);
	if (matClose(pmat) != 0) {printf("Error closing file %s\n",filename);}
}
template<class TYPE>void klMatlabExportMatrix(klMatrix<TYPE>& matrix, char* filename_prefix,TYPE dummy);
template<class TYPE> void klMatlabExportMatrix(klMatrix<TYPE>& matrix, char* filename_prefix,TYPE dummy)
{
	char* filename=new char[256];
	sprintf(filename,"%s.mat",filename_prefix);
	MATFile *pmat;
	mxArray *pa1;
	unsigned long rows, columns;
	rows=matrix.getRows();
	columns=matrix.getColumns();
	pmat = matOpen(filename, "w");
	if (pmat == NULL) {printf("Error creating file %s\n", filename);    printf("(Do you have write permission in this directory?)\n");  }
	pa1 = mxCreateDoubleMatrix(rows,columns,mxREAL);
	if (pa1 == NULL ) {printf("%s : Out of memory on line %d\n", __FILE__, __LINE__);printf("Unable to create mxArray.\n");}
	char* name1=new char[256];
	sprintf(name1,"%s",filename_prefix);
	double* data1;
	int i,j;
	unsigned long size=rows*columns;

	if(sizeof(TYPE)==sizeof(double))
	{
		data1=(double*)matrix.transpose().getMemory();
		memcpy((void *)(mxGetPr(pa1)), (void *)data1, sizeof(double)*rows*columns);  
	}
	else
	{
		data1=new double[size];
		for(i=0;i<columns;i++)
		{
			for(j=0;j<rows;j++)
			{
				klMatrix<TYPE> t=matrix.transpose();

				*(data1+i*rows+j)=(double)t[i][j];
			}
		}
		memcpy((void *)(mxGetPr(pa1)), (void *)data1, sizeof(double)*rows*columns);  
		delete data1;
	}
	int status=0;
	status |= matPutVariable(pmat, name1, pa1);
	if (status != 0) {printf("%s :  Error using matPutArray on line %d\n", __FILE__, __LINE__); }  
	mxDestroyArray(pa1);
	if (matClose(pmat) != 0) {printf("Error closing file %s\n",filename);}
}

#endif