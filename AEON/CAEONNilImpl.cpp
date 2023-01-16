//	CAEONNilImpl.cpp
//
//	CAEONNilImpl class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

TDatumPropertyHandler<int> CAEONNilImpl::m_Properties = {
	{
		"bytes",
		"Returns an array of bytes.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(CDatum::typeArray);
			},
		NULL,
		},
	{
		"chars",
		"Returns an array of Unicode characters.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(CDatum::typeArray);
			},
		NULL,
		},
	{
		"length",
		"Returns the length of the string in bytes.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum(0);
			},
		NULL,
		},
	};

TDatumMethodHandler<int> CAEONNilImpl::m_Methods = {
	{
		"endsWith",
		"*",
		".endsWith(string) -> true/false (case insensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = dLocalEnv.GetElement(0).IsNil();
			return true;
			},
		},
	{
		"endsWithExact",
		"*",
		".endsWithExact(string) -> true/false (case sensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = dLocalEnv.GetElement(0).IsNil();
			return true;
			},
		},
	{
		"find",
		"*",
		".find(x) -> Finds offset of x in string (case insensitive).",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	{
		"findExact",
		"*",
		".findExact(x) -> Finds offset of x in string (case sensitive).",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	{
		"join",
		"*",
		".join([separator, [lastSeparator]]) -> string.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum(NULL_STR);
			return true;
			},
		},
	{
		"left",
		"*",
		".left(n) -> left n bytes.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	{
		"right",
		"*",
		".right(n) -> right n bytes.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	{
		"slice",
		"*",
		".slice(start, [end]) -> slice",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	{
		"sort",
		"*",
		".sort() -> sorted array, ascending\n"
		".sort(-1) -> sorted array, descending\n",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"split",
		"*",
		".split() -> array",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum(CDatum::typeArray);
			return true;
			},
		},
	{
		"startsWith",
		"*",
		".startsWith(string) -> true/false (case insensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = dLocalEnv.GetElement(0).IsNil();
			return true;
			},
		},
	{
		"startsWithExact",
		"*",
		".startsWithExact(string) -> true/false (case sensitive)",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = dLocalEnv.GetElement(0).IsNil();
			return true;
			},
		},
	{
		"toLowerCase",
		"*",
		".toLowerCase() -> string in lowercase.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	{
		"toUpperCase",
		"*",
		".toUpperCase() -> string in uppercase.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
			return true;
			},
		},
	};

