//	CodeSlingerLib.cpp
//
//	CodeSlinger Library
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(PORT_CON,							"CON");

DECLARE_CONST_STRING(LIBRARY_CODE_SLINGER,				"codeSlinger");

DECLARE_CONST_STRING(LIBRARY_INSTANCE_CTX,				"instanceCtx");

bool codeImpl (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult);

const DWORD CODE_PRINT =								0;
DECLARE_CONST_STRING(CODE_PRINT_NAME,					"print");
DECLARE_CONST_STRING(CODE_PRINT_ARGS,					"s");
DECLARE_CONST_STRING(CODE_PRINT_HELP,					"(print ...) -> string");

CProgramInstance *GetProgramInstance (IInvokeCtx &Ctx, CDatum *retdResult);

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_CodeSlingerLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(CODE_PRINT, codeImpl, 0),
	};

const int g_iCodeSlingerLibraryDefCount = SIZEOF_STATIC_ARRAY(g_CodeSlingerLibraryDef);
static bool g_bRegistered = false;

void RegisterCodeSlingerLibrary (void)
	{
	if (!g_bRegistered)
		{
		g_bRegistered = true;
		g_HexeLibrarian.RegisterLibrary(LIBRARY_CODE_SLINGER, g_iCodeSlingerLibraryDefCount, g_CodeSlingerLibraryDef);
		}
	}

bool codeImpl (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult)
	{
	switch (dwData)
		{
		case CODE_PRINT:
			{
			CProgramInstance *pInstance = GetProgramInstance(*pCtx, retdResult);
			if (!pInstance)
				return false;

			if (dLocalEnv.GetCount() == 1)
				{
				pInstance->Output(PORT_CON, dLocalEnv.GetElement(0));
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

				pInstance->Output(PORT_CON, dResult);
				}

			*retdResult = CDatum(CDatum::constTrue);
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

CProgramInstance *GetProgramInstance (IInvokeCtx &Ctx, CDatum *retdResult)
	{
	CProgramInstance *pInstance = (CProgramInstance *)Ctx.GetLibraryCtx(LIBRARY_INSTANCE_CTX);
	if (!pInstance)
		{
		CHexeError::Create(NULL_STR, strPattern("No instanceCtx"), retdResult);
		return NULL;
		}

	return pInstance;
	}