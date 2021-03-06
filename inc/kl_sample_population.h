 /*******************************
 * Copyright (c) <2007>, <Bruce Campbell> All rights reserved. Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met: 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  *  
 * Bruce B Campbell 07 08 2014  *
 ********************************/

#ifndef __kl_sample_population__
#define __kl_sample_population__
#include "kl_matrix.h"

#include "mkl.h"

#include "kl_stat.h"

double *omnibus_moments (double * x, int n);
double *geary_test(double * x, int n);
double *student_range(double * x, int n);
double *dagostino_d (double * x, int n);
double *extreme (double * x, int n);
double *kuipers_v (double * x, int n);
double *watson_u2(double * x, int n);
double *durbins_exact(double * x, int n);
double *anderson_darling(double * x, int n);
double *cramer_von_mises(double * x, int n);
double *kolmogorov_smirnov (double * x, int n);
double *chi_square(double * x, int n);
double *shapiro_wilk(double * x, int n);;
double *shapiro_francia(double * x, int n);
double *weisberg_bingham (double * x, int n);
double *royston(double * x, int n);
double *shapiro_wilk_exp(double * x, int n);
double *kolmogorov_smirnov_exp (double * x, int n);
double *cramer_von_mises_exp(double * x, int n);
double *kuipers_v_exp (double * x, int n);
double *watson_u2_exp (double * x, int n);
double *anderson_darling_exp(double * x, int n);
double *chi_square_exp(double * x, int n);
double *mod_maxlik_ratio (double * x, int n);
double *coeff_variation(double * x, int n);
double *kotz_families(double * x, int n);

template<class TYPE> class klSamplePopulation : protected klMatrix<TYPE>
{
public:

	klSamplePopulation() : klMatrix<TYPE> ()
	{
		_statsCalculated=false;
		_mean=NULL;
		_variance=NULL;
		_skewness=NULL;
		_kurtosis=NULL;	
	}

	klSamplePopulation<TYPE> (const klSamplePopulation<TYPE>& src) : klMatrix<TYPE>(src)
	{
		_statsCalculated=false;
		_mean=NULL;
		_variance=NULL;
		_skewness=NULL;
		_kurtosis=NULL;
	}

	klSamplePopulation<TYPE> (const klMatrix<TYPE>& src) : klMatrix<TYPE>(src)
	{
		_statsCalculated=false;
		_mean=NULL;
		_variance=NULL;
		_skewness=NULL;
		_kurtosis=NULL;		
	}

	klSamplePopulation(unsigned int row,unsigned int col) : klMatrix<TYPE>(row,col)
	{
		_statsCalculated=false;
		_mean=NULL;
		_variance=NULL;
		_skewness=NULL;
		_kurtosis=NULL;
		
	}

	klSamplePopulation(klMemMgr* mgr,unsigned int row,unsigned int col) : klMatrix<TYPE>( mgr,row,col)
	{
		_statsCalculated=false;
		_mean=NULL;
		_variance=NULL;
		_skewness=NULL;
		_kurtosis=NULL;
		
	}

	klSamplePopulation(TYPE* mem,unsigned int row,unsigned int col) : klMatrix<TYPE>(mem,row,col)
	{
		_statsCalculated=false;
		_mean=NULL;
		_variance=NULL;
		_skewness=NULL;
		_kurtosis=NULL;
		
	}

	~klSamplePopulation()
	{
		if(_mean!=NULL)
			delete _mean;
		if(_variance!=NULL)
			delete _variance;
		if(_skewness!=NULL)
			delete _skewness;
		if(_kurtosis!=NULL)
			delete _kurtosis;


	}

	//String representation of the distribution tests.
	//The table only gets filed if the tests are run 
	 vector<string > getDistTestNames()
	{
		vector<string > DistTestNames;
	
		DistTestNames.push_back("durbins_exact");
		DistTestNames.push_back("kolmogorov_smirnov");
		DistTestNames.push_back("chi_square");
		DistTestNames.push_back("kolmogorov_smirnov_exp");
		DistTestNames.push_back("chi_square_exp");

		return DistTestNames;
	}
	 
	//Each row of matrix is the full set of univariate distribution tests for variable in column i of the sample population
	klMatrix<double> calcDistributionTests()
	{
		klMatrix<double> distTest(_col,5);

		unsigned int i;

		for(i=0;i<_col;i++)
		{
			klVector<double> variable;//bbc revisit - use mgr or free store for temp variable for column i?
			if(_mgr)				
				variable.setup(_row,_mgr);
			else
				variable.setup(_row);
			unsigned int k;
			for(k=0;k<_row;k++)
			{	variable[k]=(double)(_vectors+k)->operator[](i);
			}

			double *t0= durbins_exact (variable.getMemory(),_row);
			double *t1=kolmogorov_smirnov (variable.getMemory(),_row);
			double *t2=chi_square(variable.getMemory(),_row);
			double *t3=kolmogorov_smirnov_exp (variable.getMemory(),_row);
			double *t4=chi_square_exp(variable.getMemory(),_row);
			
			distTest[i][0]=*t0;
			distTest[i][1]=*t1;
			distTest[i][2]=*t2;
			distTest[i][3]=*t3;
			distTest[i][4]=*t4;
		}
		return distTest;
	}
	
	void calcDescriptiveStats()
	{
		if(_mean!=NULL)
			delete _mean;
		if(_variance!=NULL)
			delete _variance;
		if(_skewness!=NULL)
			delete _skewness;
		if(_kurtosis!=NULL)
			delete _kurtosis;

		unsigned int i;

		_mean=new TYPE[this->_col];
		_variance=new TYPE[this->_col];
		_skewness=new TYPE[this->_col];
		_kurtosis=new TYPE[this->_col];

		for(i=0;i<this->_col;i++)
		{
			klVector<TYPE> variable;
			if(this->_mgr)
				variable.setup(this->_row, this->_mgr);
			else
				variable.setup(this->_row);
			unsigned int k;
			for(k=0;k<this->_row;k++)
				variable[k]=(this->_vectors+k)->operator[](i);


			TYPE* data=variable.getMemory();
			*(_mean+i)= KL_stat::mean(data,this->_row);
			*(_variance+i)= KL_stat::variance(data,this->_row,*(_mean+i));
			*(_skewness+i)= KL_stat::skewness(data,this->_row,*(_mean+i), *(_variance+i) );
			*(_kurtosis+i)= KL_stat::kurtosis(data,this->_row,*(_mean+i), *(_variance+i) );

		}
		_statsCalculated=true;

	}

	//A measure of the linear association between variables represented as columns.
	//This matrix has no negative eigenvalues by definition.  If there are zero eigenvlaues
	//then there is a linear dependency in the dataset.  If there are small negative eigenvlaues, 
	//due to numerical roundoff error, the covariance matrix can either be peturbed to make it SPD 
	//or the negative eigenvalues can safely be removed from the factorized matrix.
	klMatrix<TYPE> covarianceMatrix()
	{
		if(!_statsCalculated)
			calcDescriptiveStats();
		klMatrix<TYPE> covariance;
		if(this->_mgr)
			covariance.setup(this->_col,this->_col,this->_mgr);
		else 
			covariance.setup(this->_col,this->_col,0);

		unsigned int i;
		unsigned int j;
		for(i=0;i<this->_col;i++)
			for(j=i;j<this->_col;j++)
			{
				if(i==j)
					covariance[i][i]=*(_variance+i);
				else
				{
					unsigned int k;
					TYPE temp=0;
					for(k=0;k<this->_row;k++)
					{
						temp+=( (this->_vectors+k)->operator[](i) - *(_mean+i) ) * ( (this->_vectors+k)->operator[](j) - *(_mean+j) );
					}
					temp/=(this->_row-1);//unbiased form of estimator
					covariance[i][j]=temp;
					covariance[j][i]=temp;

				}

			}
			return covariance;


	}
	//A measure of the linear association without dependence on units
	klMatrix<TYPE> correlationMatrix(klMatrix<TYPE> covariance)
	{
		if(!_statsCalculated)
			calcDescriptiveStats();
		klMatrix<TYPE> correlation;
		if(_mgr)
			correlation.setup(_col,_col,_mgr);
		else 
			correlation.setup(_col,_col,0);

		unsigned int i;
		unsigned int j;
		for(i=0;i<_col;i++)
			for(j=i;j<_col;j++)
			{
				if(i==j)
					correlation[i][i]=1;
				else
				{
					TYPE temp=covariance[i][j];
					temp/=pow((double)*(_variance+i),0.5);
					temp/=pow((double)*(_variance+j),0.5);
					correlation[i][j]=temp;
					correlation[j][i]=temp;

				}

			}
			return correlation;
	}

	//let \Phi be the ON eigenvectors of the covariance matrix and \Lambda the diagonal 
	//matrix of eigenvalues.  Then the whitening transform \Phi \Lambda^{fract{1}{2}} applied to the coordiantes
	//of a data vector from this distribution transforms it to one drawn from a shperically symmetric distribution; 
	//ie the covariance matrix of the transformed data is I.
	klMatrix<TYPE> whiteningTransform()
	{
		//BBC REVISIT -NOT IMPLEMENTED
		throw exception("klMatrix<TYPE> whiteningTransform() not implemented");
		return klMatrix<TYPE>(0,0);
	}

	klMatrix<TYPE> centeredData()
	{		
		//120712 bbcrevisit meanV does not own this memory. 
		//Make sure it is not being deleted.
		klVector<TYPE > meanV= sampleMean();

		klMatrix<TYPE> centered(_row,_col);
		for(int i =0; i < _row;i++)
		{
			//centered[i] returns the row 
			centered[i] = operator[](i) - meanV;
		}

		return centered;		
	}

	//Implemented for float, double, long double
	TYPE stddev(unsigned int i) 
	{
		if(_statsCalculated==true) 
			return sqrt(*(_variance+i));
		else
		{
			calcDescriptiveStats();
			_statsCalculated=true;
			return sqrt(*(_variance+i));
		}
	}

	TYPE mean(unsigned int i) 
	{
		if(_statsCalculated==true) 
			return *(_mean+i);
		else
		{
			calcDescriptiveStats();
			_statsCalculated=true;
			return *(_mean+i);
		}
	}

	TYPE variance(unsigned int i) 
	{
		if(_statsCalculated==true) 
			return *(_variance+i);
		else
		{
			calcDescriptiveStats();
			_statsCalculated=true;
			return *(_variance+i);
		}
	}

	TYPE skewness(unsigned int i) 
	{
		if(_statsCalculated==true) 
			return *(_skewness+i);
		else
		{
			calcDescriptiveStats();
			_statsCalculated=true;
			return *(_skewness+i);
		}
	}

	TYPE kurtosis(unsigned int i) 
	{
		if(_statsCalculated==true) 
			return *(_kurtosis+i);
		else
		{
			calcDescriptiveStats();
			_statsCalculated=true;
			return *(_kurtosis+i);
		}
	}
	
	klVector<TYPE> sampleMean()
	{
		if (_statsCalculated==true)
		{
			klVector<TYPE> sampleMean(getMeanVector(),_col,false);
			return sampleMean;
		}
		else
		{
			calcDescriptiveStats();
			_statsCalculated=true;
			klVector<TYPE> sampleMean(getMeanVector(),_col,false);
			return sampleMean;
		}

	}

protected:

	TYPE* getMeanVector()
	{
		return _mean;
	}
	TYPE* getVarianceVector()
	{
		return _variance;
	}
	TYPE* getSkewnessVector()
	{
		return _skewness;
	}
	TYPE* getKurtosisVector()
	{
		return _kurtosis;
	}
	TYPE* _mean;
	TYPE* _variance;
	TYPE* _skewness;
	TYPE* _kurtosis;
	bool _statsCalculated;
		
	unsigned int _numDistTests;

};

#endif //__kl_sample_population__


