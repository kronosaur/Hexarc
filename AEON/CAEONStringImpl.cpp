//	CAEONStringImpl.cpp
//
//	CAEONStringImpl class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_TYPE,						"type");

DECLARE_CONST_STRING(TYPE_DOUBLE_LINE,					"doubleLine");
DECLARE_CONST_STRING(TYPE_LINE,							"line");
DECLARE_CONST_STRING(TYPE_WHITESPACE,					"whitespace");

DECLARE_CONST_STRING(ERR_INVALID_SPLIT_TYPE,			"Invalid split type: %s.");
DECLARE_CONST_STRING(ERR_INVALID_PARAM,					"Invalid parameter: %s.");

TDatumPropertyHandler<CString> CAEONStringImpl::m_Properties = {
	{
		"bytes",
		"Returns an array of bytes.",
		[](const CString& sValue, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(sValue.GetLength());

			const char *pPos = sValue.GetParsePointer();
			const char *pEndPos = pPos + sValue.GetLength();
			while (pPos < pEndPos)
				{
				dResult.Append((int)(BYTE)*pPos);
				pPos++;
				}

			return dResult;
			},
		NULL,
		},
	{
		"chars",
		"Returns an array of Unicode characters.",
		[](const CString& sValue, const CString &sProperty)
			{
			return ExecuteSplitChars(sValue);
			},
		NULL,
		},
	{
		"datatype",
		"Returns the type of the string.",
		[](const CString& sValue, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::STRING);
			},
		NULL,
		},
	{
		"length",
		"Returns the length of the string in bytes.",
		[](const CString& sValue, const CString &sProperty)
			{
			return CDatum(sValue.GetLength());
			},
		NULL,
		},
	};

TDatumMethodHandler<CString> CAEONStringImpl::m_Methods = {
	{
		"find",
		"*",
		".find(x) -> Finds offset of x in string.",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dValueToFind = dLocalEnv.GetElement(1);
			if (dValueToFind.IsNil())
				{
				retdResult = CDatum();
				return true;
				}

			int iIndex = strFindNoCase(sValue, dValueToFind.AsString());
			if (iIndex == -1)
				{
				retdResult = CDatum();
				return true;
				}

			retdResult = iIndex;
			return true;
			},
		},
	{
		"left",
		"*",
		".left(n) -> left n bytes.",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			int iLen = Min((int)dLocalEnv.GetElement(1), sValue.GetLength());
			if (iLen <= 0)
				{
				retdResult = CDatum();
				return true;
				}

			retdResult = strSubString(sValue, 0, iLen);
			return true;
			},
		},
	{
		"right",
		"*",
		".right(n) -> right n bytes.",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			int iLen = Min((int)dLocalEnv.GetElement(1), sValue.GetLength());
			if (iLen <= 0)
				{
				retdResult = CDatum();
				return true;
				}

			retdResult = strSubString(sValue, sValue.GetLength() - iLen, iLen);
			return true;
			},
		},
	{
		"slice",
		"*",
		".slice(start, [end]) -> string.",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			int iStart;
			int iLen;
			CalcSliceParams(dLocalEnv.GetElement(1), dLocalEnv.GetElement(2), sValue.GetLength(), iStart, iLen);

			if (iLen <= 0)
				{
				retdResult = CDatum();
				return true;
				}

			retdResult = strSubString(sValue, iStart, iLen);
			return true;
			},
		},
	{
		"split",
		"*",
		".split() -> array of chars.\n"
		".split(string) -> array.\n"
		".split(arrayOfStrings) -> array.\n"
		".split(options) -> array.\n\n"

		"options:\n\n"

		"type: One of\n\n"

		"   doubleLine\n"
		"   line\n"
		"   whitespace\n",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			CDatum dOptions = dLocalEnv.GetElement(1);
			if (dOptions.IsNil())
				{
				retdResult = ExecuteSplitChars(sValue);
				return true;
				}
			else if (dOptions.GetBasicType() == CDatum::typeString)
				{
				retdResult = ExecuteSplit(sValue, (const CString&)dOptions);
				return true;
				}
			else if (dOptions.GetBasicType() == CDatum::typeArray)
				{
				retdResult = ExecuteSplitByArray(sValue, dOptions);
				return true;
				}
			else if (dOptions.GetBasicType() == CDatum::typeStruct)
				{
				const CString& sType = dOptions.GetElement(FIELD_TYPE);
				if (strEqualsNoCase(sType, TYPE_DOUBLE_LINE))
					{
					retdResult = ExecuteSplitDoubleLine(sValue);
					return true;
					}
				else if (strEqualsNoCase(sType, TYPE_LINE))
					{
					retdResult = ExecuteSplitLines(sValue);
					return true;
					}
				else if (strEqualsNoCase(sType, TYPE_WHITESPACE))
					{
					retdResult = ExecuteSplitWhitespace(sValue);
					return true;
					}
				else
					{
					retdResult = strPattern(ERR_INVALID_SPLIT_TYPE, sType);
					return false;
					}
				}
			else
				{
				retdResult = strPattern(ERR_INVALID_PARAM, dOptions.AsString());
				return false;
				}
			},
		},
	{
		"toLowerCase",
		"*",
		".toLowerCase() -> string in lowercase.",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = strToLower(sValue);
			return true;
			},
		},
	{
		"toUpperCase",
		"*",
		".toUpperCase() -> string in uppercase.",
		0,
		[](CString& sValue, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = strToUpper(sValue);
			return true;
			},
		},
	};

void CAEONStringImpl::CalcSliceParams (CDatum dStart, CDatum dEnd, int iLength, int& retStart, int& retLen)

//	CalcSliceParams
//
//	We use the same semantics as JavaScript slice:
//	https://developer.mozilla.org/en-US/docs/web/javascript/reference/global_objects/array/slice

	{
	int iStart = (int)dStart;
	int iEnd = (dEnd.IsNil() ? iLength : (int)dEnd);

	if (iStart < 0)
		iStart = Max(0, iStart + iLength);

	if (iEnd < 0)
		iEnd = Min(Max(0, iEnd + iLength), iLength);

	if (iEnd <= iStart || iStart >= iLength)
		{
		retStart = 0;
		retLen = 0;
		}
	else
		{
		retStart = iStart;
		retLen = iEnd - iStart;
		}
	}

CDatum CAEONStringImpl::CreateSplitItem (const char *pStart, const char *pEnd)

//	CreateSplitItem
//
//	Creates an item.

	{
	//	If null, we generate a null item (instead of an empty string), because
	//	we want to capture the semantic that the item is missing.

	if (pStart == pEnd)
		return CDatum();
	else
		return CDatum(CString(pStart, pEnd - pStart));
	}

CDatum CAEONStringImpl::ExecuteSplit (const CString& sString, const CString& sDelimiter)

//	ExecuteSplit
//
//	Splits a string by a single delimiter.

	{
	CDatum dResult(CDatum::typeArray);

	const char *pSrc = sString.GetParsePointer();
	const char *pSrcEnd = pSrc + sString.GetLength();

	const char *pStart = pSrc;
	while (pSrc < pSrcEnd)
		{
		if (StartsWith(pSrc, pSrcEnd, sDelimiter))
			{
			dResult.Append(CreateSplitItem(pStart, pSrc));

			pSrc += sDelimiter.GetLength();
			pStart = pSrc;
			}
		else
			{
			pSrc++;
			}
		}

	dResult.Append(CreateSplitItem(pStart, pSrc));

	return dResult;
	}

CDatum CAEONStringImpl::ExecuteSplitByArray (const CString& sString, CDatum dDelimiters)

//	ExecuteSplitByArray
//
//	Splits a string by an array of delimiters.

	{
	CDatum dResult(CDatum::typeArray);

	const char *pSrc = sString.GetParsePointer();
	const char *pSrcEnd = pSrc + sString.GetLength();

	const char *pStart = pSrc;
	while (pSrc < pSrcEnd)
		{
		//	See if any of the array delimiters match.

		bool bMatch = false;
		for (int i = 0; i < dDelimiters.GetCount(); i++)
			{
			const CString& sDelimiter = dDelimiters.GetElement(i);

			if (StartsWith(pSrc, pSrcEnd, sDelimiter))
				{
				dResult.Append(CreateSplitItem(pStart, pSrc));

				pSrc += sDelimiter.GetLength();
				pStart = pSrc;

				bMatch = true;
				break;
				}
			}

		if (!bMatch)
			{
			pSrc++;
			}
		}

	dResult.Append(CreateSplitItem(pStart, pSrc));

	return dResult;
	}

CDatum CAEONStringImpl::ExecuteSplitChars (const CString& sString)

//	ExecuteSplitChars
//
//	Split a string into an array of characters.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(sString.GetLength());

	const char *pPos = sString.GetParsePointer();
	const char *pEndPos = pPos + sString.GetLength();
	while (pPos < pEndPos)
		{
		UTF32 dwCodePoint = strParseUTF8Char(&pPos, pEndPos);
		dResult.Append(CDatum(strEncodeUTF8Char(dwCodePoint)));
		}

	return dResult;
	}

CDatum CAEONStringImpl::ExecuteSplitDoubleLine (const CString& sString)

//	ExecuteSplitDoubleLine
//
//	Splits a string into lines.

	{
	CDatum dResult(CDatum::typeArray);

	const char *pPos = sString.GetParsePointer();
	const char *pEndPos = pPos + sString.GetLength();

	const char *pStart = pPos;
	while (pPos < pEndPos)
		{
		if (pPos[0] == '\n' && pPos[1] == '\n')
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos += 2;
			pStart = pPos;
			}
		else if (pPos[0] == '\n' && pPos[1] == '\r' && pPos + 4 <= pEndPos && pPos[2] == '\n' && pPos[3] == '\r')
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos += 4;
			pStart = pPos;
			}
		else if (pPos[0] == '\r' && pPos[1] == '\r')
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos += 2;
			pStart = pPos;
			}
		else if (pPos[0] == '\r' && pPos[1] == '\n' && pPos + 4 <= pEndPos && pPos[2] == '\r' && pPos[3] == '\n')
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos += 4;
			pStart = pPos;
			}
		else
			pPos++;
		}

	dResult.Append(CreateSplitItem(pStart, pPos));
	return dResult;
	}

CDatum CAEONStringImpl::ExecuteSplitLines (const CString& sString)

//	ExecuteSplitLines
//
//	Splits a string into lines.

	{
	CDatum dResult(CDatum::typeArray);

	const char *pPos = sString.GetParsePointer();
	const char *pEndPos = pPos + sString.GetLength();

	const char *pStart = pPos;
	while (pPos < pEndPos)
		{
		if (*pPos == '\n')
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos++;
			if (*pPos == '\r')
				pPos++;

			pStart = pPos;
			}
		else if (*pPos == '\r')
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos++;
			if (*pPos == '\n')
				pPos++;

			pStart = pPos;
			}
		else
			pPos++;
		}

	dResult.Append(CreateSplitItem(pStart, pPos));
	return dResult;
	}

CDatum CAEONStringImpl::ExecuteSplitWhitespace (const CString& sString)

//	ExecuteSplitWhitespace
//
//	Splits a string by whitespace.

	{
	CDatum dResult(CDatum::typeArray);

	const char *pPos = sString.GetParsePointer();
	const char *pEndPos = pPos + sString.GetLength();

	const char *pStart = pPos;
	while (pPos < pEndPos)
		{
		if (strIsWhitespace(pPos))
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos++;
			while (strIsWhitespace(pPos))
				pPos++;

			pStart = pPos;
			}
		else
			pPos++;
		}

	dResult.Append(CreateSplitItem(pStart, pPos));
	return dResult;
	}

CDatum CAEONStringImpl::GetElementAt (const CString& sValue, CAEONTypeSystem &TypeSystem, CDatum dIndex)

//	GetElementAt
//
//	Returns the given element.

	{
	int iIndex = dIndex.AsArrayIndex();
	if (iIndex >= 0 && iIndex < sValue.GetLength())
		return CString(sValue.GetParsePointer() + iIndex, 1);

	else if (dIndex.IsContainer())
		{
		const char *pStart = sValue.GetParsePointer();

		CDatum dResult(CDatum::typeArray);
		dResult.GrowToFit(dIndex.GetCount());

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			int iIndex = dIndex.GetElement(i).AsArrayIndex();
			if (iIndex >= 0 && iIndex < sValue.GetLength())
				dResult.Append(CString(pStart + iIndex, 1));
			else
				dResult.Append(CDatum());
			}

		return dResult;
		}
	else
		return CDatum();
	}

bool CAEONStringImpl::StartsWith (const char *pSrc, const char *pSrcEnd, const CString& sValue)

//	StartsWith
//
//	Returns TRUE if sValue is the given start.

	{
	const char *pValue = sValue.GetParsePointer();
	const char *pValueEnd = pValue + sValue.GetLength();

	while (pSrc < pSrcEnd && pValue < pValueEnd)
		{
		if (*pSrc != *pValue)
			return false;

		pSrc++;
		pValue++;
		}

	return (pValue == pValueEnd) && (pSrc <= pSrcEnd);
	}
