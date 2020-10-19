//	CodeSlingerLib.cpp
//
//	CodeSlinger Library
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(FIELD_CORES,						"cores")
DECLARE_CONST_STRING(FIELD_GRID_OPTIONS,				"gridOptions")
DECLARE_CONST_STRING(FIELD_HEIGHT,						"height")
DECLARE_CONST_STRING(FIELD_MSG,							"msg");
DECLARE_CONST_STRING(FIELD_PAYLOAD,						"payload");
DECLARE_CONST_STRING(FIELD_TYPE,						"type")
DECLARE_CONST_STRING(FIELD_WIDTH,						"width")

DECLARE_CONST_STRING(MSG_CODE_MANDELBROT,				"Code.mandelbrot");

DECLARE_CONST_STRING(PORT_CON,							"CON");

DECLARE_CONST_STRING(LIBRARY_CODE_SLINGER,				"codeSlinger");

DECLARE_CONST_STRING(LIBRARY_INSTANCE_CTX,				"instanceCtx");

DECLARE_CONST_STRING(TYPE_HEXARC_MSG,					"hexarcMsg");

bool codeImpl (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult);

const DWORD CODE_MANDELBROT =							0;
DECLARE_CONST_STRING(CODE_MANDELBROT_NAME,				"mandelbrot");
DECLARE_CONST_STRING(CODE_MANDELBROT_ARGS,				"s");
DECLARE_CONST_STRING(CODE_MANDELBROT_HELP,				"(mandelbrot x y zoom options) -> string");

const DWORD CODE_PRINT =								1;
DECLARE_CONST_STRING(CODE_PRINT_NAME,					"print");
DECLARE_CONST_STRING(CODE_PRINT_ARGS,					"s");
DECLARE_CONST_STRING(CODE_PRINT_HELP,					"(print ...) -> string");

CHexeLispEnvironment *GetProgramInstance (IInvokeCtx &Ctx, CDatum *retdResult);

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_CodeSlingerLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(CODE_MANDELBROT, codeImpl, 0),
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
		case CODE_MANDELBROT:
			{
			CDatum dX = dLocalEnv.GetElement(0);
			CDatum dY = dLocalEnv.GetElement(1);
			CDatum dZoom = dLocalEnv.GetElement(2);
			CDatum dOptions = dLocalEnv.GetElement(3);

			double rPixelSize;
			double rZoom = dZoom;
			if (rZoom <= 1.0)
				rPixelSize = 0.0025;
			else
				rPixelSize = 0.0025/ rZoom;

			int cxWidth = (int)dOptions.GetElement(FIELD_WIDTH);
			if (cxWidth <= 0)
				cxWidth = 1000;

			int cyHeight = (int)dOptions.GetElement(FIELD_HEIGHT);
			if (cyHeight <= 0)
				cyHeight = 1000;

			int iCores = Max(0, (int)dOptions.GetElement(FIELD_CORES));

			CDatum dGridOptions(CDatum::typeStruct);
			dGridOptions.SetElement(FIELD_CORES, iCores);

			CDatum dInvokeOptions(CDatum::typeStruct);
			dInvokeOptions.SetElement(FIELD_GRID_OPTIONS, dGridOptions);

			CDatum dPayload(CDatum::typeArray);
			if (!dX.IsNil()) 
				dPayload.Append(dX);
			else 
				dPayload.Append(-0.5); 

			if (!dY.IsNil())
				dPayload.Append(dY);
			else
				dPayload.Append(0.0);

			dPayload.Append(rPixelSize);
			dPayload.Append(cxWidth);
			dPayload.Append(cyHeight);
			dPayload.Append(dInvokeOptions);

			//	Returns a structure to send a Hexarc message.

			CDatum dResult(CDatum::typeStruct);
			dResult.SetElement(FIELD_TYPE, TYPE_HEXARC_MSG);
			dResult.SetElement(FIELD_MSG, MSG_CODE_MANDELBROT);
			dResult.SetElement(FIELD_PAYLOAD, dPayload);

			*retdResult = dResult;

			//	FALSE means special result.

			return false;
			}

		case CODE_PRINT:
			{
			CHexeLispEnvironment *pInstance = GetProgramInstance(*pCtx, retdResult);
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

CHexeLispEnvironment *GetProgramInstance (IInvokeCtx &Ctx, CDatum *retdResult)
	{
	CHexeLispEnvironment *pInstance = (CHexeLispEnvironment *)Ctx.GetLibraryCtx(LIBRARY_INSTANCE_CTX);
	if (!pInstance)
		{
		CHexeError::Create(NULL_STR, strPattern("No instanceCtx"), retdResult);
		return NULL;
		}

	return pInstance;
	}