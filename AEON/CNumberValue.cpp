//	CNumberValue.cpp
//
//	CNumberValue class
//	Copyright (c) 2011 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const int CNumberValue::MAX_BASE_FOR_EXP[MAX_EXP_FOR_INT32 + 1] = 
	{
		0,					//	x^0
		0,					//	x^1
		46340,				//	x^2
		1290,				//	x^3
		215,				//	x^4
		73,					//	x^5
		35,					//	x^6
		21,					//	x^7
		14, 				//	x^8
		10, 				//	x^9
		8, 					//	x^10
		7, 					//	x^11
		5, 					//	x^12
		5, 					//	x^13
		4, 					//	x^14
		4, 					//	x^15
		3, 					//	x^16
		3, 					//	x^17
		3, 					//	x^18
		3, 					//	x^19
		2, 					//	x^20
		2, 					//	x^21
		2, 					//	x^22
		2, 					//	x^23
		2, 					//	x^24
		2, 					//	x^25
		2, 					//	x^26
		2, 					//	x^27
		2, 					//	x^28
		2, 					//	x^29
		2, 					//	x^30
	};

//	CNumberValue ---------------------------------------------------------------

void CNumberValue::Abs ()

//	Abs
//
//	Take the absolute value.

	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			SetInteger(abs(GetInteger()));
			break;

		case CDatum::typeDouble:
			SetDouble(abs(GetDouble()));
			break;

		case CDatum::typeIntegerIP:
			{
			const CIPInteger &X = GetIPInteger();
			if (X.IsNegative())
				SetIPInteger(-X);
			break;
			}

		case CDatum::typeTimeSpan:
			{
			const CTimeSpan &X = GetTimeSpan();
			if (X.IsNegative())
				SetTimeSpan(CTimeSpan(X, false));
			break;
			}

		case CDatum::typeNaN:
			SetNaN();
			break;

		default:
			break;
		}
	}

void CNumberValue::Add (CDatum dValue)

//	Add
//
//	Adds the value

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeInteger32:
			{
			LONGLONG iResult = (LONGLONG)GetInteger() + (LONGLONG)Src.GetInteger();
			if (iResult >= INT_MIN && iResult <= INT_MAX)
				SetInteger((int)iResult);
			else
				SetIPInteger(iResult);

			break;
			}

		case CDatum::typeInteger64:
			SetInteger64(GetInteger64() + Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			SetDouble(GetDouble() + Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			SetIPInteger(GetIPInteger() + Src.GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			SetTimeSpan(CTimeSpan::Add(GetTimeSpan(), Src.GetTimeSpan()));
			break;

		case CDatum::typeNaN:
			SetNaN();
			break;

		default:
			SetInteger(0);
			break;
		}
	}

double CNumberValue::AsDouble () const

//	AsDouble
//
//	Returns a double (or nan if we can't convert).

	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			return (double)GetInteger();

		case CDatum::typeInteger64:
			return (double)GetInteger64();

		case CDatum::typeDouble:
			return GetDouble();

		default:
			return nan("");
		}
	}

CIPInteger CNumberValue::AsIPInteger () const

//	AsIPInteger
//
//	Returns as an IP integer.

	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			return CIPInteger(GetInteger());

		case CDatum::typeInteger64:
			return CIPInteger(GetInteger64());

		case CDatum::typeIntegerIP:
			return GetIPInteger();

		default:
			return CIPInteger();
		}
	}

int CNumberValue::Compare (const CNumberValue &Value) const
	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			{
			switch (Value.m_iType)
				{
				case CDatum::typeInteger32:
					return KeyCompare(GetInteger(), Value.GetInteger());

				case CDatum::typeInteger64:
					return KeyCompare((DWORDLONG)GetInteger(), Value.GetInteger64());

				case CDatum::typeDouble:
					return KeyCompare((double)GetInteger(), Value.GetDouble());

				case CDatum::typeIntegerIP:
					return KeyCompare(CIPInteger(GetInteger()), Value.GetIPInteger());

				default:
					return 1;
				}
			}

		case CDatum::typeInteger64:
			{
			switch (Value.m_iType)
				{
				case CDatum::typeInteger32:
					return KeyCompare(GetInteger64(), (DWORDLONG)Value.GetInteger());

				case CDatum::typeInteger64:
					return KeyCompare(GetInteger64(), Value.GetInteger64());

				case CDatum::typeDouble:
					return KeyCompare((double)GetInteger64(), Value.GetDouble());

				case CDatum::typeIntegerIP:
					return KeyCompare(CIPInteger(GetInteger64()), Value.GetIPInteger());

				default:
					return 1;
				}
			}

		case CDatum::typeDouble:
			{
			switch (Value.m_iType)
				{
				case CDatum::typeInteger32:
					return KeyCompare(GetDouble(), (double)Value.GetInteger());

				case CDatum::typeInteger64:
					return KeyCompare(GetDouble(), (double)Value.GetInteger64());

				case CDatum::typeDouble:
					return KeyCompare(GetDouble(), Value.GetDouble());

				case CDatum::typeIntegerIP:
					return KeyCompare(CIPInteger(GetDouble()), Value.GetIPInteger());

				default:
					return 1;
				}
			}

		case CDatum::typeIntegerIP:
			{
			switch (Value.m_iType)
				{
				case CDatum::typeInteger32:
					return KeyCompare(GetIPInteger(), CIPInteger(Value.GetInteger()));

				case CDatum::typeInteger64:
					return KeyCompare(GetIPInteger(), CIPInteger(Value.GetInteger64()));

				case CDatum::typeDouble:
					return KeyCompare(GetIPInteger(), CIPInteger(Value.GetDouble()));

				case CDatum::typeIntegerIP:
					return KeyCompare(GetIPInteger(), Value.GetIPInteger());

				default:
					return 1;
				}
			}

		case CDatum::typeTimeSpan:
			{
			switch (Value.m_iType)
				{
				case CDatum::typeTimeSpan:
					return KeyCompare(GetTimeSpan(), Value.GetTimeSpan());

				default:
					return 1;
				}
			}

		default:
			return -1;
		}
	}

void CNumberValue::ConvertToDouble (void)

//	ConvertToDouble
//
//	Upconverts to double

	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			m_rValue = (double)GetInteger();
			break;

		case CDatum::typeInteger64:
			m_rValue = (double)GetInteger64();
			break;

		case CDatum::typeIntegerIP:
			m_rValue = GetIPInteger().AsDouble();
			break;

		case CDatum::typeDouble:
			return;

		default:
			m_rValue = 0.0;
			break;
		}

	//	Done

	m_iType = CDatum::typeDouble;
	m_bUpconverted = true;
	}

void CNumberValue::ConvertToIPInteger (void)

//	ConvertToIPInteger
//
//	Upconverts to IPInteger

	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			m_ipValue = CIPInteger(GetInteger());
			break;

		case CDatum::typeInteger64:
			m_ipValue = CIPInteger(GetInteger64());
			break;

		case CDatum::typeDouble:
			//	LATER: Do a better job
			m_ipValue = CIPInteger((int)m_rValue);
			break;

		case CDatum::typeIntegerIP:
			return;

		default:
			break;
		}

	//	Done

	m_iType = CDatum::typeIntegerIP;
	m_pValue = &m_ipValue;
	m_bUpconverted = true;
	}

CDatum CNumberValue::Divide (const CIPInteger &Dividend, const CIPInteger &Divisor)

//	Divide
//
//	Divide and return either an IPInteger or a double.

	{
	CIPInteger Quotient;
	CIPInteger Remainder;
	if (!Dividend.DivideMod(Divisor, Quotient, Remainder))
		return CDatum::CreateNaN();

	if (Remainder.IsZero())
		return CDatum(Quotient);
	else
		{
		double rQuotient = Quotient.AsDouble();
		double rRemainder = Remainder.AsDouble() / Divisor.AsDouble();
		return CDatum(rQuotient + rRemainder);
		}
	}

bool CNumberValue::Divide (CDatum dValue)

//	Divide
//
//	Divides by dValue

	{
	CNumberValue Src(dValue);

	//	NOTE: If we are a timespan, we don't bother upconverting; instead, we
	//	handle all cases.

	if (m_iType != CDatum::typeTimeSpan)
		Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			int iDividend = GetInteger();
			int iDivisor = Src.GetInteger();
			if (iDivisor == 0)
				return false;

			if ((iDividend % iDivisor) == 0)
				SetInteger(iDividend / iDivisor);
			else
				{
				ConvertToDouble();
				SetDouble((double)iDividend / (double)iDivisor);
				}
			break;
			}

		case CDatum::typeInteger64:
			{
			DWORDLONG dwDividend = GetInteger64();
			DWORDLONG dwDivisor = Src.GetInteger64();
			if (dwDivisor == 0)
				return false;

			if ((dwDividend % dwDivisor) == 0)
				SetInteger64(dwDividend / dwDivisor);
			else
				{
				ConvertToDouble();
				SetDouble((double)dwDividend / (double)dwDivisor);
				}
			break;
			}

		case CDatum::typeDouble:
			SetDouble(GetDouble() / Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			{
			CIPInteger Quotient;
			CIPInteger Remainder;
			if (!GetIPInteger().DivideMod(Src.GetIPInteger(), Quotient, Remainder))
				return false;

			if (Remainder.IsZero())
				SetIPInteger(Quotient);
			else
				{
				double rQuotient = Quotient.AsDouble();
				double rRemainder = Remainder.AsDouble() / Src.GetIPInteger().AsDouble();
				SetDouble(rQuotient + rRemainder);
				}

			break;
			}

		case CDatum::typeTimeSpan:
			{
			bool bNegative = GetTimeSpan().IsNegative();

			switch (Src.m_iType)
				{
				case CDatum::typeInteger32:
					{
					int iDivisor = Src.GetInteger();
					if (iDivisor == 0)
						{
						SetNaN();
						break;
						}

					if (iDivisor < 0)
						{
						bNegative = !bNegative;
						iDivisor = -iDivisor;
						}

					DWORDLONG dwTime = GetTimeSpan().Milliseconds64() / (DWORDLONG)iDivisor;
					SetTimeSpan(CTimeSpan(dwTime, bNegative));
					break;
					}

				case CDatum::typeInteger64:
					{
					DWORDLONG dwDivisor = Src.GetInteger64();
					if (dwDivisor == 0)
						{
						SetNaN();
						break;
						}

					DWORDLONG dwTime = GetTimeSpan().Milliseconds64() / dwDivisor;
					SetTimeSpan(CTimeSpan(dwTime, bNegative));
					break;
					}

				case CDatum::typeIntegerIP:
					{
					CIPInteger Time(GetTimeSpan().Milliseconds64());
					CIPInteger Divisor = Src.GetIPInteger();
					if (Divisor.IsZero())
						{
						SetNaN();
						break;
						}

					if (Divisor.IsNegative())
						{
						bNegative = !bNegative;
						Divisor = -Divisor;
						}

					CIPInteger Result = Time / Divisor;
					SetTimeSpan(CTimeSpan(Result.AsInteger64Unsigned(), bNegative));
					break;
					}

				case CDatum::typeDouble:
					{
					double rDivisor = Src.GetInteger();
					if (rDivisor == 0.0)
						{
						SetNaN();
						break;
						}

					if (rDivisor < 0.0)
						{
						bNegative = !bNegative;
						rDivisor = -rDivisor;
						}

					DWORDLONG dwTime = (DWORDLONG)((double)GetTimeSpan().Milliseconds64() / rDivisor);
					SetTimeSpan(CTimeSpan(dwTime, bNegative));
					break;
					}

				default:
					SetNaN();
					break;
				}

			break;
			}

		default:
			SetInteger(0);
			break;
		}

	return true;
	}

bool CNumberValue::DivideReversed (CDatum dValue)

//	Divide
//
//	Divides dValue by this value.

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			int iDividend = Src.GetInteger();
			int iDivisor = GetInteger();
			if (iDivisor == 0)
				return false;

			if ((iDividend % iDivisor) == 0)
				SetInteger(iDividend / iDivisor);
			else
				{
				ConvertToDouble();
				SetDouble((double)iDividend / (double)iDivisor);
				}
			break;
			}

		case CDatum::typeInteger64:
			{
			DWORDLONG dwDividend = Src.GetInteger64();
			DWORDLONG dwDivisor = GetInteger64();
			if (dwDivisor == 0)
				return false;

			if ((dwDividend % dwDivisor) == 0)
				SetInteger64(dwDividend / dwDivisor);
			else
				{
				ConvertToDouble();
				SetDouble((double)dwDividend / (double)dwDivisor);
				}
			break;
			}

		case CDatum::typeDouble:
			SetDouble(Src.GetDouble() / GetDouble());
			break;

		case CDatum::typeIntegerIP:
			if (GetIPInteger().IsZero())
				return false;

			SetIPInteger(Src.GetIPInteger() / GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			SetNaN();
			break;

		default:
			SetInteger(0);
			break;
		}

	return true;
	}

CDatum CNumberValue::GetDatum (void)

//	CNumberValue
//
//	Gets the datum back

	{
	if (!m_bUpconverted)
		return m_dOriginalValue;

	switch (m_iType)
		{
		case CDatum::typeNaN:
			return CDatum::CreateNaN();

		case CDatum::typeInteger32:
			return CDatum(GetInteger());

		case CDatum::typeInteger64:
			return CDatum(GetInteger64());

		case CDatum::typeDouble:
			return CDatum(m_rValue);

		case CDatum::typeIntegerIP:
			{
			CDatum dValue;
			CDatum::CreateIPInteger(m_ipValue, &dValue);
			return dValue;
			}

		case CDatum::typeTimeSpan:
			return CDatum(GetTimeSpan());

		default:
			return CDatum();
		}
	}

void CNumberValue::InitFrom (CDatum dValue)

//	InitFrom
//
//	Iniitializes from the given value.

	{
	m_bUpconverted = false;
	m_bNotANumber = false;

	//	Get the number type. If necessary, this code will convert from a string
	//	to an appropriate number.

	m_iType = dValue.GetNumberType(NULL, &m_dOriginalValue);

	switch (m_iType)
		{
		case CDatum::typeInteger32:
			m_pValue = (void *)(DWORD_PTR)(int)dValue;
			break;

		case CDatum::typeInteger64:
			m_ilValue = (DWORDLONG)dValue;
			break;

		case CDatum::typeDouble:
			m_rValue = (double)dValue;
			break;

		case CDatum::typeIntegerIP:
			m_pValue = (void *)&((const CIPInteger &)m_dOriginalValue);
			break;

		case CDatum::typeTimeSpan:
			m_pValue = (void *)&((const CTimeSpan &)m_dOriginalValue);
			break;

		case CDatum::typeNaN:
			m_bNotANumber = true;
			break;

		default:
			m_dOriginalValue = CDatum((int)0);
			m_iType = CDatum::typeInteger32;
			m_pValue = (void *)(DWORD_PTR)0;
			m_bNotANumber = true;
			break;
		}
	}

bool CNumberValue::IsNegative () const

//	IsNegative
//
//	Returns TRUE if negative.

	{
	switch (m_iType)
		{
		case CDatum::typeInteger32:
			return GetInteger() < 0;

		case CDatum::typeInteger64:
			return GetInteger64() < 0;

		case CDatum::typeDouble:
			return GetDouble() < 0.0;

		case CDatum::typeIntegerIP:
			return GetIPInteger().IsNegative();

		case CDatum::typeTimeSpan:
			return GetTimeSpan().IsNegative();

		default:
			return false;
		}
	}

void CNumberValue::Max (CDatum dValue)

//	Max
//
//	Keeps the maximum of dValue or current value.
//	NOTE: We expect dValue to be a scalar; the caller is responsible for 
//	reducing arrays, etc.

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			if (GetInteger() < Src.GetInteger())
				SetInteger(Src.GetInteger());
			break;

		case CDatum::typeInteger64:
			if (GetInteger64() < Src.GetInteger64())
				SetInteger64(Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			if (GetDouble() < Src.GetDouble())
				SetDouble(Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			if (GetIPInteger().Compare(Src.GetIPInteger()) == -1)
				SetIPInteger(Src.GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			if (GetTimeSpan().Compare(Src.GetTimeSpan()) == -1)
				SetTimeSpan(Src.GetTimeSpan());
			break;

		default:
			break;
		}
	}

void CNumberValue::Min (CDatum dValue)

//	Min
//
//	Keeps the minimum of dValue or current value.
//	NOTE: We expect dValue to be a scalar; the caller is responsible for 
//	reducing arrays, etc.

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			if (GetInteger() > Src.GetInteger())
				SetInteger(Src.GetInteger());
			break;

		case CDatum::typeInteger64:
			if (GetInteger64() > Src.GetInteger64())
				SetInteger64(Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			if (GetDouble() > Src.GetDouble())
				SetDouble(Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			if (GetIPInteger().Compare(Src.GetIPInteger()) == 1)
				SetIPInteger(Src.GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			if (GetTimeSpan().Compare(Src.GetTimeSpan()) == 1)
				SetTimeSpan(Src.GetTimeSpan());
			break;

		default:
			break;
		}
	}

bool CNumberValue::Mod (CDatum dValue)

//	Mod
//
//	Mod function

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			int iDividend = GetInteger();
			int iDivisor = Src.GetInteger();
			if (iDivisor == 0)
				return false;

			SetInteger(iDividend % iDivisor);
			break;
			}

		case CDatum::typeInteger64:
			{
			DWORDLONG dwDividend = GetInteger64();
			DWORDLONG dwDivisor = Src.GetInteger64();
			if (dwDivisor == 0)
				return false;

			SetInteger64(dwDividend % dwDivisor);
			break;
			}

		case CDatum::typeDouble:
			SetDouble(fmod(GetDouble(), Src.GetDouble()));
			break;

		case CDatum::typeIntegerIP:
			if (Src.GetIPInteger().IsZero())
				return false;

			SetIPInteger(GetIPInteger() % Src.GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			SetNaN();
			break;

		default:
			SetInteger(0);
			break;
		}

	return true;
	}

bool CNumberValue::ModClock (CDatum dValue)

//	ModClock
//
//	Mod function, with negative wrap like a clock (for angles).

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			int iDividend = GetInteger();
			int iDivisor = Src.GetInteger();
			if (iDivisor == 0)
				return false;

			int iResult = iDividend % iDivisor;
			if (iResult < 0)
				SetInteger(iDivisor + iResult);
			else
				SetInteger(iResult);
			break;
			}

		case CDatum::typeInteger64:
			{
			DWORDLONG dwDividend = GetInteger64();
			DWORDLONG dwDivisor = Src.GetInteger64();
			if (dwDivisor == 0)
				return false;

			SetInteger64(dwDividend % dwDivisor);
			break;
			}

		case CDatum::typeDouble:
			{
			double rResult = fmod(GetDouble(), Src.GetDouble());
			if (rResult < 0.0)
				SetDouble(Src.GetDouble() + rResult);
			else
				SetDouble(rResult);
			break;
			}

		case CDatum::typeIntegerIP:
			{
			auto &Dividend = GetIPInteger();
			auto &Divisor = Src.GetIPInteger();
			if (Divisor.IsZero())
				return false;

			CIPInteger Result = Dividend % Divisor;
			if (Result.IsNegative())
				SetIPInteger(Divisor + Result);
			else
				SetIPInteger(Result);
			break;
			}

		case CDatum::typeTimeSpan:
			SetNaN();
			break;

		default:
			SetInteger(0);
			break;
		}

	return true;
	}

void CNumberValue::Multiply (CDatum dValue)

//	Multiply
//
//	Multiplies the numbers

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			LONGLONG Result = (LONGLONG)GetInteger() * (LONGLONG)Src.GetInteger();
			if (Result <= INT_MAX && Result >= INT_MIN)
				SetInteger((int)Result);
			else
				SetIPInteger(CIPInteger(Result));
			break;
			}

		case CDatum::typeInteger64:
			{
			SetIPInteger(CIPInteger(GetInteger64()) * CIPInteger(Src.GetInteger64()));
			break;
			}

		case CDatum::typeDouble:
			//	LATER: Handle overflow?
			SetDouble(GetDouble() * Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			SetIPInteger(GetIPInteger() * Src.GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			SetNaN();
			break;

		default:
			SetInteger(0);
			break;
		}
	}

void CNumberValue::Power (CDatum dValue)

//	Power
//
//	Raise number to the given power.

	{
	CNumberValue Exp(dValue);
	Upconvert(Exp);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			int iExp = Exp.GetInteger();
			int iBase = GetInteger();

			if (iExp == 0)
				SetInteger(1);

			else if (iExp < 0)
				SetDouble(pow((double)iBase, (double)iExp));

			else if (iExp == 1)
				{ }

			else if (iExp <= MAX_EXP_FOR_INT32 && iBase <= MAX_BASE_FOR_EXP[iExp])
				{
				//	LATER: Do an integer algorithm.
				SetInteger((int)pow((double)iBase, (double)iExp));
				}
			else
				{
				CIPInteger IPBase(iBase);
				CIPInteger IPExp(iExp);
				SetIPInteger(IPBase.Power(IPExp));
				}

			break;
			}

		case CDatum::typeInteger64:
		case CDatum::typeIntegerIP:
			{
			CIPInteger IPBase(AsIPInteger());
			CIPInteger IPExp(Exp.AsIPInteger());
			SetIPInteger(IPBase.Power(IPExp));
			break;
			}

		case CDatum::typeDouble:
			SetDouble(pow(GetDouble(), Exp.GetDouble()));
			break;

		case CDatum::typeTimeSpan:
			SetNaN();
			break;

		default:
			SetNil();
			break;
		}
	}

void CNumberValue::Subtract (CDatum dValue)

//	Subtract
//
//	Subtract the value

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
		case CDatum::typeNaN:
			Src.SetNaN();
			SetNaN();
			break;

		case CDatum::typeInteger32:
			{
			LONGLONG iResult = (LONGLONG)GetInteger() - (LONGLONG)Src.GetInteger();
			if (iResult >= INT_MIN && iResult <= INT_MAX)
				SetInteger((int)iResult);
			else
				SetIPInteger(iResult);

			break;
			}

		case CDatum::typeInteger64:
			SetInteger64(GetInteger64() - Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			SetDouble(GetDouble() - Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			SetIPInteger(GetIPInteger() - Src.GetIPInteger());
			break;

		case CDatum::typeTimeSpan:
			SetTimeSpan(CTimeSpan::Subtract(GetTimeSpan(), Src.GetTimeSpan()));
			break;

		default:
			SetInteger(0);
			break;
		}
	}

void CNumberValue::Upconvert (CNumberValue &Src)

//	Upconvert
//
//	Upconverts either this value or the source to make sure that we can do
//	operations on a common type.

	{
	//	If we're null, then we just take on the source.

	if (m_iType == CDatum::typeNil)
		{
		switch (Src.m_iType)
			{
			case CDatum::typeInteger32:
				SetInteger(0);
				break;

			case CDatum::typeInteger64:
				Src.ConvertToIPInteger();
				SetIPInteger(0);
				break;

			case CDatum::typeIntegerIP:
				SetIPInteger(0);
				break;

			case CDatum::typeDouble:
				SetDouble(0.0);
				break;

			case CDatum::typeTimeSpan:
				SetTimeSpan(CTimeSpan());
				break;

			default:
				SetNaN();
				break;
			}
		}

	//	If either is nan, then we've got nan.

	else if (m_iType == CDatum::typeNaN || Src.m_iType == CDatum::typeNaN)
		{
		Src.SetNaN();
		SetNaN();
		}

	//	If already the same type we don't have to do anything.

	else if (m_iType == Src.m_iType)
		{
		//	We always upconvert 64-bit ints to IP.

		if (m_iType == CDatum::typeInteger64)
			{
			Src.ConvertToIPInteger();
			ConvertToIPInteger();
			}

		return;
		}

	//	If either value is a timespan (but not both) then we get nan.

	else if (m_iType == CDatum::typeTimeSpan || Src.m_iType == CDatum::typeTimeSpan)
		{
		Src.SetNaN();
		SetNaN();
		}

	//	If one is double then convert the other
	//
	//	NOTE: If we have an IPInteger and a double then we convert to double
	//	because otherwise we'd lose the decimal part.
	//
	//	LATER: We could at minimum check to see if the double is an integer, and
	//	in that case, stick to IPInteger.
	//
	//	LATER: Eventually we probably want arbitrary precision floats.

	else if (m_iType == CDatum::typeDouble)
		Src.ConvertToDouble();
	else if (Src.m_iType == CDatum::typeDouble)
		ConvertToDouble();

	//	If either is a 64-bit int, then always convert both.

	else if (m_iType == CDatum::typeInteger64 || Src.m_iType == CDatum::typeInteger64)
		{
		Src.ConvertToIPInteger();
		ConvertToIPInteger();
		}

	//	If one is IPInteger then convert the other

	else if (m_iType == CDatum::typeIntegerIP)
		Src.ConvertToIPInteger();
	else if (Src.m_iType == CDatum::typeIntegerIP)
		ConvertToIPInteger();

	else
		//	Should never get here
		;
	}
