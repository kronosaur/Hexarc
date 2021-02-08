//	CGridLangCoreLibrary.cpp
//
//	CGridLangCoreLibrary Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(LIBRARY_GRID_LANG_CORE,			"gridLangCore");

bool consoleMisc (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult);

const DWORD CONSOLE_PRINT =								0;
DECLARE_CONST_STRING(CONSOLE_PRINT_NAME,				"print");
DECLARE_CONST_STRING(CONSOLE_PRINT_ARGS,				"*");
DECLARE_CONST_STRING(CONSOLE_PRINT_HELP,				"(print ...) -> True");

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_GridLangLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(CONSOLE_PRINT, consoleMisc, 0),
	};

const int g_iGridLangLibraryDefCount = SIZEOF_STATIC_ARRAY(g_GridLangLibraryDef);

bool CGridLangCoreLibrary::m_bRegistered = false;

void CGridLangCoreLibrary::Define (const IGLType &IsA, CGLTypeNamespace &Namespace)

//	Define
//
//	Define all functions

	{
	for (int i = 0; i < g_iGridLangLibraryDefCount; i++)
		{
		if (!Namespace.Insert(IGLType::CreateFunction(IsA, NULL, g_GridLangLibraryDef[i].sName)))
			//	This only happens if we have a duplicate name, which means the
			//	library is poorly formed.
			throw CException(errFail);
		}
	}

void CGridLangCoreLibrary::Register ()

//	Register
//
//	Make sure the library is registered.

	{
	if (!m_bRegistered)
		{
		m_bRegistered = true;
		g_HexeLibrarian.RegisterLibrary(LIBRARY_GRID_LANG_CORE, g_iGridLangLibraryDefCount, g_GridLangLibraryDef);
		}
	}

bool consoleMisc (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult)
	{
	switch (dwData)
		{
		case CONSOLE_PRINT:
			{
			const CString &sLine = dLocalEnv.GetElement(0);
			printf("%s\n", (LPSTR)sLine);
			return true;
			}

		default:
			throw CException(errFail);
		}
	}
