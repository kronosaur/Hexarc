//	IHexeVMHost.cpp
//
//	IHexeVMHost class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include <iostream>
#include <string>

DECLARE_CONST_STRING(FIELD_API_VERSION,					"apiVersion");
DECLARE_CONST_STRING(FIELD_ARGS,						"args");
DECLARE_CONST_STRING(FIELD_AS,							"as");
DECLARE_CONST_STRING(FIELD_PROGRAM_ID,					"programID");
DECLARE_CONST_STRING(FIELD_RUN_ID,						"runID");
DECLARE_CONST_STRING(FIELD_SCHEMA,						"schema");
DECLARE_CONST_STRING(FIELD_USERNAME,					"username");

DECLARE_CONST_STRING(TYPE_ARRAY,						"array");
DECLARE_CONST_STRING(TYPE_TENSOR,						"tensor");

DECLARE_CONST_STRING(ERR_INVALID_MAP_FUNC,				"Invalid map function.");
DECLARE_CONST_STRING(ERR_INVALID_MAP_CONTINUE,			"Invalid map continue ctx.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED,					"Unsupported feature.");
DECLARE_CONST_STRING(ERR_INVALID_MAP_FUNC_TYPE,			"Invalid map function type: %s.");
DECLARE_CONST_STRING(ERR_INVALID_OPTION,				"Invalid map function option: %s.");

bool IHexeVMHost::Impl_DefaultVMLibraryInvoke (IInvokeCtx& Ctx, DWORD dwEntryPoint, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwEntryPoint)
		{
		case EPID_ARRAY_MAP:
			return Impl_ArrayMap(Ctx, LocalEnv, dContinueCtx, dContinueResult, retResult);

		case EPID_DICTIONARY_MAP:
			return Impl_DictionaryMap(Ctx, LocalEnv, dContinueCtx, dContinueResult, retResult);

		case EPID_TABLE_MAP:
			return Impl_TableMap(Ctx, LocalEnv, dContinueCtx, dContinueResult, retResult);

		case EPID_TENSOR_MAP:
			return CTensorMapProcessor::Impl_TensorMap(Ctx, LocalEnv, dContinueCtx, dContinueResult, retResult);

		default:
			return Impl_UnsuportedFeature(retResult.dResult);
		}
	}

bool IHexeVMHost::Impl_ArrayMap (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	if (dContinueCtx.IsNil())
		{
		int iArg = 0;
		CDatum dArray = LocalEnv.GetArgument(iArg++);
		CDatum dMapFunc = LocalEnv.GetArgument(iArg++);
		if (!dMapFunc.CanInvoke())
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_FUNC);
			return false;
			}

		CDatum dOptions;
		CDatum dProgressFn;

		if (LocalEnv.GetArgument(iArg).CanInvoke())
			dProgressFn = LocalEnv.GetArgument(iArg++);
		else
			{
			dOptions = LocalEnv.GetArgument(iArg++);
			dProgressFn = LocalEnv.GetArgument(iArg++);
			}

		//	Create a new processor to handle this

		CArrayMapProcessor *pProcessor = new CArrayMapProcessor(dArray, dOptions, dMapFunc);
		CDatum dProcessor(pProcessor);

		//	Invoke it.

		return pProcessor->Process(dProcessor, retResult);
		}
	else
		{
		CArrayMapProcessor *pProcessor = CArrayMapProcessor::Upconvert(dContinueCtx);
		if (pProcessor == NULL)
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_CONTINUE);
			return false;
			}

		return pProcessor->ProcessContinues(dContinueCtx, dContinueResult, retResult);
		}
	}

CDatum IHexeVMHost::Impl_CalcDictionaryMapType (IInvokeCtx &Ctx, CDatum dDictionary, CDatum dMapFunc, int& retiArgs)
	{
	//	Get some types to figure out the resulting dictionary. For the new
	//	dictionary, the key type is the same as the input dictionary, and the
	//	value type is the result of the map function.

	CDatum dType = dDictionary.GetDatatype();
	const IDatatype& Type = dType;
	CDatum dKeyType = Type.GetKeyType();
	if (dKeyType.IsNil())
		dKeyType = CAEONTypes::Get(IDatatype::ANY);		//	Generic Any key

	CDatum dValueType = Type.GetElementType();
	if (dValueType.IsNil())
		dValueType = CAEONTypes::Get(IDatatype::ANY);		//	Generic Any value

	CDatum dFuncType = dMapFunc.GetDatatype();
	const IDatatype& FuncType = dFuncType;

	//	See if the function can be called with two arguments (key and value)
	//
	//	NOTE: We don't fail if the types are wrong here because we don't have 
	//	to have runtime errors.

	CDatum dReturnType;
	if (FuncType.CanBeCalledWithArgCount(dType, 2, &dReturnType))
		retiArgs = 2;
	else
		{
		retiArgs = 1;
		if (!FuncType.CanBeCalledWithArgCount(dType, 1, &dReturnType))
			dReturnType = CAEONTypes::Get(IDatatype::ANY);
		}

	//	Create a dictionary type

	CDatum dNewType = CAEONTypeSystem::CreateAnonymousDictionary(NULL_STR, dKeyType, dReturnType);
	CDatum dExistingType = Ctx.GetTypeSystem().FindType(dNewType);
	if (!dExistingType.IsNil())
		dNewType = dExistingType;

	return dNewType;
	}

bool IHexeVMHost::Impl_DictionaryMap (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	if (dContinueCtx.IsNil())
		{
		int iArg = 0;
		CDatum dDictionary = LocalEnv.GetArgument(iArg++);
		CDatum dMapFunc = LocalEnv.GetArgument(iArg++);
		if (!dMapFunc.CanInvoke())
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_FUNC);
			return false;
			}

		//	Compute the resulting dictionary type based on the map function

		int iArgs;
		CDatum dResultType = Impl_CalcDictionaryMapType(Ctx, dDictionary, dMapFunc, iArgs);
		if (dResultType.IsError())
			{
			retResult.dResult = dResultType;
			return false;
			}

		//	Create a new processor to handle this

		CDictionaryMapProcessor *pProcessor = new CDictionaryMapProcessor(dDictionary, CDatum(), dMapFunc, iArgs, dResultType);
		CDatum dProcessor(pProcessor);

		//	Invoke it.

		return pProcessor->Process(dProcessor, retResult);
		}
	else
		{
		CDictionaryMapProcessor *pProcessor = CDictionaryMapProcessor::Upconvert(dContinueCtx);
		if (pProcessor == NULL)
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_CONTINUE);
			return false;
			}

		return pProcessor->ProcessContinues(dContinueCtx, dContinueResult, retResult);
		}
	}

CDatum IHexeVMHost::Impl_GetSystemObject ()
	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_API_VERSION, 1);
	dResult.SetElement(FIELD_ARGS, CDatum());
	dResult.SetElement(FIELD_PROGRAM_ID, CDatum());
	dResult.SetElement(FIELD_RUN_ID, ::GetCurrentProcessId());
	dResult.SetElement(FIELD_USERNAME, CDatum());

	return dResult;
	}

bool IHexeVMHost::Impl_StdIn (const IInvokeCtx::SInputOptions &Options, CDatum& retdResult)
	{
	static constexpr int MAX_SIZE = 2048;

	if (!Options.sPrompt.IsEmpty())
		printf("%s", (LPSTR)Options.sPrompt);

	std::wstring input;
	std::getline(std::wcin, input);
	retdResult = CDatum(CString(CString16(input.c_str(), -1)));
	return true;
	}

void IHexeVMHost::Impl_StdOut (CDatum dValue)
	{
	CString sLine = dValue.AsString();
	printf("%s\n", (LPSTR)sLine);
	}

bool IHexeVMHost::Impl_TableMap (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	if (dContinueCtx.IsNil())
		{
		int iArg = 0;
		CDatum dTable = LocalEnv.GetArgument(iArg++);
		CDatum dSchema;
		if (LocalEnv.GetArgument(iArg).GetBasicType() == CDatum::typeDatatype)
			dSchema = LocalEnv.GetArgument(iArg++);

		CDatum dMapFunc = LocalEnv.GetArgument(iArg++);

		CDatum dOptions;
		CDatum dProgressFn;

		if (LocalEnv.GetArgument(iArg).CanInvoke())
			dProgressFn = LocalEnv.GetArgument(iArg++);
		else
			{
			dOptions = LocalEnv.GetArgument(iArg++);
			dProgressFn = LocalEnv.GetArgument(iArg++);
			}

		if (!dSchema.IsNil())
			{
			if (dOptions.GetBasicType() != CDatum::typeStruct)
				dOptions = CDatum(CDatum::typeStruct);

			dOptions.SetElement(FIELD_SCHEMA, dSchema);
			}

		//	Create a new processor to handle this

		CTableMapProcessor *pProcessor = new CTableMapProcessor(Ctx.GetTypeSystem(), dTable, dOptions, dMapFunc);
		CDatum dProcessor(pProcessor);

		//	Invoke it.

		return pProcessor->Process(dProcessor, retResult);
		}
	else
		{
		CTableMapProcessor *pProcessor = CTableMapProcessor::Upconvert(dContinueCtx);
		if (pProcessor == NULL)
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_CONTINUE);
			return false;
			}

		return pProcessor->ProcessContinues(dContinueCtx, dContinueResult, retResult);
		}
	}

bool IHexeVMHost::Impl_UnsuportedFeature (CDatum &retdResult) const 
	{
	retdResult = ERR_UNSUPPORTED;
	return false;
	}
