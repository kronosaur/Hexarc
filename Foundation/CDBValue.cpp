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
//	[ 60-bit special values                                                     ]11
//
//	[ 32-bit integer                      ] ---- ---- ---- ---- ---- ---- ---- 0011
//	[ 60-bit integer														 ] 0111
//	[ 1-bit sign - 7-bit exponent - 52-bit mantissa                          ] 1011

//	[ 56-bit ID for Special Values										] 0000 1111

#include "stdafx.h"
#include "DBValueObjectImpl.h"

DECLARE_CONST_STRING(TYPE_NAME_ARRAY,				"array");
DECLARE_CONST_STRING(TYPE_NAME_DATE_TIME,			"dateTime");
DECLARE_CONST_STRING(TYPE_NAME_DOUBLE,				"double");
DECLARE_CONST_STRING(TYPE_NAME_INT32,				"int32");
DECLARE_CONST_STRING(TYPE_NAME_INT64,				"int64");
DECLARE_CONST_STRING(TYPE_NAME_NIL,					"nil");
DECLARE_CONST_STRING(TYPE_NAME_STRING,				"string");
DECLARE_CONST_STRING(TYPE_NAME_STRUCT,				"struct");
DECLARE_CONST_STRING(TYPE_NAME_TIME_SPAN,			"timeSpan");
DECLARE_CONST_STRING(TYPE_NAME_TRUE,				"true");
DECLARE_CONST_STRING(TYPE_NAME_UNKNOWN,				"unknown");

const CDBValue CDBValue::Null;

CDBValue::CDBValue (ETypes iType)

//	CDBValue constructor

	{
	switch (iType)
		{
		case typeArray:
			m_dwData = EncodeObjectPtr(new CDBValueArray);
			break;

		case typeDateTime:
			m_dwData = EncodeObjectPtr(new CDBValueDateTime);
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

CDBValue::CDBValue (bool bValue)

//	CDBValue constructo

	{
	if (bValue)
		m_dwData = SPECIAL_TRUE;
	else
		m_dwData = 0;
	}

CDBValue::CDBValue (const CString &sValue)

//	CDBValue constructor

	{
	m_dwData = EncodeString(sValue);
	}

CDBValue::CDBValue (const CDateTime &Value)

//	CDBValue constructor

	{
	m_dwData = EncodeObjectPtr(new CDBValueDateTime(Value));
	}

CDBValue::CDBValue (const CTimeSpan &Value)

//	CDBValue constructor

	{
	m_dwData = EncodeObjectPtr(new CDBValueTimeSpan(Value));
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

CDBValue::operator int () const

//	operator int

	{
	switch (DecodeDiscriminator2(m_dwData))
		{
		case TYPE_INT_32:
			return DecodeInt32(m_dwData);

		case TYPE_SPECIAL:
			{
			switch (m_dwData)
				{
				case SPECIAL_TRUE:
					return 1;

				default:
					return 0;
				}
			}

		default:
			return 0;
		}
	}

CDBValue::operator double () const

//	Operator double

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->CastDouble();
			}

		case TYPE_SPECIAL_60:
			{
			switch (DecodeDiscriminator2(m_dwData))
				{
				case TYPE_INT_32:
					return (double)DecodeInt32(m_dwData);

				default:
					return 0.0;
				}
			}

		default:
			return 0.0;
		}
	}

CDBValue::operator const CDateTime & () const

//	Operator const CDateTime &

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->CastDateTime();
			}

		default:
			return NULL_DATETIME;
		}
	}

CDBValue::operator LONGLONG () const

//	Operator const LONGLONG

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->CastLONGLONG();
			}

		case TYPE_SPECIAL_60:
			{
			switch (DecodeDiscriminator2(m_dwData))
				{
				case TYPE_INT_32:
					return (LONGLONG)DecodeInt32(m_dwData);

				default:
					return 0;
				}
			}

		default:
			return 0;
		}
	}

CDBValue::operator const CString & () const

//	Operator const CString &

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_STRING:
			return *(CString *)&m_dwData;

		default:
			return NULL_STR;
		}
	}

CDateTime CDBValue::AsDateTime (void) const

//	AsDateTime
//
//	Returns a date time representation.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->AsDateTime();
			}

		default:
			return NULL_DATETIME;
		}
	}

double CDBValue::AsDouble (bool *retbValid) const

//	AsDouble
//
//	Return as a double.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_STRING:
			{
			const CString &sValue = *(CString *)&m_dwData;
			char *pPos = sValue.GetParsePointer();
			char *pPosEnd = pPos + sValue.GetLength();

			char *pParseEnd;
			bool bNull;
			double rValue = strParseDouble(pPos, 0.0, &pParseEnd, &bNull);
			if (retbValid) *retbValid = (!bNull && pPosEnd == pParseEnd);
			return rValue;
			}

		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->AsDouble(retbValid);
			}

		case TYPE_SPECIAL_60:
			{
			switch (DecodeDiscriminator2(m_dwData))
				{
				case TYPE_INT_32:
					if (retbValid) *retbValid = true;
					return (double)DecodeInt32(m_dwData);

				default:
					if (retbValid) *retbValid = false;
					return 0.0;
				}
			}

		default:
			{
			if (retbValid) *retbValid = false;
			return 0.0;
			}
		}
	}

int CDBValue::AsInt32 (bool *retbValid) const

//	AsInt32
//
//	Return as an integer.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_STRING:
			{
			const CString &sValue = *(CString *)&m_dwData;
			char *pPos = sValue.GetParsePointer();
			char *pPosEnd = pPos + sValue.GetLength();

			char *pParseEnd;
			bool bNull;
			int iValue = strParseInt(pPos, 0, &pParseEnd, &bNull);
			if (retbValid) *retbValid = (!bNull && pPosEnd == pParseEnd);
			return iValue;
			}

		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->AsInt32(retbValid);
			}

		case TYPE_SPECIAL_60:
			{
			switch (DecodeDiscriminator2(m_dwData))
				{
				case TYPE_INT_32:
					if (retbValid) *retbValid = true;
					return DecodeInt32(m_dwData);

				case TYPE_SPECIAL:
					{
					switch (m_dwData)
						{
						case SPECIAL_TRUE:
							if (retbValid) *retbValid = true;
							return 1;

						default:
							if (retbValid) *retbValid = false;
							return 0;
						}
					}
			
				default:
					if (retbValid) *retbValid = false;
					return 0;
				}
			}

		default:
			{
			if (retbValid) *retbValid = false;
			return 0;
			}
		}
	}

CTimeSpan CDBValue::AsTimeSpan (void) const

//	AsTimeSpan
//
//	Returns a timespan

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->AsTimeSpan();
			}

		default:
			return CTimeSpan();
		}
	}

CString CDBValue::AsString (void) const

//	AsString
//
//	Return a string representation.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_STRING:
			return *(CString *)&m_dwData;

		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->AsString();
			}

		case TYPE_SPECIAL_60:
			{
			switch (DecodeDiscriminator2(m_dwData))
				{
				case TYPE_INT_32:
					return strFromInt(DecodeInt32(m_dwData));

				case TYPE_SPECIAL:
					{
					switch (m_dwData)
						{
						case SPECIAL_TRUE:
							return TYPE_NAME_TRUE;

						default:
							return NULL_STR;
						}
					}
			
				default:
					return NULL_STR;
				}
			}

		default:
			return NULL_STR;
		}
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

CDBValue::ETypes CDBValue::Coerce (ETypes iType1, ETypes iType2)

//	Coerce
//
//	Coerce types when combining in a binary operation.

	{
	if (iType1 == iType2)
		return iType1;

	switch (iType1)
		{
		//	In a binary operation, nil always coerces to the other type. For
		//	example, 1 + nil == 1.

		case typeNil:
			return iType2;

		case typeTrue:
			switch (iType2)
				{
				case typeNil:
					return typeTrue;

				default:
					return iType2;
				}

		case typeInt32:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
					return typeInt32;

				default:
					return iType2;
				}

		case typeInt64:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
					return typeInt64;

				default:
					return iType2;
				}

		case typeDouble:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
				case typeInt64:
					return typeDouble;

				default:
					return iType2;
				}

		case typeTimeSpan:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
				case typeInt64:
				case typeDouble:
					return typeTimeSpan;

				default:
					return iType2;
				}

		case typeDateTime:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
				case typeInt64:
				case typeDouble:
				case typeTimeSpan:
					return typeDateTime;

				default:
					return iType2;
				}

		case typeString:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
				case typeInt64:
				case typeDouble:
				case typeTimeSpan:
				case typeDateTime:
					return typeString;

				default:
					return iType2;
				}

		case typeArray:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
				case typeInt64:
				case typeDouble:
				case typeTimeSpan:
				case typeDateTime:
				case typeString:
					return typeArray;

				default:
					return iType2;
				}

		case typeStruct:
			switch (iType2)
				{
				case typeNil:
				case typeTrue:
				case typeInt32:
				case typeInt64:
				case typeDouble:
				case typeTimeSpan:
				case typeDateTime:
				case typeString:
				case typeArray:
					return typeStruct;

				default:
					return iType2;
				}

		default:
			return typeUnknown;
		}
	}

int CDBValue::Compare (const CDBValue &Left, const CDBValue &Right, ETypes *retiType)

//	Compare
//
//	Compares two values:
//
//	 0	if Left == Right
//	 1	if Left > Right
//	-1	if Left < Right
//
//	-2	if we get an error comparing

	{
	CDBValue::ETypes iLeftType = Left.GetType();
	CDBValue::ETypes iRightType = Right.GetType();

	if (iLeftType == iRightType)
		{
		switch (iLeftType)
			{
			case typeNil:
			case typeTrue:
				return 0;

			case typeInt32:
				{
				if ((int)Left == (int)Right)
					return 0;
				else if ((int)Left > (int)Right)
					return 1;
				else
					return -1;
				}

			case typeDateTime:
				return ::KeyCompare((const CDateTime &)Left, (const CDateTime &)Right);

			case typeDouble:
				{
				if (isnan((double)Left))
					{
					if (isnan((double)Right))
						return 0;
					else
						return -1;
					}
				else if (isnan((double)Right))
					return 1;
				else if ((double)Left == (double)Right)
					return 0;
				else if ((double)Left > (double)Right)
					return 1;
				else
					return -1;
				}

			case typeString:
				return ::KeyCompareNoCase((const CString &)Left, (const CString &)Right);

			case typeArray:
				//	Not yet implemented
				return -2;

			case typeStruct:
				//	Not Yet implemented
				return -2;

			default:
				return -2;
			}
		}
	else
		{
		if (iLeftType == typeNil)
			{
			switch (iRightType)
				{
				case typeNil:
					return 0;

				case typeTrue:
					return -1;

				case typeInt32:
				case typeDouble:
				case typeString:
					//	Nil is always less than everything
					return -1;

				case typeArray:
				case typeStruct:
					//	Not yet implemented
					return -2;

				default:
					return -2;
				}
			}
		else if (iRightType == typeNil)
			{
			int iResult = Compare(Right, Left);
			if (iResult == -1)
				return 1;
			else if (iResult == 1)
				return -1;
			else
				return iResult;
			}
		else if (iLeftType == typeDouble)
			{
			switch (iRightType)
				{
				case typeTrue:
					{
					if (isnan((double)Left))
						return -1;
					else if ((double)Left == 1.0)
						return 0;
					else if ((double)Left > 1.0)
						return 1;
					else
						return -1;
					}

				case typeInt32:
					{
					double rRightValue = (double)Right;
					if (isnan((double)Left))
						return -1;
					else if ((double)Left == rRightValue)
						return 0;
					else if ((double)Left > rRightValue)
						return 1;
					else
						return -1;
					}

				case typeString:
					{
					bool bValid;
					double rRightValue = Right.AsDouble(&bValid);
					if (!bValid)
						return -2;
					else if (isnan((double)Left))
						return -1;
					else if ((double)Left == rRightValue)
						return 0;
					else if ((double)Left > rRightValue)
						return 1;
					else
						return -1;
					}

				default:
					return -2;
				}
			}
		else if (iRightType == typeDouble)
			{
			int iResult = Compare(Right, Left);
			if (iResult == -1)
				return 1;
			else if (iResult == 1)
				return -1;
			else
				return iResult;
			}
		else if (iLeftType == typeInt32)
			{
			switch (iRightType)
				{
				case typeTrue:
					{
					if ((int)Left == 1)
						return 0;
					else if ((int)Left > 1)
						return 1;
					else
						return -1;
					}

				case typeString:
					{
					bool bValid;
					int iRightValue = Right.AsInt32(&bValid);
					if (!bValid)
						return -2;
					else if ((int)Left == iRightValue)
						return 0;
					else if ((int)Left > iRightValue)
						return 1;
					else
						return -1;
					}

				default:
					return -2;
				}
			}
		else if (iRightType == typeInt32)
			{
			int iResult = Compare(Right, Left);
			if (iResult == -1)
				return 1;
			else if (iResult == 1)
				return -1;
			else
				return iResult;
			}
		else
			return -2;
		}
	}

CDBValue CDBValue::ConvertSASDate (double rValue)

//	ConvertSASDate
//
//	Converts a SAS date to a CDateTime.
//	See: http://support.sas.com/documentation/cdl/en/lrcon/62955/HTML/default/viewer.htm#a002200738.htm

	{
	//	0.0 is considered a null value
	if (rValue == 0.0)
		return CDBValue();

	CDateTime Epoch(1, 1, 1960);
	double rMsecs = rValue * 24.0 * 60.0 * 60.0 * 1000.0;
	if (rMsecs >= 0.0)
		{
		CTimeSpan SinceEpoch((DWORDLONG)rMsecs);
		return timeAddTime(Epoch, SinceEpoch);
		}
	else
		{
		CTimeSpan SinceEpoch((DWORDLONG)-rMsecs);
		return timeSubtractTime(Epoch, SinceEpoch);
		}
	}

CDBValue CDBValue::ConvertSASDateTime (double rValue)

//	ConvertSASDateTime
//
//	Converts a SAS datetime to a CDateTime.

	{
	//	0.0 is considered a null value
	if (rValue == 0.0)
		return CDBValue();

	CDateTime Epoch(1, 1, 1960);
	double rMsecs = rValue * 1000.0;
	if (rMsecs >= 0.0)
		{
		CTimeSpan SinceEpoch((DWORDLONG)rMsecs);
		return timeAddTime(Epoch, SinceEpoch);
		}
	else
		{
		CTimeSpan SinceEpoch((DWORDLONG)-rMsecs);
		return timeSubtractTime(Epoch, SinceEpoch);
		}
	}

CDBValue CDBValue::ConvertSASTime (double rValue)

//	ConvertSASTime
//
//	Converts a SAS time to a CTimeSpan.

	{
	return CTimeSpan((int)(rValue * 1000.0));
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

CDBValue::ETypes CDBValue::GetType (void) const

//	GetType
//
//	Returns a type.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_STRING:
			return (m_dwData == 0 ? typeNil : typeString);

		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->GetType();
			}

		case TYPE_SPECIAL_60:
			{
			switch (DecodeDiscriminator2(m_dwData))
				{
				case TYPE_INT_32:
					return typeInt32;

				case TYPE_SPECIAL:
					{
					switch (m_dwData)
						{
						case SPECIAL_TRUE:
							return typeTrue;

						default:
							return typeNil;
						}
					}

				default:
					return typeNil;
				}
			}

		default:
			return typeNil;
		}
	}

const CDBValue &CDBValue::GetElement (const CString &sKey) const

//	GetElement
//
//	Returns an element.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			return pObj->GetElement(sKey);
			}

		default:
			return Null;
		}
	}

void CDBValue::SetElement (const CString &sKey, const CDBValue &Value)

//	SetElement
//
//	Sets an element.

	{
	switch (DecodeDiscriminator1(m_dwData))
		{
		case TYPE_OBJECT:
			{
			IDBValueObject *pObj = DecodeObject(m_dwData);
			pObj->SetElement(sKey, Value);
			break;
			}

		default:
			break;
		}
	}

const CString &CDBValue::TypeName (ETypes iType)

//	TypeName
//
//	Returns a typename

	{
	switch (iType)
		{
		case typeNil:
			return TYPE_NAME_NIL;

		case typeTrue:
			return TYPE_NAME_TRUE;

		case typeInt32:
			return TYPE_NAME_INT32;

		case typeInt64:
			return TYPE_NAME_INT64;

		case typeDouble:
			return TYPE_NAME_DOUBLE;

		case typeString:
			return TYPE_NAME_STRING;

		case typeArray:
			return TYPE_NAME_ARRAY;

		case typeStruct:
			return TYPE_NAME_STRUCT;

		default:
			return TYPE_NAME_UNKNOWN;
		}
	}
