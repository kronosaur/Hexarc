//	CDatumInterpret.cpp
//
//	CDatumInterpret class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_FORMAT,						"format");

DECLARE_CONST_STRING(FORMAT_BIG_ENDIAN,					"big-endian");
DECLARE_CONST_STRING(FORMAT_LITTLE_ENDIAN,				"little-endian");

DECLARE_CONST_STRING(ERR_EXPECTED_DATATYPE,				"Not a datatype: %s.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_DATATYPE,			"Unsupported datatype: %s.");
DECLARE_CONST_STRING(ERR_INSUFFICIENT_DATA,				"Insufficient data: %d byte(s) required.");

int CDatumInterpret::CalcSizeOf (const IDatatype& Type)

//	CalcSizeOf
//
//	Computes the size of the datatype. Returns 0 if we cannot write the type
//	to a binary buffer.

	{
	//	If this is a nullable type, then we use the underlying type.

	if (Type.IsNullable())
		{
		CDatum dActualType = Type.GetVariantType();
		return CalcSizeOf(dActualType);
		}

	//	If this is a structure, then we compute the size of each field

	else if (Type.GetClass() == IDatatype::ECategory::Schema)
		{
		int iSize = 0;

		for (int i = 0; i < Type.GetMemberCount(); i++)
			{
			IDatatype::SMemberDesc Member = Type.GetMember(i);
			if (Member.iType == IDatatype::EMemberType::InstanceKeyVar || Member.iType == IDatatype::EMemberType::InstanceVar)
				{
				int iFieldSize = CalcSizeOf(Member.dType);
				if (iFieldSize == 0)
					return 0;

				iSize += iFieldSize;
				}
			}

		return iSize;
		}

	//	Otherwise, we handle an explicit core type

	else
		{
		switch (Type.GetCoreType())
			{
			case IDatatype::INT_8:
			case IDatatype::UINT_8:
				return 1;

			case IDatatype::INT_16:
			case IDatatype::UINT_16:
				return 2;

			case IDatatype::INT_32:
			case IDatatype::UINT_32:
				return 4;

			default:
				return 0;
			}
		}
	}

bool CDatumInterpret::EncodeAs (BYTE* pPos, BYTE* pEndPos, CDatum dType, CDatum dValue, const SOptions& Options, BYTE** retpNewPos, CString* retsError)

//	EncodeAs
//
//	Encodes the given value as the given type. Returns FALSE if there is an
//	error.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		{
		if (retsError) *retsError = strPattern(ERR_EXPECTED_DATATYPE, dType.AsString());
		return false;
		}

	const IDatatype& Type = dType;

	//	If this is a nullable type, then we use the underlying type.

	if (Type.IsNullable())
		{
		CDatum dActualType = Type.GetVariantType();
		return EncodeAs(pPos, pEndPos, dActualType, dValue, Options, retpNewPos, retsError);
		}

	//	If this is a structure, then we write it out

	else if (Type.GetClass() == IDatatype::ECategory::Schema)
		{
		for (int i = 0; i < Type.GetMemberCount(); i++)
			{
			IDatatype::SMemberDesc Member = Type.GetMember(i);
			if (Member.iType != IDatatype::EMemberType::InstanceKeyVar && Member.iType != IDatatype::EMemberType::InstanceVar)
				continue;

			//	Look for the value

			CDatum dFieldValue = dValue.GetElement(Member.sID);
			if (!EncodeAs(pPos, pEndPos, Member.dType, dFieldValue, Options, &pPos, retsError))
				return false;
			}

		return true;
		}

	//	Otherwise, we handle an explicit core type

	else
		{
		switch (Type.GetCoreType())
			{
			case IDatatype::INT_8:
				if (!CheckLength(pPos, pEndPos, 1, retsError))
					return false;

				*(char*)pPos = (char)Clamp((int)dValue, (int)MININT8, (int)MAXINT8);
				pPos += 1;
				break;

			case IDatatype::UINT_8:
				if (!CheckLength(pPos, pEndPos, 1, retsError))
					return false;

				*(BYTE*)pPos = (BYTE)Clamp((int)dValue, 0, (int)MAXUINT8);
				pPos += 1;
				break;

			case IDatatype::INT_16:
				{
				if (!CheckLength(pPos, pEndPos, 2, retsError))
					return false;

				WORD wValue = (WORD)Clamp((int)dValue, (int)MININT16, (int)MAXINT16);
				if (Options.bBigEndian)
					{
					pPos[0] = (BYTE)((wValue >> 8) & 0xff);
					pPos[1] = (BYTE)(wValue & 0xff);
					}
				else
					{
					pPos[0] = (BYTE)(wValue & 0xff);
					pPos[1] = (BYTE)((wValue >> 8) & 0xff);
					}
				pPos += 2;
				break;
				}

			case IDatatype::UINT_16:
				{
				if (!CheckLength(pPos, pEndPos, 2, retsError))
					return false;

				WORD wValue = (WORD)Clamp((int)dValue, 0, (int)MAXUINT16);
				if (Options.bBigEndian)
					{
					pPos[0] = (BYTE)((wValue >> 8) & 0xff);
					pPos[1] = (BYTE)(wValue & 0xff);
					}
				else
					{
					pPos[0] = (BYTE)(wValue & 0xff);
					pPos[1] = (BYTE)((wValue >> 8) & 0xff);
					}
				pPos += 2;
				break;
				}

			case IDatatype::INT_32:
				{					
				if (!CheckLength(pPos, pEndPos, 4, retsError))
					return false;

				DWORD dwValue = (DWORD)(int)dValue;
				if (Options.bBigEndian)
					{
					pPos[0] = (BYTE)((dwValue >> 24) & 0xff);
					pPos[1] = (BYTE)((dwValue >> 16) & 0xff);
					pPos[2] = (BYTE)((dwValue >> 8) & 0xff);
					pPos[3] = (BYTE)(dwValue & 0xff);
					}
				else
					{
					pPos[0] = (BYTE)(dwValue & 0xff);
					pPos[1] = (BYTE)((dwValue >> 8) & 0xff);
					pPos[2] = (BYTE)((dwValue >> 16) & 0xff);
					pPos[3] = (BYTE)((dwValue >> 24) & 0xff);
					}
				pPos += 4;
				break;
				}

			case IDatatype::UINT_32:
				{
				if (!CheckLength(pPos, pEndPos, 4, retsError))
					return false;

				DWORD dwValue = (DWORD)dValue;
				if (Options.bBigEndian)
					{
					pPos[0] = (BYTE)((dwValue >> 24) & 0xff);
					pPos[1] = (BYTE)((dwValue >> 16) & 0xff);
					pPos[2] = (BYTE)((dwValue >> 8) & 0xff);
					pPos[3] = (BYTE)(dwValue & 0xff);
					}
				else
					{
					pPos[0] = (BYTE)(dwValue & 0xff);
					pPos[1] = (BYTE)((dwValue >> 8) & 0xff);
					pPos[2] = (BYTE)((dwValue >> 16) & 0xff);
					pPos[3] = (BYTE)((dwValue >> 24) & 0xff);
					}
				pPos += 4;
				break;
				}

			default:
				if (retsError) *retsError = strPattern(ERR_UNSUPPORTED_DATATYPE, Type.GetName());
				return false;
			}

		//	Done

		if (retpNewPos)
			*retpNewPos = pPos;

		return true;
		}
	}

bool CDatumInterpret::InterpretAs (const BYTE* pPos, const BYTE* pEndPos, CDatum dType, const SOptions& Options, CDatum& retdValue, const BYTE** retpNewPos, CString* retsError)

//	InterpretAs
//
//	Interprets the given memory as the given type. Returns FALSE if there is an
//	error.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		{
		if (retsError) *retsError = strPattern(ERR_EXPECTED_DATATYPE, dType.AsString());
		return false;
		}

	const IDatatype& Type = dType;

	//	If this is a nullable type, then try to convert but return null if we fail
	//	(instead of an error).

	if (Type.IsNullable())
		{
		CDatum dActualType = Type.GetVariantType();
		if (!InterpretAs(pPos, pEndPos, dActualType, Options, retdValue, retpNewPos, retsError))
			{
			retdValue = CDatum();
			if (retpNewPos)
				*retpNewPos = pPos;
			}

		return true;
		}

	//	If this is a structure, then we parse

	else if (Type.GetClass() == IDatatype::ECategory::Schema)
		{
		retdValue = CDatum::CreateAsType(dType);

		for (int i = 0; i < Type.GetMemberCount(); i++)
			{
			IDatatype::SMemberDesc Member = Type.GetMember(i);
			if (Member.iType != IDatatype::EMemberType::InstanceKeyVar && Member.iType != IDatatype::EMemberType::InstanceVar)
				continue;

			//	Look for the value

			CDatum dValue;
			if (!InterpretAs(pPos, pEndPos, Member.dType, Options, dValue, &pPos, retsError))
				return false;

			retdValue.SetElement(Member.sID, dValue);
			}

		//	Done

		if (retpNewPos)
			*retpNewPos = pPos;

		return true;
		}

	//	Otherwise, we handle an explicit core type

	else
		{
		switch (Type.GetCoreType())
			{
			case IDatatype::INT_8:
				if (!CheckLength(pPos, pEndPos, 1, retsError))
					return false;

				retdValue = CDatum((int)(*(char*)pPos));
				pPos += 1;
				break;

			case IDatatype::UINT_8:
				if (!CheckLength(pPos, pEndPos, 1, retsError))
					return false;

				retdValue = CDatum((int)(*(BYTE*)pPos));
				pPos += 1;
				break;

			case IDatatype::INT_16:
				{
				if (!CheckLength(pPos, pEndPos, 2, retsError))
					return false;

				if (Options.bBigEndian)
					{
					//	Big-endian
					WORD wValue = (pPos[0] << 8) + pPos[1];
					retdValue = CDatum((int)(short)wValue);
					}
				else
					{
					//	Little-endian
					WORD wValue = pPos[0] + (pPos[1] << 8);
					retdValue = CDatum((int)(short)wValue);
					}
				pPos += 2;
				break;
				}

			case IDatatype::UINT_16:
				{
				if (!CheckLength(pPos, pEndPos, 2, retsError))
					return false;

				if (Options.bBigEndian)
					{
					//	Big-endian
					WORD wValue = (pPos[0] << 8) + pPos[1];
					retdValue = CDatum((int)wValue);
					}
				else
					{
					//	Little-endian
					WORD wValue = pPos[0] + (pPos[1] << 8);
					retdValue = CDatum((int)wValue);
					}
				pPos += 2;
				break;
				}

			case IDatatype::INT_32:
				{					
				if (!CheckLength(pPos, pEndPos, 4, retsError))
					return false;

				if (Options.bBigEndian)
					{
					//	Big-endian
					DWORD dwValue = (DWORD)(pPos[0] << 24) + (pPos[1] << 16) + (pPos[2] << 8) + pPos[3];
					retdValue = CDatum((int)dwValue);
					}
				else
					{
					//	Little-endian
					DWORD dwValue = pPos[0] + (pPos[1] << 8) + (pPos[2] << 16) + (pPos[3] << 24);
					retdValue = CDatum((int)dwValue);
					}
				pPos += 4;
				break;
				}

			case IDatatype::UINT_32:
				{
				if (!CheckLength(pPos, pEndPos, 4, retsError))
					return false;

				if (Options.bBigEndian)
					{
					//	Big-endian
					DWORD dwValue = (DWORD)(pPos[0] << 24) + (pPos[1] << 16) + (pPos[2] << 8) + pPos[3];
					retdValue = CDatum(dwValue);
					}
				else
					{
					//	Little-endian
					DWORD dwValue = pPos[0] + (pPos[1] << 8) + (pPos[2] << 16) + (pPos[3] << 24);
					retdValue = CDatum(dwValue);
					}
				pPos += 4;
				break;
				}

			default:
				if (retsError) *retsError = strPattern(ERR_UNSUPPORTED_DATATYPE, Type.GetName());
				return false;
			}

		//	Done

		if (retpNewPos)
			*retpNewPos = pPos;

		return true;
		}
	}

bool CDatumInterpret::CheckLength (const BYTE* pPos, const BYTE* pEndPos, size_t iLength, CString* retsError)
	{
	if ((pEndPos - pPos) < (LONGLONG)iLength)
		{
		if (retsError) *retsError = strPattern(ERR_INSUFFICIENT_DATA, (int)iLength);
		return false;
		}

	return true;
	}

bool CDatumInterpret::ParseOptions (CDatum dOptions, SOptions& retOptions)
	{
	CStringView sFormat;
	if (dOptions.IsNil())
		;
	else if (dOptions.GetBasicType() == CDatum::typeString)
		sFormat = dOptions;
	else if (dOptions.GetBasicType() == CDatum::typeStruct)
		sFormat = dOptions.GetElement(FIELD_FORMAT);
	else
		return false;

	if (sFormat.IsEmpty() || strEqualsNoCase(sFormat, FORMAT_BIG_ENDIAN))
		retOptions.bBigEndian = true;
	else if (strEqualsNoCase(sFormat, FORMAT_LITTLE_ENDIAN))
		retOptions.bBigEndian = false;
	else
		return false;

	return true;
	}
