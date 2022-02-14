//	CIPInteger.cpp
//
//	CIPInteger class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	Based in part on BigDigits multiple-precision arithmetic library
//	Version 2.2 originally written by David Ireland, copyright
//	(c) 2001-2008 D.I. Management Services Pty Limitied, all rights reserved.
//
//******************** COPYRIGHT AND LICENCE NOTICE **********************
//	This source code is part of the BIGDIGITS multiple-precision
//	arithmetic library Version 2.2 originally written by David Ireland,
//	copyright (c) 2001-8 D.I. Management Services Pty Limited, all rights
//	reserved. You are permitted to use compiled versions of this code at
//	no charge as part of your own executable files and to distribute
//	unlimited copies of such executable files for any purposes including
//	commercial ones provided you agree to these terms and conditions and 
//	keep the copyright notices intact in the source code and you ensure 
//	that the following characters remain in any object or executable files
//	you distribute AND clearly in any accompanying documentation:
//
//	"Contains BIGDIGITS multiple-precision arithmetic code originally
//	written by David Ireland, copyright (c) 2001-8 by D.I. Management
//	Services Pty Limited <www.di-mgt.com.au>, and is used with
//	permission."
//
//	David Ireland and DI Management Services Pty Limited make no
//	representations concerning either the merchantability of this software
//	or the suitability of this software for any particular purpose. It is
//	provided "as is" without express or implied warranty of any kind. Our
//	liability will be limited exclusively to the refund of the money you
//	paid us for the software, namely nothing. By using the software you
//	expressly agree to such a waiver. If you do not agree to the terms, do
//	not use the software.
//
//	Please forward any comments and bug reports to <www.di-mgt.com.au>.
//	The latest version of the source code can be downloaded from
//	<www.di-mgt.com.au/bigdigits.html>.
//
//	Last updated: 31 July 2008.
//*************** END OF COPYRIGHT AND LICENCE NOTICE ********************/

#include "stdafx.h"

#ifndef USE_BOOST_MULTIPRECISION

#include "BigDigits\bigd.h"
#include "BigDigits\bigdRand.h"

const DWORD STREAM_SIGNATURE_POSITIVE = 'IP1+';
const DWORD STREAM_SIGNATURE_NEGATIVE = 'IP1-';

CIPInteger NULL_IPINTEGER;
static CIPInteger MIN_INT32((LONGLONG)INT_MIN);
static CIPInteger MAX_INT32((LONGLONG)INT_MAX);

CIPInteger::CIPInteger (const CIPInteger &Src)

//	CIPInteger constructor

	{
	if (Src.m_Value)
		{
		m_Value = bdNew();
		bdSetEqual((BIGD)m_Value, (BIGD)Src.m_Value);
		m_bNegative = Src.m_bNegative;
		}
	}

CIPInteger::CIPInteger(CIPInteger&& Src) noexcept

//	CIPInteger constructor

	{
	m_Value = Src.m_Value;
	m_bNegative = Src.m_bNegative;

	Src.m_Value = NULL;
	}

CIPInteger::CIPInteger (int iSrc)

//	CIPInteger constructor

	{
	m_Value = bdNew();
	if (iSrc >= 0)
		{
		bdSetShort((BIGD)m_Value, iSrc);
		m_bNegative = false;
		}
	else
		{
		//	Since -INT_MIN > INT_MAX we need to do it a different way.

		if (iSrc == INT_MIN)
			{
			bdSetShort((BIGD)m_Value, INT_MAX);
			bdIncrement((BIGD)m_Value);
			}
		else
			bdSetShort((BIGD)m_Value, -iSrc);

		m_bNegative = true;
		}
	}

CIPInteger::CIPInteger (DWORDLONG ilSrc)

//	CIPInteger constructor

	{
	m_Value = bdNew();

	//	We reverse the bytes in ilSrc since we store the integers in
	//	big-endian order.

	BYTE BigEndian[sizeof(DWORDLONG)];
	utlMemReverse(&ilSrc, BigEndian, sizeof(DWORDLONG));

	//	Load

	bdConvFromOctets((BIGD)m_Value, BigEndian, sizeof(DWORDLONG));

	//	DWORDLONGs are unsigned

	m_bNegative = false;
	}

CIPInteger::CIPInteger (LONGLONG ilSrc)

//	CIPInteger constructor

	{
	m_Value = bdNew();

	if (ilSrc < 0)
		{
		DWORDLONG dwSource = -ilSrc;
		BYTE BigEndian[sizeof(DWORDLONG)];
		utlMemReverse(&dwSource, BigEndian, sizeof(DWORDLONG));

		bdConvFromOctets((BIGD)m_Value, BigEndian, sizeof(DWORDLONG));
		m_bNegative = true;
		}
	else
		{
		BYTE BigEndian[sizeof(DWORDLONG)];
		utlMemReverse(&ilSrc, BigEndian, sizeof(DWORDLONG));

		bdConvFromOctets((BIGD)m_Value, BigEndian, sizeof(DWORDLONG));
		m_bNegative = false;
		}
	}

CIPInteger::CIPInteger (double rSrc)

//	CIPInteger constructor

	{
	//	LATER: Need to convert a double to an IP.
	m_Value = bdNew();
	if (rSrc >= 0)
		{
		bdSetShort((BIGD)m_Value, (int)rSrc);
		m_bNegative = false;
		}
	else
		{
		bdSetShort((BIGD)m_Value, (int)-rSrc);
		m_bNegative = true;
		}
	}

CIPInteger::~CIPInteger (void)

//	CIPInteger destructor

	{
	if (m_Value)
		bdFree((BIGD *)&m_Value);
	}

CIPInteger &CIPInteger::operator= (const CIPInteger &Src)

//	CIPInteger operator =

	{
	if (!m_Value)
		m_Value = bdNew();

	bdSetEqual((BIGD)m_Value, (BIGD)Src.m_Value);
	m_bNegative = Src.m_bNegative;
	return *this;
	}

CIPInteger& CIPInteger::operator= (CIPInteger&& Src) noexcept

//	CIPInteger operator =

	{
	if (m_Value)
		bdFree((BIGD *)&m_Value);

	m_Value = Src.m_Value;
	m_bNegative = Src.m_bNegative;

	Src.m_Value = NULL;
	return *this;
	}

bool CIPInteger::operator== (const CIPInteger &Src) const

//	CIPInteger operator ==

	{
	if (m_Value && Src.m_Value)
		return ((bdCompare((BIGD)m_Value, (BIGD)Src.m_Value) == 0) && (m_bNegative == Src.m_bNegative));
	else
		return (m_Value == Src.m_Value);
	}

bool CIPInteger::operator!= (const CIPInteger &Src) const

//	CIPInteger operator !=

	{
	if (m_Value && Src.m_Value)
		return ((bdCompare((BIGD)m_Value, (BIGD)Src.m_Value) != 0) || (m_bNegative != Src.m_bNegative));
	else
		return (m_Value != Src.m_Value);
	}

CIPInteger &CIPInteger::operator+= (const CIPInteger &Src)

//	CIPInteger operator +=

	{
	if (!Src.m_Value)
		return *this;
	else if (!m_Value)
		m_Value = bdNew();

	if (this == &Src)
		{
		CIPInteger Result = *this + Src;
		*this = Result;
		}
	else if (m_bNegative == Src.m_bNegative)
		bdAdd_s((BIGD)m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
	else
		{
		int iCompare = bdCompare((BIGD)m_Value, (BIGD)Src.m_Value);
		if (iCompare == 1)
			bdSubtract_s((BIGD)m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
		else if (iCompare == -1)
			{
			bdSubtract_s((BIGD)m_Value, (BIGD)Src.m_Value, (BIGD)m_Value);
			m_bNegative = !m_bNegative;
			}
		else
			{
			bdSetZero((BIGD)m_Value);
			m_bNegative = false;
			}
		}

	return *this;
	}

CIPInteger CIPInteger::operator+ (const CIPInteger &Src) const

//	CIPInteger operator +

	{
	if (!Src.m_Value)
		return *this;
	else if (!m_Value)
		return Src;

	void *NewValue = bdNew();
	bool bNewNegative = m_bNegative;

	if (m_bNegative == Src.m_bNegative)
		bdAdd((BIGD)NewValue, (BIGD)m_Value, (BIGD)Src.m_Value);
	else
		{
		int iCompare = bdCompare((BIGD)m_Value, (BIGD)Src.m_Value);
		if (iCompare == 1)
			bdSubtract((BIGD)NewValue, (BIGD)m_Value, (BIGD)Src.m_Value);
		else if (iCompare == -1)
			{
			bdSubtract((BIGD)NewValue, (BIGD)Src.m_Value, (BIGD)m_Value);
			bNewNegative = !m_bNegative;
			}
		else
			{
			bdSetZero((BIGD)m_Value);
			bNewNegative = false;
			}
		}

	return CIPInteger(NewValue, bNewNegative);
	}

CIPInteger &CIPInteger::operator-= (const CIPInteger &Src)

//	CIPInteger operator -=

	{
	if (!Src.m_Value)
		return *this;
	else if (!m_Value)
		m_Value = bdNew();

	if (m_bNegative != Src.m_bNegative)
		bdAdd_s((BIGD)m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
	else
		{
		int iCompare = bdCompare((BIGD)m_Value, (BIGD)Src.m_Value);
		if (iCompare == 1)
			bdSubtract_s((BIGD)m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
		else if (iCompare == -1)
			{
			bdSubtract_s((BIGD)m_Value, (BIGD)Src.m_Value, (BIGD)m_Value);
			m_bNegative = !m_bNegative;
			}
		else
			{
			bdSetZero((BIGD)m_Value);
			m_bNegative = false;
			}
		}

	return *this;
	}

CIPInteger CIPInteger::operator - (void) const

//	CIPInteger operator -

	{
	if (!m_Value)
		return *this;

	void *NewValue = bdNew();
	bdSetEqual((BIGD)NewValue, (BIGD)m_Value);
	return CIPInteger(NewValue, !m_bNegative);
	}

CIPInteger CIPInteger::operator- (const CIPInteger &Src) const

//	CIPInteger operator -

	{
	if (!Src.m_Value)
		return *this;
	else if (!m_Value)
		return -Src;

	void *NewValue = bdNew();
	bool bNewNegative = m_bNegative;

	if (m_bNegative != Src.m_bNegative)
		bdAdd((BIGD)NewValue, (BIGD)m_Value, (BIGD)Src.m_Value);
	else
		{
		int iCompare = bdCompare((BIGD)m_Value, (BIGD)Src.m_Value);
		if (iCompare == 1)
			bdSubtract((BIGD)NewValue, (BIGD)m_Value, (BIGD)Src.m_Value);
		else if (iCompare == -1)
			{
			bdSubtract((BIGD)NewValue, (BIGD)Src.m_Value, (BIGD)m_Value);
			bNewNegative = !m_bNegative;
			}
		else
			{
			bdSetZero((BIGD)m_Value);
			bNewNegative = false;
			}
		}

	return CIPInteger(NewValue, bNewNegative);
	}

CIPInteger &CIPInteger::operator *= (const CIPInteger &Src)

//	CIPInteger operator *=

	{
	if (!Src.m_Value)
		{
		*this = CIPInteger(0);
		return *this;
		}
	else if (!m_Value)
		m_Value = bdNew();

	if (this == &Src)
		{
		CIPInteger Result = *this * Src;
		*this = Result;
		}
	else
		{
		bdMultiply_s((BIGD)m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
		m_bNegative = (m_bNegative != Src.m_bNegative);
		}

	return *this;
	}

CIPInteger CIPInteger::operator * (const CIPInteger &Src) const

//	CIPInteger operator *

	{
	if (!Src.m_Value)
		return CIPInteger(0);
	else if (!m_Value)
		return CIPInteger(0);

	void *NewValue = bdNew();
	bdMultiply_s((BIGD)NewValue, (BIGD)m_Value, (BIGD)Src.m_Value);
	bool bNewNegative = (m_bNegative != Src.m_bNegative);

	return CIPInteger(NewValue, bNewNegative);
	}

CIPInteger &CIPInteger::operator /= (const CIPInteger &Src)

//	CIPInteger operator /=

	{
	if (!Src.m_Value)
		{
		*this = CIPInteger(0);
		return *this;
		}
	else if (!m_Value)
		m_Value = bdNew();

	CIPInteger Remainder(bdNew(), false);

	bdDivide_s((BIGD)m_Value, (BIGD)Remainder.m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
	m_bNegative = (m_bNegative != Src.m_bNegative);

	return *this;
	}

CIPInteger CIPInteger::operator / (const CIPInteger &Src) const

//	CIPInteger operator /

	{
	if (!Src.m_Value)
		return CIPInteger(0);
	else if (!m_Value)
		return CIPInteger(0);

	CIPInteger Remainder(bdNew(), false);

	void *NewValue = bdNew();
	bdDivide_s((BIGD)NewValue, (BIGD)Remainder.m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
	bool bNewNegative = (m_bNegative != Src.m_bNegative);

	return CIPInteger(NewValue, bNewNegative);
	}

CIPInteger &CIPInteger::operator %= (const CIPInteger &Src)

//	CIPInteger operator %=

	{
	if (!Src.m_Value)
		{
		*this = CIPInteger(0);
		return *this;
		}
	else if (!m_Value)
		m_Value = bdNew();

	CIPInteger Quotient(bdNew(), false);

	bdDivide_s((BIGD)Quotient.m_Value, (BIGD)m_Value, (BIGD)m_Value, (BIGD)Src.m_Value);
	m_bNegative = (m_bNegative != Src.m_bNegative);

	return *this;
	}

CIPInteger CIPInteger::operator % (const CIPInteger &Src) const

//	CIPInteger operator %

	{
	if (!Src.m_Value)
		return CIPInteger(0);
	else if (!m_Value)
		return CIPInteger(0);

	CIPInteger Quotient(bdNew(), false);

	void *NewValue = bdNew();
	bdDivide_s((BIGD)Quotient.m_Value, (BIGD)NewValue, (BIGD)m_Value, (BIGD)Src.m_Value);
	bool bNewNegative = (m_bNegative != Src.m_bNegative);

	return CIPInteger(NewValue, bNewNegative);
	}

CIPInteger &CIPInteger::operator >>= (size_t iBits)

//	CIPInteger operator >>=

	{
	if (!m_Value)
		m_Value = bdNew();

	void *NewValue = bdNew();
	bdShiftRight((BIGD)NewValue, (BIGD)m_Value, iBits);

	*this = CIPInteger(NewValue, m_bNegative);
	return *this;
	}

CIPInteger CIPInteger::operator >> (size_t iBits) const

//	CIPInteger operator >>

	{
	if (!m_Value)
		return CIPInteger(0);

	void *NewValue = bdNew();
	bdShiftRight((BIGD)NewValue, (BIGD)m_Value, iBits);

	return CIPInteger(NewValue, m_bNegative);
	}

CIPInteger &CIPInteger::operator <<= (size_t iBits)

//	CIPInteger operator <<=

	{
	if (!m_Value)
		m_Value = bdNew();

	void *NewValue = bdNew();
	bdShiftLeft((BIGD)NewValue, (BIGD)m_Value, iBits);

	*this = CIPInteger(NewValue, m_bNegative);
	return *this;
	}

CIPInteger CIPInteger::operator << (size_t iBits) const

//	CIPInteger operator <<

	{
	if (!m_Value)
		return CIPInteger(0);

	void *NewValue = bdNew();
	bdShiftLeft((BIGD)NewValue, (BIGD)m_Value, iBits);

	return CIPInteger(NewValue, m_bNegative);
	}

int CIPInteger::AsByteArray (TArray<BYTE> *retValue) const

//	AsByteArray
//
//	Initializes retValue with an array of bytes representing the number. We
//	return the number of bytes in the array.

	{
	retValue->DeleteAll();
	if (!m_Value)
		return 0;

	DWORD dwSize = (DWORD)bdConvToOctets((BIGD)m_Value, NULL, 0);
	if (dwSize > 0)
		{
		retValue->InsertEmpty(dwSize);
		bdConvToOctets((BIGD)m_Value, &retValue->GetAt(0), dwSize);
		}

	return dwSize;
	}

int CIPInteger::AsInteger32Signed (void) const

//	AsInteger32Signed
//
//	Returns 32-bit int.
//
//	NOTE: Callers must first make sure that the value fits in 32 bits

	{
	if (!m_Value)
		return 0;

	BYTE BigEndian[sizeof(DWORD)];

	//	Get the bytes

	bdConvToOctets((BIGD)m_Value, BigEndian, sizeof(DWORD));
	
	//	Reverse to little-endian

	DWORD Result;
	utlMemReverse(BigEndian, &Result, sizeof(DWORD));

	//	Done

	if (m_bNegative)
		return -(int)Result;
	else
		return (int)Result;
	}

DWORDLONG CIPInteger::AsInteger64Unsigned (void) const

//	AsInteger64Unsigned
//
//	Returns the integer as a DWORDLONG.
//	
//	NOTE: Callers must first call FitsAsInteger64Unsiged to make sure that the
//	integer fits.

	{
	if (!m_Value)
		return 0;

	BYTE BigEndian[sizeof(DWORDLONG)];

	//	Get the bytes

	bdConvToOctets((BIGD)m_Value, BigEndian, sizeof(DWORDLONG));
	
	//	Reverse to little-endian

	DWORDLONG Result;
	utlMemReverse(BigEndian, &Result, sizeof(DWORDLONG));

	//	Done

	return Result;
	}

CString CIPInteger::AsString (void) const

//	AsString
//
//	Convert to a string

	{
	if (!m_Value)
		return NULL_STR;

	CStringBuffer Buffer;
	int iOffset = 0;

	//	Write the negative sign, if necessary

	if (m_bNegative)
		{
		Buffer.Write("-", 1);
		iOffset = 1;
		}

	//	The number

	int iSize = (int)bdConvToDecimal((BIGD)m_Value, NULL, 0);
	Buffer.Write(NULL, iSize);

	//	Need to add 1 for the NULL-termination (note that CStringBuffer
	//	properly allocates enough space for it).

	bdConvToDecimal((BIGD)m_Value, Buffer.GetPointer() + iOffset, iSize + 1);

	CString sString;
	sString.TakeHandoff(Buffer);

	return sString;
	}

int CIPInteger::Compare (const CIPInteger &Src) const

//	Compare
//
//	If this > Src,		1
//	If this == Src,		0
//	If this < Src,		-1

	{
	if (!Src.m_Value && !m_Value)
		return 0;
	else if (!Src.m_Value)
		return 1;
	else if (!m_Value)
		return -1;

	else if (m_bNegative && !Src.m_bNegative)
		return -1;
	else if (!m_bNegative && Src.m_bNegative)
		return 1;
	else
		{
		int iCompare = bdCompare((BIGD)m_Value, (BIGD)Src.m_Value);
		if (!m_bNegative)
			return iCompare;
		else
			return iCompare * -1;
		}
	}

bool CIPInteger::Deserialize (IByteStream &Stream, CIPInteger *retpValue)

//	Deserialize
//
//	Deserialize from a stream

	{
	DWORD dwLoad;

	Stream.Read(&dwLoad, sizeof(DWORD));
	if (dwLoad == STREAM_SIGNATURE_POSITIVE)
		retpValue->m_bNegative = false;
	else if (dwLoad == STREAM_SIGNATURE_NEGATIVE)
		retpValue->m_bNegative = true;
	else
		return false;

	if (!retpValue->m_Value)
		retpValue->m_Value = bdNew();

	DWORD dwSize;
	Stream.Read(&dwSize, sizeof(DWORD));

	BYTE *pTemp = new BYTE [dwSize];
	Stream.Read(pTemp, dwSize);
	bdConvFromOctets((BIGD)retpValue->m_Value, pTemp, dwSize);
	delete [] pTemp;

	return true;
	}

bool CIPInteger::FitsAsInteger32Signed (void) const

//	FitsAsInteger32Signed
//
//	Returns TRUE if we can represent this integer in a 32-bit signed int.

	{
	if (!m_Value)
		return true;

	DWORD dwSize = (DWORD)bdConvToOctets((BIGD)m_Value, NULL, 0);
	if (dwSize > 4)
		return false;

	return (*this >= MIN_INT32 && *this <= MAX_INT32);
	}

bool CIPInteger::FitsAsInteger64Unsigned (void) const

//	FitsAsInteger64Unsigned
//
//	Returns TRUE if we can represent this integer as a DWORDLONG without loss
//	of data.

	{
	if (!m_Value)
		return true;

	else if (m_bNegative)
		return false;

	DWORD dwSize = (DWORD)bdConvToOctets((BIGD)m_Value, NULL, 0);
	if (dwSize > (DWORD)sizeof(DWORDLONG))
		return false;

	return true;
	}

DWORD CIPInteger::GetSize (void) const

//	GetSize
//
//	Returns the size in bytes

	{
	if (!m_Value)
		return 0;

	return (DWORD)bdConvToOctets((BIGD)m_Value, NULL, 0);
	}

void CIPInteger::InitFromBytes (const IMemoryBlock &Data)

//	InitFromBytes
//
//	Initializes from an array of bytes. The bytes must be in big-endian order
//	(most significant byte is first--which is the reverse of Intel architectures).

	{
	if (!m_Value)
		m_Value = bdNew();

	bdConvFromOctets((BIGD)m_Value, (BYTE *)Data.GetPointer(), Data.GetLength());
	m_bNegative = false;
	}

void CIPInteger::InitFromString (const CString &sString)

//	InitFromString
//
//	Initializes from a string representation

	{
	if (!m_Value)
		m_Value = bdNew();

	char *pPos = sString.GetParsePointer();
	char *pPosEnd = pPos + sString.GetLength();

	//	See if we have a leading 0x (to indicate that it is a hex number)

	if (pPos[0] == '0' && pPos + 1 < pPosEnd && (pPos[1] == 'x' || pPos[1] == 'X'))
		{
		bdConvFromHex((BIGD)m_Value, pPos + 2);
		m_bNegative = false;
		}

	//	Otherwise convert from decimal

	else
		{
		if (*pPos == '-')
			{
			m_bNegative = true;
			pPos++;
			}
		else
			m_bNegative = false;

		bdConvFromDecimal((BIGD)m_Value, pPos);
		}
	}

bool CIPInteger::IsEmpty (void) const

//	IsEmpty
//
//	Returns TRUE if we are empty (i.e., just allocated). Note that this is
//	different from being equal to 0.

	{
	return !m_Value || (bdIsZero((BIGD)m_Value) == -1);
	}

bool CIPInteger::IsEven () const

//	IsEven
//
//	Returns true if we're even.

	{
	return (IsEmpty() || bdIsEven((BIGD)m_Value) != 0);
	}

bool CIPInteger::IsOdd () const

//	IsOdd
//
//	Returns true if we're odd.

	{
	return (!IsEmpty() && bdIsOdd((BIGD)m_Value) != 0);
	}

bool CIPInteger::IsZero () const

//	IsZero
//
//	Returns true if 0.

	{
	return (m_Value && bdIsZero((BIGD)m_Value) != 0);
	}

CIPInteger CIPInteger::Power (const CIPInteger &Exp) const

//	Power
//
//	Raise to the given power.

	{
	if (!m_Value)
		return CIPInteger(0);
	else if (!Exp.m_Value)
		return CIPInteger(0);

	const CIPInteger TWO(2);

	if (Exp.IsNegative())
		return CIPInteger();
	else if (Exp.IsZero())
		return CIPInteger(1);
	else
		{
		CIPInteger Result(1);
		CIPInteger BaseTmp(*this);
		CIPInteger ExpTmp(Exp);

		while (true)
			{
			if (ExpTmp.IsOdd())
				Result *= BaseTmp;

			ExpTmp >>= 1;
			if (ExpTmp.IsZero())
				break;

			BaseTmp *= BaseTmp;
			}

		if (m_bNegative)
			Result.m_bNegative = Exp.IsOdd();

		return Result;
		}
	}

void CIPInteger::TakeHandoff (CIPInteger &Src)

//	TakeHandoff
//
//	Takes ownership of Src's memory.

	{
	//	We swap so we don't actually have to do any memory allocation

	Swap(m_Value, Src.m_Value);
	m_bNegative = Src.m_bNegative;
	}

bool CIPInteger::Serialize (IByteStream &Stream) const

//	Serialize
//
//	Serialize to a stream

	{
	DWORD dwSave;

	dwSave = (m_bNegative ? STREAM_SIGNATURE_NEGATIVE : STREAM_SIGNATURE_POSITIVE);
	Stream.Write(&dwSave, sizeof(DWORD));

	if (!m_Value)
		{
		dwSave = 0;
		Stream.Write(&dwSave, sizeof(DWORD));
		return true;
		}

	DWORD dwSize = (DWORD)bdConvToOctets((BIGD)m_Value, NULL, 0);
	Stream.Write(&dwSize, sizeof(DWORD));

	BYTE *pTemp = new BYTE [dwSize];
	bdConvToOctets((BIGD)m_Value, pTemp, dwSize);
	Stream.Write(pTemp, dwSize);
	delete [] pTemp;

	return true;
	}

void CIPInteger::WriteBytes (IByteStream &Stream) const

//	WriteBytes
//
//	Write out the bytes of the number in big-endian order. This is the opposite
//	of InitFromBytes.
//
//	NOTE: Unlike Serialize() we don't write out metadata (such as size or
//	signature).

	{
	if (!m_Value)
		return;

	DWORD dwSize = (DWORD)bdConvToOctets((BIGD)m_Value, NULL, 0);

	BYTE *pTemp = new BYTE [dwSize];
	bdConvToOctets((BIGD)m_Value, pTemp, dwSize);
	Stream.Write(pTemp, dwSize);
	delete [] pTemp;
	}

#endif