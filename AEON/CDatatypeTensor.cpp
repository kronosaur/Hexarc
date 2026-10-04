//	CDatatypeTensor.cpp
//
//	CDatatypeTensor class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_ARRAY,							"array");

DECLARE_CONST_STRING(ERR_DIMENSION_COUNT_MISMATCH,		"Expected %d dimensions.");
DECLARE_CONST_STRING(ERR_INVALID_DIM_TYPES,				"Expected dimension types: %s.");
DECLARE_CONST_STRING(ERR_EXPECT_INTEGER_INDEX,			"Expected integer index.");

CDatatypeTensor::CDatatypeTensor (const SCreate &Create) : IDatatype(Create.bBuiltIn, Create.sFullyQualifiedName, Create.dwCoreType),
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

CDatatypeTensor::CDatatypeTensor (bool bBuiltIn, CStringView sFullyQualifiedName, CDatum dElementType, int iRows, int iCols, DWORD dwCoreType) : IDatatype(bBuiltIn, sFullyQualifiedName, dwCoreType),
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

	return CAEONTypes::CreateTensor(NULL_STR, dElementType, std::move(DimTypes));
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
		m_dSliceType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, std::move(DimTypes));
		}
	else
		throw CException(errFail);
	}

TArray<CDatum> CDatatypeTensor::MakeDimensions (int iRows, int iCols)
	{
	TArray<CDatum> Dims;

	//	Dynamic 2D array

	if (iRows == 0 || iCols == 0)
		{
		Dims.InsertEmpty(2);
		Dims[0] = CAEONTypes::Get(IDatatype::INTEGER);
		Dims[1] = CAEONTypes::Get(IDatatype::INTEGER);
		}

	//	1D array

	else if (iRows == 1 && iCols > 1)
		{
		Dims.InsertEmpty(1);
		Dims[0] = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iCols - 1);
		}

	//	2D array

	else if (iRows > 1 && iCols > 1)
		{
		Dims.InsertEmpty(2);
		Dims[0] = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iRows - 1);
		Dims[1] = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iCols - 1);
		}
	else
		throw CException(errFail);

	return Dims;
	}

bool CDatatypeTensor::OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType, CString* retsError) const
	{
	if (ArgTypes.GetCount() > m_Dims.GetCount())
		{
		if (retsError) *retsError = strPattern(ERR_DIMENSION_COUNT_MISMATCH, m_Dims.GetCount());
		return false;
		}

	//	Make sure all the types match and compute the shape of the result.

	bool bAnyResult = false;
	TArray<CDatum> SliceDims;
	for (int i = 0; i < Min(m_Dims.GetCount(), ArgTypes.GetCount()); i++)
		{
		const IDatatype& DimType = m_Dims[i].dType;
		const IDatatype& IndexType = ArgTypes[i];
		CDatum dLiteral = (i < ArgLiteralTypes.GetCount() ? ArgLiteralTypes[i] : CDatum());

		//	Any might be either a scalar coordinate or a slice, so we cannot know
		//	the static result rank.

		if (IndexType.IsAny())
			{
			bAnyResult = true;
			continue;
			}

		//	A wildcard selects the complete dimension.

		else if (IndexType.IsA(IDatatype::WILDCARD))
			{
			SliceDims.Insert(m_Dims[i].dType);
			continue;
			}

		//	For compatibility, literal true also selects the complete dimension. A
		//	general Bool is ambiguous because false is not a tensor coordinate.

		else if (IndexType.IsA(IDatatype::BOOL))
			{
			if (!dLiteral.IsIdenticalToTrue())
				{
				if (retsError) *retsError = ERR_EXPECT_INTEGER_INDEX;
				return false;
				}

			SliceDims.Insert(m_Dims[i].dType);
			continue;
			}

		//	A range produces one result dimension. Preserve its exact length when
		//	we have a literal; otherwise use an unspecified fixed dimension.

		else if (IndexType.IsA(IDatatype::RANGE))
			{
			if (!DimType.IsA(IDatatype::INTEGER))
				{
				if (retsError) *retsError = strPattern(ERR_INVALID_DIM_TYPES, DimType.GetName());
				return false;
				}

			const IAEONRange* pRange = (dLiteral.GetBasicType() == CDatum::typeRange ? dLiteral.GetRangeInterface() : NULL);
			if (pRange && pRange->GetLength() > 0)
				SliceDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, pRange->GetLength() - 1));
			else
				SliceDims.Insert(CAEONTypes::Get(IDatatype::INTEGER));

			continue;
			}

		//	An array of coordinates also produces one result dimension.

		else if (IndexType.IsA(IDatatype::ARRAY))
			{
			const IDatatype& IndexElementType = IndexType.GetElementType();
			if (DimType.IsA(IDatatype::INTEGER))
				{
				if (!IndexElementType.IsAny()
						&& !IndexElementType.IsA(IDatatype::NUMBER)
						&& !IndexElementType.IsA(IDatatype::RANGE))
					{
					if (retsError) *retsError = ERR_EXPECT_INTEGER_INDEX;
					return false;
					}
				}
			else if (DimType.GetClass() == IDatatype::ECategory::Enum)
				{
				if (!IndexElementType.IsAny() && !IndexElementType.IsA(DimType))
					{
					if (retsError) *retsError = strPattern(ERR_INVALID_DIM_TYPES, DimType.GetName());
					return false;
					}
				}
			else
				{
				if (retsError) *retsError = strPattern(ERR_INVALID_DIM_TYPES, DimType.GetName());
				return false;
				}

			if (dLiteral.IsArray())
				{
				CDatum dExpanded = CAEONTensor::ExpandIndexRange(dLiteral);
				SliceDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, dExpanded.GetCount() - 1));
				}
			else
				SliceDims.Insert(CAEONTypes::Get(IDatatype::INTEGER));

			continue;
			}

		//	A scalar coordinate removes this dimension from the result.

		else if (DimType.IsA(IDatatype::INTEGER))
			{
			if (!IndexType.IsA(IDatatype::NUMBER))
				{
				if (retsError) *retsError = ERR_EXPECT_INTEGER_INDEX;
				return false;
				}

			continue;
			}
		else if (DimType.GetClass() == IDatatype::ECategory::Enum && IndexType.IsA(DimType))
			continue;

		if (retsError) *retsError = strPattern(ERR_INVALID_DIM_TYPES, DimType.GetName());
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
			*retdReturnType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, std::move(SliceDims));
		}
	else
		{
		if (retdReturnType)
			*retdReturnType = m_dElementType;
		}

	return true;
	}

static bool GetTensorDimensionLength (CDatum dType, int* retiLength)
	{
	const IDatatype& Type = dType;
	auto NumberDesc = Type.GetNumberDesc();
	if (NumberDesc.bNumber && NumberDesc.bSubRange)
		{
		LONGLONG iLength = (LONGLONG)NumberDesc.iSubRangeMax - (LONGLONG)NumberDesc.iSubRangeMin + 1;
		if (iLength < 0 || iLength > INT_MAX)
			return false;

		if (retiLength) *retiLength = (int)iLength;
		return true;
		}
	else if (Type.GetClass() == IDatatype::ECategory::Enum)
		{
		if (retiLength) *retiLength = Type.GetMemberCount();
		return true;
		}
	else
		return false;
	}

static bool CanConstructTensorFrom (const IDatatype& TargetType, CDatum dSourceType, bool bExplicit)
	{
	const IDatatype& SourceType = dSourceType;

	if (SourceType.IsAny() || SourceType.IsA(TargetType))
		return true;

	//	Vectors have a known rank-1 numeric shape, but require conversion to
	//	tensor storage. This is a construction rule, not an IsA relationship.

	if (SourceType.GetCoreType() == IDatatype::VECTOR_2D_F64
			|| SourceType.GetCoreType() == IDatatype::VECTOR_3D_F64)
		{
		const IDatatype& ElementType = TargetType.GetElementType();
		if (!ElementType.IsAny() && !ElementType.IsA(IDatatype::NUMBER))
			return false;

		int iLength = (SourceType.GetCoreType() == IDatatype::VECTOR_2D_F64 ? 2 : 3);
		TArray<CDatum> Dims;
		Dims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iLength - 1));
		CDatum dTensorType = CAEONTypes::CreateTensor(NULL_STR, CAEONTypes::Get(IDatatype::FLOAT_64), std::move(Dims));
		return CanConstructTensorFrom(TargetType, dTensorType, bExplicit);
		}

	if (SourceType.GetClass() != IDatatype::ECategory::Tensor
			&& SourceType.GetClass() != IDatatype::ECategory::Array)
		return false;

	TArray<CDatum> TargetDims = TargetType.GetDimensionTypes();
	int iTargetRank = TargetDims.GetCount();
	CDatum dTargetElementType = TargetType.GetElementType();
	const IDatatype& TargetElementType = dTargetElementType;

	if (SourceType.GetClass() == IDatatype::ECategory::Array)
		{
		CDatum dLeafType = dSourceType;
		int iSourceRank = 0;
		while (((const IDatatype&)dLeafType).GetClass() == IDatatype::ECategory::Array
				&& iSourceRank < iTargetRank)
			{
			dLeafType = ((const IDatatype&)dLeafType).GetElementType();
			iSourceRank++;
			}

		if (iSourceRank == 0)
			return false;

		const IDatatype& LeafType = dLeafType;
		if (LeafType.IsAny() || LeafType.IsA(TargetElementType))
			return true;

		return (bExplicit
				? TargetElementType.CanBeConstructedExplicitlyFrom(dLeafType)
				: TargetElementType.CanBeConstructedFrom(dLeafType));
		}

	CDatum dSourceElementType = SourceType.GetElementType();
	const IDatatype& SourceElementType = dSourceElementType;
	if (!SourceElementType.IsAny() && !SourceElementType.IsA(TargetElementType))
		{
		bool bCanConstructElement = (bExplicit
				? TargetElementType.CanBeConstructedExplicitlyFrom(dSourceElementType)
				: TargetElementType.CanBeConstructedFrom(dSourceElementType));
		if (!bCanConstructElement)
			return false;
		}

	TArray<CDatum> SourceDims = SourceType.GetDimensionTypes();
	int iSourceRank = SourceDims.GetCount();
	if (iSourceRank > iTargetRank)
		return false;

	bool bTargetConcrete = true;
	LONGLONG iTargetCount = 1;
	for (int i = 0; i < iTargetRank; i++)
		{
		int iLength;
		if (!GetTensorDimensionLength(TargetDims[i], &iLength))
			{
			bTargetConcrete = false;
			break;
			}

		if (iLength == 0)
			iTargetCount = 0;
		else if (iTargetCount > INT_MAX / iLength)
			return false;
		else
			iTargetCount *= iLength;
		}

	//	A rank-1 tensor may linearly initialize a fully concrete target.

	if (bTargetConcrete && iTargetRank > 1 && iSourceRank == 1)
		{
		int iSourceLength;
		return (!GetTensorDimensionLength(SourceDims[0], &iSourceLength) || iSourceLength <= iTargetCount);
		}

	int iLeadingDims = iTargetRank - iSourceRank;
	for (int i = 0; i < iSourceRank; i++)
		{
		int iTargetLength;
		int iSourceLength;
		if (GetTensorDimensionLength(TargetDims[iLeadingDims + i], &iTargetLength)
				&& GetTensorDimensionLength(SourceDims[i], &iSourceLength)
				&& iSourceLength > iTargetLength)
			return false;
		}

	return true;
	}

bool CDatatypeTensor::OnCanBeConstructedExplicitlyFrom (CDatum dType) const
	{
	return CanConstructTensorFrom(*this, dType, true);
	}

bool CDatatypeTensor::OnCanBeConstructedFrom (CDatum dType) const
	{
	return CanConstructTensorFrom(*this, dType, false);
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

IDatatype::EMemberType CDatatypeTensor::OnHasMember (CStringView sName, CDatum* retdType, int* retiOrdinal) const

//	OnHasMember
//
//	Returns static type information for tensor methods.

	{
	int iIndex = CAEONTensor::FindMethodByKey(sName);
	if (iIndex != -1)
		{
		CDatum dMethodType = CAEONTensor::GetMethodType(iIndex);
		if (retdType)
			*retdType = dMethodType;
		if (retiOrdinal)
			*retiOrdinal = iIndex + 1;

		return EMemberType::InstanceMethod;
		}

	return EMemberType::DynamicMember;
	}

IDatatype::SMemberDesc CDatatypeTensor::OnGetMember (int iIndex) const
	{
	if (iIndex == 0)
		return SMemberDesc({ EMemberType::ArrayElement, NULL_STR, m_dElementType });

	iIndex--;
	if (iIndex >= CAEONTensor::GetMethodCount())
		throw CException(errFail);

	SMemberDesc Member({ EMemberType::InstanceMethod, CAEONTensor::GetMethodKey(iIndex), CAEONTensor::GetMethodType(iIndex) });
	if (CAEONTensor::GetMethodFlags(iIndex) & IInvokeCtx::EXEC_FLAG_CONST)
		Member.dwFlags |= MEMBER_FLAG_CONST;

	return Member;
	}

DWORD CDatatypeTensor::OnGetMemberFlags (CStringView sName) const
	{
	int iIndex = CAEONTensor::FindMethodByKey(sName);
	if (iIndex == -1)
		return 0;

	return ((CAEONTensor::GetMethodFlags(iIndex) & IInvokeCtx::EXEC_FLAG_CONST) ? MEMBER_FLAG_CONST : 0);
	}

int CDatatypeTensor::OnGetMemberCount () const
	{
	return 1 + CAEONTensor::GetMethodCount();
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
				const IDatatype& OtherDimType = OtherDims[i];

				//	Concrete dimensions are part of a tensor's substitutable shape,
				//	so they must match exactly. A narrower integer subrange is a
				//	subtype as a scalar value, but it cannot supply all indices of a
				//	wider tensor dimension. Abstract dimensions retain normal subtype
				//	matching so that concrete tensors implement array[*,...].

				if (GetTensorDimensionLength(OtherDims[i], NULL))
					{
					if (DimType != OtherDimType)
						return false;
					}
				else if (!DimType.IsA(OtherDimType))
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
