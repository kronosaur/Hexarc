//	CAEONStringImpl.cpp
//
//	CAEONStringImpl class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_TYPE,						"type");

DECLARE_CONST_STRING(TYPE_DOUBLE_LINE,					"doubleLine");
DECLARE_CONST_STRING(TYPE_LINE,							"line");
DECLARE_CONST_STRING(TYPE_WHITESPACE,					"whitespace");

DECLARE_CONST_STRING(ERR_INVALID_SPLIT_TYPE,			"Invalid split type: %s.");
DECLARE_CONST_STRING(ERR_INVALID_PARAM,					"Invalid parameter: %s.");

TDatumPropertyHandler<LPCSTR> CAEONStringImpl::m_Properties = {
	{
		"bytes",
		"a",
		"Returns an array of bytes.",
		[](LPCSTR pValue, const CString &sProperty)
			{
			CStringView sValue = CStringView::FromCStringPtr(pValue);
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
		"a",
		"Returns an array of Unicode characters.",
		[](LPCSTR pValue, const CString &sProperty)
			{
			return ExecuteSplitChars(CStringView::FromCStringPtr(pValue));
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the string.",
		[](LPCSTR pValue, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::STRING);
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the length of the string in bytes.",
		[](LPCSTR pValue, const CString &sProperty)
			{
			return CDatum(CStringView::FromCStringPtr(pValue).GetLength());
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the length of the string in bytes.",
		[](LPCSTR pValue, const CString &sProperty)
			{
			return CDatum(CStringView::FromCStringPtr(pValue).GetLength());
			},
		NULL,
		},
	};

TDatumMethodHandler<char> CAEONStringImpl::m_Methods = {
	{
		"cleaned",
		"s:",
		".cleaned() -> string",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(CStringView::FromCStringPtr(&Value).Clean());
			return true;
			},
		},
	{
		"edited",
		"s:pos=?,replace=s|start=?,len=?|start=?,len=?,replace=s",
		".edited(start, end, replace) -> string.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CDatum dStart = LocalEnv.GetArgument(1);
			int iStart = (int)dStart;
			if (iStart < 0)
				iStart = Max(0, iStart + sValue.GetLength());

			if (iStart > sValue.GetLength())
				{
				retResult.dResult = CDatum();
				return true;
				}

			CDatum dEnd = LocalEnv.GetArgument(2);
			if (dEnd.GetBasicType() == CDatum::typeString)
				{
				CStringView sReplace = dEnd;

				CString sResult(sValue.GetLength() + sReplace.GetLength());
				char *pDest = sResult.GetPointer();
				utlMemCopy(sValue.GetParsePointer(), pDest, iStart);
				pDest += iStart;
				utlMemCopy(sReplace.GetParsePointer(), pDest, sReplace.GetLength());
				pDest += sReplace.GetLength();
				utlMemCopy(sValue.GetParsePointer() + iStart, pDest, sValue.GetLength() - iStart);

				retResult.dResult = CDatum(sResult);
				return true;
				}
			else
				{
				CStringView sReplace = LocalEnv.GetArgument(3);
				int iLen = Max(0, Min((int)dEnd, sValue.GetLength() - iStart));

				CString sResult(sValue.GetLength() - iLen + sReplace.GetLength());
				char *pDest = sResult.GetPointer();
				utlMemCopy(sValue.GetParsePointer(), pDest, iStart);
				pDest += iStart;
				utlMemCopy(sReplace.GetParsePointer(), pDest, sReplace.GetLength());
				pDest += sReplace.GetLength();
				utlMemCopy(sValue.GetParsePointer() + iStart + iLen, pDest, sValue.GetLength() - iStart - iLen);

				retResult.dResult = CDatum(sResult);
				return true;
				}
			},
		},
	{
		"endsWith",
		"b:str=?",
		".endsWith(string) -> true/false (case insensitive)",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CString sStr = LocalEnv.GetArgument(1).AsString();
			retResult.dResult = CDatum(strEndsWithNoCase(sValue, sStr));
			return true;
			},
		},
	{
		"endsWithExact",
		"b:str=?",
		".endsWithExact(string) -> true/false (case sensitive)",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CString sStr = LocalEnv.GetArgument(1).AsString();
			retResult.dResult = CDatum(strEndsWith(sValue, sStr));
			return true;
			},
		},
	{
		"find",
		"?:str=?",
		".find(x) -> Finds offset of x in string (case insensitive).",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CDatum dValueToFind = LocalEnv.GetArgument(1);
			if (dValueToFind.IsNil())
				{
				retResult.dResult = CDatum();
				return true;
				}

			int iIndex = strFindNoCase(sValue, dValueToFind.AsString());
			if (iIndex == -1)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = iIndex;
			return true;
			},
		},
	{
		"findExact",
		"?:str=?",
		".findExact(x) -> Finds offset of x in string (case sensitive).",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CDatum dValueToFind = LocalEnv.GetArgument(1);
			if (dValueToFind.IsNil())
				{
				retResult.dResult = CDatum();
				return true;
				}

			int iIndex = strFind(sValue, dValueToFind.AsString());
			if (iIndex == -1)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = iIndex;
			return true;
			},
		},
	{
		"format",
		"s:data=x|*=?",
		".format(data) -> string.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CDatum dData = LocalEnv.GetArgument(1);
			if (dData.IsStruct() || dData.IsArray())
				retResult.dResult = CDatumFormat::FormatParams(sValue, dData);
			else
				retResult.dResult = CDatumFormat::FormatParamsByOrdinal(sValue, LocalEnv);

			return true;
			},
		},
	{
		"hashed",
		"v:|options=?",
		".hashed() -> BLAKE2 hash.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CBuffer Buffer(sValue);
			retResult.dResult = CDatum::CreateBinary(cryptoBLAKE2(Buffer));
			return true;
			},
		},
	{
		"left",
		"s:chars=?",
		".left(n) -> left n bytes.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			int iLen = Min((int)LocalEnv.GetArgument(1), sValue.GetLength());
			if (iLen <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = strSubString(sValue, 0, iLen);
			return true;
			},
		},
	{
		"lowercased",
		"s:",
		".lowercased() -> string in lowercase.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			retResult.dResult = strToLower(sValue);
			return true;
			},
		},
	{
		"repeat",
		"s:copies=?",
		"DEPRECATED: Use .repeated() instead.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			int iCount = (int)LocalEnv.GetArgument(1);
			if (iCount <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}
			else if (iCount == 1)
				{
				retResult.dResult = CDatum(sValue);
				return true;
				}
			else
				{
				CStringBuffer Buffer;
				Buffer.SetLength(sValue.GetLength() * iCount);
				char *pDest = Buffer.GetPointer();

				for (int i = 0; i < iCount; i++)
					{
					utlMemCopy(sValue.GetParsePointer(), pDest, sValue.GetLength());
					pDest += sValue.GetLength();
					}

				retResult.dResult = CDatum(std::move(Buffer));
				return true;
				}
			},
		},
	{
		"repeated",
		"s:copies=?",
		".repeated(n) -> string.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			int iCount = (int)LocalEnv.GetArgument(1);
			if (iCount <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}
			else if (iCount == 1)
				{
				retResult.dResult = sValue;
				return true;
				}
			else
				{
				CStringBuffer Buffer;
				Buffer.SetLength(sValue.GetLength() * iCount);
				char *pDest = Buffer.GetPointer();

				for (int i = 0; i < iCount; i++)
					{
					utlMemCopy(sValue.GetParsePointer(), pDest, sValue.GetLength());
					pDest += sValue.GetLength();
					}

				retResult.dResult = CDatum(std::move(Buffer));
				return true;
				}
			},
		},
	{
		"right",
		"s:chars=?",
		".right(n) -> right n bytes.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			int iLen = Min((int)LocalEnv.GetArgument(1), sValue.GetLength());
			if (iLen <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = strSubString(sValue, sValue.GetLength() - iLen, iLen);
			return true;
			},
		},
	{
		"slice",
		"s:start=?|start=?,end=?",
		"DEPRECATED: Use sliced() instead.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			int iStart;
			int iLen;
			CAEONOp::CalcSliceParams(LocalEnv.GetArgument(1), LocalEnv.GetArgument(2), sValue.GetLength(), iStart, iLen);

			if (iLen <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = strSubString(sValue, iStart, iLen);
			return true;
			},
		},
	{
		"sliced",
		"s:start=?|start=?,end=?",
		".sliced(start, [end]) -> string.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			int iStart;
			int iLen;
			CAEONOp::CalcSliceParams(LocalEnv.GetArgument(1), LocalEnv.GetArgument(2), sValue.GetLength(), iStart, iLen);

			if (iLen <= 0)
				{
				retResult.dResult = CDatum();
				return true;
				}

			retResult.dResult = strSubString(sValue, iStart, iLen);
			return true;
			},
		},
	{
		"split",
		"a:|options=?",
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
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CDatum dOptions = LocalEnv.GetArgument(1);
			if (dOptions.IsNil())
				{
				retResult.dResult = ExecuteSplitChars(sValue);
				return true;
				}
			else if (dOptions.GetBasicType() == CDatum::typeString)
				{
				retResult.dResult = ExecuteSplit(sValue, dOptions.AsStringView());
				return true;
				}
			else if (dOptions.GetBasicType() == CDatum::typeArray || dOptions.GetBasicType() == CDatum::typeTensor)
				{
				retResult.dResult = ExecuteSplitByArray(sValue, dOptions);
				return true;
				}
			else if (dOptions.GetBasicType() == CDatum::typeStruct)
				{
				CStringView sType = dOptions.GetElement(FIELD_TYPE);
				if (strEqualsNoCase(sType, TYPE_DOUBLE_LINE))
					{
					retResult.dResult = ExecuteSplitDoubleLine(sValue);
					return true;
					}
				else if (strEqualsNoCase(sType, TYPE_LINE))
					{
					retResult.dResult = ExecuteSplitLines(sValue);
					return true;
					}
				else if (strEqualsNoCase(sType, TYPE_WHITESPACE))
					{
					retResult.dResult = ExecuteSplitWhitespace(sValue);
					return true;
					}
				else
					{
					retResult.dResult = strPattern(ERR_INVALID_SPLIT_TYPE, sType);
					return false;
					}
				}
			else
				{
				retResult.dResult = strPattern(ERR_INVALID_PARAM, dOptions.AsString());
				return false;
				}
			},
		},
	{
		"startsWith",
		"b:str=?",
		".startsWith(string) -> true/false (case insensitive)",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CString sStr = LocalEnv.GetArgument(1).AsString();
			retResult.dResult = CDatum(strStartsWithNoCase(sValue, sStr));
			return true;
			},
		},
	{
		"startsWithExact",
		"b:str=?",
		".startsWithExact(string) -> true/false (case sensitive)",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			CString sStr = LocalEnv.GetArgument(1).AsString();
			retResult.dResult = CDatum(strStartsWith(sValue, sStr));
			return true;
			},
		},
	{
		"toLowerCase",
		"s:",
		"DEPRECATED: Use .lowercased() instead.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			retResult.dResult = strToLower(sValue);
			return true;
			},
		},
	{
		"toUpperCase",
		"s:",
		"DEPRECATED: Use .uppercased() instead.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			retResult.dResult = strToUpper(sValue);
			return true;
			},
		},
	{
		"uppercased",
		"s:",
		".uppercased() -> string in uppercase.",
		0,
		[](char& Value, IInvokeCtx& Ctx, const CString& sMethod, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			CStringView sValue = CStringView::FromCStringPtr(&Value);

			retResult.dResult = strToUpper(sValue);
			return true;
			},
		},
	};

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
			CStringView sDelimiter = dDelimiters.GetElement(i);

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
		const char *pNext = pPos;
		UTF32 dwCodePoint = strParseUTF8Char(&pNext, pEndPos);

		if (strIsWhitespaceChar(dwCodePoint))
			{
			dResult.Append(CreateSplitItem(pStart, pPos));

			pPos = pNext;
			while (pPos < pEndPos)
				{
				dwCodePoint = strParseUTF8Char(&pNext, pEndPos);
				if (!strIsWhitespaceChar(dwCodePoint))
					break;

				pPos = pNext;
				}

			pStart = pPos;
			}
		else
			pPos = pNext;
		}

	dResult.Append(CreateSplitItem(pStart, pPos));
	return dResult;
	}

CDatum CAEONStringImpl::GetElementAt (const CString& sValue, CAEONTypeSystem &TypeSystem, CDatum dIndex)

//	GetElementAt
//
//	Returns the given element.

	{
	if (dIndex.IsContainer())
		{
		const char* pStart = sValue.GetParsePointer();

		CString sResult(dIndex.GetCount());
		char* pDestStart = sResult.GetPointer();
		char* pDest = pDestStart;

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			int iIndex = dIndex.GetElement(i).AsArrayIndex(sValue.GetLength());
			if (iIndex >= 0 && iIndex < sValue.GetLength())
				{
				*pDest++ = *(pStart + iIndex);
				}
			}

		sResult.SetLength(pDest - pDestStart);

		return CDatum(sResult);
		}
	else
		{
		int iIndex = dIndex.AsArrayIndex(sValue.GetLength());
		if (iIndex >= 0 && iIndex < sValue.GetLength())
			return CString(sValue.GetParsePointer() + iIndex, 1);
		else
			return CDatum();
		}
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
