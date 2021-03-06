 /*******************************
 * Copyright (c) <2007>, <Bruce Campbell> All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  *  
 * Bruce B Campbell 07 08 2014  *
 ********************************/

#ifndef __kl_matrix__
#define __kl_matrix__

#include "mkl.h"
#include "kl_memory.h"
#include "kl_util.h"
#include "kl_vector.h"
#include <limits.h>
#include <float.h>
#include <string>
#include <vector>
#include <list>
#include <complex>
#include <typeinfo>
#include <stdio.h>

using namespace std;

#ifdef _DEBUGKL
extern __int64_t globalKlMatrixCopyConstructorCallCount;
extern __int64_t globalKlMatrixMoveConstructorCallCount;

extern __int64_t globalKlMatrixCopyConstructorBytesCount;
extern __int64_t globalKlMatrixMoveConstructorBytesCount;
#endif


template<class TYPE> class klMatrix
{
public:
	klMatrix(TYPE* mem,__int64_t row,__int64_t col,bool own=0) 
	{
		__int64_t i;
		_vectors=new klVector<TYPE>[row];
		for(i=0;i<row;i++)
			(_vectors+i)->setup(col,0,mem+i*col);
		if(own)
			_own=1;
		else
			_own=0;
		_row=row;
		_col=col;
		_memory=mem;
		_contiguous=true;
		_mgr=0;
	}

	klMatrix(klMemMgr* mgr, __int64_t row, __int64_t col) 
	{                   
		_memory=(TYPE*)mgr->allocate(row*col*sizeof(TYPE) );
		_mgr=mgr;
		__int64_t i;
		_vectors=new klVector<TYPE>[row];
		for(i=0;i<row;i++)
			(_vectors+i)->setup(col,0,_memory+i*col);
		_own=1;
		_row=row;
		_col=col;
		_contiguous=true;

	}

	klMatrix(__int64_t row,__int64_t col) 
	{
		size_t memrequest = row*col;
		_memory=new TYPE[memrequest];
		__int64_t i;
		_vectors=new klVector<TYPE>[row];
		for(i=0;i<row;i++)
			(_vectors+i)->setup(col,0,_memory+i*col);
		_own=1;
		_row=row;
		_col=col;
		_contiguous=true;
		_mgr=0;

	}

	klMatrix() 
	{
		_memory=NULL;
		_mgr=0;
		_col=0;
		_row=0;
		_contiguous=0;
		_own=0;
		_vectors=0;
	}

	klMatrix(const klMatrix<TYPE>& src)
	{

		_memory=NULL;
		_mgr=src._mgr;
		_row=src._row;
		_col=src._col;
		_contiguous=true;
		_own=1;
		setup(_row,_col,_mgr);
		__int64_t i,j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)=src[i][j];
			}
		}
#ifdef _DEBUGKL
		  __int64_t byteCount = _row*_col * sizeof(TYPE);
		  globalKlMatrixCopyConstructorBytesCount +=byteCount;
		  cerr<<"klMatrix(klMatrix<TYPE>& src) call count = "<<globalKlMatrixCopyConstructorCallCount++<<" Bytes Count "<<globalKlMatrixCopyConstructorBytesCount<<endl; 
#endif
	}

	klMatrix(klMatrix<TYPE>&& src) :
	_vectors(src._vectors) , _own(src._own),_contiguous(src._contiguous),
		_row(src._row),	_col(src._col),	_memory(src._memory),
		_mgr(src._mgr),	_filename(src._filename)
	{

#ifdef _DEBUGKL
		  __int64_t byteCount = _row*_col * sizeof(TYPE);
		  globalKlMatrixMoveConstructorBytesCount +=byteCount;
		  cerr<<"klMatrix(klMatrix<TYPE>&& src) call count = "<<globalKlMatrixMoveConstructorCallCount++<<" Bytes Count "<<globalKlMatrixMoveConstructorBytesCount<<endl; 
#endif

		src._vectors=NULL;
		src._own=0;
		src._contiguous=true;
		src._row=0;
		src._col=0;
		src._memory=NULL;
		src._mgr=NULL;
		src._filename="";
	}

	~klMatrix()
	{

		if(_own)
		{
			if(_mgr)
				_mgr->free(_memory);
			else
				delete _memory;
			delete[] _vectors;
		}
		else
		{
			delete[] _vectors;
		}
	}

	klMatrix<TYPE>& operator=(const klMatrix<TYPE>& src)
	{
		_row=src._row;
		_col=src._col;
		_contiguous=true;
		setup(_row,_col,src._mgr);
		_mgr=src._mgr;//don't step on our mgr until we've had the opportunity to free resources

		__int64_t i,j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)=src[i][j];
			}
		}
		return *this;
	}

	klVector<TYPE>& operator[](__int64_t row) const 
	{
		if(row<_row )
			return _vectors[row]; 
		else
		{
			ANSI_INFO; throw klError(err + " klVector<TYPE>& operator[](__int64_t row) const ERROR: Memory index out of range in klMatrix");
		}
	}

	klMatrix<TYPE> operator*(klMatrix<TYPE> a) const
	{
		if(_col!=a.getRows())
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE> operator*(klMatrix<TYPE> a) const ERROR: invalid dimension in operator* overlaoad");
		}
		klMatrix<TYPE> product;
		if(_mgr)
			product.setup(_row,a.getColumns(),_mgr);
		else 
			product.setup(_row,a.getColumns(),0);

		__int64_t i,j,k;
		//#pragma omp parallel num_threads(omp_get_num_procs( ) )
		for(i=0;i<_row;i++)
		{
			for(j=0;j<a.getColumns();j++)
			{
				TYPE temp=0;
				for(k=0;k<a.getRows();k++)
					temp+= (_vectors+i)->operator[](k)* a[k][j];
				product[i][j]=temp;

			}
		}
		return product;
	}
	klVector<TYPE> operator*(klVector<TYPE> a) const
	{
		if(_col!=a.getRowSize())
		{
			ANSI_INFO; throw klError(err + "klVector<TYPE> klMatrix ::operator*(klVector<TYPE> a) const ERROR : invalid dimension in operator* overlaoad");
		}
		klVector<TYPE> product;
		if(_mgr)
			product.setup(_row,_mgr);
		else 
			product.setup(_row);
		__int64_t i,j;
		for(i=0;i<_row;i++)
		{
			TYPE temp=0;
			for(j=0;j<_col;j++)
			{
				temp+=(_vectors+i)->operator[](j) * a[j];
			}
			product[i]=temp;
		}
		return product;
	}

	//Sets the entries of this klMatrix to c.
	klMatrix<TYPE>& operator=(TYPE c) 
	{
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)=c;
			}

		}

		return *this;
	}

	klMatrix<TYPE>& operator+=(TYPE c)
	{     
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)+=c;
			}

		}

		return *this;
	}

	klMatrix<TYPE>& operator+=(const klMatrix<TYPE> &c)
	{
		if(_row!=c.getRows() || _col !=c.getColumns())
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE>& klMatrix<TYPE>::operator+=(const klMatrix<TYPE> &c) ERROR: incompatible dimensions.");
		}
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)+=c[i][j];
			}

		}
		return *this;
	}

	klMatrix<TYPE>& operator-=(const klMatrix<TYPE> &c)
	{
		if(_row!=c.getRows() || _col !=c.getColumns())
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE>& klMatrix<TYPE>::operator+=(const klMatrix<TYPE> &c) ERROR: incompatible dimensions.");
		}
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)-=c[i][j];
			}

		}
		return *this;
	}

	klMatrix<TYPE>& operator-=(TYPE c)
	{
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)-=c;
			}

		}
		return *this;
	}
	
	klMatrix<TYPE>& operator*=(const klMatrix<TYPE> &a)
	{
		if(_col!=a.getRows())
		{
			ANSI_INFO; throw klError(err + "klMatrix ERROR: invalid dimension in operator* overlaoad");
		}
		klMatrix<TYPE> product;
		if(_mgr)
			product.setup(_row,a.getColumns(),_mgr);
		else 
			product.setup(_row,a.getColumns(),0);

		__int64_t i,j,k;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<a.getColumns();j++)
			{
				TYPE temp=0;
				for(k=0;k<_row;k++)
					temp+= (_vectors+i)->operator[](k)* a[k][j];
				product[i][j]=temp;

			}
		}
		return product;
	}

	klMatrix<TYPE>& operator*=(TYPE c)
	{

		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)*=c;
			}

		}
		return *this;
	}

	klMatrix<TYPE>& operator/=(TYPE c)
	{
		if(c==0)
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE>& klMatrix<TYPE>::operator/=(TYPE c): ERROR: attempting to divide by zero .");
		}
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				(_vectors+i)->operator[](j)/=c;
			}

		}
		return *this;

	}
	
	bool operator==(const klMatrix<TYPE> &m) const
	{ 
		if(this->_row != m.getRows() ||this->_col!=m.getColumns())
		{
			ANSI_INFO; throw klError(err + "Bad dimensions in klMatrix<TYPE> operator==(const klMatrix<TYPE> &m)");
		}

		__int64_t i;
		__int64_t j;
		for(i=0;i<m.getRows();i++)
		{
			for(j=0;j<m.getColumns();j++)
			{
				if( (_vectors+i)->operator [](j) != m[i][j])
					return false;
			}
		}
		return true;
	}

	//Elementwise 
	klMatrix<TYPE>& operator/=(const klMatrix<TYPE> &c)
	{

		if(_row!=c.getRows() || _col !=c.getColumns())
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE>& klMatrix<TYPE>::operator+=(const klMatrix<TYPE> &c) ERROR: incompatible dimensions.");
		}
		__int64_t i;
		__int64_t j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				if(c[i][j]==0)
				{
					ANSI_INFO; throw klError(err + "klMatrix<TYPE>& operator/=(const klMatrix<TYPE> &c) ERROR: attempting to divide by zero");
				}
				(_vectors+i)->operator[](j)/=c[i][j];
			}

		}
		return *this;
	}
		
	void setRow(__int64_t j,klVector<TYPE> r)
	{
		if(r.getColumns() != _col)
		{
			ANSI_INFO; throw klError(err + "klMatrix::setRow(__int64_t j,klVector<TYPE> r) ERROR: invalid dimension in setRow parameter");
		}

		__int64_t i;

		for(i=0;i<_col;i++)
		{
			(_vectors+j)->operator[](i)=r[i];
		}
	}
	
	klMatrix<TYPE> transpose() const
	{
		klMatrix<TYPE> transpose;
		if(_mgr)
			transpose.setup(_col,_row,_mgr);
		else 
			transpose.setup(_col,_row,0);
		__int64_t i,j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{                   
				transpose[j][i]=(_vectors+i)->operator[](j);    
			}
		}
		//bbcrevisit verify there is no copy
		return transpose;
	}

	//Maps x[i][j]<low ----> low and  x[i][j]>high ----> high
	//Defaults of 
	void threshold(double low=DBL_MIN, double lowVal= DBL_MIN, double high=DBL_MAX,double highVal=DBL_MAX)
	{
		if(high<low)
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE> threshold(double low=DBL_MIN, double high=DBL_MAX) called with high<low");
		}
		__int64_t i,j;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{                   
				TYPE c =(_vectors+i)->operator[](j);    
				if(c<low)
					(_vectors+i)->operator[](j)=lowVal + 0.0;//Add 0.0 to clear sign bit when clipping- bbcrevisit odd behavior
				if(c>high)
					(_vectors+i)->operator[](j)=highVal + 0.0;
			}
		}
	}
		
	//Calculates the determinant via LU decomp.  Implemented for float and double types
	//Calculating the determinanat via cofactors is O(n!)
	//An LU decomp, reduces to O(\frac{2 n^3}{3} )
	//If the matrix is SPD we can do a Choleski (LL) decomp. which is O(\frac{n^3}{3}) 
	complex<double> det() //const
	{
		//The LU factorization of A allows the linear system A*x = b to be solved quickly with x = U\(L\b)
		//Determinants and inverses are computed from the LU factorization using det(A) = det(L)*det(U)
		//and inv(A) = inv(U)*inv(L).
		//You can also compute the determinants using det(A) = prod(diag(U)), 
		//though the signs of the determinants may be reversed.
		klVector<complex<double> > eigen=this->eigenvalues();
		complex<double>  detProd=eigen[0];
		for(int i=1;i<_row;i++)
		{
			detProd *= eigen[i];
		}
		
		return detProd;
	}

	//Returns the subblock (i,j):(k,l) indicated by the indices.
	klMatrix<TYPE> getSubBlock(__int64_t i,__int64_t j,__int64_t k,__int64_t l)
	{
		//First Verify the indices are in range
		if(i<0|| j<0|| k-i<0 || l-j<0 || k>_row || l>_col)
		{
			std::stringstream ANSI_INFO_ss (std::stringstream::in | std::stringstream::out );
			ANSI_INFO_ss<<"ANSI COMPILE INFO: " <<__DATE__<<"     "<<__TIME__<<"   "<<__FILE__<<"   "<<__LINE__<<"       "<<std::endl;
			std::string err ="klMatrix<TYPE> klMatrix<TYPE>::getSubBlock ERROR index out of bounds." + ANSI_INFO_ss.str();		
			throw klError(err);			
		}
		klMatrix<TYPE> ret(k-i+1,l-j+1);
		__int64_t n,m;
		for(n=i;n<=k;n++)
			for(m=j;m<=l;m++)
			{
				ret[n-i][m-j]=(_vectors+n)->operator[](m);
			}

		return ret;
	}

	//Sets the subblock (i,j):(k,l) indicated by the indices.
	void setSubBlock(klMatrix<TYPE> block,__int64_t i,__int64_t j)
	{
		//First Verify the indices are in range
		__int64_t k=block.getRows()+i-1;
		__int64_t l=block.getColumns()+j-1;
		if(k-i<=0 || l-j<=0 || k>_row || l>_col)
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE> klMatrix<TYPE>::getSubBlock ERROR index out of bounds.");
		}
		__int64_t n,m;
		for(n=i;n<=k;n++)
			for(m=j;m<=l;m++)
				(_vectors+m)->operator[](n)=block[n-i][m-j];
	}
	
	void setColumn(__int64_t i,klVector<TYPE> v)
	{
		if (_row != v.getRowSize())
		{
			ANSI_INFO; throw klError(err + "Bad dimensions in klMatrix::setRow");
		}
		__int64_t n = 0;
		__int64_t m =i;
			for(n=0;n<=_row;n++)
				(_vectors+n)->operator[](i)=v[n];
	}

	//Calculates the matrix inverse for a square matrix.  Implemented for types double and float
	//Generally we do not want to calculate the inverse of a matrix
	klMatrix<TYPE> inverse() const
	{
		if(_row!=_col  )
		{
			ANSI_INFO; throw klError(err + "klMatrix<TYPE>::inverse() ERROR: non square matrix");
		}
		klMatrix<TYPE> temp;
		temp=this->transpose(); //deep copy 
		__int64_t size=0;
		__int64_t i,j;
		__int64_t index=0;
		char uplo='L';
		int info=0;
		int n=_row;
		int m=_col;
		size_t ipivSz=std::max<int>(1,std::min(_row,_col));
		int* ipiv=new int[ipivSz];
		//Calculating the determinanat via cofactors is O(n!)
		//An LU decomp, reduces to O(\frac{2 n^3}{3} )
		//If the matrix is SPD we can do a Choleski (LL) decomp. which is O(\frac{n^3}{3}) 
		if(typeid(TYPE) ==typeid(float) ) 
		{
			MKL_INT lwork =  2*n;
			float* work= new float[lwork];
			sgetrf(&m, &n, (float*)temp.getMemory(), &m, ipiv, &info);
			sgetri( &n, (float*)temp.getMemory(), &n, ipiv, work, &lwork, &info );
			delete work;
		}

		if(typeid(TYPE) == typeid(double) )
		{
			MKL_INT lwork =  2*n;
			double* work= new double[lwork];
			dgetrf(&m, &n, (double*)temp.getMemory(), &m, ipiv, &info);
			dgetri( &n, (double*)temp.getMemory(), &n, ipiv, work, &lwork, &info );
			delete work;
		}

		if(info<0)
		{
			ANSI_INFO; throw klError(err + "klMatrix::inverse ERROR: parameter error in MKL call/");
		}
		if(info>0)
		{
			ANSI_INFO; throw klError(err + "klMatrix::inverse ERROR: error in MKL call.");
		}
		
		return temp.transpose();

	}
		
	/*
		This method calls the LAPACK driver DGEES which computes for an	N-by-N real nonsymmetric matrix	A, the eigenvalues,
		the real Schur form T, and, optionally, the matrix of Schur vectors Z.
		This gives the Schur factorization A = Z*T*(Z**T).

		Optionally, it also orders the eigenvalues on	the diagonal of	the real
		Schur	form so	that selected eigenvalues are at the top left.	The leading
		columns of Z then form an orthonormal	basis for the invariant	subspace
		corresponding	to the selected	eigenvalues.

		A matrix is in real Schur form if it is upper	quasi-triangular with 1-by-1
		and 2-by-2 blocks. 2-by-2 blocks will	be standardized	in the form
		[  a	b  ]
		[  c	a  ]

		where	b*c < 0. The eigenvalues of such a block are a +- sqrt(bc).
	*/
	klVector<complex<double> > eigenvalues()
	{
		if(_row!=_col)
		{
			ANSI_INFO; throw klError(err + "klVector<complex<double> > eigenvalues() ERROR: trying to calculate eigenvalues of non-square matrix.");
		}

		klVector<complex<double> > eigenvalues(_row);

		//uses MKL LAPACK driver routine routine to calculate eigenvalues and Shur vectors
		//bbc revisit - we can define a select function that is used to order the eigenvalues
		//on the Shur form

		char jobvs='V';//Shur forms are computed

		//If sort = 'N', then eigenvalues are not ordered.
		//If sort = 'S', eigenvalues are ordered (see select).
		char sort='S';//don't use select function to order eigenvalues
		int n=_row;

		klMatrix<double> a;
		a.setup(_row,_col,_mgr);// bbc revisit - this is a problem if TYPE is not double 
		__int64_t i,j;
		for(i=0;i<_row;i++)
			for(j=0;j<_col;j++)
				a[i][j]=(_vectors+j)->operator [](i);
		int lwork=3*_col;
		double* work=new double[lwork];
		int lda=_row;
		int ldvs=_col;
		int* bwork=NULL;//not used if sort='N'

		//output arguments
		int sdim;//ouput is 0 if is sort='N', otherwise it equals the number of eigenvalues for which sort returned true
		double* wr=new double[n];//real part of eigenvalues, sorted by sort fn if we use it
		double* wi=new double[n];//imaginary part of eigenvalues, sorted by sort fn if we use it
		double* vs =new double[ldvs*n];//if jobvs this contains the orthogonal/unitary matrix of Shur vectors
		//work[1]=returns the required minimal size of work - so we can reduce work size in subsequent calls
		int info;

		int (*select)(double*,double*);
		//select=NULL;//empty function pointer
		select=mkl_eigs_select;

		dgees(&jobvs,&sort,select,&n,a.getMemory(),&lda,&sdim,wr,wi,vs,&ldvs,work,&lwork,bwork,&info);

		//bbs revisit - store Shur form
		for(i=0;i<_row;i++)
		{
			complex<double> e(*(wr+i),*(wi+i));
			eigenvalues[i]=e;
		}
		delete work;
		delete wr;
		delete wi;
		delete vs;
		return eigenvalues; 
	}

	//If the matrix is not square, we return \sum\limits_{i=0}^{min{_row,_col}}   
	TYPE trace()
	{
		__int64_t min=min(_row,_col);
		__int64_t i;
		TYPE r=0;
		for(i=0;i<min;i++)
		{
			r += (_vectors+i)->operator [](i);
		}
		return r;
	}

	//See float and double specializations.
	//Defaults to the L1 norm, ellone=false computes the L\infty norm
	TYPE ConditionNumber(bool ellone=0)
	{
		return 0; 
	}
	
	//Returns the \Ell_p norm of the matrix.  This method is specialized for double & float.
	//Requires p \in [0,
	TYPE norm(bool ell_infty=0)
	{
		return 0;
	}

	//Returns the 0-based indexed column
	klVector<TYPE> getColumn(__int64_t col)
	{
		klVector<TYPE> column;
		if(_mgr)
			column.setup(_row,_mgr);
		else
			column.setup(_row);
		__int64_t i;
		for(i=0;i<_row;i++)
			column[i]=(_vectors+i)->operator[](col);
		return column;
	}

	
	__int64_t getRows() const
	{
		return _row;
	}

	
	__int64_t getColumns()const
	{
		return _col;
	}

	
	TYPE* getMemory() const 
	{
		return _memory;
	}

	
	klMemMgr* getMemMgr() const 
	{
		return _mgr;
	}

	
	bool isContiguous() const
	{
		return _contiguous;
	}

	klMatrix<TYPE> columnToRowMajor()
	{
		klMatrix<TYPE> ans(_row,_col);
		TYPE* pM=ans.getMemory();
		TYPE* pI=getMemory();
		__int64_t i=0;
		__int64_t j=0;
		__int64_t k=0;
		for(i=0;i<_row;i++)
		{
			for(j=0;j<_col;j++)
			{
				TYPE A = *(pI+j*_row+i);
				ans[i][j]=A;
			}
		}
		return ans;
	}
	
	//Initializes memory to NaN for types double and float
	void makeNanFriendly() 
	{
		if(sizeof(TYPE)==8)
		{
			unsigned long nan[2]={0xffffffff, 0x7fffffff};
			double x = *( double* )nan;
			bool check = isnan(x);
			if (check)
			{
				//memset(_memory,0,row+col);
				//if exponent = 2e - 1 and fraction is not 0, the number being represented is not a number (NaN). 
				__int64_t i,j;
				for(i=0;i<_row;i++)
					for(j=0;j<_col;j++)
						(_vectors+j)->operator [](i)=x;
			}
		}
		if(sizeof(TYPE)==4)
		{
			unsigned short nan[2]={0xffff, 0x7fff};
			float x = *( float* )nan;
			bool check = isnan(x);
			if (check)
			{
				//memset(_memory,0,row+col);
				//if exponent = 2e - 1 and fraction is not 0, the number being represented is not a number (NaN). 
				__int64_t i,j;
				for(i=0;i<_row;i++)
					for(j=0;j<_col;j++)
						(_vectors+j)->operator [](i)=x;
			}
		}
	}

	size_t precision() const
	{
		return sizeof(TYPE);
	}

	void setup(__int64_t row,__int64_t col,klMemMgr* mgr=0)
	{
		if(_own && _memory)
		{
			if(_mgr)
			{
				_mgr->free(_memory);
				_memory=NULL;
			}
			else
			{
				delete _memory;
				_memory=NULL;
			}
			delete[] _vectors;
		}

		_row=row;
		_col=col;
		_contiguous=true;
		_mgr=mgr;

		if(_mgr)
			_memory=(TYPE*)_mgr->allocate(_row*_col*sizeof(TYPE) );
		else
			_memory=new TYPE[_row*_col];
		_vectors=new klVector<TYPE>[_row];
		_own=true;

		__int64_t i;
		for(i=0;i<_row;i++)
			(_vectors+i)->setup(col,0,_memory+i*_col);
	}

	//An STL container for the user to fill with descriptive names
	vector<std::string> _dimensionNames;

	klMatrix<TYPE> upper()
	{
		klMatrix<TYPE> U(_row,_col);
		U=(TYPE)0.0;
		for(int i=0;i<_row;i++)
		{	
			for(int j =0;j<_col;j++)
			{
				if(i>j)
					continue;
				else
					U[i][j] = (_vectors+j)->operator [](i);
			}
		}
		return U;
	}
	
	klMatrix<TYPE> lower()
	{
		klMatrix<TYPE> L(_row,_col);
		L=(TYPE)0.0;
		for(int i=0;i<_row;i++)
		{	
			for(int j =0;j<_col;j++)
			{
				if(i<j)
					continue;
				else
					L[i][j] = (_vectors+j)->operator [](i);
			}
		}
		return L;
	}

	klVector<TYPE> diag()
	{
		klVector<TYPE> result(min(_row,_col));
		result =0;
		for(int i =0;i< min(_row,_col);i++)
		{
			result[i] = (_vectors+i)->operator [](i);
		}
		return result;
	}
protected:
	klVector<TYPE>* _vectors;
	mutable __int64_t _own;
	
	//It is possible to set up matrices with non contiguous memory layout.	
	bool _contiguous;
	__int64_t  _row;
	__int64_t  _col;
	TYPE* _memory;
	klMemMgr* _mgr;
	string _filename;
};

inline klMatrix<double> mmBLAS(double alpha, klMatrix<double> a, klMatrix<double> b, double beta, klMatrix<double> c)
{
	if(a.getColumns()!=b.getRows())
	{
		ANSI_INFO; throw klError(err + "double klMatrix<double>::mmBLAS const ERROR: invalid dimension a.getColumns()!=b.getRows() ");
	}
	if(a.getRows() !=c.getRows() )
	{
		ANSI_INFO; throw klError(err + "double klMatrix<double>::mmBLAS const ERROR: invalid dimension a.getRows() !=c.getRows() ");
	}
	if(b.getColumns() !=c.getColumns() )
	{
		ANSI_INFO; throw klError(err + "double klMatrix<double>::mmBLAS const ERROR: invalid dimension b.getColumns() !=c.getColumns()");
	}

	klMatrix<double> C=c;

	const char transa = 'T';
	const char transb = 'T';
	int m =c.getRows();
	MKL_INT *mp=&m;
	int n =b.getColumns();
	MKL_INT *np=&n;
	int k =a.getColumns();
	MKL_INT *kp=&k;
	const double *alphap=&alpha;
	const double *ap=(double*)a.getMemory();
	const MKL_INT *lda=kp;
	const double *bp=(double*)b.getMemory();
	const MKL_INT *ldb=np;
	const double *betap=&beta;
	double *cp=(double*)C.getMemory();
	const MKL_INT *ldc=mp;

	dgemm(&transa,&transb, mp, np, kp, alphap, ap, lda, bp, ldb, betap, cp, ldc);

	//In Fortran, column-major ordering of storage is assumed. This means that elements of the
	//same column occupy successive storage locations.
	//Three quantities are usually associated with a two-dimensional array argument: its leading
	//dimension, which specifies the number of storage locations between elements in the same row,
	//its number of rows, and its number of columns. For a matrix in full storage, the leading dimension
	//of the array must be at least as large as the number of rows in the matrix.
	return C.columnToRowMajor();

}

inline klMatrix<float> mmBLAS(float alpha, klMatrix<float> a, klMatrix<float> b, float beta, klMatrix<float> c)
{
	if(a.getColumns()!=b.getRows())
	{
		ANSI_INFO; throw klError(err + "double klMatrix<float>::mmBLAS const ERROR: invalid dimension a.getColumns()!=b.getRows() ");
	}
	if(a.getRows() !=c.getRows() )
	{
		ANSI_INFO; throw klError(err + "double klMatrix<float>::mmBLAS const ERROR: invalid dimension a.getRows() !=c.getRows()");
	}
	if(b.getColumns() !=c.getColumns() )
	{
		ANSI_INFO; throw klError(err + "double klMatrix<float>::mmBLAS const ERROR: invalid dimension b.getColumns() !=c.getColumns() ");
	}

	klMatrix<float> C=c;

	const char transa = 'T';
	const char transb = 'T';
	int m =c.getRows();
	MKL_INT *mp=&m;
	int n =b.getColumns();
	MKL_INT *np=&n;
	int k =a.getColumns();
	MKL_INT *kp=&k;
	const float *alphap=&alpha;
	const float *ap=(float*)a.getMemory();
	const MKL_INT *lda=kp;
	const float *bp=(float*)b.getMemory();
	const MKL_INT *ldb=np;
	const float *betap=&beta;
	float *cp=(float*)C.getMemory();
	const MKL_INT *ldc=mp;

	sgemm(&transa,&transb, mp, np, kp, alphap, ap, lda, bp, ldb, betap, cp, ldc);

	//In Fortran, column-major ordering of storage is assumed. This means that elements of the
	//same column occupy successive storage locations.
	//Three quantities are usually associated with a two-dimensional array argument: its leading
	//dimension, which specifies the number of storage locations between elements in the same row,
	//its number of rows, and its number of columns. For a matrix in full storage, the leading dimension
	//of the array must be at least as large as the number of rows in the matrix.
	return C.columnToRowMajor();
}

//y := alpha*A*x + beta*y
inline klVector<double> mvpBLAS(double alpha, klMatrix<double> a, klVector<double> x, double beta,klVector<double> yi)
{
	klVector<double> y(yi);
	const char trans = 'N';
	MKL_INT m=a.getRows();
	MKL_INT n=a.getColumns();
	MKL_INT lda=n;
	MKL_INT incx=1;
	MKL_INT incy=1;

	dgemv(&trans, &m, &n, &alpha, a.getMemory(), &lda, x.getMemory(), &incx, &beta, yi.getMemory(), &incy);

	return yi;
}

inline klVector<float> mvpBLAS(float alpha,  klMatrix<float> a,klVector<float> x, float beta,klVector<float> yi)
{
	klVector<float> y(yi);
	const char trans = 'N';
	MKL_INT m=a.getRows();
	MKL_INT n=a.getColumns();
	MKL_INT lda=n;
	MKL_INT incx=1;
	MKL_INT incy=1;

	sgemv(&trans, &m, &n, &alpha, a.getMemory(), &lda, x.getMemory(), &incx, &beta, yi.getMemory(), &incy);

	return yi;
}

//Specialization for float matrix multiply that uses BLAS Level 3 sgemm
template< > klMatrix<float> klMatrix<float>::operator*(klMatrix<float> a) const
{
	klMatrix<float> I(_row,a.getColumns());
	I=0;
	return mmBLAS(1.0f, (klMatrix<float>)*this, a, 0.0f, I);
}

//Specialization for double matrix multiply that uses BLAS Level 3 sgemm
template< > klMatrix<double> klMatrix<double>::operator*(klMatrix<double> a) const
{
	klMatrix<double> I(_row,a.getColumns());
	I=0;
	return mmBLAS(1.0f, (klMatrix<double>)*this, a, 0.0f, I);
}

//Apply a univariate function to a klVector
//Some commone ones available from math.h;
//double  __cdecl acos(double);
//double  __cdecl asin(double);
//double  __cdecl atan(double);
//double  __cdecl cos(double);
//double  __cdecl cosh(double);
//double  __cdecl exp(double);
//double  __cdecl fabs(double);
//long    __cdecl labs(long);
//double  __cdecl log(double);
//double  __cdecl log10(double);
//double  __cdecl sin(double);
//double  __cdecl sinh(double);
//double  __cdecl tan(double);
//double  __cdecl tanh(double);
//double  __cdecl sqrt(double);
template<class TYPE, class TYPE1> klVector<TYPE> klApplyFn(TYPE1 (*f)(TYPE1), const klVector<TYPE> &c)
{
	klVector<TYPE> r(c.getColumns() );
	__int64_t i;
	for (i=0;i<c.getColumns();i++)
		r[i]=TYPE(f(TYPE1(c[i])));
	return r;
}

//Apply a univariate function to a klMatrix
template<class TYPE, class TYPE1> klMatrix<TYPE> klApplyFn(TYPE1 (*f)(TYPE1), const klMatrix<TYPE> &c)
{
	klMatrix<TYPE> r(c.getRows(),c.getColumns());
	__int64_t i;
	__int64_t j;
	for ( i=0;i<c.getRows();i++)
		for ( j=0;j<c.getColumns();j++)
			//out(i,j)=static_cast<T>(f(static_cast<fT>(data(i,j))));
			r[i][j]=TYPE(f(TYPE1(c[i][j])));

	return r;
}

template<class TYPE> inline const klMatrix<TYPE> operator+(const klMatrix<TYPE> &c1, const klMatrix<TYPE> &c2)
{
	if(c1.getRows() !=c2.getRows() || c1.getColumns() !=c2.getColumns())
	{
		ANSI_INFO; throw klError(err + "const klMatrix<TYPE> operator+(const klMatrix<TYPE> &c1, const klMatrix<TYPE> &c2) ERROR: incompatible dimensions.");
	}
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j]+c2[i][j];
		}

	}
	return r;
}

template<class TYPE> inline const klMatrix<TYPE> operator+(const klMatrix<TYPE> &c1, TYPE c2)
{
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j]+c2;
		}

	}
	return r;
}

template<class TYPE> inline const klMatrix<TYPE> operator+(TYPE c1, const klMatrix<TYPE> &c2)
{
	klMatrix<TYPE> r(c2.getRows(),c2.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c2.getRows();i++)
	{
		for(j=0;j<c2.getColumns();j++)
		{
			r[i][j]=c2[i][j]+c1;
		}

	}
	return r;
}

template<class TYPE> inline	const klMatrix<TYPE> operator-(const klMatrix<TYPE> &c1, const klMatrix<TYPE> &c2)
{
	if(c1.getRows() !=c2.getRows() || c1.getColumns() !=c2.getColumns())
	{
		ANSI_INFO; throw klError(err + "const klMatrix<TYPE> operator+(const klMatrix<TYPE> &c1, const klMatrix<TYPE> &c2) ERROR: incompatible dimensions.");
	}
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j] - c2[i][j];
		}

	}
	return r;

}

template<class TYPE> inline	const klMatrix<TYPE> operator-(const klMatrix<TYPE> &c2, TYPE c1)
{
	klMatrix<TYPE> r(c2.getRows(),c2.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c2.getRows();i++)
	{
		for(j=0;j<c2.getColumns();j++)
		{
			r[i][j]=c2[i][j]-c1;
		}

	}
	return r;

}

template<class TYPE> inline	const klMatrix<TYPE> operator-(TYPE c1, const klMatrix<TYPE> &c2)
{
	klMatrix<TYPE> r(c2.getRows(),c2.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c2.getRows();i++)
	{
		for(j=0;j<c2.getColumns();j++)
		{
			r[i][j]=c2[i][j]-c1;
		}

	}
	return r;

}

template<class TYPE> inline const klMatrix<TYPE> operator/(const klMatrix<TYPE> &c1, TYPE c2)
{
	if(c2==0)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> operator/(const klMatrix<TYPE> &c1, TYPE c2) : ERROR: attempting to divide by zero .");
	}
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j] / c2;
		}

	}
	return r;

}
template<class TYPE> inline const klMatrix<TYPE> operator/(TYPE c2,const klMatrix<TYPE> &c1 )
{ 
	if(c2==0)
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> operator/(const klMatrix<TYPE> &c1, TYPE c2) : ERROR: attempting to divide by zero .");
	}
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j] / c2;
		}

	}
	return r;
}

template<class TYPE> inline const klMatrix<TYPE> operator/(klMatrix<TYPE> &c1,const klMatrix<TYPE> &c2 )
{ 

	if(c1.getRows()!=c2.getRows() || c1.getColumns() !=c2.getColumns())
	{
		ANSI_INFO; throw klError(err + "klMatrix<TYPE> operator/(klMatrix<TYPE> &c1,const klMatrix<TYPE> &c2 ) ERROR: incompatible dimensions.");
	}

	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());

	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			if(c2[i][j]==0)
			{
				ANSI_INFO; throw klError(err + "klMatrix<TYPE>& operator/=(const klMatrix<TYPE> &c) ERROR: attempting to divide by zero");
			}
			r[i][j]=c1[i][j] / c2[i][j];
		}

	}
	return r;
}

template<class TYPE> inline	const klMatrix<TYPE> operator*(const klMatrix<TYPE> &c1, TYPE c2 )
{
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j] * c2;
		}

	}
	return r;

}

template<class TYPE> inline const klMatrix<TYPE> operator*(TYPE c2, const klMatrix<TYPE> &c1)
{
	klMatrix<TYPE> r(c1.getRows(),c1.getColumns());
	__int64_t i;
	__int64_t j;
	for(i=0;i<c1.getRows();i++)
	{
		for(j=0;j<c1.getColumns();j++)
		{
			r[i][j]=c1[i][j] * c2;
		}

	}
	return r;
}


template<class TYPE> inline	const klVector<TYPE> klMatrixToLowerRowType(const klMatrix<TYPE> &c)
{
	__int64_t i;
	__int64_t j;
	__int64_t lowerSize = 0;
	for(i=1;i<=c.getRows();i++)
		lowerSize +=i;

	klVector<TYPE> L(lowerSize) ;
	L=(TYPE)0.0;
	__int64_t index=0;
	for(int i=0;i<c.getRows();i++)
	{	
		for(int j =0;j<c.getColumns();j++)
		{
			if(i<j)
				continue;
			else
				L[index++] = c[i][j];
		}
	}
	return L;
}

template<class TYPE> inline	const klVector<TYPE> klMatrixToLowerColType(const klMatrix<TYPE> &c)
{
	__int64_t i;
	__int64_t j;
	__int64_t lowerSize = 0;
	for(i=1;i<=c.getRows();i++)
		lowerSize +=i;

	klVector<TYPE> L(lowerSize) ;
	L=(TYPE)0.0;
	__int64_t index=0;
	for(int j =0;j<c.getColumns();j++)
	{	for(int i=0;i<c.getRows();i++)
		{
			if(i<j)
				continue;
			else
				L[index++] = c[i][j];
		}
	}
	return L;
}


/*
template<class TYPE> inline	const klMatrix<TYPE> elem_mult(const klMatrix<TYPE> &m1, const klMatrix<TYPE> &m2)
{

}
template<class TYPE>	const klMatrix<TYPE> operator*(const klMatrix<TYPE> &c1, const klMatrix<TYPE> &c2)
{ 
}

template<class TYPE>	const klVector<TYPE> operator*(const klMatrix<TYPE> &m, const klVector<TYPE> &v)
{
}

template<class TYPE> inline const klMatrix<TYPE> operator*(const klVector<TYPE> &v, const klMatrix<TYPE> &m)
{
}


template<class TYPE> inline const klMatrix<TYPE> elem_div(const klMatrix<TYPE> &m1, const klMatrix<TYPE> &m2)
{
}

template<class TYPE> 	bool klMatrix<TYPE>::operator==(const klMatrix<TYPE> &m) const
{ 
}

template<class TYPE> 	bool klMatrix<TYPE>::operator!=(const klMatrix<TYPE> &m) const
{
}

*/

//Returns a diagonal matrix D_ij = c[i]
template<class TYPE> klMatrix<TYPE> diag(klVector<TYPE> c)
{
	klMatrix<TYPE> result(c.getColumns(),c.getColumns());
	result=0.0;
	__int64_t i=0;
	__int64_t j=0;
	for(i=0;i<c.getColumns();i++)
	{
		for(j=0;j<c.getColumns();j++)
		{
			if(i==j)
				result[i][j] = c[i];
		}
	}
	return result;
}
template<> double klMatrix<double>::norm(bool ell_infty)
{
	char norm = 'G' ;
	if(ell_infty==true)
		norm = 'I';
	else 
		norm = '1';
	int m=_row;
	int n=_col;
	int lda=m;
	double* work = new double[m*3];
	double val = dlange(&norm, &m, &n, this->transpose().getMemory(), &lda,work);

	//bbcrevisit 
	norm='G';double val2 = dlange(&norm, &m, &n, this->transpose().getMemory(), &lda,work);
	norm='I';double val3 = dlange(&norm, &m, &n, this->transpose().getMemory(), &lda,work);
	norm='1';double val4 = dlange(&norm, &m, &n, this->transpose().getMemory(), &lda,work);
	norm='F';double val5 = dlange(&norm, &m, &n, this->transpose().getMemory(), &lda,work);

	/*
	= 'M' or 'm': val = max(abs(Aij)), largest absolute value of the matrix A.
	= '1' or 'O' or 'o': val = norm1(A), 1-norm of the matrix A (maximum column sum),
	= 'I' or 'i': val = normI(A), infinity norm of the matrix A (maximum row sum),
	= 'F', 'f', 'E' or 'e': val = normF(A), Frobenius norm of the matrix A (square root of sum of squares).
	*/
	norm='G';double val6 = dlange(&norm, &n, &m, this->getMemory(), &n,work);
	norm='I';double val7 = dlange(&norm, &n, &m, this->getMemory(), &n,work);
	norm='1';double val8 = dlange(&norm, &n, &m, this->getMemory(), &n,work);
	norm='F';double val9 = dlange(&norm, &n, &m, this->getMemory(), &n,work);
	
	delete work;
	return val;
}

template<> float klMatrix<float>::norm(bool ell_infty )
{
	char norm = 'G' ;
	if(ell_infty==true)
		norm = 'I';
	else 
		norm = '1';
	int m=_row;
	int n=_col;
	int lda = m;
	float* work = new float[m*3];
	float val = slange(&norm, &m, &n, this->transpose().getMemory(), &lda, work);
	delete work;
	return val;	
}

template<  > double klMatrix<double>::ConditionNumber(bool ellone)
{
	//Compute the Norm
	__int64_t i,j;
	
	double anorm =0.0f;
	if(ellone)
		anorm = norm();
	else
		anorm = norm(true);
	//Compute the LU factorization
	if(!_contiguous)
	{
		ANSI_INFO; throw klError(err + "template<  > double klMatrix<double>::ConditionNumber(bool ellone) ERROR: this routine is only supported for matrices with contiguous memory layout ");
	}

	//transpose to get into FORTRAN column major storage format
	klMatrix<double> tr=transpose();
	//allocate memory for returned pivot indices
	size_t size=_row>_col? _row : _col;
	int* ipiv=new int[2*size];
	int info=0;
	int n=_row;
	int m=_col;
	dgetrf(&n,&m,tr.getMemory(),&n,ipiv,&info);
	//tr is now LU factored

	char Norm;
	if(ellone==true)
		Norm='O';//doing L1 norm
	else
		Norm='I';//doing L\infty norm
	int* iwork=new int[2*_col+1];
	double* work=new double[4*_col+1]; 
	double rcond=0;

	dgecon(&Norm,&m,tr.getMemory(),&n,&anorm,&rcond,work,iwork,&info);

	if(info<0)
	{
		ANSI_INFO; throw klError(err + "template<  > double klMatrix<double>::ConditionNumber(bool ellone) ERROR: parameter error in MKL call to dgecon.");
	}

	delete ipiv;
	delete iwork;
	delete work;

	//dgecon returns 1/cond in case condition number is very big
	//check for rcond near EPS and then return the reciprocal
	if(rcond<2*DBL_EPSILON)
		return 0;
	else 
		return 1/rcond; 
}

template<  > float klMatrix<float>::ConditionNumber(bool ellone)
{
	//Compute the Norm
	__int64_t i,j;
	
	float anorm =0;
	if(ellone)
		anorm = norm();
	else
		anorm = norm(true);

	//Compute the LU factorization
	if(!_contiguous)
	{
		ANSI_INFO; throw klError(err + "template<  > float klMatrix<float>::ConditionNumber(bool ellone) ERROR: this routine is only supported for matrices with contiguous memory layout ");
	}

	//transpose to get into FORTRAN column major storage format
	klMatrix<float> tr=transpose();
	//allocate memory for returned pivot indices
	size_t size=_row>_col? _row : _col;
	int* ipiv=new int[2*size];
	int info=0;
	int n=_row;
	int m=_col;
	sgetrf(&n,&m,tr.getMemory(),&n,ipiv,&info);
	//tr is now LU factored
	char Norm;
	if(ellone==true)
		Norm='O';//doing L1 norm
	else
		Norm='I';
	int* iwork=new int[2*_col+1];
	float* work=new float[4*_col+1]; 
	float rcond=0;

	sgecon(&Norm,&m,tr.getMemory(),&n,&anorm,&rcond,work,iwork,&info);

	if(info<0)
	{
		ANSI_INFO; throw klError(err + "template<  > float klMatrix<float>::ConditionNumber(bool ellone) ERROR: parameter error in MKL call to dgecon.");
	}
	delete ipiv;
	delete iwork;
	delete work;

	//dgecon returns 1/cond in case condition number is very big
	//check for rcond near EPS and then return the reciprocal
	if(rcond<2*FLT_EPSILON)
		return FLT_MAX;
	else 
		return 1.0/rcond; 

}

//blas min
inline void minV(const klMatrix<double>& X,klVector<double>& minVals ,bool rowMins=1)
{	
	if(rowMins==1)
	{
		if( minVals.getColumns() != X.getRows())
		{			
			std::stringstream ANSI_INFO_ss (std::stringstream::in | std::stringstream::out );
			ANSI_INFO_ss<<" minVals.getColumns() != X.getRows() in void minV(const klMatrix<double>& X,klVector<double>& minVals ,bool rowMins=1) ";
			ANSI_INFO_ss<<"ANSI COMPILE INFO: " <<__DATE__<<"     "<<__TIME__<<"   "<<__FILE__<<"   "<<__LINE__<<"       "<<std::endl;
			std::string err = ANSI_INFO_ss.str();		
			throw klError(err);
		}
		const int N=X.getColumns();
		for(int i=0;i<X.getRows();i++)
		{
			void* pMem =X.getMemory()+i*X.getColumns();
			const int incX = 1;
			__int64_t index = cblas_idamin (N, (double*)pMem,incX);
			minVals[i]=X[i][index];
		}
	}
	else
	{
		if( minVals.getColumns() != X.getColumns())
		{			
			std::stringstream ANSI_INFO_ss (std::stringstream::in | std::stringstream::out );
			ANSI_INFO_ss<<" mminVals.getColumns() != X.getColumns() in void minV(const klMatrix<double>& X,klVector<double>& minVals ,bool rowMins=1) ";
			ANSI_INFO_ss<<"ANSI COMPILE INFO: " <<__DATE__<<"     "<<__TIME__<<"   "<<__FILE__<<"   "<<__LINE__<<"       "<<std::endl;
			std::string err = ANSI_INFO_ss.str();		
			throw klError(err);
		}
		const int N=X.getRows();
		for(int i=0;i<X.getColumns();i++)
		{
			void* pMem =X.getMemory()+i;
			const int incX = X.getColumns();
			__int64_t index = cblas_idamin (N, (double*)pMem,incX);
			minVals[i]=X[index][i];
		}
	}
}

//blas max
inline void maxV(const klMatrix<double>& X,klVector<double>& maxVals ,bool rowMins=1)
{	
	if(rowMins==1)
	{
		if( maxVals.getColumns() != X.getRows())
		{			
			std::stringstream ANSI_INFO_ss (std::stringstream::in | std::stringstream::out );
			ANSI_INFO_ss<<" minVals.getColumns() != X.getRows() in void minV(const klMatrix<double>& X,klVector<double>& minVals ,bool rowMins=1) ";
			ANSI_INFO_ss<<"ANSI COMPILE INFO: " <<__DATE__<<"     "<<__TIME__<<"   "<<__FILE__<<"   "<<__LINE__<<"       "<<std::endl;
			std::string err = ANSI_INFO_ss.str();		
			throw klError(err);
		}
		const int N=X.getColumns();
		for(int i=0;i<X.getRows();i++)
		{
			void* pMem =X.getMemory()+i*X.getColumns();
			const int incX = 1;
			__int64_t index = cblas_idamax (N, (double*)pMem,incX);
			maxVals[i]=X[i][index];
		}
	}
	else
	{
		if( maxVals.getColumns() != X.getColumns())
		{			
			std::stringstream ANSI_INFO_ss (std::stringstream::in | std::stringstream::out );
			ANSI_INFO_ss<<" mminVals.getColumns() != X.getColumns() in void minV(const klMatrix<double>& X,klVector<double>& minVals ,bool rowMins=1) ";
			ANSI_INFO_ss<<"ANSI COMPILE INFO: " <<__DATE__<<"     "<<__TIME__<<"   "<<__FILE__<<"   "<<__LINE__<<"       "<<std::endl;
			std::string err = ANSI_INFO_ss.str();		
			throw klError(err);
		}
		const int N=X.getRows();
		for(int i=0;i<X.getColumns();i++)
		{
			void* pMem =X.getMemory()+i;
			const int incX = X.getColumns();
			__int64_t index = cblas_idamax (N, (double*)pMem,incX);
			maxVals[i]=X[index][i];
		}
	}
}


//klMatrix stream io.  Operator <<  override for klMatrix class
template <class TYPE> static ostream& operator<<(ostream& str , const klMatrix<TYPE>& v) {
	int i = v.getRows();
	int k = v.getColumns();
	for (int l=0;l<i;l++)
	{
		for (int j=0; j<k; j++) 
		{
			if(j!=k-1)
				str << v[l][j]<<", ";
			else
				str<<v[l][j];
		}
		if(l!=i-1)
			str<<"\n";
	}
	return str<<"\n";
}

//klMatrix stream io.  Operator >> override for klMatrix class
template <class TYPE> static inline istream& operator>>(istream& c, klMatrix<TYPE> & v) 
{
	char ch;
	int j=0;
	do
	{
		int i=0;
		do
		{
			TYPE d;
			c >> d;
			v[j][i]=d;
			i++;
			if(i!=v[j].getRowSize())
				c >> ch;
		} while (i<(int)v[j].getRowSize());
		j++;
	}while(j<(int)v.getRows());
	return c;
}

class klBinaryIO
{
public:
	
    const static __int64_t _chunkThresh=4294967296LL;

	const static __int64_t _chunkSize = 1073741824LL;

	static inline void WriteWinx64( klMatrix<double>& out, string fileName)
	{
		FILE* fd=NULL;
		if(fileName.substr(fileName.find_last_of(".") + 1) == "klmd") 
		{
			try
			{
				fd = fopen( fileName.c_str(), "wb+" );
				if(fd==NULL)
				{
					ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::WriteWinx64( klMatrix<double>& out, string fileName)");
				}
				
				__int64_t ebuf[2]={0,0};
				ebuf[0]=out.getRows();
				ebuf[1]=out.getColumns();

				fwrite(ebuf, sizeof(__int64_t),2,fd);

				__int64_t writeSize = sizeof(double)*ebuf[0]*ebuf[1];

				if(writeSize < _chunkThresh)
				{					
					void* writeP = out.getMemory();
					fwrite(writeP,sizeof(double),ebuf[0]*ebuf[1],fd);
					fclose(fd);
				}
				else
				{
					__int64_t chunkSize = _chunkSize;//In bytes, so divide by 8 in the write call
					__int64_t numWrites=0;
					__int64_t bytesRemaining = writeSize;
					while(bytesRemaining>0)
					{
						void* writeP = out.getMemory();
						double* writeDP= (double*)writeP;
						writeDP  +=(chunkSize /sizeof(double)) *numWrites;

						if (bytesRemaining>=chunkSize)
						{
							fwrite(writeDP,sizeof(double),chunkSize /sizeof(double) ,fd);
							bytesRemaining -=chunkSize;
							numWrites++;
							continue;
						}
						else
						{
							//Last Write
							fwrite(writeDP,sizeof(double),bytesRemaining /sizeof(double),fd);
							bytesRemaining -=bytesRemaining;
						}
					}

					fclose(fd);
				}
			}
			catch(...)
			{
				klError err(" klMatrix<double> klFastWriteWinx64(string fileName) error writing ");
				if(fd)
				{
					try
					{
						fclose(fd);
					}
					catch(...)
					{
					}
				}
				throw err;
			}
		} 
		else 
		{
			klError err(" void  klFastWriteWinx64( klMatrix<double> out, string fileName) called with bad file extension");
			throw err;
		}
	}

	static inline void WriteWinx64(klVector<double>& out, string fileName)
	{
		FILE* fd=NULL;
		if(fileName.substr(fileName.find_last_of(".") + 1) == "klvd") 
		{
			try
			{
				fd = fopen( fileName.c_str(), "wb+" );
				if (fd ==NULL)
				{
					ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::WriteWinx64(klVector<double>& out, string fileName)");
				}
				__int64_t ebuf[1]={0};
				ebuf[0]=out.getColumns();
				
				fwrite(ebuf, sizeof(__int64_t),1,fd);

				__int64_t writeSize = sizeof(double)*ebuf[0];

				if(writeSize < _chunkThresh)
				{					
					void* writeP = out.getMemory();
					fwrite(writeP,sizeof(double),ebuf[0],fd);
					fclose(fd);
				}
				else
				{
					__int64_t chunkSize = _chunkSize;//In bytes, so divide by 8 in the write call
					__int64_t numWrites=0;
					__int64_t bytesRemaining = writeSize;
					while(bytesRemaining>0)
					{
						void* writeP = out.getMemory();
						double* writeDP= (double*)writeP;
						writeDP  +=(chunkSize /sizeof(double)) *numWrites;

						if (bytesRemaining>=chunkSize)
						{
							fwrite(writeDP,sizeof(double),chunkSize /sizeof(double) ,fd);
							bytesRemaining -=chunkSize;
							numWrites++;
							continue;
						}
						else
						{
							//Last Write
							fwrite(writeDP,sizeof(double),bytesRemaining /sizeof(double) ,fd);
							bytesRemaining -=bytesRemaining;
						}
					}

					fclose(fd);
				}

			}
			catch(...)
			{
				klError err(" klMatrix<double> klFastReadWinx64(string fileName) error writing ");
				if(fd)
				{
					try
					{
						fclose(fd);
					}
					catch(...)
					{
					}
				}
				throw err;
			}
		} 
		else 
		{
			klError err(" void  klFastWriteWinx64( klMatrix<double> out, string fileName) called with bad file extension");
			throw err;
		}
	}

	static inline void MatReadWinx64(string fileName, klMatrix<double>& klmd )
	{
		if(fileName.substr(fileName.find_last_of(".") + 1) == "klmd") 
		{
			FILE* fd=NULL;
			try
			{
				fd = fopen( fileName.c_str(), "rb" );
				if (fd==NULL)
				{
					ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::MatReadWinx64(string fileName, klMatrix<double>& klmd )");
				}
				__int64_t ebuf[2]={0,0};

				fread(ebuf, sizeof(__int64_t),2,fd);

				if(ebuf[0]>0 && ebuf[1]>0)
				{
					if( klmd.getRows() !=ebuf[0] || klmd.getColumns() !=ebuf[1])
					{
						ANSI_INFO; throw klError(err + "In MatReadWinx64(string fileName, klMatrix<double>& klmd ) (klmd.getRows() !==ebuf[0] || klmd.getColumns() !=ebuf[1]) is false"); 
					}
					
					void* readP = klmd.getMemory();
					__int64_t readSize = sizeof(double)*ebuf[0]*ebuf[1];

					if(readSize < _chunkThresh)
					{
						fread(readP,sizeof(double),ebuf[0]*ebuf[1],fd);
					}
					else
					{
						__int64_t chunkSize = _chunkSize;//In bytes, so divide by 8 in the write call
						__int64_t numReads=0;
						__int64_t bytesRemaining = readSize;
						while(bytesRemaining>0)
						{
							readP = klmd.getMemory();
							double* readDP= (double*)readP;
							readDP  +=(chunkSize /sizeof(double))*numReads;

							if (bytesRemaining>=chunkSize)
							{
								fread(readDP,sizeof(double),chunkSize /sizeof(double),fd);
								bytesRemaining -=chunkSize;
								numReads++;
								continue;
							}
							else
							{
								//Last Read
								fread(readDP,sizeof(double),bytesRemaining /sizeof(double),fd);
								bytesRemaining -=bytesRemaining;
							}
						}
					}

				}
				fclose(fd);
			}
			catch(...)
			{
				klError err(" klMatrix<double> klFastReadWinx64(string fileName) error reading or allocating");
				if(fd)
				{
					try
					{
						fclose(fd);
					}
					catch(...)
					{
					}
				}
				throw err;
			}
		} 
		else 
		{
			klError err(" klMatrix<double> MatReadWinx64(string fileName) called with bad file extension");
			throw err;
		}
	}

	//bbctodo - get move constructor figured out
	/*static inline klMatrix<double>& MatReadWinx64(string fileName)  
	{
	__int64_t rows,cols;
	klBinaryIO::QueryWinx64(fileName,rows,cols);

	klMatrix<double> klmd(rows,cols);

	if(fileName.substr(fileName.find_last_of(".") + 1) == "klmd") 
	{
	FILE* fd=NULL;
	try
	{
	fd = fopen( fileName.c_str(), "rb" );
	if (fd==NULL)
	{ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::MatReadWinx64(string fileName, klMatrix<double>& klmd )");}
	__int64_t ebuf[2]={0,0};

	fread(ebuf, sizeof(__int64_t),2,fd);

	if(ebuf[0]>0 && ebuf[1]>0)
	{
	if( klmd.getRows() !=ebuf[0] || klmd.getColumns() !=ebuf[1])
	{ANSI_INFO; throw klError(err + "In MatReadWinx64(string fileName, klMatrix<double>& klmd ) (klmd.getRows() !==ebuf[0] || klmd.getColumns() !=ebuf[1]) is false"; }
	void* readP = klmd.getMemory();
	fread(readP,sizeof(double),ebuf[0]*ebuf[1],fd);
	}
	fclose(fd);
	}
	catch(...)
	{
	klError err(" klMatrix<double> klFastReadWinx64(string fileName) error reading or allocating");
	if(fd)
	{
	try
	{
	fclose(fd);
	}
	catch(...)
	{
	}
	}
	throw err;
	}
	} 
	else 
	{
	klError err(" klMatrix<double> MatReadWinx64(string fileName) called with bad file extension");
	throw err;
	}
	}*/

	static inline bool QueryWinx64(string fileName, __int64_t& rows, __int64_t&  cols)
	{ 
		FILE* fd=NULL;
		try
		{
			fd = fopen( fileName.c_str(), "rb" );
			if (fd==NULL)
			{
				ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::QueryWinx64(string fileName, __int64_t& rows, __int64_t&  cols)");
			}
			if(fileName.substr(fileName.find_last_of(".") + 1) == "klvd")
			{
				__int64_t ebuf[1]={0};
				fread(ebuf, sizeof(__int64_t),1,fd);
				rows =0;
				cols = ebuf[0];
			}
			if(fileName.substr(fileName.find_last_of(".") + 1) == "klmd")
			{
				__int64_t ebuf[2]={0,0};
				fread(ebuf, sizeof(__int64_t),2,fd);
				rows=ebuf[0];
				cols=ebuf[1];
			}
		}
		catch(...)
		{
			return false;
		}
		return true;
	}

	static inline void VecReadWinx64(string fileName,klVector<double>& klvd)
	{
		if(fileName.substr(fileName.find_last_of(".") + 1) == "klvd") 
		{
			FILE* fd=NULL;
			try
			{
				fd = fopen( fileName.c_str(), "rb" );
				if(fd==NULL)
				{
					ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::VecReadWinx64(string fileName,klVector<double>& klvd)");
				}
				__int64_t ebuf[1]={0};

				fread(ebuf, sizeof(__int64_t),1,fd);

				if(ebuf[0]>0)
				{
					if( klvd.getColumns() !=ebuf[0])
					{
						ANSI_INFO; throw klError(err + "In static inline void VecReadWinx64(string fileName,klVector<double>& klvd) (klvd.getColumns() !=ebuf[0]) is false");
					}
								
					void* readP = klvd.getMemory();
					__int64_t readSize = sizeof(double)*ebuf[0];

					if(readSize < _chunkThresh)
					{
						fread(readP,sizeof(double),ebuf[0],fd);
					}
					else
					{
						__int64_t chunkSize = _chunkSize;//In bytes, so divide by 8 in the write call
						__int64_t numReads=0;
						__int64_t bytesRemaining = readSize;
						while(bytesRemaining>0)
						{
							readP = klvd.getMemory();
							double* readDP= (double*)readP;
							readDP  +=(chunkSize /sizeof(double))*numReads;

							if (bytesRemaining>=chunkSize)
							{fread(readDP,sizeof(double),chunkSize /sizeof(double),fd);
							bytesRemaining -=chunkSize;
							numReads++;
							continue;
							}
							else
							{
								//Last Read
								fread(readDP,sizeof(double),bytesRemaining /sizeof(double),fd);
								bytesRemaining -=bytesRemaining;
							}
						}
					}
				}
				fclose(fd);
			}
			catch(...)
			{
				klError err(" klVector<double> VecReadWinx64(string fileName) error reading or allocating");
				if(fd)
				{
					try
					{
						fclose(fd);
					}
					catch(...)
					{
					}
				}

				throw err;
			}
		} 
		else 
		{
			klError err(" klMatrix<double> klFastReadWinx64(string fileName) called with bad file extension");
			throw err;
		}
	}

	////Same methods but they write to an open file handle.
	//static inline void WriteWinx64( klMatrix<double>& out, FILE* fd)
	//{
	//	try
	//	{
	//		if(fd==NULL)
	//		{	ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::WriteWinx64( klMatrix<double>& out, string fileName))";} 
	//		__int64_t ebuf[2]={0,0};
	//		ebuf[0]=out.getRows();
	//		ebuf[1]=out.getColumns();
	//		fwrite(ebuf, sizeof(__int64_t),2,fd);
	//		void* writeP = out.getMemory();
	//		fwrite(writeP,sizeof(double),ebuf[0]*ebuf[1],fd);
	//		fclose(fd);
	//	}
	//	catch(...)
	//	{
	//		klError err(" klMatrix<double> klFastReadWinx64(string fileName) error writing ");
	//		if(fd)
	//		{
	//			try
	//			{
	//				fclose(fd);
	//			}
	//			catch(...)
	//			{
	//			}
	//		}
	//		throw err;
	//	}	
	//}

	//static inline void WriteWinx64(klVector<double>& out, FILE* fd)
	//{
	//	try
	//	{
	//		if (fd ==NULL)
	//		{	ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::WriteWinx64(klVector<double>& out, string fileName)");}
	//		__int64_t ebuf[1]={0};
	//		ebuf[0]=out.getColumns();
	//		fwrite(ebuf, sizeof(__int64_t),1,fd);
	//		void* writeP = out.getMemory();
	//		fwrite(writeP,sizeof(double),ebuf[0],fd);
	//		fclose(fd);
	//	}
	//	catch(...)
	//	{
	//		klError err(" klMatrix<double> klFastReadWinx64(string fileName) error writing ");
	//		if(fd)
	//		{
	//			try
	//			{
	//				fclose(fd);
	//			}
	//			catch(...)
	//			{
	//			}
	//		}
	//		throw err;
	//	}	}

	static inline void MatReadWinx64(FILE* fd, klMatrix<double>& klmd )
	{
		try
		{
			if (fd==NULL)
			{
				ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::MatReadWinx64(string fileName, klMatrix<double>& klmd )");
			}
			__int64_t ebuf[2]={0,0};

			fread(ebuf, sizeof(__int64_t),2,fd);

			if(ebuf[0]>0 && ebuf[1]>0)
			{
				if( klmd.getRows() !=ebuf[0] || klmd.getColumns() !=ebuf[1])
				{
					ANSI_INFO; throw klError(err + "In MatReadWinx64(string fileName, klMatrix<double>& klmd ) (klmd.getRows() !==ebuf[0] || klmd.getColumns() !=ebuf[1]) is false"); 
				}
				void* readP = klmd.getMemory();
				fread(readP,sizeof(double),ebuf[0]*ebuf[1],fd);
			}
			fclose(fd);
		}
		catch(...)
		{
			klError err(" klMatrix<double> klFastReadWinx64(string fileName) error reading or allocating");
			if(fd)
			{
				try
				{
					fclose(fd);
				}
				catch(...)
				{
				}
			}
			throw err;
		}

	}

	static inline void VecReadWinx64(FILE* fd,klVector<double>& klvd)
	{

		try
		{
			if(fd==NULL)
			{
				ANSI_INFO; throw klError(err + "Bad file handle in klBinaryIO::VecReadWinx64(string fileName,klVector<double>& klvd)");
			}
			__int64_t ebuf[1]={0};

			fread(ebuf, sizeof(__int64_t),1,fd);

			if(ebuf[0]>0)
			{
				if( klvd.getColumns() !=ebuf[0])
				{
					ANSI_INFO; throw klError(err + "In static inline void VecReadWinx64(string fileName,klVector<double>& klvd) (klvd.getColumns() !=ebuf[0]) is false");
				}

				void* readP = klvd.getMemory();
				fread(readP,sizeof(double),ebuf[0],fd);
			}
			fclose(fd);
		}
		catch(...)
		{
			klError err(" klVector<double> VecReadWinx64(string fileName) error reading or allocating");
			if(fd)
			{
				try
				{
					fclose(fd);
				}
				catch(...)
				{
				}
			}

			throw err;
		}
	}


};

#endif __kl_matrix__
