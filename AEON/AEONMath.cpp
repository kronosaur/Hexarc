//	AEONMath.cpp
//
//	CDatum class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void CDatum::AccumulateNumericVectors (CDatum &dResult, CDatum dValue)

//	AccumulateNumericVectors
//
//	Generates a numeric vector in dResult by concatenating it with the value.
//	Null values are ignored. NaN poisons the result. Callers guarantee that 
//	dValue is a numeric vector.
//
//	The result is a numeric vector with homogeneous types (or null or NaN).

	{
	if (dResult.IsNil())
		dResult = dValue;
	else if (dValue.IsNil())
		{ }
	else if (dResult.IsNaN())
		{ }
	else if (dValue.IsNaN())
		dResult = dValue;
	else if (dValue.IsArray())
		{
		for (int i = 0; i < dValue.GetCount(); i++)
			AccumulateNumericVectors(dResult, dValue.GetElement(i));
		}
	else
		{
		//	Turn into a vector.

		if (!dResult.IsArray())
			{
			switch (dResult.GetBasicType())
				{
				case CDatum::typeInteger32:
				case CDatum::typeIntegerIP:
				case CDatum::typeDouble:
					{
					CDatum dNewVector = CDatum::VectorOf(dResult.GetBasicType());
					dNewVector.Append(dResult);
					dResult = dNewVector;
					break;
					}

				default:
					throw CException(errFail);
				}
			}

		const IDatatype& ResultType = dResult.GetDatatype();
		DWORD dwResultType = ResultType.GetCoreType();

		switch (dValue.GetBasicType())
			{
			case CDatum::typeInteger32:
				//	All vector types are compatible with integer
				break;

			case CDatum::typeIntegerIP:
				if (dwResultType != IDatatype::ARRAY_FLOAT_64
						&& dwResultType != IDatatype::ARRAY)
					dResult = CDatum::VectorOf(CDatum::typeIntegerIP, dResult);
				break;

			case CDatum::typeDouble:
				if (dwResultType != IDatatype::ARRAY_FLOAT_64)
					dResult = CDatum::VectorOf(CDatum::typeDouble, dResult);
				break;

			default:
				throw CException(errFail);
			}

		dResult.Append(dValue);
		}
	}

CDatum CDatum::AsNumericVector () const

//	AsNumericVector
//
//	Converts this datum to either a single numeric value, or a vector of 
//	homogeneous numeric values. If this is not a number (or if any component is
//	not a number, then we return NaN). Null values are ignored and removed.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return CDatum();
			else
				{
				CNumberValue X(*this);
				if (X.IsValidNumber())
					return X.GetDatum();
				else
					return CreateNaN();
				}
			}

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					return CreateNaN();

				case AEON_NUMBER_28BIT:
				case AEON_NUMBER_32BIT:
				case AEON_NUMBER_DOUBLE:
					return *this;

				default:
					return CreateNaN();
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->AsNumericVector();

		default:
			return CreateNaN();
		}
	}

CDatum CDatum::MathAbs () const

//	MathAbs
//
//	Return the absolute value.

	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			{
			if (m_dwData == 0)
				return CDatum();
			else
				{
				CNumberValue X(*this);
				X.Abs();
				return X.GetDatum();
				}
			}

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
						case CONST_TRUE:
							return *this;

						default:
							return CreateNaN();
						}
					}

				case AEON_NUMBER_28BIT:
					return CDatum(abs((int)(m_dwData & AEON_NUMBER_MASK) >> 4));

				case AEON_NUMBER_32BIT:
					return CDatum(abs(CAEONStore::GetInt(GetNumberIndex())));

				case AEON_NUMBER_DOUBLE:
					return CDatum(abs(CAEONStore::GetDouble(GetNumberIndex())));

				default:
					return CreateNaN();
				}

		case AEON_TYPE_COMPLEX:
			return raw_GetComplex()->MathAbs();

		default:
			return CreateNaN();
		}
	}

template<class FUNC> CDatum CDatum::MathArrayOp () const
	{
	switch (m_dwData & AEON_TYPE_MASK)
		{
		case AEON_TYPE_STRING:
			//	NOTE: The output of this passes through CNumberValue, which will
			//	handle the case where a string is a number.

			return *this;

		case AEON_TYPE_NUMBER:
			switch (m_dwData & AEON_NUMBER_TYPE_MASK)
				{
				case AEON_NUMBER_CONSTANT:
					{
					switch (m_dwData)
						{
						case CONST_NAN:
						case CONST_TRUE:
							return *this;

						default:
							return CreateNaN();
						}
					}

				case AEON_NUMBER_28BIT:
				case AEON_NUMBER_32BIT:
				case AEON_NUMBER_DOUBLE:
					return *this;

				default:
					return CreateNaN();
				}

		case AEON_TYPE_COMPLEX:
			return FUNC::Op(raw_GetComplex());

		default:
			return CreateNaN();
		}
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
