//	CGridLangCoreLibrary.cpp
//
//	CGridLangCoreLibrary Class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(LIBRARY_GRID_LANG_CORE,			"gridLangCore");

DECLARE_CONST_STRING(PORT_CON,							"CON");

bool consoleMisc (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult);

const DWORD CONSOLE_INPUT =								0;
DECLARE_CONST_STRING(CONSOLE_INPUT_NAME,				"input");
DECLARE_CONST_STRING(CONSOLE_INPUT_ARGS,				"*");
DECLARE_CONST_STRING(CONSOLE_INPUT_HELP,				"input() -> data\n"
														"input({prompt}) -> data");

const DWORD CONSOLE_PRINT =								1;
DECLARE_CONST_STRING(CONSOLE_PRINT_NAME,				"print");
DECLARE_CONST_STRING(CONSOLE_PRINT_ARGS,				"*");
DECLARE_CONST_STRING(CONSOLE_PRINT_HELP,				"print(...) -> True");

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_GridLangLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(CONSOLE_INPUT, consoleMisc, 0),
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
		case CONSOLE_INPUT:
			{
			IGridLangEnvironment *pEnv = IGridLangEnvironment::Get(*pCtx, retdResult);
			if (!pEnv)
				return false;

			CString sPrompt;
			if (dLocalEnv.GetCount() > 0)
				sPrompt = dLocalEnv.GetElement(0);
			else
				sPrompt = CString("> ");

			return pEnv->GetInput(PORT_CON, sPrompt, retdResult);
			}

		case CONSOLE_PRINT:
			{
			IGridLangEnvironment *pEnv = IGridLangEnvironment::Get(*pCtx, retdResult);
			if (!pEnv)
				return false;

			if (dLocalEnv.GetCount() == 1)
				{
				pEnv->Output(PORT_CON, dLocalEnv.GetElement(0));
				}
			else
				{
				//	Concatenate the args

				CStringBuffer Buffer;

				for (int i = 0; i < dLocalEnv.GetCount(); i++)
					{
					CString sString = dLocalEnv.GetElement(i).AsString();
					Buffer.Write((LPSTR)sString, sString.GetLength());
					}

				CDatum dResult;
				CDatum::CreateStringFromHandoff(Buffer, &dResult);

				pEnv->Output(PORT_CON, dResult);
				}

			*retdResult = CDatum(CDatum::constTrue);
			return true;
			}

		default:
			throw CException(errFail);
		}
	}
