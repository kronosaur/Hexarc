//	CAEONStringImpl.cpp
//
//	CAEONStringImpl class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

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
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(sValue.GetLength());

			const char *pPos = sValue.GetParsePointer();
			const char *pEndPos = pPos + sValue.GetLength();
			while (pPos < pEndPos)
				{
				UTF32 dwCodePoint = strParseUTF8Char(&pPos, pEndPos);
				dResult.Append(CDatum(strEncodeUTF8Char(dwCodePoint)));
				}

			return dResult;
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
