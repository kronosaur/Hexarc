//	CAEONNilImpl.cpp
//
//	CAEONNilImpl class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

TDatumPropertyHandler<int> CAEONNilImpl::m_Properties = {
	{
		"length",
		"Returns the length of the string in bytes.",
		[](const int& iDummy, const CString &sProperty)
			{
			return CDatum();
			},
		NULL,
		},
	};

TDatumMethodHandler<int> CAEONNilImpl::m_Methods = {
	{
		"find",
		"*",
		".find(x) -> Finds offset of x in string.",
		0,
		[](int& iDummy, IInvokeCtx& Ctx, const CString& sMethod, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
			{
			retdResult = CDatum();
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

