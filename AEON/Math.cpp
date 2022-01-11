//	Math.cpp
//
//	Math functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

const int CNumberValue::MAX_BASE_FOR_EXP[MAX_EXP_FOR_INT32 + 1] = { 0, 0, 46340, 1290, 215, 73, 35, 21, 14, 10 };

//	CNumberValue ---------------------------------------------------------------

CNumberValue::CNumberValue (CDatum dValue) :
		m_bUpconverted(false),
		m_bNotANumber(false)

//	CNumberValue
//
//	CNumberValue constructor

	{
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
			SetInteger(GetInteger() + Src.GetInteger());
			break;

		case CDatum::typeInteger64:
			SetInteger64(GetInteger64() + Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			SetDouble(GetDouble() + Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			SetIPInteger(GetIPInteger() + Src.GetIPInteger());
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

		case CDatum::typeDouble:
		case CDatum::typeIntegerIP:
			ASSERT(false);
			break;

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

bool CNumberValue::Divide (CDatum dValue)

//	Divide
//
//	Divides by dValue

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
			if (Src.GetIPInteger().IsZero())
				return false;

			SetIPInteger(GetIPInteger() / Src.GetIPInteger());
			break;

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

		default:
			return CDatum();
		}
	}

void CNumberValue::Max (CDatum dValue)

//	Max
//
//	Keeps the maximum of dValue or current value.

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

		default:
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

		default:
			return false;
		}
	}

void CNumberValue::Min (CDatum dValue)

//	Min
//
//	Keeps the minimum of dValue or current value.

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
			SetInteger(GetInteger() - Src.GetInteger());
			break;

		case CDatum::typeInteger64:
			SetInteger64(GetInteger64() - Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			SetDouble(GetDouble() - Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			SetIPInteger(GetIPInteger() - Src.GetIPInteger());
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
	//	If either is nan, then we've got nan.

	if (m_iType == CDatum::typeNaN || Src.m_iType == CDatum::typeNaN)
		{
		Src.SetNaN();
		SetNaN();
		}

	//	We always upconvert 64-bit ints to IP.

	else if (m_iType == CDatum::typeInteger64 || Src.m_iType == CDatum::typeInteger64)
		{
		Src.ConvertToIPInteger();
		ConvertToIPInteger();
		}

	//	If already the same type we don't have to do anything.

	else if (m_iType == Src.m_iType)
		return;

	//	If one is IPInteger then convert the other

	else if (m_iType == CDatum::typeIntegerIP)
		Src.ConvertToIPInteger();
	else if (Src.m_iType == CDatum::typeIntegerIP)
		ConvertToIPInteger();

	//	If one is double then convert the other

	else if (m_iType == CDatum::typeDouble)
		Src.ConvertToDouble();
	else if (Src.m_iType == CDatum::typeDouble)
		ConvertToDouble();

	else
		//	Should never get here
		;
	}

