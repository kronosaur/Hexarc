//	CDatatypeTensor.cpp
//
//	CDatatypeTensor class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_ARRAY,							"array");

DECLARE_CONST_STRING(ERR_DIMENSION_COUNT_MISMATCH,		"Expected %d dimensions.");
DECLARE_CONST_STRING(ERR_INVALID_DIM_TYPES,				"Expected dimension types: %s.");
DECLARE_CONST_STRING(ERR_EXPECT_INTEGER_INDEX,			"Expected integer index.");

CDatatypeTensor::CDatatypeTensor (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType),
		m_dElementType(Create.dElementType)

//	CDatatypeTensor constructor

	{
	//	Initialize dimensions.

	if (Create.Dimensions.GetCount() == 0)
		throw CException(errFail);

	m_Dims.InsertEmpty(Create.Dimensions.GetCount());
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		InitDimFromType(i, Create.Dimensions[i], m_Dims[i]);
		}

	m_dSliceType = CalcSliceType(m_dElementType, m_Dims);
	}

CDatatypeTensor::CDatatypeTensor (CStringView sFullyQualifiedName, CDatum dElementType, int iRows, int iCols, DWORD dwCoreType) : IDatatype(sFullyQualifiedName, dwCoreType),
		m_dElementType(dElementType)
	{
	InitDims(iRows, iCols);
	}

CDatum CDatatypeTensor::CalcSliceType (CDatum dElementType, const TArray<SDimDesc>& Dims)

//	CalcSliceType
//
//	Create the datatype of a slice if you dereference the tensor with the given dimensions.

	{
	if (Dims.GetCount() < 2)
		return CDatum();

	TArray<CDatum> DimTypes;
	for (int i = 1; i < Dims.GetCount(); i++)
		DimTypes.Insert(Dims[i].dType);

	return CAEONTypes::CreateTensor(NULL_STR, dElementType, DimTypes);
	}

void CDatatypeTensor::InitDimFromType (int iOrdinal, CDatum dType, SDimDesc& retDim)
	{
	const IDatatype& DimType = dType;
	retDim.iOrdinal = iOrdinal;
	retDim.dType = dType;

	if (DimType.GetCoreType() == IDatatype::INTEGER)
		{
		retDim.iStart = 0;
		retDim.iLength = 0;
		}
	else if (DimType.GetClass() == IDatatype::ECategory::Enum)
		{
		retDim.iStart = 0;
		retDim.iLength = DimType.GetMemberCount();
		retDim.bEnum = true;
		}
	else
		{
		SNumberDesc NumberDesc = DimType.GetNumberDesc();
		if (!NumberDesc.bNumber || !NumberDesc.bSubRange)
			throw CException(errFail);

		retDim.iStart = NumberDesc.iSubRangeMin;
		retDim.iLength = NumberDesc.iSubRangeMax - NumberDesc.iSubRangeMin + 1;
		}
	}

void CDatatypeTensor::InitDims (int iRows, int iCols)
	{
	m_Dims.DeleteAll();

	//	Dynamic 2D array

	if (iRows == 0 || iCols == 0)
		{
		m_Dims.InsertEmpty(2);
		m_Dims[0].iOrdinal = 0;
		m_Dims[0].dType = CAEONTypes::Get(IDatatype::INTEGER);
		m_Dims[0].iStart = 0;
		m_Dims[0].iLength = 0;

		m_Dims[1].iOrdinal = 1;
		m_Dims[1].dType = CAEONTypes::Get(IDatatype::INTEGER);
		m_Dims[1].iStart = 0;
		m_Dims[1].iLength = 0;
		}

	//	1D array

	else if (iRows == 1 && iCols > 1)
		{
		m_Dims.InsertEmpty(1);
		m_Dims[0].iOrdinal = 0;
		m_Dims[0].dType = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iCols - 1);
		m_Dims[0].iStart = 0;
		m_Dims[0].iLength = iCols;
		}

	//	2D array

	else if (iRows > 1 && iCols > 1)
		{
		m_Dims.InsertEmpty(2);
		m_Dims[0].iOrdinal = 0;
		m_Dims[0].dType = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iRows - 1);
		m_Dims[0].iStart = 0;
		m_Dims[0].iLength = iRows;

		m_Dims[1].iOrdinal = 1;
		m_Dims[1].dType = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iCols - 1);
		m_Dims[1].iStart = 0;
		m_Dims[1].iLength = iCols;

		//	Create the slice type

		TArray<CDatum> DimTypes;
		DimTypes.Insert(m_Dims[1].dType);
		m_dSliceType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, DimTypes);
		}
	else
		throw CException(errFail);
	}

bool CDatatypeTensor::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const
	{
	if (ArgTypes.GetCount() > m_Dims.GetCount())
		{
		if (retsError) *retsError = strPattern(ERR_DIMENSION_COUNT_MISMATCH, m_Dims.GetCount());
		return false;
		}

	//	Make sure all the types match.

	bool bAnyResult = false;
	TArray<CDatum> SliceDims;
	for (int i = 0; i < Min(m_Dims.GetCount(), ArgTypes.GetCount()); i++)
		{
		const IDatatype& DimType = m_Dims[i].dType;
		const IDatatype& IndexType = ArgTypes[i];

		//	If the index is an integer, then we skip.

		if (IndexType.IsA(IDatatype::NUMBER))
			{ }

		//	Or if it is an enum

		else if (IndexType.GetClass() == IDatatype::ECategory::Enum)
			{
			if (!DimType.IsA(IDatatype::ENUM))
				{
				if (retsError) *retsError = strPattern(ERR_INVALID_DIM_TYPES, DimType.GetName());
				return false;
				}
			}

		//	If the index type is * then we add this entire dimension to the slice type.

		else if (IndexType.IsA(IDatatype::BOOL))
			SliceDims.Insert(m_Dims[i].dType);

		//	Otherwise, we don't know what it is so we assume a dimensions of any size.

		else
			{
			bAnyResult = true;
			SliceDims.Insert(CAEONTypes::Get(IDatatype::INTEGER));
			}

		//	If we expect an integer of some kind, then we accept any number or range (or Any).

		if (DimType.IsA(IDatatype::INTEGER))
			{
			//	We expect a number, a range, or a boolean (true means everything), or Any.

			if (!IndexType.IsA(IDatatype::NUMBER) 
					&& !IndexType.IsA(IDatatype::RANGE) 
					&& !IndexType.IsA(IDatatype::BOOL) 
					&& !IndexType.IsA(IDatatype::ARRAY)
					&& !IndexType.IsAny())
				{
				if (retsError) *retsError = ERR_EXPECT_INTEGER_INDEX;
				return false;
				}

			continue;
			}

		//	If the types match, the OK.

		else if (IndexType.IsA(DimType))
			{
			continue;
			}

		//	Otherwise, we compose an error.

		if (retsError)
			{
			//	Compose a string of the input argument types.

			CString sArgTypes;
			for (int j = 0; j < m_Dims.GetCount(); j++)
				{
				CString sType = ((const IDatatype&)m_Dims[j].dType).GetName();
				if (j == 0)
					sArgTypes = sType;
				else
					sArgTypes = strPattern("%s, %s", sArgTypes, sType);
				}

			*retsError = strPattern(ERR_INVALID_DIM_TYPES, sArgTypes);
			}

		return false;
		}

	//	Add extra dimensions to the slice type is we have fewer args than dimensions.
	//	We do this by adding the extra dimensions to the slice type.

	for (int i = ArgTypes.GetCount(); i < m_Dims.GetCount(); i++)
		SliceDims.Insert(m_Dims[i].dType);

	//	If we have a slice, then return the slice type. Otherwise we return the
	//	element type.

	if (bAnyResult)
		{
		if (retdReturnType)
			*retdReturnType = CAEONTypes::Get(IDatatype::ANY);
		}
	else if (SliceDims.GetCount() > 0)
		{
		if (retdReturnType)
			*retdReturnType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, SliceDims);
		}
	else
		{
		if (retdReturnType)
			*retdReturnType = m_dElementType;
		}

	return true;
	}

bool CDatatypeTensor::OnCanBeConstructedFrom (CDatum dType) const
	{
	const IDatatype& OtherType = dType;

	//	We always support construction from Any. This allows us to have type
	//	annotations be optional.

	if (OtherType.IsAny())
		return true;

	if (IsA(dType))
		return true;

	//	The other type must be an array or tensor.

	if (OtherType.GetClass() != IDatatype::ECategory::Tensor
			&& OtherType.GetClass() != IDatatype::ECategory::Array)
		return false;

	//	If our element types are not compatible, then we can't be constructed from it.

	const IDatatype& ElementType = GetElementType();
	if (!OtherType.IsAny() && !ElementType.CanBeConstructedFrom(OtherType.GetElementType()))
		return false;

	//	If different number of dimensions, then we can't be constructed from it.

	if (m_Dims.GetCount() != OtherType.GetDimensionTypes().GetCount())
		return false;

	//	Otherwise, we can be constructed from it.

	return true;
	}

bool CDatatypeTensor::OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion)
	{
	SetCoreType(Stream.ReadDWORD());

	if (!CComplexDatatype::CreateFromStream(Stream, m_dElementType))
		return false;

	int iRows = Stream.ReadInt();
	int iCols = Stream.ReadInt();
	InitDims(iRows, iCols);

	return true;
	}

bool CDatatypeTensor::OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized)
	{
	m_dElementType = CDatum::DeserializeAEON(Stream, Serialized);

	if (dwVersion == 1)
		{
		int iRows = Stream.ReadInt();
		int iCols = Stream.ReadInt();
		InitDims(iRows, iCols);
		}
	else
		{
		DWORD dwFlags = Stream.ReadDWORD();

		int iDims = Stream.ReadInt();
		m_Dims.InsertEmpty(iDims);
		for (int i = 0; i < iDims; i++)
			{
			CDatum dType = CDatum::DeserializeAEON(Stream, Serialized);
			InitDimFromType(i, dType, m_Dims[i]);
			}

		m_dSliceType = CalcSliceType(m_dElementType, m_Dims);
		}

	return true;
	}

bool CDatatypeTensor::OnEquals (const IDatatype &Src) const
	{
	auto &Other = (const CDatatypeTensor &)Src;

	if (GetCoreType() != Other.GetCoreType())
		return false;

	if ((const IDatatype &)m_dElementType != (const IDatatype &)Other.m_dElementType)
		return false;

	if (m_Dims.GetCount() != Other.m_Dims.GetCount())
		return false;

	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		if ((const IDatatype &)m_Dims[i].dType != (const IDatatype &)Other.m_Dims[i].dType)
			return false;
		}

	return true;
	}

TArray<CDatum> CDatatypeTensor::OnGetDimensionTypes () const
	{
	TArray<CDatum> Result;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		Result.Insert(m_Dims[i].dType);

	return Result;
	}

CString CDatatypeTensor::OnGetName () const
	{
	if (m_Dims.GetCount() == 0)
		return STR_ARRAY;
	else if (IsAnonymous())
		{
		CStringBuffer Output;
		Output.Write(STR_ARRAY);

		Output.WriteChar('[');
		for (int i = 0; i < m_Dims.GetCount(); i++)
			{
			if (i != 0)
				{
				Output.WriteChar(',');
				Output.WriteChar(' ');
				}

			if (m_Dims[i].iLength == 0)
				Output.WriteChar('*');
			else if (m_Dims[i].bEnum)
				Output.Write(strPattern("%s", ((const IDatatype&)m_Dims[i].dType).GetName()));
			else if (m_Dims[i].iStart == 0)
				{
				Output.Write(strPattern("%d", m_Dims[i].iLength));
				}
			else
				{
				Output.Write(strPattern("%d...%d", m_Dims[i].iStart, m_Dims[i].iStart + m_Dims[i].iLength - 1));
				}
			}
		Output.WriteChar(']');

		const IDatatype& ElementType = m_dElementType;
		if (!ElementType.IsAny())
			{
			Output.Write(strPattern(" of %s", ElementType.GetName()));
			}

		return CString(std::move(Output));
		}
	else
		return DefaultGetName();
	}

bool CDatatypeTensor::OnIsA (const IDatatype &Type) const
	{
	//	We implement the following abstract types

	switch (Type.GetCoreType())
		{
		case IDatatype::ARRAY:
			return true;

		case IDatatype::INDEXED:
		case IDatatype::MUTABLE_INDEXED:
			return true;
		}

	//	Otherwise, see if we're an array of the same type or subtype.

	switch (Type.GetClass())
		{
		case IDatatype::ECategory::Tensor:
			{
			if (GetClass() != Type.GetClass())
				return false;

			CDatum dElementType = Type.GetElementType();
			if (!((const IDatatype&)m_dElementType).IsA(dElementType))
				return false;

			TArray<CDatum> OtherDims = Type.GetDimensionTypes();
			if (OtherDims.GetCount() != m_Dims.GetCount())
				return false;

			for (int i = 0; i < m_Dims.GetCount(); i++)
				{
				const IDatatype& DimType = m_Dims[i].dType;
				if (!DimType.IsA(OtherDims[i]))
					return false;
				}

			return true;
			}
		}

	return false;
	}

void CDatatypeTensor::OnMark () 
	{
	m_dElementType.Mark();
	m_dSliceType.Mark();
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		m_Dims[i].dType.Mark();
		}
	}

void CDatatypeTensor::OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_dElementType.SerializeAEON(Stream, Serialized);

	DWORD dwFlags = 0;
	Stream.Write(dwFlags);

	Stream.Write(m_Dims.GetCount());
	for (int i = 0; i < m_Dims.GetCount(); i++)
		m_Dims[i].dType.SerializeAEON(Stream, Serialized);
	}
