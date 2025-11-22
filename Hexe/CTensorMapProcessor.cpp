//	CTensorMapProcessor.cpp
//
//	CTensorMapProcessor Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ALLOW_NULL,					"allowNull");
DECLARE_CONST_STRING(FIELD_AS,							"as");

DECLARE_CONST_STRING(TYPE_ARRAY,						"array");
DECLARE_CONST_STRING(TYPE_TENSOR,						"tensor");

DECLARE_CONST_STRING(TYPENAME_TENSOR_MAP_PROC,			"tensorMapProcessor");
const CString& CTensorMapProcessor::StaticGetTypename (void) { return TYPENAME_TENSOR_MAP_PROC; }

DECLARE_CONST_STRING(ERR_INVALID_MAP_FUNC,				"Invalid map function.");
DECLARE_CONST_STRING(ERR_INVALID_MAP_CONTINUE,			"Invalid map continue ctx.");
DECLARE_CONST_STRING(ERR_INVALID_OPTION,				"Invalid map function option: %s.");

CTensorMapProcessor::CTensorMapProcessor (CDatum dTensor, CDatum dOptions, CDatum dMapFunc, int iFuncArgs, CDatum dResultType) :
		m_dTensor(dTensor),
		m_dOptions(dOptions),
		m_dMapFunc(dMapFunc),
		m_iFuncArgs(iFuncArgs),
		m_bAllowNull(dOptions.GetElement(FIELD_ALLOW_NULL).AsBool()),
		m_iResultType(ParseResultType(dOptions.GetElement(FIELD_AS).AsString()))

//	CArrayMapProcess constructor

	{
	switch (m_iResultType)
		{
		case EResultType::Tensor:
			m_dResult = CDatum::CreateTensorAsType(dResultType);
			break;

		default:
			m_dResult = CDatum::CreateArrayAsType(dResultType);
			break;
		}
	}

CDatum CTensorMapProcessor::CalcTensorMapType (IInvokeCtx& Ctx, CDatum dTensor, CDatum dMapFunc, EResultType iResultType, int& retiArgs)
	{
	//	Get some types to figure out the resulting tensor type.

	CDatum dType = dTensor.GetDatatype();
	const IDatatype& Type = dType;
	CDatum dElementType = Type.GetElementType();
	if (dElementType.IsNil())
		dElementType = CAEONTypes::Get(IDatatype::ANY);		//	Generic Any key

	CDatum dFuncType = dMapFunc.GetDatatype();
	const IDatatype& FuncType = dFuncType;

	//	See if the function can be called with two arguments (key and value)
	//
	//	NOTE: We don't fail if the types are wrong here because we don't want 
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

	//	Create a tensor type with the same dimensions as the input tensor
	//	but the element type based on the function return type.

	switch (iResultType)
		{
		case EResultType::Tensor:
			return Ctx.GetTypeSystem().AddAnonymousTensor(dReturnType, Type.GetDimensionTypes());

		default:
			return Ctx.GetTypeSystem().AddAnonymousArray(dReturnType);
		}
	}

bool CTensorMapProcessor::Impl_TensorMap (IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	if (dContinueCtx.IsNil())
		{
		CDatum dTensor = LocalEnv.GetArgument(0);
		CDatum dOptions;
		CDatum dMapFunc;

		//	If the first method argument is a struct, then it is options. 
		//	Otherwise, we expect the map function.

		if (LocalEnv.GetArgument(1).IsStruct())
			{
			dOptions = LocalEnv.GetArgument(1);
			dMapFunc = LocalEnv.GetArgument(2);
			}
		else
			{
			dMapFunc = LocalEnv.GetArgument(1);
			dOptions = LocalEnv.GetArgument(2);
			}

		if (!dMapFunc.CanInvoke())
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_FUNC);
			return false;
			}

		//	Figure out what type of result we want.

		auto iResultType = CTensorMapProcessor::ParseResultType(dOptions.GetElement(FIELD_AS).AsStringView());
		if (iResultType == CTensorMapProcessor::EResultType::Unknown)
			{
			retResult.dResult = CDatum::CreateError(strPattern(ERR_INVALID_OPTION, dOptions.GetElement(FIELD_AS).AsString()));
			return false;
			}

		//	Compute the resulting tensor type based on the map function. We also
		//	get back the number of arguments in the map function, because we 
		//	want to call it with the right arguments.

		int iArgs;
		CDatum dResultType = CalcTensorMapType(Ctx, dTensor, dMapFunc, iResultType, iArgs);
		if (dResultType.IsError())
			{
			retResult.dResult = dResultType;
			return false;
			}

		//	Create a new processor to handle this

		CTensorMapProcessor *pProcessor = new CTensorMapProcessor(dTensor, dOptions, dMapFunc, iArgs, dResultType);
		CDatum dProcessor(pProcessor);

		//	Invoke it.

		return pProcessor->Process(dProcessor, retResult);
		}
	else
		{
		CTensorMapProcessor *pProcessor = CTensorMapProcessor::Upconvert(dContinueCtx);
		if (pProcessor == NULL)
			{
			retResult.dResult = CDatum::CreateError(ERR_INVALID_MAP_CONTINUE);
			return false;
			}

		return pProcessor->ProcessContinues(dContinueCtx, dContinueResult, retResult);
		}
	}

void CTensorMapProcessor::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
	m_dTensor.Mark();
	m_dOptions.Mark();
	m_dMapFunc.Mark();
	m_dResult.Mark();
	}

bool CTensorMapProcessor::Process (CDatum dSelf, SAEONInvokeResult &retResult)

//	Process
//
//	We return under the following conditions:
//
//	1.	If processing has completed, we return TRUE and retResult has
//		the result (mapped array).
//
//	2.	If we need to call a lambda function to continue processing, we return
//		FALSE and retResult has the proper calling parameters (ready for
//		Compute.
//
//	3.	If there is an error we return FALSE and retResult is an error string.

	{
	//	Must be a tensor. If not, we just return nil

	if (m_dTensor.GetBasicType() != CDatum::typeTensor)
		{
		retResult.dResult = CDatum();
		return true;
		}
	else if (m_dTensor.GetCount() == 0)
		{
		retResult.dResult = m_dResult;
		return true;
		}

	m_Pos = m_dTensor.raw_IteratorStart();

	//	Run the mapping function

	if (m_iFuncArgs == 1)
		return CHexe::RunFunction1Arg(m_dMapFunc, m_dTensor.raw_IteratorGetElement(m_Pos), dSelf, retResult);
	else
		return CHexe::RunFunction2Args(m_dMapFunc, m_dTensor.raw_IteratorGetKey(m_Pos), m_dTensor.raw_IteratorGetElement(m_Pos), dSelf, retResult);
	}

bool CTensorMapProcessor::ProcessContinues (CDatum dSelf, CDatum dResult, SAEONInvokeResult &retResult)

//	ProcessContinues
//
//	Handle the next element.

	{
	//	Add the mapped result.

	if (!dResult.IsIdenticalToNil() || m_bAllowNull)
		{
		switch (m_iResultType)
			{
			case EResultType::Tensor:
				m_dResult.raw_IteratorSetElement(m_Pos, dResult);
				break;

			default:
				m_dResult.Append(dResult);
				break;
			}
		}

	//	Are we done?

	m_dTensor.raw_IteratorNext(m_Pos);
	if (!m_dTensor.raw_IteratorHasMore(m_Pos))
		{
		retResult.dResult = m_dResult;
		return true;
		}

	//	Continue mapping

	if (m_iFuncArgs == 1)
		return CHexe::RunFunction1Arg(m_dMapFunc, m_dTensor.raw_IteratorGetElement(m_Pos), dSelf, retResult);
	else
		return CHexe::RunFunction2Args(m_dMapFunc, m_dTensor.raw_IteratorGetKey(m_Pos), m_dTensor.raw_IteratorGetElement(m_Pos), dSelf, retResult);
	}

CTensorMapProcessor::EResultType CTensorMapProcessor::ParseResultType (CStringView sType)

//	ParseResultType
//
//	Parse a result type string.

	{
	if (sType.IsEmpty() || strEqualsNoCase(sType, TYPE_ARRAY))
		return EResultType::Array;
	else if (strEqualsNoCase(sType, TYPE_TENSOR))
		return EResultType::Tensor;
	else
		return EResultType::Unknown;
	}
