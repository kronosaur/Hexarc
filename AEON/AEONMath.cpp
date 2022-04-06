//	AEONMath.cpp
//
//	CDatum class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

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

				case AEON_NUMBER_INTEGER:
					return CDatum(abs((int)HIDWORD(m_dwData)));

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

				case AEON_NUMBER_INTEGER:
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

	bool bSuccess = dCollection.EnumElements([&Total, &iCount](CDatum dValue)
		{
		if (dValue.IsNil())
			return true;
		else if (dValue.IsNaN())
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

	bool bSuccess = dCollection.EnumElements([&Result](CDatum dValue)
		{
		if (dValue.IsNil())
			return true;
		else if (dValue.IsNaN())
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
