//	AEONMath.cpp
//
//	CDatum class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_AVERAGE,						"average");
DECLARE_CONST_STRING(FIELD_COUNT,						"count");
DECLARE_CONST_STRING(FIELD_MAX,							"max");
DECLARE_CONST_STRING(FIELD_MEAN,						"mean");
DECLARE_CONST_STRING(FIELD_MIN,							"min");
DECLARE_CONST_STRING(FIELD_NANS,						"nans");
DECLARE_CONST_STRING(FIELD_NULLS,						"nulls");
DECLARE_CONST_STRING(FIELD_RANGE,						"range");
DECLARE_CONST_STRING(FIELD_STDEV,						"stdev");
DECLARE_CONST_STRING(FIELD_STDEV_P,						"stdev_p");
DECLARE_CONST_STRING(FIELD_VARIANCE,					"variance");
DECLARE_CONST_STRING(FIELD_VARIANCE_P,					"variance_p");

CDatum CDatum::MathAbs () const

//	MathAbs
//
//	Return the absolute value.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CDatum();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return *this;

				default:
					ASSERT(false);
					return *this;
				}
			}

		case TYPE_INT32:
			return abs(DecodeInt32(m_dwData));

		case TYPE_ENUM:
			return abs(DecodeEnumValue(m_dwData));

		case TYPE_STRING:
			{
			CNumberValue X(*this);
			X.Abs();
			return X.GetDatum();
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathAbs();

		case TYPE_ROW_REF:
			return CDatum::CreateNaN();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return *this;

		default:
			return abs(DecodeDouble(m_dwData));
		}
	}

void CDatum::MathAccumulateStats (SStatsCtx& Stats, double rValue)

//	MathAccumulateStats
//
//	Accumulates a double value.

	{
	//	Check to see if rValue is NaN. If so, then we don't accumulate.

	if (!std::isfinite(rValue))
		{
		Stats.iNaNCount++;
		return;
		}

	Stats.iCount++;

	double rDelta = rValue - Stats.rMean;
	Stats.rMean += rDelta / Stats.iCount;
	double rDelta2 = rValue - Stats.rMean;
	Stats.M2 += rDelta * rDelta2;

	if (rValue > Stats.rMax)
		Stats.rMax = rValue;

	if (rValue < Stats.rMin)
		Stats.rMin = rValue;
	}

void CDatum::MathAccumulateStats (SStatsCtx& Stats) const

//	MathAccumulateStats
//
//	Accumulate stats.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			Stats.iNullCount++;
			break;

		case TYPE_CONSTANTS:
			Stats.iNaNCount++;
			break;

		case TYPE_INT32:
			MathAccumulateStats(Stats, (double)DecodeInt32(m_dwData));
			break;

		case TYPE_ENUM:
			MathAccumulateStats(Stats, (double)DecodeEnumValue(m_dwData));
			break;

		case TYPE_STRING:
			{
			CNumberValue X(*this);
			if (X.IsValidNumber())
				MathAccumulateStats(Stats, X.AsDouble());
			else
				Stats.iNaNCount++;
			break;
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathAccumulateStats(Stats);

		case TYPE_ROW_REF:
			Stats.iNaNCount++;
			break;

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			Stats.iNaNCount++;
			break;

		default:
			MathAccumulateStats(Stats, DecodeDouble(m_dwData));
			break;
		}
	}

CDatum CDatum::MathCeil () const

//	MathCeil
//
//	Return the ceiling value.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CDatum();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return *this;

				default:
					ASSERT(false);
					return *this;
				}
			}

		case TYPE_INT32:
			return *this;

		case TYPE_ENUM:
			return DecodeEnumValue(m_dwData);

		case TYPE_STRING:
			{
			CNumberValue X(*this);
			X.Ceil();
			return X.GetDatum();
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathCeil();

		case TYPE_ROW_REF:
			return CDatum::CreateNaN();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return *this;

		default:
			return ceil(DecodeDouble(m_dwData));
		}
	}

CDatum CDatum::MathFloor () const

//	MathFloor
//
//	Return the floor value.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CDatum();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return *this;

				default:
					ASSERT(false);
					return *this;
				}
			}

		case TYPE_INT32:
			return *this;

		case TYPE_ENUM:
			return DecodeEnumValue(m_dwData);

		case TYPE_STRING:
			{
			CNumberValue X(*this);
			X.Floor();
			return X.GetDatum();
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathFloor();

		case TYPE_ROW_REF:
			return CDatum::CreateNaN();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return *this;

		default:
			return floor(DecodeDouble(m_dwData));
		}
	}

CDatum CDatum::MathStdDev () const

//	MathStdDev
//
//	Returns the standard deviation.

	{
	SStatsCtx Stats;
	MathAccumulateStats(Stats);

	if (Stats.iCount == 0)
		return CDatum();

	else if (Stats.iCount >= 2)
		{
		//	Variance (sample)
		double rVariance = Stats.M2 / (Stats.iCount - 1);
		double rStdDev = sqrt(rVariance);
		return CDatum(rStdDev);
		}

	else
		return CDatum(0.0);
	}

CDatum CDatum::MathStdError () const

//	MathStdError
//
//	Returns the standard error.

	{
	SStatsCtx Stats;
	MathAccumulateStats(Stats);
	if (Stats.iCount == 0)
		return CDatum();

	else if (Stats.iCount >= 2)
		{
		//	Variance (sample)
		double rVariance = Stats.M2 / (Stats.iCount - 1);
		double rStdDev = sqrt(rVariance);
		double rStdError = rStdDev / sqrt((double)Stats.iCount);
		return CDatum(rStdError);
		}

	else
		return CDatum(0.0);
	}

CDatum CDatum::MathRound () const

//	MathRound
//
//	Return the rounded value.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CDatum();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return *this;

				default:
					ASSERT(false);
					return *this;
				}
			}

		case TYPE_INT32:
			return *this;

		case TYPE_ENUM:
			return CDatum(DecodeEnumValue(m_dwData));

		case TYPE_STRING:
			{
			CNumberValue X(*this);
			X.Round();
			return X.GetDatum();
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathRound();

		case TYPE_ROW_REF:
			return CDatum::CreateNaN();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return *this;

		default:
			{
			double rValue = round(DecodeDouble(m_dwData));
			if (rValue <= INT_MAX && rValue >= INT_MIN)
				return CDatum((int)rValue);
			else
				return CDatum(CIPInteger(rValue));
			}
		}
	}

CDatum CDatum::MathSign () const

//	MathRound
//
//	Return the rounded value.

	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return CDatum();

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
					return CDatum(0);

				case VALUE_TRUE:
					return CDatum(1);

				default:
					ASSERT(false);
					return *this;
				}
			}

		case TYPE_INT32:
			return CDatum(Sign(DecodeInt32(m_dwData)));

		case TYPE_ENUM:
			return CDatum(Sign(DecodeEnumValue(m_dwData)));

		case TYPE_STRING:
			{
			CNumberValue X(*this);
			X.Sign();
			return X.GetDatum();
			}

		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathSign();

		case TYPE_ROW_REF:
			return CDatum::CreateNaN();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return *this;

		default:
			{
			double rValue = Sign(DecodeDouble(m_dwData));
			return CDatum((int)rValue);
			}
		}
	}

CDatum CDatum::MathStats () const

//	MathStats
//
//	Returns a struct with the following fields:
//
//	count: Number of elements (excluding null and NaN)
//	average: Average value
//	variance: Variance
//	variance_p: Variance (population)
//	stdev: Standard deviation (sample)
//	stdev_p: Standard deviation (population)
//	max: Maximum value
//	min: Minimum value
//	range: Range (max - min)
//	nulls: Count of nulls found
//	nans: Count of nans found

	{
	SStatsCtx Stats;
	MathAccumulateStats(Stats);

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_COUNT, CDatum(Stats.iCount));
	dResult.SetElement(FIELD_NULLS, CDatum(Stats.iNullCount));
	dResult.SetElement(FIELD_NANS, CDatum(Stats.iNaNCount));

	if (Stats.iCount == 0)
		{
		dResult.SetElement(FIELD_AVERAGE, CDatum());
		dResult.SetElement(FIELD_VARIANCE, CDatum());
		dResult.SetElement(FIELD_VARIANCE_P, CDatum());
		dResult.SetElement(FIELD_VARIANCE, CDatum());
		dResult.SetElement(FIELD_STDEV, CDatum());
		dResult.SetElement(FIELD_STDEV_P, CDatum());
		dResult.SetElement(FIELD_MAX, CDatum());
		dResult.SetElement(FIELD_MIN, CDatum());
		dResult.SetElement(FIELD_RANGE, CDatum());

		return dResult;
		}
	else
		{
		if (Stats.iCount >= 2)
			{
			//	Variance (sample)
			double rVariance = Stats.M2 / (Stats.iCount - 1);
			double rStdDev = sqrt(rVariance);

			//	Variance (population)
			double rVarianceP = Stats.M2 / Stats.iCount;
			double rStdDevP = sqrt(rVarianceP);
			
			dResult.SetElement(FIELD_VARIANCE, CDatum(rVariance));
			dResult.SetElement(FIELD_STDEV, CDatum(rStdDev));
			dResult.SetElement(FIELD_VARIANCE_P, CDatum(rVarianceP));
			dResult.SetElement(FIELD_STDEV_P, CDatum(rStdDevP));
			}
		else
			{
			dResult.SetElement(FIELD_VARIANCE, CDatum(0.0));
			dResult.SetElement(FIELD_STDEV, CDatum(0.0));
			dResult.SetElement(FIELD_VARIANCE_P, CDatum(0.0));
			dResult.SetElement(FIELD_STDEV_P, CDatum(0.0));
			}

		dResult.SetElement(FIELD_AVERAGE, CDatum(Stats.rMean));
		dResult.SetElement(FIELD_MAX, CDatum(Stats.rMax));
		dResult.SetElement(FIELD_MIN, CDatum(Stats.rMin));
		dResult.SetElement(FIELD_RANGE, CDatum(Stats.rMax - Stats.rMin));
		}

	return dResult;
	}

CDatum CDatum::MathAddToElements (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathAddToElements(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathAddElementsTo (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathAddElementsTo(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathDivideElementsBy (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathDivideElementsBy(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathDivideByElements (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathDivideByElements(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathExpElementsTo (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathExpElementsTo(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathExpToElements (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathExpToElements(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathInvert () const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathInvert();

		default:
			return CDatum();
		}
	}

CDatum CDatum::MathMatMul (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathMatMul(dValue);

		default:
			return CDatum();
		}
	}

CDatum CDatum::MathModElementsBy (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathModElementsBy(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathModByElements (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathModByElements(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathMultiplyElements (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathMultiplyElements(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathNegateElements () const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathNegateElements();

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathSubtractFromElements (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathSubtractFromElements(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

CDatum CDatum::MathSubtractElementsFrom (CDatum dValue) const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_COMPLEX:
			return DecodeComplex(m_dwData).MathSubtractElementsFrom(dValue);

		default:
			return CDatum::CreateNaN();
		}
	}

template<class FUNC> CDatum CDatum::MathArrayOp () const
	{
	switch (DecodeType(m_dwData))
		{
		case TYPE_NULL:
			return *this;

		case TYPE_CONSTANTS:
			{
			switch (m_dwData)
				{
				case VALUE_FALSE:
				case VALUE_TRUE:
					return *this;

				default:
					ASSERT(false);
					return CreateNaN();
				}
			}

		case TYPE_INT32:
			return *this;

		case TYPE_ENUM:
			return *this;

		case TYPE_STRING:
			//	NOTE: The output of this passes through CNumberValue, which will
			//	handle the case where a string is a number.
			return *this;

		case TYPE_COMPLEX:
			return FUNC::Op(raw_GetComplex());

		case TYPE_ROW_REF:
			return CreateNaN();

		case TYPE_NAN:
		case TYPE_INFINITY_N:
		case TYPE_INFINITY_P:
			return *this;

		default:
			return *this;
		}
	}

CDatum IComplexDatum::MathAverage () const

//	MathAverage
//
//	Default implementation.
//
//	NOTE: Vector implementation (e.g., CAEONVectorInt32) override this method
//	and have a more efficient implementation.

	{
	CDatum dCollection(CDatum::raw_AsComplex(this));
	if (dCollection.IsNil())
		return CDatum();

	CNumberValue Total;
	int iCount = 0;

	bool bSuccess = dCollection.EnumElements(CDatum::FLAG_RECURSIVE, [&Total, &iCount](CDatum dValue)
		{
		if (!dValue.CanSum())
			return false;
		else
			{
			iCount++;

			Total.Add(dValue);
			if (!Total.IsValidNumber())
				return false;
			}

		return true;
		});

	if (!bSuccess)
		return CDatum::CreateNaN();

	if (iCount == 0)
		return CDatum();

	if (!Total.Divide(iCount))
		return CDatum::CreateNaN();

	return Total.GetDatum();
	}

CDatum IComplexDatum::MathSum () const

//	MathSum
//
//	Default implementation.
//
//	NOTE: Vector implementation (e.g., CAEONVectorInt32) override this method
//	and have a more efficient implementation.

	{
	CDatum dCollection(CDatum::raw_AsComplex(this));
	if (dCollection.IsNil())
		return CDatum();

	CNumberValue Result;

	bool bSuccess = dCollection.EnumElements(CDatum::FLAG_RECURSIVE, [&Result](CDatum dValue)
		{
		if (!dValue.CanSum())
			return false;
		else
			{
			Result.Add(dValue);
			if (!Result.IsValidNumber())
				return false;
			}

		return true;
		});

	if (!bSuccess)
		return CDatum::CreateNaN();

	return Result.GetDatum();
	}

CDatum IComplexDatum::MathAddToElements (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Add(GetElement(i), dValue));

	return dResult;
	}

CDatum IComplexDatum::MathAddElementsTo (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Add(dValue, GetElement(i)));

	return dResult;
	}

CDatum IComplexDatum::MathDivideElementsBy (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Divide(GetElement(i), dValue));

	return dResult;
	}

CDatum IComplexDatum::MathDivideByElements (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Divide(dValue, GetElement(i)));

	return dResult;
	}

CDatum IComplexDatum::MathExpElementsTo (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Power(GetElement(i), dValue));

	return dResult;
	}

CDatum IComplexDatum::MathExpToElements (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Power(dValue, GetElement(i)));

	return dResult;
	}

CDatum IComplexDatum::MathModElementsBy (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Mod(GetElement(i), dValue));

	return dResult;
	}

CDatum IComplexDatum::MathModByElements (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Mod(dValue, GetElement(i)));

	return dResult;
	}

CDatum IComplexDatum::MathMultiplyElements (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Multiply(GetElement(i), dValue));

	return dResult;
	}

CDatum IComplexDatum::MathNegateElements () const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Negate(GetElement(i)));

	return dResult;
	}

CDatum IComplexDatum::MathSubtractFromElements (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Subtract(GetElement(i), dValue));

	return dResult;
	}

CDatum IComplexDatum::MathSubtractElementsFrom (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.Append(CAEONOp::Subtract(dValue, GetElement(i)));

	return dResult;
	}

class MATH_AVERAGE { public: static CDatum Op (IComplexDatum *pDatum) { return pDatum->MathAverage(); }	};
class MATH_MAX { public: static CDatum Op (IComplexDatum *pDatum) { return pDatum->MathMax(); }	};
class MATH_MEDIAN { public: static CDatum Op (IComplexDatum *pDatum) { return pDatum->MathMedian(); }	};
class MATH_MIN { public: static CDatum Op (IComplexDatum *pDatum) { return pDatum->MathMin(); }	};
class MATH_SUM { public: static CDatum Op (IComplexDatum *pDatum) { return pDatum->MathSum(); }	};

CDatum CDatum::MathAverage () const	{ return MathArrayOp<MATH_AVERAGE>(); }
CDatum CDatum::MathMax () const	{ return MathArrayOp<MATH_MAX>(); }
CDatum CDatum::MathMedian () const { return MathArrayOp<MATH_MEDIAN>(); }
CDatum CDatum::MathMin () const	{ return MathArrayOp<MATH_MIN>(); }
CDatum CDatum::MathSum () const	{ return MathArrayOp<MATH_SUM>(); }
