//	AI1Lib.cpp
//
//	AI1 HexeLisp library
//	Copyright (c) 2015 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"
#include "Hexe.h"

DECLARE_CONST_STRING(LIBRARY_AI1,						"ai1")

bool ai1Misc (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult);

const DWORD AI1_PRINT =									0;
DECLARE_CONST_STRING(AI1_PRINT_NAME,					"print")
DECLARE_CONST_STRING(AI1_PRINT_ARGS,					"*")
DECLARE_CONST_STRING(AI1_PRINT_HELP,					"(print ...)")

const DWORD AI1_READ_FILE =								1;
DECLARE_CONST_STRING(AI1_READ_FILE_NAME,				"readFile")
DECLARE_CONST_STRING(AI1_READ_FILE_ARGS,				"*")
DECLARE_CONST_STRING(AI1_READ_FILE_HELP,				"(readFile filespec [options]) -> data")

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_AI1LibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(AI1_PRINT, ai1Misc),
	DECLARE_DEF_LIBRARY_FUNC(AI1_READ_FILE, ai1Misc),
	};

const int g_iAI1LibraryDefCount = SIZEOF_STATIC_ARRAY(g_AI1LibraryDef);
bool g_bRegistered = false;

void RegisterAI1Library (void)
	{
	if (!g_bRegistered)
		{
		g_bRegistered = true;
		g_HexeLibrarian.RegisterLibrary(LIBRARY_AI1, g_iAI1LibraryDefCount, g_AI1LibraryDef);
		}
	}

bool ai1Misc (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult)
	{
	int i;

	switch (dwData)
		{
		case AI1_PRINT:
			{
			CStringBuffer Result;
			for (i = 0; i < dLocalEnv.GetCount(); i++)
				Result.Write(dLocalEnv.GetElement(i).AsString());

			printf("%s\n", Result.GetPointer());
			*retdResult = CDatum(CDatum::constTrue);
			return true;
			}

		case AI1_READ_FILE:
			{
			CString sFilespec = dLocalEnv.GetElement(0).AsString();

			CString sError;
			if (!CDatum::CreateFromFile(sFilespec, CDatum::formatTextUTF8, retdResult, &sError))
				{
				CHexeError::Create(NULL_STR, sError, retdResult);
				return false;
				}

			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

