//	Math.cpp
//
//	Math functions
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

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
			m_pValue = (void *)&((const CIPInteger &)dValue);
			break;

		default:
			m_dOriginalValue = CDatum((int)0);
			m_iType = CDatum::typeInteger32;
			m_pValue = (void *)(DWORD_PTR)0;
			m_bNotANumber = true;
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

		default:
			SetInteger(0);
			break;
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
			ASSERT(false);
			break;

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
			//	NOT YET IMPLEMENTED
			return false;

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
			//	NOT YET IMPLEMENTED
			return false;

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

void CNumberValue::Min (CDatum dValue)

//	Min
//
//	Keeps the minimum of dValue or current value.

	{
	CNumberValue Src(dValue);
	Upconvert(Src);

	switch (m_iType)
		{
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
			//	NOT YET IMPLEMENTED
			return false;

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
			//	NOT YET IMPLEMENTED
			return false;

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
		case CDatum::typeInteger32:
			//	LATER: Handle overflow
			SetInteger(GetInteger() * Src.GetInteger());
			break;

		case CDatum::typeInteger64:
			//	LATER: Handle overflow
			SetInteger64(GetInteger64() * Src.GetInteger64());
			break;

		case CDatum::typeDouble:
			SetDouble(GetDouble() * Src.GetDouble());
			break;

		case CDatum::typeIntegerIP:
			//	NOT YET IMPLEMENTED
			break;

		default:
			SetInteger(0);
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
	//	If already the same type we don't have to do anything.

	if (m_iType == Src.m_iType)
		return;

	//	If one is IPInteger then convert the other

	if (m_iType == CDatum::typeIntegerIP)
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

