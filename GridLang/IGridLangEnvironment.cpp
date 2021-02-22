//	IGridLangEnvironment.cpp
//
//	IGridLangEnvironment Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(LIBRARY_GRID_LANG_ENV,				"gridLangEnv");

IGridLangEnvironment *IGridLangEnvironment::Get (IInvokeCtx &Ctx, CDatum *retdResult)
	{
	IGridLangEnvironment *pEnv = (IGridLangEnvironment *)Ctx.GetLibraryCtx(LIBRARY_GRID_LANG_ENV);
	if (!pEnv)
		{
		CHexeError::Create(NULL_STR, strPattern("No GridLang environment."), retdResult);
		return NULL;
		}

	return pEnv;
	}
