//	CDBValue.cpp
//
//	CDBValue class
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	CDBValue is a variant type that can store all basic types. It encodes a 
//	value (if small enough) and allocates a pointer (if large). We use a 64-bit
//	value to handle pointers. The bit pattern is as follows:
//
//	6666 5555 5555 5544 4444 4444 3333 3333 3322 2222 2222 1111 1111 1100 0000 0000
//	3210 9876 5432 1098 7654 3210 9876 5432	1098 7654 3210 9876 5432 1098 7654 3210
//
//	[ Pointer to string                                                         ]00		NULL == Nil
//	[ Pointer to IDBValueObject                                                 ]01
//	[ Unused                                                                    ]10
//
//	[ 32-bit integer                      ] ---- ---- ---- ---- ---- ---- ---- 0011
//	[ 60-bit integer														 ] 0111
//	[ 1-bit sign - 7-bit exponent - 52-bit mantissa                          ] 1011

//	[ 56-bit ID for Special Values										] 0000 1111

#include "stdafx.h"
#include "DBValueObjectImpl.h"

const CDBValue CDBValue::Null;

CDBValue::CDBValue (ETypes iType)

//	CDBValue constructor

	{
	switch (iType)
		{
		case typeArray:
			m_dwData = EncodeObjectPtr(new CDBValueArray);
			break;

		case typeDouble:
			m_dwData = EncodeDouble(0.0);
			break;

		case typeInt32:
			m_dwData = EncodeInt32(0);
			break;

		case typeNil:
			m_dwData = 0;
			break;

		case typeString:
			m_dwData = EncodeString(NULL_STR);
			break;

		case typeStruct:
			m_dwData = EncodeObjectPtr(new CDBValueStruct);
			break;

		case typeTrue:
			m_dwData = SPECIAL_TRUE;
			break;

		default:
			ASSERT(false);
			m_dwData = 0;
		}
	}

CDBValue::CDBValue (const CString &sValue)

//	CDBValue constructor

	{
	m_dwData = EncodeString(sValue);
	}

CDBValue::CDBValue (int iValue)

//	CDBValue constructor

	{
	m_dwData = EncodeInt32(iValue);
	}

CDBValue::CDBValue (double rValue)

//	CDBValue constructor

	{
	m_dwData = EncodeDouble(rValue);
	}

void CDBValue::CleanUp (void)

//	CleanUp
//
//	Clean up any allocations

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_STRING:
			{
			LPSTR pString = DecodeString(m_dwData);
			if (pString)
				{
				CString sValue;
				sValue.TakeHandoffLPSTR(pString);
				}

			break;
			}

		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			if (pObj)
				delete pObj;
			break;
			}
		}
	}

void CDBValue::Copy (const CDBValue &Src)

//	Copy
//
//	Copy from the given value.

	{
	switch (DecodeDiscriminator1(Src.m_dwData))
		{
		case TYPE_STRING:
			{
			LPSTR pString = DecodeString(Src.m_dwData);
			if (pString)
				{
				CString sValue;
				sValue.TakeHandoffLPSTR(pString);

				m_dwData = EncodeString(sValue);
				sValue.Handoff();
				}
			else
				m_dwData = 0;

			break;
			}

		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(Src.m_dwData);
			if (pObj)
				m_dwData = EncodeObjectPtr(pObj->Clone());
			else
				m_dwData = 0;
			break;
			}

		default:
			m_dwData = Src.m_dwData;
		}
	}

DWORDLONG CDBValue::EncodeDouble (double rValue)

//	EncodeDouble
//
//	Encodes a 64-bit IEEE double value into a DWORDLONG. NOTE: We may have to 
//	allocate the double. The return value must be used.

	{
	return EncodeObjectPtr(new CDBValueDouble(rValue));
	}

DWORDLONG CDBValue::EncodeString (const CString &sValue)

//	EncodeString
//
//	Encodes an allocated pointer to a string.

	{
	if (sValue.IsEmpty())
		return 0;

	//	If this is a literal string, then we can just
	//	take the value, because it doesn't need to be freed.
	//	NOTE: We can only do this if the literal pointer is
	//	DWORD aligned.

	else if (sValue.IsLiteral())
		{
		DWORDLONG dwData = (DWORD_PTR)(LPSTR)sValue;
		if ((dwData & DISCRIMINATOR_1_MASK) == 0)
			return dwData;

		//	Otherwise we drop through and make a copy.
		}

	//	Get a copy of the naked LPSTR out of the string

	CString sNewString(sValue);
	LPSTR pString = sNewString.Handoff();

	//	Return the new pointer. Callers must handle this properly.

	return (DWORDLONG)pString;
	}

CDBValue CDBValue::FromHandoff (CString &Src)

//	FromHandoff
//
//	Takes the allocated value from the given string.

	{
	CDBValue Value;

	LPSTR pStr = Src.Handoff();
	Value.m_dwData = (DWORDLONG)pStr;
	return Value;
	}
