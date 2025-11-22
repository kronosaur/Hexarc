//	COpConcatenate.cpp
//
//	COpConcatenate class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_OP_NOT_DEFINED,				"Operator undefined for %s & %s.");

CAEONOperatorTableNew COpConcatenate::CreateTable ()

//	CreateTable
//
//	Returns a table of operator implementation functions.

	{
	//	By default for anything we don't handle we treat the two operands as
	//	scalars and combine them into an array.

	CAEONOperatorTableNew Table(ExecScalar_Scalar);

	//	If the array is on the left, then we default to treating the right as
	//	a scalar.

	Table.SetOpLeft(IDatatype::ARRAY, ExecArray_Scalar);
	Table.SetOp(IDatatype::ARRAY, IDatatype::OBJECT, ExecArray_Object);

	//	If an array is on the right, then we default to treating the left as a
	//	scalar.

	Table.SetOpRight(IDatatype::ARRAY, ExecScalar_Array);

	//	Array
	//
	//	We handle arrays on both sides.

	Table.SetOp(IDatatype::ARRAY, IDatatype::ARRAY, ExecArray_Array);

	//	Classes & schemas

	Table.SetOp(IDatatype::CLASS_T, IDatatype::CLASS_T, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::CLASS_T, IDatatype::SCHEMA, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::CLASS_T, IDatatype::STRUCT, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::SCHEMA, IDatatype::CLASS_T, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::SCHEMA, IDatatype::SCHEMA, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::SCHEMA, IDatatype::STRUCT, ExecClassInstance_ClassInstance);

	//	Null

	Table.SetOpLeft(IDatatype::NULL_T, ExecNull_Any);
	Table.SetOpRight(IDatatype::NULL_T, ExecAny_Null);

	//	Object
	//
	//	TextLines is treated like an object, and we need to do some special 
	//	handling.

	Table.SetOp(IDatatype::OBJECT, IDatatype::OBJECT, ExecObject_Object);
	Table.SetOp(IDatatype::OBJECT, IDatatype::ARRAY, ExecObject_Array);
	Table.SetOp(IDatatype::OBJECT, IDatatype::STRING, ExecObject_String);

	//	String

	Table.SetOp(IDatatype::STRING, IDatatype::OBJECT, ExecString_Object);

	//	Struct

	Table.SetOp(IDatatype::STRUCT, IDatatype::CLASS_T, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::STRUCT, IDatatype::SCHEMA, ExecClassInstance_ClassInstance);
	Table.SetOp(IDatatype::STRUCT, IDatatype::STRUCT, ExecStruct_Struct);
	Table.SetOp(IDatatype::STRUCT, IDatatype::TABLE, ExecStruct_Table);

	//	Table

	Table.SetOp(IDatatype::TABLE, IDatatype::TABLE, ExecTable_Table);
	Table.SetOp(IDatatype::TABLE, IDatatype::STRUCT, ExecTable_Struct);

	//	Done

	return Table;
	}

CDatum COpConcatenate::ExecAny_Null (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	//	If the left is collection, then we just return it.

	const IDatatype& LeftType = dLeft.GetDatatype();
	if (LeftType.IsA(IDatatype::ARRAY) || LeftType.IsA(IDatatype::TABLE) || LeftType.IsA(IDatatype::DICTIONARY))
		return dLeft;

	//	If the left is also null, then we return an empty array.

	else if (dLeft.IsNil())
		return CDatum(CDatum::typeArray);

	//	Otherwise, we create an array with one element.

	else
		{
		CDatum dNewDatatype = CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dLeft.GetDatatype());
		CDatum dResult = CDatum::CreateArrayAsType(dNewDatatype);
		dResult.Append(dLeft);

		return dResult;
		}
	}

CDatum COpConcatenate::CalcType (CDatum dLeftType, CDatum dRightType)
	{
	const IDatatype& LeftType = dLeftType;
	const IDatatype& RightType = dRightType;

	//	If either side is null, then the result is the other side.

	if (LeftType.IsNullType())
		{
		if (RightType.IsA(IDatatype::ARRAY))
			return dRightType;
		else
			return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY);
		}
	else if (RightType.IsNullType())
		{
		if (LeftType.IsA(IDatatype::ARRAY))
			return dLeftType;
		else
			return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY);
		}

	//	If either side is an array, then we return an array.

	else if (LeftType.IsA(IDatatype::ARRAY))
		return dLeftType;
	else if (RightType.IsA(IDatatype::ARRAY))
		return dRightType;

	//	If either side is a table, then we return a table.

	else if (LeftType.IsA(IDatatype::TABLE))
		return dLeftType;
	else if (RightType.IsA(IDatatype::TABLE))
		return dRightType;

	//	If both sides are class instances, then we may be able to combine them.

	else if ((LeftType.IsA(IDatatype::CLASS_T) && RightType.IsA(IDatatype::CLASS_T))
			|| (LeftType.IsA(IDatatype::SCHEMA) && RightType.IsA(IDatatype::SCHEMA))
			|| (LeftType.IsA(IDatatype::SCHEMA) && RightType.IsA(IDatatype::STRUCT))
			|| (LeftType.IsA(IDatatype::STRUCT) && RightType.IsA(IDatatype::SCHEMA)))
		{
		//	If both types are the same, then we can combine them.

		if (LeftType == RightType)
			return dLeftType;

		//	If the left type is-a right type, then we can combine them.

		else if (LeftType.IsA(RightType))
			return dRightType;

		//	If the right type is-a left type, then we can combine them.

		else if (RightType.IsA(LeftType))
			return dLeftType;

		//	Otherwise, we return an array.

		else
			return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY);
		}

	//	If one side is a class instance and the other side is a struct, then we 
	//	end up with a class instance.

#if 0
	else if ((LeftType.IsA(IDatatype::CLASS_T) || LeftType.IsA(IDatatype::SCHEMA)) && RightType.IsA(IDatatype::STRUCT))
		return dLeftType;

	else if ((RightType.IsA(IDatatype::CLASS_T) || RightType.IsA(IDatatype::SCHEMA)) && LeftType.IsA(IDatatype::STRUCT))
		return dRightType;
#endif

	//	If both sides are structs, then we return a struct

	else if (LeftType.IsA(IDatatype::STRUCT) && RightType.IsA(IDatatype::STRUCT))
		return dLeftType;

	//	Otherwise, we return an array.

	else
		return CAEONTypeSystem::GetCoreType(IDatatype::ARRAY);
	}

CDatum COpConcatenate::ExecArray_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return dLeft.OpConcatenated(Ctx, dRight);
	}

CDatum COpConcatenate::ExecClassInstance_ClassInstance (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	const IDatatype& LeftType = dLeft.GetDatatype();
	const IDatatype& RightType = dRight.GetDatatype();

	//	If both types are the same, then we can combine them.

	if (LeftType == RightType)
		{
		CDatum dResult = dLeft.Clone();
		dResult.Append(dRight);
		return dResult;
		}

	//	If the left type is-a right type, then we can combine them.

	else if (LeftType.IsA(RightType))
		{
		CDatum dResult = CDatum::CreateAsType(dRight.GetDatatype(), dLeft);
		dResult.Append(dRight);
		return dResult;
		}

	//	If the right type is-a left type, then we can combine them.

	else if (RightType.IsA(LeftType))
		{
		CDatum dResult = dLeft.Clone();
		dResult.Append(dRight);
		return dResult;
		}

	//	Otherwise, just combine them into an array

	else
		return ExecScalar_Scalar(Ctx, dLeft, dRight);
	}

CDatum COpConcatenate::ExecArray_Object (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	//	If we're concatenating an array and text lines, then we treat the array
	//	like an array of strings.

	if (dRight.GetBasicType() == CDatum::typeTextLines)
		{
		int iResultCount = dLeft.GetCount() + dRight.GetCount();
		if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
			return CAEONOp::ErrorTooBig(Ctx);

		CDatum dResult = CDatum::CreateTextLines();
		dResult.GrowToFit(iResultCount);
		dResult.Append(dLeft);
		dResult.Append(dRight);

		return dResult;
		}
	else
		return ExecArray_Scalar(Ctx, dLeft, dRight);
	}

CDatum COpConcatenate::ExecArray_Scalar (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	int iArrayCount = dLeft.GetCount();
	int iResultCount = iArrayCount + 1;
	if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
		return CAEONOp::ErrorTooBig(Ctx);

	//	Make sure the type is compatible.

	CDatum dNewDatatype = CalcDatatypeArray_Scalar(dLeft.GetDatatype(), dRight.GetDatatype());

	//	Create the new array.

	CDatum dResult = CDatum::CreateArrayAsType(dNewDatatype);
	dResult.GrowToFit(iResultCount);

	for (int i = 0; i < dLeft.GetCount(); i++)
		dResult.Append(dLeft.GetElement(i));

	dResult.Append(dRight);

	return dResult;
	}

CDatum COpConcatenate::ExecError (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return CDatum::CreateError(strPattern(ERR_OP_NOT_DEFINED, ((const IDatatype&)dLeft.GetDatatype()).GetName(), ((const IDatatype&)dRight.GetDatatype()).GetName()));
	}

CDatum COpConcatenate::ExecNull_Any (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	return ExecAny_Null(Ctx, dRight, dLeft);
	}

CDatum COpConcatenate::ExecObject_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	//	If we're concatenating an array and text lines, then we treat the array
	//	like an array of strings.

	if (dLeft.GetBasicType() == CDatum::typeTextLines)
		{
		int iResultCount = dLeft.GetCount() + dRight.GetCount();
		if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
			return CAEONOp::ErrorTooBig(Ctx);

		CDatum dResult = CDatum::CreateTextLines();
		dResult.GrowToFit(iResultCount);
		dResult.Append(dLeft);
		dResult.Append(dRight);

		return dResult;
		}
	else
		return ExecScalar_Array(Ctx, dLeft, dRight);
	}

CDatum COpConcatenate::ExecObject_Object (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	//	If both sides are a text lines object, then concatenate them.

	if (dLeft.GetBasicType() == CDatum::typeTextLines && dRight.GetBasicType() == CDatum::typeTextLines)
		{
		int iResultCount = dLeft.GetCount() + dRight.GetCount();
		if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
			return CAEONOp::ErrorTooBig(Ctx);
		
		CDatum dResult = CDatum::CreateTextLines();
		dResult.GrowToFit(iResultCount);
		dResult.Append(dLeft);
		dResult.Append(dRight);
		
		return dResult;
		}

	//	Otherwise, combine both objects in a generic array.

	else
		return ExecScalar_Scalar(Ctx, dLeft, dRight);
	}

CDatum COpConcatenate::ExecObject_String (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	if (dLeft.GetBasicType() == CDatum::typeTextLines)
		{
		CDatum dResult = dLeft.Clone();
		dResult.Append(dResult);
		return dResult;
		}
	else
		return ExecScalar_Scalar(Ctx, dLeft, dRight);
	}

CDatum COpConcatenate::ExecScalar_Array (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	int iArrayCount = dRight.GetCount();
	int iResultCount = iArrayCount + 1;
	if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
		return CAEONOp::ErrorTooBig(Ctx);

	//	Make sure the type is compatible.

	CDatum dNewDatatype = CalcDatatypeArray_Scalar(dRight.GetDatatype(), dLeft.GetDatatype());

	//	Create the new array.

	CDatum dResult = CDatum::CreateArrayAsType(dNewDatatype);
	dResult.GrowToFit(iResultCount);

	dResult.Append(dLeft);

	for (int i = 0; i < dRight.GetCount(); i++)
		dResult.Append(dRight.GetElement(i));

	return dResult;
	}

CDatum COpConcatenate::ExecScalar_Scalar (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	//	Figure out the array datatype.

	CDatum dLeftElementType = dLeft.GetDatatype();
	const IDatatype& LeftElementType = dLeftElementType;

	CDatum dRightElementType = dRight.GetDatatype();
	const IDatatype& RightElementType = dRightElementType;

	CDatum dNewDatatype;
	if (RightElementType.IsA(LeftElementType))
		dNewDatatype = CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dLeftElementType);

	//	If the array element is a subtype of the scalar, then we create a new
	//	array type for the scalar.

	else if (LeftElementType.IsA(RightElementType))
		dNewDatatype = CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dRightElementType);

	//	Otherwise, generic type.

	else
		dNewDatatype = CAEONTypes::Get(IDatatype::ARRAY);

	//	Create the new array.

	CDatum dResult = CDatum::CreateArrayAsType(dNewDatatype);
	dResult.GrowToFit(2);

	dResult.Append(dLeft);
	dResult.Append(dRight);

	return dResult;
	}

CDatum COpConcatenate::ExecString_Object (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	if (dRight.GetBasicType() == CDatum::typeTextLines)
		{
		int iResultCount = 1 + dRight.GetCount();
		if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
			return CAEONOp::ErrorTooBig(Ctx);

		CDatum dResult = CDatum::CreateTextLines();
		dResult.GrowToFit(iResultCount);
		dResult.Append(dLeft);
		dResult.Append(dRight);

		return dResult;
		}
	else
		return ExecScalar_Scalar(Ctx, dLeft, dRight);
	}

CDatum COpConcatenate::ExecStruct_Struct (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	int iResultCount = dLeft.GetCount() + dRight.GetCount();
	if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
		return CAEONOp::ErrorTooBig(Ctx);

	CDatum dResult(CDatum::typeStruct);
	dResult.GrowToFit(iResultCount);

	dResult.Append(dLeft);
	dResult.Append(dRight);

	return dResult;
	}

CDatum COpConcatenate::ExecStruct_Table (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	if (dRight.GetCount() + 1 > Ctx.GetLimits().iMaxArrayLen)
		return CAEONOp::ErrorTooBig(Ctx);

	CDatum dResult = CDatum::CreateTable(dRight.GetDatatype());
	dResult.Append(dLeft);
	dResult.Append(dRight);

	return dResult;
	}

CDatum COpConcatenate::ExecTable_Struct (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	if (dLeft.GetCount() + 1 > Ctx.GetLimits().iMaxArrayLen)
		return CAEONOp::ErrorTooBig(Ctx);

	CDatum dResult = dLeft.Clone();
	dResult.Append(dRight);

	return dResult;
	}

CDatum COpConcatenate::ExecTable_Table (IInvokeCtx& Ctx, CDatum dLeft, CDatum dRight)
	{
	int iResultCount = dLeft.GetCount() + dRight.GetCount();
	if (iResultCount > Ctx.GetLimits().iMaxArrayLen)
		return CAEONOp::ErrorTooBig(Ctx);

	//	Combine the schemas for the two tables.

	CDatum dSchema;
	if (!IAEONTable::CombineSchema(dLeft.GetDatatype(), dRight.GetDatatype(), dSchema))
		return CDatum();

	CDatum dResult = CDatum::CreateTable(dSchema);
	dResult.Append(dLeft);
	dResult.Append(dRight);

	return dResult;
	}

CDatum COpConcatenate::CalcDatatypeArray_Scalar (CDatum dArrayType, CDatum dScalarType)

//	CalcDatatypeArray_Scalar
//
//	dArrayType is the datatype of an array. dScalarType is a scalar type. We 
//	return the array datatype that results from concatenating the scalar to the
//	array.

	{
	//	Make sure the type is compatible.

	const IDatatype& ArrayType = dArrayType;
	CDatum dArrayElementType = ArrayType.GetElementType();
	const IDatatype& ArrayElementType = dArrayElementType;

	const IDatatype& ScalarElementType = dScalarType;

	//	If the scalar is a subtype or equal to the array element type, then we
	//	use that original array type.

	CDatum dNewDatatype;
	if (ScalarElementType.IsA(ArrayElementType))
		return dArrayType;

	//	If the array element is a subtype of the scalar, then we create a new
	//	array type for the scalar.

	else if (ArrayElementType.IsA(ScalarElementType))
		return CAEONTypeSystem::CreateAnonymousArray(NULL_STR, dScalarType);

	//	Otherwise, generic type.

	else
		return CAEONTypes::Get(IDatatype::ARRAY);
	}

