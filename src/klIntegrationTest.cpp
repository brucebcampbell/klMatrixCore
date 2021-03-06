 /*******************************
 * Copyright (c) <2007>, <Bruce Campbell> All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  *    *  
 * Bruce B Campbell 05 27 2013  *
 ********************************/
#include "kl_matrix.h"
#include "kl_stat.h"
#include "kl_random_number_generator.h"
#include "kl_time_series.h"
#include "kl_multivariate_random_variable.h"
#include "kl_sample_population.h"
#include "kl_principal_components.h"
#include "kl_regression.h"
#include "kl_multiclass_svm.h"
#include "kl_wavelet.h"
#include "kl_matrix_helper_fns.h"
#include "kl_divergence_functions.h"
#include "kl_util.h"
#include "kl_unit_tests.h"
#include "kl_matrix_facorizations.h"
#include "kl_unit_test_wrapper.h"
#include "kl_matlab_dependent_unit_tests.h"
#include "kl_matlab_iface.h"
#include "kl_arpack.h"
#include "kl_latex_helper_fns.h"
#include "kl_algorithm_paramters.h"
#include "kl_fast_gauss_transform.h"
#include "kl_point_cloud_generator.h"
#include <math.h>
char* basefilename; 

static klTestType klTestSize= klTestType::SMALL;

//klMutex klMatlabEngineThreadMap::lock;
//map<klThreadId, Engine*> klMatlabEngineThreadMap::engineMap;
//DWORD gdwTlsIndex;
void __cdecl klNewHandler( )
{
	char* arg=new char[256];
	sprintf(arg,"klNewHandlerCalled.txt");
	ios_base::openmode wMode = ios_base::app;
	ofstream _tex(arg,wMode);

	klPrintModules(_tex);
	klWinMemoryInfo(_tex);
	klWinSystemInfo(_tex);

	_tex.close();
	throw bad_alloc( );
	return;
}

void RandomBallCoverTest(ofstream &_tex, klAlgorithmParameterContainer& klapc);
void FastGaussTransformTest(ofstream &_tex, klAlgorithmParameterContainer& klapc );
void MatrixNormTest(ofstream &_tex,__int64_t  &n);
void MatrixEigenSolverTest(ofstream &_tex,__int64_t  &n);
void MatrixOpsQuickCheck(ofstream &_tex,__int64_t  &n);//In unit test header
void SemidefiniteProgrammingTest(ofstream &_tex,__int64_t  &n);
void PrincipalComponentsTest(ofstream &_tex,__int64_t  &n);
void MemoryManagement(ofstream &_tex,__int64_t  &n);
void IterativeKrylovCheck(ofstream &_tex,__int64_t  &n,const char* fileName=NULL);
void GenerateTraceyWidomSample(ofstream &_tex,__int64_t  &n);
void Utility(ofstream &_tex,__int64_t  &n);
void MatrixMultiplicationCheck(ofstream &_tex,__int64_t  &n );
void LinearRegression(ofstream &_tex,__int64_t  &n);
void LinearRegressionAtanSet(ofstream &_tex,__int64_t  &n);
void MatrixExponential(ofstream &_tex,__int64_t  &n);
void MutithreadedWorkflow(void);
void VerifyWingerLaw(ofstream &_tex, __int64_t& n);
void GenerativeGramConsistencyCheck(ofstream &_tex,__int64_t  &n);
void Arpack_MKLsyevxSmokeTest(ofstream &_tex,__int64_t &n,const char* fileName);
void FEATSEigensolver(ofstream &_tex,__int64_t &n,const char* fileName);
void BinaryIO(ofstream &_tex,__int64_t &n);
void PointCloudAndLatexPlots(ofstream &_tex,__int64_t &n);
void RandomMatrixNorms(ofstream &_tex,__int64_t &n);
void ARPACK_VS_SYEVX(ofstream &_tex,unsigned int  &n);
void ConvertCSVMatrixFilesToBinFormat();
#include "kl_time_series.h"
#include "kl_random_number_generator.h"
void IteratedExponentialFiltering(ofstream &_tex,__int64_t &n);
#include "kl_vsl.h"
void VSLFunctions(ofstream &_tex,__int64_t &n);

#include <errno.h> 

void klIntegrationTest(bool useIntelMemMgr,klTestType klItegrationTestSize )
{
	basefilename = new char[2048];
	if(useIntelMemMgr)
	{
		if (klItegrationTestSize==klTestType::SMALL)
		{
			sprintf(basefilename,"small_data_IntelMemmgr");
			klTestSize= klTestType::SMALL;
		}
		if (klItegrationTestSize==klTestType::MEDIUM)
		{
			sprintf(basefilename,"medium_data_IntelMemmgr");
			klTestSize= klTestType::MEDIUM;
		}
		if (klItegrationTestSize==klTestType::LARGE)
		{
			sprintf(basefilename,"large_data_IntelMemmgr");
			klTestSize= klTestType::LARGE;
		}
		if (klItegrationTestSize==klTestType::VERYLARGE)
		{
			sprintf(basefilename,"verylarge_data_IntelMemmgr");
			klTestSize= klTestType::VERYLARGE;
		}
	}
	else
	{
		if (klItegrationTestSize==klTestType::SMALL)
		{
			sprintf(basefilename,"small_data_NoIntelMemmgr");
			klTestSize= klTestType::SMALL;
		}
		if (klItegrationTestSize==klTestType::MEDIUM)
		{
			sprintf(basefilename,"medium_data_NoIntelMemmgr");
			klTestSize= klTestType::MEDIUM;
		}
		if (klItegrationTestSize==klTestType::LARGE)
		{
			sprintf(basefilename,"large_data_NoIntelMemmgr");
			klTestSize= klTestType::LARGE;
		}
		if (klItegrationTestSize==klTestType::VERYLARGE)
		{
			sprintf(basefilename,"verylarge_data_NoIntelMemmgr");
			klTestSize= klTestType::VERYLARGE;
		}
	}
		
	klTimer klt;
	klt.tic();

	char* syscmd = new char[2048];
	sprintf(syscmd,"mkdir %s",basefilename);
	system(syscmd); 
	time_t time_of_day;
	struct tm *tm_buf;
	time_of_day = time( NULL );
	tm_buf=localtime(&time_of_day);
	char* testRunDateTime = new char[1024];
	char* regressionFile = new char[1024];
	char* coutFile = new char[1024];
	char* sysInfoFile = new char[1024];

	sprintf(testRunDateTime,"%d_%d_%d_%d_%d",tm_buf->tm_mon+1,tm_buf->tm_mday+1,tm_buf->tm_hour+1,tm_buf->tm_min+1,tm_buf->tm_sec+1);
	sprintf(regressionFile,"%skl_Regression%s.tex",basefilename,testRunDateTime);
	sprintf(coutFile,"%skl_cout%s.txt",basefilename,testRunDateTime);
	sprintf(sysInfoFile,"%skl_cout%s.txt",basefilename,testRunDateTime);

	string baseFileNameString(basefilename);

	FILE *stream;

#ifndef _DEBUG
	if((stream = freopen(coutFile, "a", stdout)) == NULL)
	{
		ANSI_INFO; throw klError(err + "kl: error redirecting std::cout to a file.");
	}
	cout<<"Redirecting std::cout to"<<coutFile<<"file via freopen."<<endl;
#endif

	ofstream _tex(regressionFile);

	ofstream _systemText(sysInfoFile);
		
	startLatexDoc("Regression of KL Software Distribution   ","KL Software Libraries",asctime(tm_buf),_tex, "");

	_tex<<"\\textbf{ KL Library test output.  This LaTex file and the associated diagrams are produced by the KL software libraries.}"<<endl;
	
	klUnitTestWrapper klutw(_tex,_systemText);

	__int64_t n = 512;
	klutw.setDimension(n);
	
	_tex.flush();
		

	if(useIntelMemMgr)
	{
		klMKLMemMgr*  mgr = new klMKLMemMgr();
		klGlobalMemoryManager::setklVectorGlobalMemoryManager((klMemMgr*)mgr);
	}
	else
	{
		klMemMgr*  mgr = NULL;
		klGlobalMemoryManager::setklVectorGlobalMemoryManager((klMemMgr*)mgr);
	}

	makeLatexSection("Matrix Quick Check <double>",_tex),
	klutw.runTest(MatrixOpsQuickCheck<double>);

	unsigned int di=0;
	ARPACK_VS_SYEVX(_tex,di);

	Utility(_systemText,n);

	MemoryManagement(_systemText,n);
	
	klutw.HardwareConfiguration(_systemText);
	MutithreadedWorkflow();
	
	makeLatexSection("Random Ball Cover ",_tex),
	klutw.runTest(RandomBallCoverTest);
	
	klutw.runTest( BinaryIO);

		
	klMatrix<double> lattice = generate2DHexagonalLattice(1.0f/10);
	stringstream fileName;stringstream title;
	fileName.str("");fileName.clear();
	title.str(""); title.clear();
	fileName<<"2DHexagonalLattice";
	title<<"2D Hexagonal Lattice";
	char* color="'c.'";
	//LatexInsert2DScatterPlot(lattice.getColumn(0),lattice.getColumn(1),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::NoHold, color);
	
	makeLatexSection("Linear Regression atan data 3x1",_tex);
	klutw.runTest(LinearRegressionAtanSet);	

	makeLatexSection("Linear Regression 3x1",_tex);
	klutw.runTest(LinearRegression);
//
//	makeLatexSection("Fast Gauss Transform",_tex);
//	klFastGaussAlgorithmParameters klfgp;
//	klfgp.serialize(baseFileNameString +"klFastGaussAlgorithmParameters_2D.klap");
//	klutw.setAlgorithmParameters(klfgp);
//	klutw.runTest(FastGaussTransformTest);
//
//	klfgp.setParameter(klAlgorithmParameter("Scale",1.0f /(8*1250.0f)));
//	klfgp.setParameter(klAlgorithmParameter("NumberOfCenters",(__int64_t) 24));
//	klfgp.setParameter(klAlgorithmParameter("NumberOfPoints",(__int64_t) 9984)) ;
//	klfgp.setParameter(klAlgorithmParameter("NumberOfSources",(__int64_t) 9984)) ;;
//	klutw.setAlgorithmParameters(klfgp);
//	klutw.runTest(FastGaussTransformTest);
//
//	klfgp.setParameter(klAlgorithmParameter("Scale",1.0f /(16*1250.0f)));
//	klfgp.setParameter(klAlgorithmParameter("NumberOfCenters",(__int64_t) 4));
//	klfgp.setParameter(klAlgorithmParameter("NumberOfPoints",(__int64_t) 4*400)) ;
//	klfgp.setParameter(klAlgorithmParameter("NumberOfSources",(__int64_t) 4*400)) ;;
//	klutw.setAlgorithmParameters(klfgp);
//	klutw.runTest(FastGaussTransformTest);
//
//	klfgp.setParameter(klAlgorithmParameter("NumberOfPoints",(__int64_t) 10000)) ;
//	klfgp.setParameter(klAlgorithmParameter("NumberOfSources",(__int64_t) 10000)) ;;
//	klfgp.setParameter(klAlgorithmParameter("NumberOfCenters",(__int64_t) 20)) ;;
//	klfgp.setParameter(klAlgorithmParameter("Dimension",(__int64_t) 3)) ;
//
//	klfgp.serialize(baseFileNameString +"klFastGaussAlgorithmParameters_3D.klap");
//
//	klutw.setAlgorithmParameters(klfgp);
//
	klutw.runTest(FastGaussTransformTest);

	makeLatexSection("Matrix Norms",_tex);
	klutw.runTest(MatrixNormTest);

	makeLatexSection("Principal Components Matlab ",_tex);
	klutw.runTest(PrincipalComponentsTest);

	makeLatexSection("Matrix Multiply",_tex);
	klutw.runTest(MatrixMultiplicationCheck);

	makeLatexSection("Descriptive Statistics",_tex);
	klutw.runTest(testKLDescriptiveStatistics<double>);

	makeLatexSection("Time Series ",_tex);
	klutw.runTest(testKLTimeSeries2<double>);

	klutw.runTest(RandomMatrixNorms);

	klutw.runTest(IteratedExponentialFiltering);
		
	klutw.runTest( PointCloudAndLatexPlots);

	klutw.runTest(VSLFunctions);

	klutw.runTest(GenerativeGramConsistencyCheck);
	
	klutw.runTest(MatrixEigenSolverTest);
		
	klutw.runTest(GenerateTraceyWidomSample);
	
	klutw.runTest(VerifyWingerLaw);
	
	makeLatexSection("Matrix Exponential ",_tex);
	klutw.runTest(MatrixExponential);

	
	makeLatexSection("Semidefinite Programming SDPA",_tex);
	klutw.runTest(SemidefiniteProgrammingTest);
	
	//makeLatexSection"Test Wavelet <double>",_tex);
	//klutw.runTest(testKLWavelet<double>);
	
	endLatexDoc(_tex);

	_tex.close();

	delete testRunDateTime;
	delete regressionFile;
	delete coutFile;
	delete sysInfoFile;
	delete basefilename;

	delete syscmd;

	double time = klt.toc();
	cout<<"klIntegrationTest run time " <<time<<endl;
}

void VerifyWingerLaw(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Approximate Winger Distribution",_tex);
	
	makeLatexSection("Verfy Winger Law.",_tex);

	_tex<<"Let $M_n = [X_{ij} ]$ a symmetric n x n matrix with Random entries such that $X_{i,j} = X_{j,i}$, \
		  and $X_{i,j}$ are iid $\forall i < j,$ and $Xjj$ are iid $\forall j  :  \; E[X^2_{ij} ] = 1, \& E[X_{ij}] = 0$ \
		  and that all moments exists for each of the entries.  \
		  The eigenvector of this random matrix; $ \lambda_1 \leq ... \leq \lambda_n$ depends continuously on $Mn$."<<endl;
	
	if (klTestSize==klTestType::VERYLARGE)
	{
     n=16384;
	}
	
	if (klTestSize==klTestType::LARGE)
	{
     n=4096;
	}

	if (klTestSize==klTestType::MEDIUM)
	{
     n=1024;
	}

	if (klTestSize==klTestType::SMALL)
	{
     n=512;
	}
	

	_tex<<"Dimension $n = "<<n<<"$"<<endl<<endl;
	
	klMatrix<double> A = SampleSymmetricStandardNormalRM(n);
	
	klVector< complex<double> >  eigs = A.eigenvalues();
	
//	LatexInsertHistogram(RE(eigs),100,_tex, basefilename,"Re_lambda_n","Histogram of Re(\\lambda_i) for $X$");
//	LatexInsertHistogram(IM(eigs),100,_tex, basefilename,"Im_lambda_n","Histogram of Im(\\lambda_i) for $X$");

	_tex.flush();

}

//I got a zero finally out of my Merssene Twistor (MT) random variate generator.  
//The range of the one I use is [0, 2^32-1].  It happened after 1,171,079,842 calls.
//I wrote a Normal RV  based on the  MT that uses the approximation to the inverse CDF of the normal distribution by 
//Peter J. Acklam.  See http://home.online.no/~pjacklam/notes/invnorm/index.html
//This method of generating normal variates avoids the undesirable side effects 
//found by Neave in H.R. Neave "On using the Box-Muller transformation with multiplicative congurential 
//psuedo-random number generators"  Applied Statistics. 22 pp92, 1973
//The uniform vairates produced by this generator are suitable for use with low discrepancy numbers and quasi-Monte Carlo simulations.
//Inside the routine to generate the next variate, I asserted that the underlying uniform would be in (0,1).
//I hit the assert when the MT returned zero.  The assert was there to avoid calling log(0) in the tail part.
//So, I changed the code 
//assert( 0 <= u && u <= 1 );
//if (u==0)
//{
//      u=DBL_MIN; //So we don't take log of 0 down below.
//}
//So that I'm taking log ( 2.2250738585072014e-308)
//For N(0,1/2) the zero in the MT then gives a value of z = -26.530234309983367.
//Then I calculate for z \approx N(0,1/2)
//CDF(-26.530234309983367) = P( z <= -26.530234309983367 ) =2.1775825702786199e-155
//I'm unappy with that
//My plan is to change the undelrying uniform generator (based on MT) of N(0,1/2)  to have a range of [1/(2^32-1),1] insatead of [0,1]
//And compare with Intel MKL RV.
void TestMerssenePeriodIssue()
{
	unsigned int seed = 4357;

	klMersenneTwister badNumber(seed);
	klNormalInverseApproxRV<double> N_0_frac_1_2(0,0.5,false,seed);
	unsigned int max = 1171079842;
	for(int i=0;i<= max;i++)
	{
		unsigned int a =0;
		double d =0;

		if(i==max-1 || i==max)
		{
			a = badNumber.nextValue();
			d= N_0_frac_1_2();
			double prob_X_lt_d =  N_0_frac_1_2.NormalCDF(d);

		}
		else
		{
			a = badNumber.nextValue();
			d= N_0_frac_1_2();
		}
		if(a==0)
		{
			std::cout<< "umm...WE HAVE A PROBLEM HOUSTON"<<endl;
		}
	}

	unsigned int n= 4096*10;
	klMatrix<double> A_GOE =SampleGOE(n);// SampleSymmetricStandardNormalRM(n);

	A_GOE /=n;
	unsigned int numEigenvalues= 16;//floor(n*.10);
	klArpackFunctor klaf;

	klVector<complex<double> > eigsAP = klaf.run( A_GOE,numEigenvalues);

	exit(42);
}

void GenerateTraceyWidomSample(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Generate Tracey Widom Sample",_tex);

	//n is the size of the matrix that will be sampled
	unsigned int m; //Number or samples to generate 
	if (klTestSize==klTestType::VERYLARGE)
	{
     	n=1024*2;
		m=1024*4;
	}
	
	if (klTestSize==klTestType::LARGE)
	{
		//With n = 1024 m= 1024 this test should take about five hours on a 12 core 3Gz 96GB RAM workstation
		n=1024;
		m=1024;
	}

	if (klTestSize==klTestType::MEDIUM)
	{
		n=512;
		m=512;
	}

	if (klTestSize==klTestType::SMALL)
	{
		n=128;
		m=32;
	}

	makeLatexSection("Sample from $W_n m$ times and calculate empirical PDF of the first eig",_tex);
	_tex<<"Here we generate histograms of $\\lambda_1$ for GOE (Gaussian Orthogonal Ensemble), and W (Wishart) \
		 distributed of random matrices"<<endl;
	_tex<<"These should approximate the celebrated Tracy Widom distribution."<<endl;

	_tex<<"Dimension $n = "<<n<<"$"<<endl<<endl;
	_tex<<"Sample size $m = "<<m<<"$"<<endl<<endl;
	klTimer dti;
	{	
		klVector<complex<double> > lambda_1_Hist(m);
		unsigned int i;
		//#pragma omp parallel num_threads(4)
		for(i=0;i<m;i++)
		{
			dti.tic();
			klMatrix<double> A(n,n);
			A=SampleWishart(n,i);
			klVector<complex<double> > eigs=A.eigenvalues();
			lambda_1_Hist[i]=eigs[0];
			double dt = dti.toc();
			cerr<<"dt("<<i<<") = "<<dt<<endl;
		}

		//Real part of first eig
		klVector<double> L_re(m);
		//Imaginary part of first eig 
		klVector<double> L_im(m);

		unsigned int j=0;
		for(j=0;j<m;j++)
		{
			L_re[j]=lambda_1_Hist[j].real();
			L_im[j]=lambda_1_Hist[j].imag();
		}
		
		char* fileNameTW= new char[1024];
		sprintf(fileNameTW,"%sTraceyWidom_re.txt",basefilename);
		ofstream fileostreamobj(fileNameTW );
		fileostreamobj<<L_re<<endl;
		fileostreamobj.close();

		sprintf(fileNameTW,"%sTraceyWidom_im.txt",basefilename);
		fileostreamobj.open(fileNameTW);
		fileostreamobj<<L_im<<endl;
		fileostreamobj.close();
		delete fileNameTW;
		 
		//LatexInsertHistogram(L_re,30,_tex, basefilename,"Re_TraceyWidom","Histogram of Re(\\lambda_1) A \\in W");
		//LatexInsertHistogram(L_im,30,_tex, basefilename,"Im_TraceyWidom","Histogram of Im(\\lambda_1) A \\in W ");
		_tex.flush();
	}

	{			
		klVector<complex<double> > lambda_1_Hist(m);
		unsigned int i;
		//#pragma omp parallel num_threads(4)
		for(i=0;i<m;i++)
		{
			klMatrix<double> A(n,n);
			A=SampleGOE(n,i);
			klVector<complex<double> > eigs=A.eigenvalues();
			lambda_1_Hist[i]=eigs[0];
		}

		//Real part of first eig
		klVector<double> L_re(m);
		//Imaginary part of first eig 
		klVector<double> L_im(m);

		unsigned int j=0;
		for(j=0;j<m;j++)
		{
			L_re[j]=lambda_1_Hist[j].real();
			L_im[j]=lambda_1_Hist[j].imag();
		}
		
		char* fileName= new char[1024];
		sprintf(fileName,"%sWinger_re.txt",basefilename);
		ofstream fileostreamobj(fileName );
		fileostreamobj<<L_re<<endl;
		fileostreamobj.close();

		sprintf(fileName,"%sWinger_im.txt",basefilename);
		fileostreamobj.open(fileName);
		fileostreamobj<<L_im<<endl;
		fileostreamobj.close();

		//LatexInsertHistogram(L_re,30,_tex, basefilename,"Re_Winger","Histogram of Re(\\lambda_1) A \\in GOE(1024)");
		//LatexInsertHistogram(L_im,30,_tex, basefilename,"Im_Winger","Histogram of Im(\\lambda_1) A \\in GOE(1024) ");
		_tex.flush();

		delete fileName;
	}



}

void MatrixMultiplicationCheck(ofstream &_tex,__int64_t  &n  )
{		
	{
		n=2048;
		_tex<<"Comparing naive matrix multiply verus Intel MKL dgemm for matrix of size "<<n<<"."<<endl;
		_tex<<"This is for type double (hence the d in dgemm)."<<endl;

		unsigned int m=n+128;
		unsigned int p=n+64;

		klMatrix<double> A(n,m);

		unsigned int i=0;
		unsigned int j=0;
		for(i=0;i<n;i++)
		{
			for(j=0;j<m;j++)
			{
				A[i][j]=i+j;
			}
		}

		klMatrix<double> B(m,p);
		for(i=0;i<m;i++)
		{
			for(j=0;j<p;j++)
			{
				B[i][j]=i+j;
			}
		}

		klMatrix<double> C = A*B;
		klMatrix<double> Cp(n,p);

		klMatrix<double> Ap =mmBLAS(1.0, A, B, 1.0,Cp);	

	}

	{
		n=2048;
		_tex<<"Comparing naive matrix multiply verus Intel MKL sgemm for matrix of size "<<n<<"."<<endl;
		_tex<<"This is for type float (hence the s in dgemm)."<<endl;

		unsigned int m=n+128;
		unsigned int p=n+64;

		klMatrix<float> A(n,m);

		unsigned int i=0;
		unsigned int j=0;
		for(i=0;i<n;i++)
		{
			for(j=0;j<m;j++)
			{
				A[i][j]=i+j;
			}
		}

		klMatrix<float> B(m,p);
		for(i=0;i<m;i++)
		{
			for(j=0;j<p;j++)
			{
				B[i][j]=i+j;
			}
		}

		klMatrix<float> C = A*B;

		klMatrix<float> Cp(n,p);	

		klMatrix<float> Ap =mmBLAS(1.0, A, B, 1.0,Cp);
			}
	_tex.flush();
}

void LinearRegression(ofstream &_tex,__int64_t &n)
{	
	time_t time_of_day;
	struct tm *tm_buf;
	time_of_day = time( NULL );
	tm_buf=localtime(&time_of_day);
	makeLatexSection("3 x 1 Linear Regression",_tex);

	unsigned int sampleSize=64;
	_tex<<"Sample size = "<<sampleSize<<endl<<endl;
	unsigned int numFeatures=3;
	_tex<<"Number of features = "<<numFeatures<<endl<<endl;

	klMatrix<double> X(sampleSize,numFeatures); X=0;
	klMatrix<double> Y(sampleSize,1);

	klUniformRV<double> uniformRV(0,1);
	klNormalInverseApproxRV<double> N_0_eps(0,0.05,false,42);
	unsigned int i;
	unsigned int j;

	klVector<double> mean(numFeatures);
	klVector<double> beta(numFeatures);
	klVector<double> zero(numFeatures);

	mean = 0;
	zero =0;
	for(i=0;i<numFeatures;i++)
	{
		beta[i]=uniformRV();
	}

	klMatrix<double> Sigma = klGenerateRandomSymmetricPositiveDefiniteMatrix<double> (numFeatures);
	LatexPrintMatrix(Sigma, "\\sigma",_tex);
	klNormalMultiVariate<double> features(zero,Sigma);
		
	// Y = \beta \dot X
	for(j=0;j<sampleSize;j++)
	{
		klVector<double> sample =features();
		cout<<sample<<endl;
		klVector<double > rowj =X[j];
		X.setRow(j, sample);
		double response = X[j].dot(beta);
		Y[j][0] =  response; 
	}

	//LatexInsert1DPlot( Y.getColumn(0), _tex, basefilename,"regression_response_no_noise","Regression Response No Noise");

	//Add niose to the features ; Y = (\beta \dot \ X )  +\epsilon
	for(j=0;j<sampleSize;j++)
	{
		double response = Y[j][0];
		response+= N_0_eps() ;
		Y[j][0] =  response;  //This works because Y is 1 dimensional, otherwise use setRow
	}
	//LatexInsert1DPlot( Y.getColumn(0), _tex, basefilename,"regression_response_with_noise","Regression Response With Noise");


	//bbcrevisit expand model to multiple linear regression and [standardize notation ]
	//Model Y=BX + \epsilon where \epsilon =_d N(\mathbf{0},mathbf{\Sigma})
	//Usually E[Y_i]=\mu_i=x_i^T \beta where Y_i =_d N(\mu_i,\sigma);

	//LatexInsert3DPlot(X, _tex, basefilename,"regression_features","Features");

	_tex<<"Beta"<<endl;

	_tex<<beta<<endl;
	_tex<<"Response"<<endl;
	_tex<<Y;
	klLinearRegression<double> R(X,Y);
	klMatrix<double> betahat=R();
	klout(betahat);
	klMatrix<double> betaEst=betahat.getSubBlock(0,0,2,0);
	_tex<<"Estimate for Beta"<<endl;
	_tex<<betaEst;

	_tex<<"Error:"<<endl;
	klVector<double> error(numFeatures);
	for(int j =0 ; j<numFeatures;j++)
	{
		double b1=betahat[j][0];
		double b2 =beta[j];
		error[j] = b1-b2;
	}
	_tex<<error<<endl<<endl;
	_tex.flush();
}

void LinearRegressionAtanSet(ofstream &_tex,__int64_t &n)
{
	unsigned int sampleSize=4000;
	klMatrix<double> dataSet(sampleSize,3);
	klVector<int> classLabels(sampleSize);
	generateSicknessManifoldDataSet(  dataSet , classLabels);
	//LatexInsert3DPlot(dataSet, _tex, basefilename,"AtanDataSet","Mixture of four 2d gussians, reponse is atan of x coord");
	
	time_t time_of_day;
	struct tm *tm_buf;
	time_of_day = time( NULL );
	tm_buf=localtime(&time_of_day);
	makeLatexSection("3 x 1 Linear Regression",_tex);
		
	_tex<<"Sample size = "<<sampleSize<<endl<<endl;
	unsigned int numFeatures=3;
	_tex<<"Number of features = "<<numFeatures<<endl<<endl;

	klMatrix<double> X(dataSet); ///bbc check shallow copy
	klMatrix<double> Y(sampleSize,1);

	klNormalInverseApproxRV<double> N_0_eps(0,0.05,false,42);
	unsigned int i;
	unsigned int j;

	klVector<double> beta(numFeatures);
			
	// Y = dataSet[*][2] + \epsilon
	for(j=0;j<sampleSize;j++)
	{
		double response = X[j][2];
		Y[j][0] =  response; 
	}

	//LatexInsert1DPlot( Y.getColumn(0), _tex, basefilename,"AtanDataSet_regression_response_no_noise","Regression Response No Noise");

	//Add niose to the features ; Y =  dataSet[*][2]  +\epsilon
	for(j=0;j<sampleSize;j++)
	{
		double response = Y[j][0];
		response+= N_0_eps() ;
		Y[j][0] =  response;  //This works because Y is 1 dimensional, otherwise use setRow
	}
	//LatexInsert1DPlot( Y.getColumn(0), _tex, basefilename,"AtanDataSet_regression_response_with_noise","Regression Response With Noise");
	
	//bbcrevisit expand model to multiple linear regression and [standardize notation ]
	//Model Y=BX + \epsilon where \epsilon =_d N(\mathbf{0},mathbf{\Sigma})
	//Usually E[Y_i]=\mu_i=x_i^T \beta where Y_i =_d N(\mu_i,\sigma);

	//LatexInsert3DPlot(X, _tex, basefilename,"regression_features","Features");

	_tex<<"Response"<<endl;
	_tex<<Y;
	klLinearRegression<double> R(X,Y);
	klMatrix<double> betahat=R();
	klout(betahat);
	klMatrix<double> betaEst=betahat.getSubBlock(0,0,2,0);
	_tex<<"Estimate for Beta"<<endl;
	_tex<<betaEst;
	_tex.flush();
}

void GenerativeGramConsistencyCheck(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Gram Matrix Consistency Check",_tex);

	unsigned int numFeatures=3;
	unsigned int sampleSize=4096;

	klMatrix<double> SigmaW=SampleWishart(numFeatures);

	_tex<<"Sample Size = "<<sampleSize<<endl;

	_tex<<"Feature dim = "<<numFeatures<<endl<<endl;

	LatexPrintMatrix<double>(SigmaW,"$\Sigma$",_tex);

	unsigned int i;
	unsigned int j;
	klVector<double> meanVector(numFeatures);
	meanVector=1;
	
	klNormalMultiVariate<double> T(meanVector,SigmaW );
	klSamplePopulation<double> X(sampleSize,numFeatures);
	for(j=0;j<sampleSize;j++)
	{
		X.setRow(j,T());
	}

	klMatrix<double> SampleCovariance = X.covarianceMatrix();

	LatexPrintMatrix<double>(SampleCovariance,"Sample Covariance",_tex);
	
	LatexPrintVector<double>(X.sampleMean(),"Sample Mean",_tex);

	//Git ISSUE #10 ( Makes extra Copy ?)
	klMatrix<double> D  = SampleCovariance-SigmaW;                           
	LatexPrintMatrix<double>(D,"Sample Covariance-$\Omega$",_tex);
	
	klPrincipalComponents<double> pca=X;
	klMatrix<double> VC =pca();
	klVector<complex<double> > covarianceEigs=SampleCovariance.eigenvalues();
	klVector<double > pcaEigs = pca.eigenvalues();
	LatexPrintVector<complex<double>  >(covarianceEigs,"Sample Covariance Eigs",_tex);
		
	klout(SigmaW);
	klout(SampleCovariance);
	klout(D);
	klout(VC);
	klout(covarianceEigs);
	klout(pcaEigs);

	//How close is the sample covariance to a PSD matrix?
	//First we'll need a measure of matrix closeness and a way to find the 
	//nearest PSD matrix in that measure. 

	//The Gram matrix G of a set of vectors x_1, ...,x_i,...x_n in an inner product space is the Hermitian matrix of inner products, whose entries are given by G_{ij} = <x_i,x_j>
	//An important application is to compute linear independence: a set of vectors is linearly independent if and only if the determinant of the Gram matrix is non-zero.

	klMatrix<double> G(sampleSize,sampleSize);
	G.makeNanFriendly();
	for(i=0;i<sampleSize;i++)
	{
		for(j=0;j<sampleSize;j++)
		{
			G[i][j]=( X[i]).dotBLAS(X[j]);
		}
	}

	klMatrix<double> centered = X.centeredData();
	klSamplePopulation<double> normalizedSample(centered);
	klVector<double> normedMean = normalizedSample.sampleMean();

	LatexPrintVector<double>(normalizedSample.sampleMean(),"Centered Mean",_tex);

	LatexPrintMatrix<double>(normalizedSample.covarianceMatrix(),"Centered Covariance",_tex);
	
	klMatrix<double> Gf(numFeatures,numFeatures);
	for(i=0;i<numFeatures;i++)
	{
		for(j=0;j<numFeatures;j++)
		{
			Gf[i][j]=(centered.getColumn(i)).dotBLAS(centered.getColumn(j));
		}
	}

	LatexPrintMatrix<double>( Gf,"Gram Matrix Gf Not scaled by sample size",_tex);

	//bbcreivist Git ISSUE #10 ( Makes extra Copy ?)
	klMatrix<double> scalesGf = Gf / (double) sampleSize;
	LatexPrintMatrix<double>(scalesGf ,"Gram Matrix Gf  scaled by sample size",_tex);

	klMatrix<double> diff = SampleCovariance / (double) sampleSize;

	LatexPrintMatrix<double>(diff ,"SampleCovariance - Scaled Gf",_tex);

	LatexPrintMatrix<double>(VC ,"EigenDecomp of SampleCovariance",_tex);
	
	klPrincipalComponents<double> pcaGram=Gf;
	klMatrix<double> VCGram =pcaGram();

	LatexPrintMatrix<double>(VCGram ,"EigenDecomp of Gram Matrix",_tex);

	}

void IteratedExponentialFiltering(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Iterated Exponential Filtering ",_tex);
	
	size_t popsize=1024*2;
	klTimeSeries<double> c(popsize);
	klNormalInverseApproxRV<double> normalinv(0,0.1);
	unsigned i;
	for(i=0;i<popsize;i++)
	{
		double pi= 3.141592653589793238462643383279502;
		c[i]=normalinv()+ .5* sin(4*pi*(double(i)/popsize)) + 1* sin(7*pi*(double(i)/popsize) );
	}
	

	_tex<<"$\\mu_1 =" <<c.mean()<<"$"<<endl;
	_tex<<"$\\mu_2 =" <<c.variance()<<"$"<<endl;
	_tex<<"$\\mu_3 =" <<c.skewness()<<"$"<<endl;
	_tex<<"$\\mu_4 =" <<c.kurtosis()<<"$"<<endl;

	klTimeSeries<double>::klTimeSeriesInterpolation interp=klTimeSeries<double>::klTimeSeriesInterpolation::PREVIOUS;

	double tau=12.0;

	klTimeSeries<double> ema=c.EMA(popsize,tau,interp);

	klTimeSeries<double> iema=c.IEMA(popsize,6,tau,interp);

	klTimeSeries<double> ma=c.MA(popsize,6,tau,interp);

	double gamma=1.22208;
	double beta=0.65;
	double alpha=1/(gamma*(8* beta - 3));

	klTimeSeries<double> diff=c.DIFF(popsize,gamma,beta,alpha,64,interp);
	
	//LatexInsert1DPlot(c,_tex,basefilename,"EMA_signal","EMA Signal",klHoldOnStatus::NoHold);

	//LatexInsert1DPlot(ma,_tex,basefilename,"MA","MA",klHoldOnStatus::NoHold);

	//LatexInsert1DPlot(iema,_tex,basefilename,"IEMA","Iterated Exponential Moving Average",klHoldOnStatus::NoHold);

	//LatexInsert1DPlot(ema,_tex,basefilename,"EMA","Exponential Moving Average",klHoldOnStatus::NoHold);

	//LatexInsert1DPlot(diff,_tex,basefilename,"DIFF","Diff operator",klHoldOnStatus::NoHold);

	//Put everything in onle plot
	{
		char* markerSpec = "'r'";
		//LatexInsert1DPlot(c,_tex,basefilename,"EMA_signal","EMA Signal",klHoldOnStatus::FirstPlot,markerSpec);

		markerSpec = "'g'";
		//LatexInsert1DPlot(ma,_tex,basefilename,"MA","MA",klHoldOnStatus::HoldOn,markerSpec);

		markerSpec = "'b'";
		//LatexInsert1DPlot(iema,_tex,basefilename,"IEMA","Iterated Exponential Moving Average",klHoldOnStatus::HoldOn,markerSpec);

		markerSpec = "'c'";
		//LatexInsert1DPlot(ema,_tex,basefilename,"EMA","Exponential Moving Average",klHoldOnStatus::HoldOn,markerSpec);
		
		markerSpec = "'m'";
		//LatexInsert1DPlot(diff,_tex,basefilename,"IteratedExponentailOperators","Iterated Exponentail Operators",klHoldOnStatus::LastPlot,markerSpec);
					
	}


	//Put everything in onle plot
	{
		char* markerSpec = "'r'";
		//LatexInsert1DPlot(c,_tex,basefilename,"EMA_signal","EMA Signal",klHoldOnStatus::FirstPlot,markerSpec);

		markerSpec = "'g'";
		//LatexInsert1DPlot(ma,_tex,basefilename,"MA","MA",klHoldOnStatus::HoldOn,markerSpec);

		markerSpec = "'b'";
		//LatexInsert1DPlot(iema,_tex,basefilename,"IEMA","Iterated Exponential Moving Average",klHoldOnStatus::HoldOn,markerSpec);

		markerSpec = "'c'";
		//LatexInsert1DPlot(ema,_tex,basefilename,"EMA","Exponential Moving Average",klHoldOnStatus::HoldOn,markerSpec);
		
		markerSpec = "'m'";
		//LatexInsert1DPlot(diff,_tex,basefilename,"IteratedExponentailOperators","Iterated Exponentail Operators",klHoldOnStatus::HoldOn,markerSpec);

		//LatexInsertLegend("'Signal','MA','IEMA','EMA','Diff'");

		//WritePlot(_tex, basefilename,"IteratedExponentailOperators","Iterated Exponentail Operators");
		
	}

	c[popsize-1024]=1237;//big shock

	_tex.flush();

}

void VSLFunctions(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Intel VSL Function Check",_tex);
	
	//vdInv		Inversion of vector elements
//void klVSLInv(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLInv(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLInv","Inversion of vector elements",klHoldOnStatus::NoHold);
	}
	
////vdSqrt Computation of the square root of vector elements
//void klVSLSqrt(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLSqrt(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLSqrt","sqrt of vector elements",klHoldOnStatus::NoHold);
	}
////vdExp	Computation of the exponential of vector elements
//void klVSLExp(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLExp(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLExp","exponentail of vector elements",klHoldOnStatus::NoHold);
	}
////vdExpm1		Computation of the exponential of vector elements decreased by 1
//void klVSLExpm1(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLExpm1(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLExpm1","exponential of vector elements decreased by 1",klHoldOnStatus::NoHold);
	}
////vdLn	Computation of the natural logarithm of vector elements
//void klVSLLn(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLLn(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLLn","natural logarithm of vector elements",klHoldOnStatus::NoHold);
	}
////vdLog10		Computation of the denary logarithm of vector elements
//void klVSLLog10(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLLog10(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLLog10","denary lograrithm of vector elements",klHoldOnStatus::NoHold);
	}
////vdCos		Computation of the cosine of vector elements
//void klVSLCos(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLCos(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLCos","cosine of vector elements",klHoldOnStatus::NoHold);
	}
////vdSin		Computation of the sine of vector elements
//void klVSLSin(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLSin(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLSin","sine of vector elements",klHoldOnStatus::NoHold);
	}
////vdTan		Computation of the tangent of vector elements
//void klVSLTan(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLTan(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLTan","tangent of vector elements",klHoldOnStatus::NoHold);
	}
////vdAcos		Computation of the inverse cosine of vector elements
//void klVSLAcos(klVector<double>&  v,klVector<double>& ans);
	//{
	//klVector<double> a(0.01,0.01,1);
	//klVSLAcos(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLAcos","inverse cosine of vector elements decreased by 1",false);
	//}
////vdAsin		Computation of the inverse sine of vector elements
//void klVSLAsin(klVector<double>&  v,klVector<double>& ans);
//
////vdAtan		Computation of the inverse tangent of vector elements
//void klVSLAtan(klVector<double>&  v,klVector<double>& ans);
//
////vdCosh		Computation of the hyperbolic cosine of vector elements
//void klVSLCosh(klVector<double>&  v,klVector<double>& ans);
//
////vdSinh		Computation of the hyperbolic sine of vector elements
//void klVSLSinh(klVector<double>&  v,klVector<double>& ans);
//
////vdTanh		Computation of the hyperbolic tangent of vector elements
//void klVSLTanh(klVector<double>&  v,klVector<double>& ans);
//
////vdAcosh		Computation of the inverse hyperbolic cosine of vector elements
//void klVSLAcosh(klVector<double>&  v,klVector<double>& ans);
//
////vdAsinh		Computation of the inverse hyperbolic sine of vector elements
//void klVSLAsinh(klVector<double>&  v,klVector<double>& ans);
//
////vdAtanh		Computation of the inverse hyperbolic tangent of vector elements
//void klVSLAtanh(klVector<double>&  v,klVector<double>& ans);
//
////vdErf		Computation of the error function value of vector elements
//void klVSLErf(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLErf(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLErf","error function of vector elements",klHoldOnStatus::NoHold);
	}
////vdErfc		Computation of the complementary error function value of vector elements
//void klVSLErfc(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLErfc(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLErfc","complementary error function of vector elements",klHoldOnStatus::NoHold);
	}
////vdCdfNorm		Computation of the cumulative normal distribution function value of vector elements
//void klVSLCdfNorm(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLCdfNorm(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLCdfNorm","cumulative normal distribution function of vector elements",klHoldOnStatus::NoHold);
	}
////vdErfInv		Computation of the inverse error function value of vector elements
//void klVSLErfInv(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLErfInv(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLErfInv","inverse error function of vector elements",klHoldOnStatus::NoHold);
	}
////vdErfcInv		Computation of the inverse complementary error function value of vector elements
//void klVSLErfcInv(klVector<double>&  v,klVector<double>& ans);
//
////vdCdfNormInv		Computation of the inverse cumulative normal distribution function value of vector elements
//void klVSLCdfNormInv(klVector<double>&  v,klVector<double>& ans);
//
////vdLGamma		Computation of the natural logarithm for the absolute value of the gamma function of vector elements
//void klVSLLGamma(klVector<double>&  v,klVector<double>& ans);
	{
	klVector<double> a(0.01,0.01,1);
	klVSLLGamma(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLLGamma","logarithm for the absolute value of the gamma function of vector elements",klHoldOnStatus::NoHold);
	}
////vdTGamma		Computation of the gamma function of vector elements 
//void klVSLTGamma(klVector<double>&  v,klVector<double>& ans);
		{
	klVector<double> a(0.01,0.01,1);
	klVSLTGamma(a,a);
	//LatexInsert1DPlot(a,_tex,basefilename,"klVSLTGamma","gamma function of vector elements",klHoldOnStatus::NoHold);
	}
}

void BinaryIO(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Testing binary writer",_tex);
	double pi= 3.141592653589793238462643383279502;
	klTimer klt;
	{
		__int64_t GBWorthOfDoubles = __int64_t(1073741824LL/sizeof(double));
		__int64_t rzG = __int64_t (std::sqrt ((double)GBWorthOfDoubles)) ;



	{	//klMatrix<double> klmd (rzG,rzG);
		klMatrix<double> klmd (362,362);
		klmd =pi;
		stringstream ss;
		klt.tic();
		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestMatrix.klmd";
		klBinaryIO::WriteWinx64( klmd, ss.str() );
		double bwtoc=klt.toc();

		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestMatrix.txt";
		ofstream fileostreamobj(ss.str() );
		klt.tic();
        fileostreamobj<<klmd<<endl;
        fileostreamobj.close();
		double swtoc = klt.toc();

		_tex<<"Binary writer Speedup 1GB Double Matrix "<< swtoc/bwtoc<<endl<<endl;

		__int64_t rows,cols;

		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestMatrix.klmd";	
		klBinaryIO::QueryWinx64(ss.str(),rows,cols);

		klMatrix<double> klmdMat(rows,cols);
		klt.tic();
		klBinaryIO::MatReadWinx64(ss.str(),klmdMat);
		bwtoc =klt.toc();

		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestMatrix.txt";
		ifstream fileistreamobj(ss.str() );
		klt.tic();
        fileistreamobj>>klmdMat;
        fileistreamobj.close();
		swtoc = klt.toc();
		_tex<<"Binary reader Speedup 1GB Double Matrix "<< swtoc/bwtoc<<endl<<endl;
	}

		
	}

	{
		__int64_t GBWorthOfDoubles = __int64_t(1073741824LL/sizeof(double));

		//klVector<double> klvd (GBWorthOfDoubles);
		klVector<double> klvd (131072);
		klvd =pi;

		stringstream ss;
		klt.tic();
		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestVector.klvd";
		klBinaryIO::WriteWinx64( klvd, ss.str() );
		double bwtoc=klt.toc();

		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestVector.txt";
		ofstream fileostreamobj(ss.str() );
		klt.tic();
        fileostreamobj<<klvd<<endl;
        fileostreamobj.close();
		double swtoc = klt.toc();

		_tex<<"Binary writer Speedup 1GB Double vector "<< swtoc/bwtoc<<endl<<endl;

		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestVector.klvd";
		__int64_t rows,cols;
		klBinaryIO::QueryWinx64(ss.str(),rows,cols);

		if (rows!=0)
		{
			ANSI_INFO; throw klError(err + "klBinaryIO::QueryWinx64(fileName ,rows,cols) returned non zero rows for vector");
		}
		klVector<double> readklvd(cols);
		klt.tic();
		klBinaryIO::VecReadWinx64(ss.str(),readklvd);
		bwtoc =klt.toc();

		ss.str("");ss.clear();
		ss<<basefilename<<"//WriterTestVector.txt";
		ifstream fileistreamobj(ss.str() );
		klt.tic();
        fileistreamobj>>readklvd;
        fileistreamobj.close();
		swtoc = klt.toc();
		_tex<<"Binary reader Speedup 1GB Double Matrix "<< swtoc/bwtoc<<endl<<endl;
	}
}

void PointCloudAndLatexPlots(ofstream &_tex,__int64_t &n)
{
	makeLatexSection("Testing Gaussian Mixture Point Cloud and Latex Plotting Capabilities.",_tex);

	{	
		unsigned int numPoints = 800;
		unsigned int numSources=numPoints;
		unsigned int numCenters = 2;
		int dimension =3;

		//__int64_t numPointsPerCenter, __int64_t numCenters,__int64_t dimension ,double scale
		klGaussianMixture X(numPoints/numCenters,numCenters,dimension,1.0f /950.0f);
		klGaussianMixture Y(numPoints/numCenters,numCenters,dimension,1.0f /950.0f);
		klGaussianMixture Z(numPoints/numCenters,numCenters,dimension,1.0f /950.0f);

		stringstream fileName;stringstream title;
		fileName.str("");fileName.clear();
		title.str(""); title.clear();
		fileName<<"GaussianMixture_Dim_3"<<"_Centers"<<numCenters;
		title<<"3 Gaussian Mixtures"<<numCenters<<"_Centers";
		char* color="'r.'";
		//LatexInsert3DPlot(X.getData(),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::FirstPlot, color);
		color ="'g.'";
		//LatexInsert3DPlot(Y.getData(),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::HoldOn, color);
		color= "'b.'";
		//LatexInsert3DPlot(Z.getData(),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::LastPlot, color);
	}
	
	{	
		unsigned int numPoints = 800;
		unsigned int numSources=numPoints;
		unsigned int numCenters = 2;
		int dimension =1;

		//__int64_t numPointsPerCenter, __int64_t numCenters,__int64_t dimension ,double scale
		klGaussianMixture X(numPoints/numCenters,numCenters,dimension,1.0f /950.0f);
		klGaussianMixture Y(numPoints/numCenters,numCenters,dimension,1.0f /950.0f);
		klGaussianMixture Z(numPoints/numCenters,numCenters,dimension,1.0f /950.0f);
						
		stringstream fileName;stringstream title;
		fileName.str("");fileName.clear();
		title.str(""); title.clear();
		fileName<<"GaussianMixture_Dim_1"<<"_Centers"<<numCenters;
		title<<"Gaussian Mixtures"<<numCenters<<"_Centers";
		char* color="'r.'";
		//LatexInsert1DPlot(X.getData().getColumn(0),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::FirstPlot, color);
		color ="'g.'";
		//LatexInsert1DPlot(Y.getData().getColumn(0),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::HoldOn, color);
		color= "'b.'";
		//LatexInsert1DPlot(Z.getData().getColumn(0),_tex,basefilename,fileName.str().c_str(),title.str().c_str(),klHoldOnStatus::LastPlot, color);
	}
}

void RandomMatrixNorms(ofstream &_tex,__int64_t &n)
{	
	{
		klMatrix<double> G(4,3);

	G[0][0]= 1, G[0][1]=2, G[0][2]=3,
	G[1][0]= 4, G[1][1]=5, G[1][2]=6,
	G[2][0]= 7, G[2][1]=8, G[2][2]=9,
	G[3][0]= 10, G[3][1]=11, G[3][2]=12;

	double n1=G.norm(false);

	double n2=G.norm(true);

	double c1 = G.ConditionNumber(false);

	double c2 = G.ConditionNumber(true);
	}
	
	
	{
		klMatrix<double> G =SampleGOE(n);
	
	double n1=G.norm(false);

	double n2=G.norm(true);

	double c1 = G.ConditionNumber(false);

	double c2 = G.ConditionNumber(true);
	}

		{
		klMatrix<double> G =HilbertMatrix(n);
		
	double n1=G.norm(false);

	double n2=G.norm(true);

	double c1 = G.ConditionNumber(false);

	double c2 = G.ConditionNumber(true);
	}
}

