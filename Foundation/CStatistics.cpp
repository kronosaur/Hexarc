//	CStatistics.cpp
//
//	Math functions and classes
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/math/distributions/students_t.hpp>

double CStatistics::ErrorFunction (double x)

//	ErrorFunction
//
//	Returns the error function of x

	{
	//	Coefficients for the approximation to the error function

	const double a1 =  0.254829592;
	const double a2 = -0.284496736;
	const double a3 =  1.421413741;
	const double a4 = -1.453152027;
	const double a5 =  1.061405429;
	const double p  =  0.3275911;

	//	Save the sign of x
	int sign = x < 0 ? -1 : 1;
	x = fabs(x);

	//	A&S formula 7.1.26 for erf approximation

	double t = 1.0 / (1.0 + p * x);
	double y = (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t;
	y = 1.0 - y * exp(-x * x);

	return sign * y;
	}

double CStatistics::ExponentialCDF (double x, double lambda)

//	ExponentialCDF
//
//	Returns the cumulative distribution function of the exponential distribution

	{
	if (x < 0.0)
		return 0.0;

	return 1.0 - exp(-lambda * x);
	}

double CStatistics::ExponentialInverse (double x, double lambda)

//	ExponentialInverse
//
//	Returns the inverse exponential distribution function.

	{
	if (x < 0.0 || x >= 1.0 || lambda <= 0.0)
		throw CException(errFail);

	return -log(1.0 - x) / lambda;
	}

double CStatistics::ExponentialPDF (double x, double lambda)

//	ExponentialPDF
//
//	Returns the probability density function of the exponential distribution

	{
	if (x < 0.0)
		return 0.0;

	return lambda * exp(-lambda * x);
	}

double CStatistics::FDistCDF (double x, double df1, double df2)

//	FDistCDF
//
//	Returns the cumulative distribution function of the F-distribution

	{
	if (df1 <= 0.0 || df2 <= 0.0)
		throw CException(errFail);

	if (x < 0.0)
		return 0.0;

	//	Define the F-distribution

	boost::math::fisher_f_distribution<> f_dist(df1, df2);

	//	Return the CDF

	return boost::math::cdf(f_dist, x);
	}

double CStatistics::FDistInverse (double x, double df1, double df2)

//	FDistInverse
//
//	Returns the inverse F-distribution function

	{
	if (df1 <= 0.0 || df2 <= 0.0)
		throw CException(errFail);

	if (x < 0.0)
		return 0.0;

	//	Define the F-distribution

	boost::math::fisher_f_distribution<> f_dist(df1, df2);

	//	Return the inverse

	return boost::math::quantile(f_dist, x);
	}

double CStatistics::FDistPDF (double x, double df1, double df2)

//	FDistPDF
//
//	Returns the probability density function of the F-distribution

	{
	if (df1 <= 0.0 || df2 <= 0.0)
		throw CException(errFail);

	if (x < 0.0)
		return 0.0;

	//	At x = 0, df1 = 1, df2 = 1, the F-distribution is infinite, so we return
	//	nan.

	if (x == 0.0 && df1 == 1.0 && df2 == 1.0)
		return std::numeric_limits<double>::quiet_NaN();

	//	Define the F-distribution

	boost::math::fisher_f_distribution<> f_dist(df1, df2);

	//	Return the PDF

	return boost::math::pdf(f_dist, x);
	}

double CStatistics::NormalCDF (double x)

//	NormalCDF
//
//	Returns the cumulative distribution function of the standard normal distribution

	{
	return 0.5 * (1.0 + ErrorFunction(x / sqrt(2.0)));
	}

double CStatistics::NormalPDF (double x)

//	NormalPDF
//
//	Returns the probability density function of the standard normal distribution

	{
	const double inv_sqrt_2pi = 0.3989422804014327;
	double part = exp(-0.5 * x * x);
	return inv_sqrt_2pi * part;
	}

double CStatistics::TDistCDF (double x, double df)

//	TDistCDF
//
//	Returns the cumulative distribution function of the T-distribution

	{
	if (df <= 0.0)
		throw CException(errFail);

	//	Define the T-distribution with the given degrees of freedom

	boost::math::students_t_distribution<> t_dist(df);

	//	Return the CDF

	return boost::math::cdf(t_dist, x);
	}

double CStatistics::TDistInverse (double x, double df)

//	TDistInverse
//
//	Returns the inverse T-distribution function

	{
	if (df <= 0.0)
		throw CException(errFail);

	//	Define the T-distribution with the given degrees of freedom

	boost::math::students_t_distribution<> t_dist(df);

	//	Return the CDF

	return boost::math::quantile(t_dist, x);
	}

double CStatistics::TDistPDF (double x, double df)

//	TDistPDF
//
//	Returns the probability density function of the T-distribution

	{
	if (df <= 0.0)
		throw CException(errFail);

	//	Define the T-distribution with the given degrees of freedom

	boost::math::students_t_distribution<> t_dist(df);

	// Return the PDF at the given x

	return boost::math::pdf(t_dist, x);
	}
