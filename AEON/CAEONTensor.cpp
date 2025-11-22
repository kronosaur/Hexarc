//	CAEONTensor.cpp
//
//	CAEONTensor classes
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_SHAPE,						"shape");

DECLARE_CONST_STRING(TYPENAME_TENSOR,					"tensor");

DECLARE_CONST_STRING(ERR_INVALID_AXIS,					"Invalid axis.");
DECLARE_CONST_STRING(ERR_CONCAT_INVALID_DIMENSIONS,		"Cannot concatenate %dD array to %dD array.");
DECLARE_CONST_STRING(ERR_CONCAT_INVALID_DIMENSION_SIZE,	"Source array dimensions %d must be %d in size (is %d).");
DECLARE_CONST_STRING(ERR_TENSOR_EXPECTED,				"Parameter must be a tensor: %s.");
DECLARE_CONST_STRING(ERR_TENSOR_DOT_MISMATCH,			"Tensors must have the same number of elements for dot product.");
DECLARE_CONST_STRING(ERR_TENSOR_MATMUL_MISMATCH,		"Left tensor must have the same number of columns as right tensor rows.");
DECLARE_CONST_STRING(ERR_TENSOR_MATMUL_CANT_BROADCAST,	"Unable to broadcast higher dimensions for matmul.");
DECLARE_CONST_STRING(ERR_TENSOR_MATMUL_ORIGIN,			"matmul is not supported for tensors with dimensions that do not start at 0.");

TDatumPropertyHandler<CAEONTensor> CAEONTensor::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			return Obj.m_Dims.GetCount();
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			return Obj.m_dElementType;
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			if (Obj.m_Dims.GetCount() == 1)
				{
				return Obj.m_Dims[0].dType;
				}
			else
				{
				CDatum dResult(CDatum::typeArray);
				for (int i = 0; i < Obj.m_Dims.GetCount(); i++)
					dResult.Append(Obj.m_Dims[i].dType);
				return dResult;
				}
			},
		NULL,
		},
	{
		"keys",
		"a",
		"Returns an array of valid indices.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			if (Obj.m_Dims.GetCount() == 0)
				return dResult;
			else if (Obj.m_Dims.GetCount() == 1)
				{
				if (Obj.m_Dims[0].bEnum)
					{
					const IDatatype& DimType = Obj.m_Dims[0].dType;
					for (int i = 0; i < DimType.GetMemberCount(); i++)
						dResult.Append(CDatum::CreateEnum(i, Obj.m_Dims[0].dType));
					}
				else
					{
					for (int i = 0; i < Obj.m_Dims[0].iLength; i++)
						dResult.Append(i + Obj.m_Dims[0].iOrigin);
					}
				}
			else
				{
				for (Iterator i = Obj.begin(); i != Obj.end(); ++i)
					dResult.Append(i.AsIndexDatum());
				}
			return dResult;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the first dimension.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Dims.GetCount(); i++)
				dResult.Append(Obj.m_Dims[i].iLength);
			return dResult;
			},
		NULL,
		},
	{
		"size",
		"i",
		"Returns the total number of element in the tensor.",
		[](const CAEONTensor& Obj, const CString &sProperty)
			{
			return CDatum(Obj.CalcDataCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONTensor::m_pMethodsExt = NULL;

const CString &CAEONTensor::GetTypename (void) const
	{
	return TYPENAME_TENSOR;
	}

bool CAEONTensor::AddDim (TArray<CDatum>& ResultDims, int iDim, CDatum dDim) const
	{
	if (dDim.IsNil())
		return false;

	else if (dDim.IsIdenticalToTrue())
		ResultDims.Insert(m_Dims[iDim].dType);

	else if (dDim.GetBasicType() == CDatum::typeRange)
		{
		const IAEONRange* pRange = dDim.GetRangeInterface();
		if (pRange == NULL || pRange->GetLength() == 0)
			return false;

		ResultDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, pRange->GetLength() - 1));
		}

	else if (dDim.IsArray())
		{
		ResultDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, dDim.GetCount() - 1));
		}

	return true;
	}

void CAEONTensor::ApplyBatchIndices (Iterator& Pos, const TArray<int>& Indices, int iLowerDims)

//	ApplyBatchIndices
//
//	Sets the position to the given indices, which are relative to the
//	lower dimensions. For example, if we have a 3D tensor and we want to
//	set the position to (2, 3), then we set the position to (3, 4, 0).

	{
	for (int i = 0; i < Indices.GetCount(); i++)
		{
		int iDimIndex = Pos.GetDimCount() - iLowerDims - 1 - i;
		int iSrcIndex = Indices.GetCount() - 1 - i;

		if (iDimIndex < 0)
			return; // No more dimensions to set

		int iDimPos = Indices[iSrcIndex];
		if (iDimPos < Pos.GetDimLength(iDimIndex))
			{
			//	Set the position for this dimension
			Pos.SetDim(iDimIndex, iDimPos);
			}
		else
			{
			//	If the index is out of bounds, then we set the position to 0.
			Pos.SetDim(iDimIndex, 0);
			}
		}
	}

const CAEONTensor& CAEONTensor::AsTensor (CDatum dTensor)

//	AsTensor
//
//	Converts a datum to a tensor. NOTE: Callers must guarantee that the datum
//	is a tensor type.

	{
	//	LATER: For now we assume that all tensor types are implemented 
	//	with this class.

	ASSERT(dTensor.GetBasicType() == CDatum::typeTensor);
	return *(CAEONTensor*)dTensor.GetComplex();
	}

TArray<int> CAEONTensor::CalcBroadcastDims (const TArray<int>& Dims1, const TArray<int>& Dims2)

//	CalcBroadcastDims
//
//	Calculates the broadcast dimensions for two tensors.

	{
	int iMaxDims = Max(Dims1.GetCount(), Dims2.GetCount());
	TArray<int> BroadcastDims;
	BroadcastDims.InsertEmpty(iMaxDims);

	for (int i = 0; i < iMaxDims; i++)
		{
		int iDim1Index = Dims1.GetCount() - 1 - i;
		int iDim2Index = Dims2.GetCount() - 1 - i;
		int iDim1 = (iDim1Index >= 0 ? Dims1[iDim1Index] : 1);
		int iDim2 = (iDim2Index >= 0 ? Dims2[iDim2Index] : 1);

		if (iDim1 == iDim2)
			BroadcastDims[iMaxDims - 1 - i] = iDim1;
		else if (iDim1 == 1)
			BroadcastDims[iMaxDims - 1 - i] = iDim2;
		else if (iDim2 == 1)
			BroadcastDims[iMaxDims - 1 - i] = iDim1;
		else
			{
			BroadcastDims.DeleteAll();
			return BroadcastDims; // Incompatible dimensions
			}
		}

	return BroadcastDims;
	}

CDatum CAEONTensor::CalcData (CDatum dInitialData) const

//	CalcData
//
//	Creates a new data array using the current dimensions and the given initial data.

	{
	switch (dInitialData.GetBasicType())
		{
		//	If the data is an array, then we traverse it (even if it is nested) and
		//	copy the elements.

		case CDatum::typeArray:
			return CalcDataFromArray(dInitialData);

		case CDatum::typeTensor:
			{
			const CAEONTensor& SrcTensor = AsTensor(dInitialData);

			//	If this is a 1D tensor then we treat it as an array.

			if (SrcTensor.m_Dims.GetCount() == 1)
				return CalcDataFromArray(SrcTensor.m_dData);

			//	Otherwise, if we have identical dimensions, then we can just take
			//	the data.

			else if (IsSameSize(SrcTensor))
				return CDatum::CreateArrayAsTypeOfElement(m_dElementType, SrcTensor.m_dData);

			//	Otherwise, create a new data array and copy the elements.

			else
				{
				CDatum dData = CDatum::CreateArrayAsTypeOfElement(m_dElementType);
				dData.InsertEmpty(CalcDataSize());

				Iterator SrcIter = SrcTensor.begin();
				for (int i = 0; i < dData.GetCount(); i++)
					{
					if (SrcIter != SrcTensor.end())
						{
						dData.SetElement(i, *SrcIter);
						++SrcIter;
						}
					}

				return dData;
				}
			}

		default:
			{
			CDatum dData = CDatum::CreateArrayAsTypeOfElement(m_dElementType);
			dData.InsertEmpty(CalcDataSize());
			return dData;
			}
		}
	}

CDatum CAEONTensor::CalcDataFromArray (CDatum dArray) const

//	CalcDataFromArray
//
//	Creates a new data array using the current dimensions and the given initial data.

	{
	struct SPosEntry
		{
		CDatum dArray;
		int iPos = 0;
		};

	CDatum dData = CDatum::CreateArrayAsTypeOfElement(m_dElementType);
	int iDataSize = CalcDataSize();
	dData.InsertEmpty(iDataSize);

	TArray<SPosEntry> Stack;
	int iDestPos = 0;

	Stack.Insert ({ dArray });

	while (Stack.GetCount () > 0)
		{
		SPosEntry &Pos = Stack[Stack.GetCount () - 1];
		if (Pos.iPos >= Pos.dArray.GetCount ())
			{
			Stack.Delete(Stack.GetCount () - 1);
			continue;
			}

		CDatum dElement = Pos.dArray.GetElement(Pos.iPos);
		Pos.iPos++;

		if (dElement.IsArray() && Stack.GetCount() < m_Dims.GetCount())
			{
			Stack.Insert({ dElement });
			}
		else
			{
			dData.SetElement(iDestPos++, dElement);
			}
		}

	return dData;
	}

int CAEONTensor::CalcDataCount () const

//	CalcDataCount
//
//	Returns the total number of elements across all dimensions. NOTE: This is 
//	not always the same as CalcDataSize because it ignores stride.

	{
	int iCount = 1;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		iCount *= m_Dims[i].iLength;

	return iCount;
	}

int CAEONTensor::CalcDataSize () const

//	CalcDataSize
//
//	Returns the size of the data array.

	{
	int iMaxIndex = m_iDataStart;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		iMaxIndex += (m_Dims[i].iLength - 1) * m_Dims[i].iStride;
		}

	return iMaxIndex + 1;  // +1 because indices start at 0
	}

TArray<CAEONTensor::SDimDesc> CAEONTensor::CalcDims (CDatum dType, bool bColumnMajor)

//	CalcDims
//
//	Returns an array of dimensions based on the type. If we cannot create valid
//	dimensions, we return an empty array. Caller is responsible for checking.

	{
	const IDatatype& Type = dType;
	TArray<CDatum> SrcDims = Type.GetDimensionTypes();
	if (SrcDims.GetCount() == 0)
		return TArray<SDimDesc>();

	TArray<SDimDesc> Dims;
	Dims.InsertEmpty(SrcDims.GetCount());

	for (int i = 0; i < SrcDims.GetCount(); i++)
		{
		const IDatatype& DimType = SrcDims[i];
		IDatatype::SNumberDesc DimDesc = DimType.GetNumberDesc();
		if (DimDesc.bNumber && DimDesc.bSubRange)
			{
			Dims[i].dType = SrcDims[i];
			Dims[i].iOrigin = DimDesc.iSubRangeMin;
			Dims[i].iLength = DimDesc.iSubRangeMax - DimDesc.iSubRangeMin + 1;
			}
		else if (DimType.GetClass() == IDatatype::ECategory::Enum)
			{
			Dims[i].dType = SrcDims[i];
			Dims[i].iOrigin = 0;
			Dims[i].iLength = DimType.GetMemberCount();
			Dims[i].bEnum = true;
			}
		else
			return TArray<SDimDesc>();
		}

	//	Initialize the strides (either row-major or column-major)

	if (!bColumnMajor)
		{
		int iStride = 1;
		for (int i = Dims.GetCount() - 1; i >= 0; i--)
			{
			Dims[i].iStride = iStride;
			Dims[i].iFlatStride = iStride;	//	Same since the data is the same as the slice.
			iStride *= Dims[i].iLength;
			}
		}
	else
		{
		int iStride = 1;
		for (int i = 0; i < Dims.GetCount(); i++)
			{
			Dims[i].iStride = iStride;
			Dims[i].iFlatStride = iStride;	//	Same since the data is the same as the slice.

			iStride *= Dims[i].iLength;
			}
		}

	return Dims;
	}

CDatum CAEONTensor::CalcElementType (CDatum dType)
	{
	return ((const IDatatype&)dType).GetElementType();
	}

int CAEONTensor::CalcFlatIndex (const TArray<int>& Indices) const
	{
	int iIndex = 0;

	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		if (Indices[i] < m_Dims[i].iOrigin)
			return -1;

		int iOffset = Indices[i] - m_Dims[i].iOrigin;
		if (iOffset >= m_Dims[i].iLength)
			return -1;

		iIndex += m_Dims[i].iFlatStride * iOffset;
		}

	return m_iDataStart + iIndex;
	}

int CAEONTensor::CalcIndex (const TArray<int>& Indices) const

//	CalcIndex
//
//	Returns the index in the data array given the current indices. If the indices
//	are out of bounds, we return -1. Indices are based at the origin of the 
//	dimension.

	{
	int iIndex = 0;

	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		if (Indices[i] < m_Dims[i].iOrigin)
			return -1;

		int iOffset = Indices[i] - m_Dims[i].iOrigin;
		if (iOffset >= m_Dims[i].iLength)
			return -1;

		iIndex += m_Dims[i].iStride * iOffset;
		}

	return m_iDataStart + iIndex;
	}

int CAEONTensor::CalcIndex (CDatum dIndices) const

//	CalcIndex
//
//	Returns the index in the data array given the current indices. If the indices
//	are out of bounds, we return -1. Indices are based at the origin of the 
//	dimension.

	{
	int iPos = m_iDataStart;

	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		int iIndex = (int)dIndices.GetElement(i);

		if (iIndex < m_Dims[i].iOrigin)
			return -1;

		int iOffset = iIndex - m_Dims[i].iOrigin;
		if (iOffset >= m_Dims[i].iLength)
			return -1;

		iPos += m_Dims[i].iStride * iOffset;
		}

	return iPos;
	}

int CAEONTensor::CalcIndex1D (int iIndex) const

//	CalcIndex1D
//
//	This is an optimized version of CalcIndex for 1-dimensional tensors.

	{
	if (iIndex < m_Dims[0].iOrigin)
		return -1;

	int iOffset = iIndex - m_Dims[0].iOrigin;
	if (iOffset >= m_Dims[0].iLength)
		return -1;

	return m_iDataStart + (m_Dims[0].iStride * iOffset);
	}

int CAEONTensor::CalcIndex2D (int iIndex1, int iIndex2) const
	{
	int iPos = m_iDataStart;

	//	Dim 0

	if (iIndex1 < m_Dims[0].iOrigin)
		return -1;

	int iOffset = iIndex1 - m_Dims[0].iOrigin;
	if (iOffset >= m_Dims[0].iLength)
		return -1;

	iPos += m_Dims[0].iStride * iOffset;

	//	Dim 1

	if (iIndex2 < m_Dims[1].iOrigin)
		return -1;

	iOffset = iIndex2 - m_Dims[1].iOrigin;
	if (iOffset >= m_Dims[1].iLength)
		return -1;

	iPos += m_Dims[1].iStride * iOffset;

	return iPos;
	}

int CAEONTensor::CalcIndex3D (int iIndex1, int iIndex2, int iIndex3) const
	{
	int iPos = m_iDataStart;

	//	Dim 0

	if (iIndex1 < m_Dims[0].iOrigin)
		return -1;

	int iOffset = iIndex1 - m_Dims[0].iOrigin;
	if (iOffset >= m_Dims[0].iLength)
		return -1;

	iPos += m_Dims[0].iStride * iOffset;

	//	Dim 1

	if (iIndex2 < m_Dims[1].iOrigin)
		return -1;

	iOffset = iIndex2 - m_Dims[1].iOrigin;
	if (iOffset >= m_Dims[1].iLength)
		return -1;

	iPos += m_Dims[1].iStride * iOffset;

	//	Dim 2

	if (iIndex3 < m_Dims[2].iOrigin)
		return -1;

	iOffset = iIndex3 - m_Dims[2].iOrigin;
	if (iOffset >= m_Dims[2].iLength)
		return -1;

	iPos += m_Dims[2].iStride * iOffset;

	return iPos;
	}

TArray<TArray<int>> CAEONTensor::CalcIndexPermutations (const TArray<int>& Dims)

//	CalcIndexPermutations
//
//	Calculates all permutations of the indices for the given dimensions.

	{
	TArray<TArray<int>> Result;

	if (Dims.GetCount() == 0)
		{
		//	Insert a single empty array to represent the empty tensor.
		Result.Insert(TArray<int>());
		return Result;
		}

	//	Recursive helper.

	TArray<int> Current;
	Current.InsertEmpty(Dims.GetCount());
	for (int i = 0; i < Dims.GetCount(); i++)
		Current[i] = 0;

	std::function<void(int)> recurse = [&](int iIndex)
		{
		if (iIndex == Dims.GetCount())
			{
			Result.Insert(Current);
			return;
			}
		for (int i = 0; i < Dims[iIndex]; i++)
			{
			Current[iIndex] = i;
			recurse(iIndex + 1);
			}
		};

	recurse(0);
	return Result;
	}

TArray<int> CAEONTensor::CalcIndicesFromFlat (int iIndex) const

//	CalcIndicesFromFlat
//
//	Converts from a flat index to an array of indices.

	{
	TArray<int> Indices;
	Indices.InsertEmpty(m_Dims.GetCount());
	
	int iRemainder = iIndex;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		Indices[i] = m_Dims[i].iOrigin + (iRemainder / m_Dims[i].iFlatStride);
		iRemainder = iRemainder % m_Dims[i].iFlatStride;
		}
	
	return Indices;
	}

CAEONTensor::SSliceDesc CAEONTensor::CalcSlice (CDatum dIndex) const

//	CalcSlice
//
//	dIndex is the value passed into a single index dereference. We need to 
//	handle the following cases:
//
//	1.	A 1D tensor with a single index (e.g., t[5])
//	2.	A 1D tensor with an array of indices (e.g., t[[5, 6, 7]])
//	3.	An n-D tensor with a single index (e.g., t[5])
//	4.	An n-D tensor with an array of dimensions (e.g., t[[5, 6, 7]])
//	5.	An n-D tensor with an array of arrays of indices (e.g., t[[[1,2,3], [4,5,6]]])
//
//	Also, each index may be either a single integer or a range.

	{
	SSliceDesc Slice;

	if (dIndex.GetCount() == 0)
		return Slice;	//	No index, no slice

	//	If we only have a single dimension, then we interpret the index as 
	//	a single index or an array of indices.

	else if (m_Dims.GetCount() == 1)
		{
		int iIndex;
		if (dIndex.IsNumberInt32(&iIndex))
			{
			Slice.iPos = CalcIndex1D(iIndex);
			if (Slice.iPos == -1)
				return Slice; // Invalid index

			return Slice; // Valid index, return the slice
			}
		else
			{
			//	If dIndex is an array, then it might contain a mix of ranges and
			//	indices. In that case, we expand the ranges into a flat array of indices.

			if (dIndex.IsArray())
				dIndex = ExpandIndexRange(dIndex);

			//	If the index is an array, then we need to create a slice, so we
			//	need to compute the dimensions of the slice.

			TArray<CDatum> SliceDims;
			if (!AddDim(SliceDims, 0, dIndex))
				return Slice; // Invalid index

			//	If no dimensions then it means we have specified a single element.

			if (SliceDims.GetCount() == 0)
				{
				Slice.iPos = CalcIndex(dIndex);
				if (Slice.iPos == -1)
					return Slice; // Invalid index

				return Slice; // Valid index, return the slice
				}

			//	Otherwise we need a slice.

			else
				{
				Slice.dSliceType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, SliceDims);
				Slice.SliceIndex.Insert(dIndex);
				return Slice;
				}
			}
		}

	//	For multiple dimensions, we expect the index to be an array of indices,
	//	one for each dimension.

	else
		{
		TArray<CDatum> SliceIndex;
		if (dIndex.IsArray())
			{
			for (int i = 0; i < dIndex.GetCount(); i++)
				{
				CDatum dDimIndex = dIndex.GetElement(i);
				if (dDimIndex.IsArray())
					dDimIndex = ExpandIndexRange(dDimIndex);

				SliceIndex.Insert(dDimIndex);
				}
			}
		else
			{
			SliceIndex.Insert(dIndex);
			}

		//	Can't have more indices than dimensions.

		if (SliceIndex.GetCount() > m_Dims.GetCount())
			return Slice; // Invalid index

		//	Compute the dimensions of the slice.

		TArray<CDatum> SliceDims;
		for (int i = 0; i < SliceIndex.GetCount(); i++)
			{
			if (!AddDim(SliceDims, i, SliceIndex[i]))
				return Slice; // Invalid index
			}

		//	Missing dimensions are the same as the original tensor (e.g., same
		//	as *).

		for (int i = SliceIndex.GetCount(); i < m_Dims.GetCount(); i++)
			SliceDims.Insert(m_Dims[i].dType);

		//	If no dimensions then it means we have specified a single cell.

		if (SliceDims.GetCount() == 0)
			{
			TArray<int> Indices;
			for (int i = 0; i < SliceIndex.GetCount(); i++)
				Indices.Insert((int)SliceIndex[i]);

			Slice.iPos = CalcIndex(Indices);
			if (Slice.iPos == -1)
				return Slice; // Invalid index

			return Slice;
			}

		//	Otherwise, we create a new tensor with the given dimensions.

		else
			{
			Slice.dSliceType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, SliceDims);
			Slice.SliceIndex = std::move(SliceIndex);
			return Slice;
			}
		}
	}

CAEONTensor::SSliceDesc CAEONTensor::CalcSlice2D (CDatum dIndex1, CDatum dIndex2) const
	{
	SSliceDesc Slice;

	//	If exactly 2 dimensions, then we just return the element.

	int iIndex1;
	int iIndex2;

	if (m_Dims.GetCount() == 2 
			&& dIndex1.IsNumberInt32(&iIndex1) && dIndex2.IsNumberInt32(&iIndex2))
		{
		Slice.iPos = CalcIndex2D(iIndex1, iIndex2);
		if (Slice.iPos == -1)
			return Slice; // Invalid index

		return Slice;
		}

	//	Now get the element or slice

	else if (m_Dims.GetCount() >= 2)
		{
		TArray<CDatum> SliceIndex;
		if (dIndex1.IsArray())
			SliceIndex.Insert(ExpandIndexRange(dIndex1));
		else
			SliceIndex.Insert(dIndex1);

		if (dIndex2.IsArray())
			SliceIndex.Insert(ExpandIndexRange(dIndex2));
		else
			SliceIndex.Insert(dIndex2);

		//	Compute the dimensions of the slice.

		TArray<CDatum> SliceDims;
		for (int i = 0; i < SliceIndex.GetCount(); i++)
			{
			if (!AddDim(SliceDims, i, SliceIndex[i]))
				return Slice; // Invalid index
			}

		//	Missing dimensions are the same as the original tensor (e.g., same
		//	as *).

		for (int i = SliceIndex.GetCount(); i < m_Dims.GetCount(); i++)
			SliceDims.Insert(m_Dims[i].dType);

		//	If no dimensions then it means we have specified a single cell.

		if (SliceDims.GetCount() == 0)
			{
			Slice.iPos = CalcIndex2D(SliceIndex[0], SliceIndex[1]);
			if (Slice.iPos == -1)
				return Slice; // Invalid index

			return Slice;
			}

		//	Otherwise, we create a new tensor with the given dimensions.

		else
			{
			Slice.dSliceType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, SliceDims);
			Slice.SliceIndex = std::move(SliceIndex);
			return Slice;
			}
		}
	else
		return Slice; // Invalid index
	}

CAEONTensor::SSliceDesc CAEONTensor::CalcSlice3D (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const
	{
	SSliceDesc Slice;

	//	If exactly 3 dimensions, then we just return the element.

	int iIndex1;
	int iIndex2;
	int iIndex3;

	if (m_Dims.GetCount() == 3 
			&& dIndex1.IsNumberInt32(&iIndex1) && dIndex2.IsNumberInt32(&iIndex2) && dIndex3.IsNumberInt32(&iIndex3))
		{
		Slice.iPos = CalcIndex3D(iIndex1, iIndex2, iIndex3);
		if (Slice.iPos == -1)
			return Slice; // Invalid index

		return Slice;
		}

	//	Now get the element or slice

	else if (m_Dims.GetCount() >= 3)
		{
		TArray<CDatum> SliceIndex;
		if (dIndex1.IsArray())
			SliceIndex.Insert(ExpandIndexRange(dIndex1));
		else
			SliceIndex.Insert(dIndex1);

		if (dIndex2.IsArray())
			SliceIndex.Insert(ExpandIndexRange(dIndex2));
		else
			SliceIndex.Insert(dIndex2);

		if (dIndex3.IsArray())
			SliceIndex.Insert(ExpandIndexRange(dIndex3));
		else
			SliceIndex.Insert(dIndex3);

		//	Compute the dimensions of the slice.

		TArray<CDatum> SliceDims;
		for (int i = 0; i < SliceIndex.GetCount(); i++)
			{
			if (!AddDim(SliceDims, i, SliceIndex[i]))
				return Slice; // Invalid index
			}

		//	Missing dimensions are the same as the original tensor (e.g., same
		//	as *).

		for (int i = SliceIndex.GetCount(); i < m_Dims.GetCount(); i++)
			SliceDims.Insert(m_Dims[i].dType);

		//	If no dimensions then it means we have specified a single cell.

		if (SliceDims.GetCount() == 0)
			{
			Slice.iPos = CalcIndex3D(SliceIndex[0], SliceIndex[1], SliceIndex[2]);
			if (Slice.iPos == -1)
				return Slice; // Invalid index

			return Slice;
			}

		//	Otherwise, we create a new tensor with the given dimensions.

		else
			{
			Slice.dSliceType = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, SliceDims);
			Slice.SliceIndex = std::move(SliceIndex);
			return Slice;
			}
		}
	else
		return Slice; // Invalid index
	}

bool CAEONTensor::CalcIsStandardDims () const

//	CalcIsStandardDims
//
//	Returns TRUE if m_iDataStart is 0 and the strides are contiguous.

	{
	if (m_iDataStart != 0 || m_bColumnMajor)
		return false;

	int iStride = 1;
	for (int i = m_Dims.GetCount() - 1; i >= 0; i--)
		{
		if (m_Dims[i].iStride != iStride || m_Dims[i].iOrigin != 0)
			return false;

		iStride *= m_Dims[i].iLength;
		}

	return true;
	}

IComplexDatum* CAEONTensor::Clone (CDatum::EClone iMode) const
	{
	CAEONTensor *pClone = new CAEONTensor(*this);
	pClone->m_dData = m_dData.Clone(iMode);
	return pClone;
	}

int CAEONTensor::CompareTensor (const TArray<SDimDesc>& Dims1, int iDataStart1, CDatum dData1, const TArray<SDimDesc>& Dims2, int iDataStart2, CDatum dData2, int iDim)

//	CompareTensor
//
//	Recursively compares each dimension of the tensor.

	{
	if (Dims1.GetCount() != Dims2.GetCount())
		throw CException(errFail);

	else if (Dims1.GetCount() == 0)
		return 0;

	//	If we're at the last dimension then we compare the data

	else if (iDim == Dims1.GetCount() - 1)
		{
		int iPos1 = iDataStart1;
		int iPos2 = iDataStart2;

		int iCount = Min(Dims1[iDim].iLength, Dims2[iDim].iLength);
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = dData1.GetElement(iPos1).OpCompare(dData2.GetElement(iPos2));
			if (iCompare != 0)
				return iCompare;

			iPos1 += Dims1[iDim].iStride;
			iPos2 += Dims2[iDim].iStride;
			}

		//	If elements are equal, then we compare the size of the array

		return KeyCompare(Dims1[iDim].iLength, Dims2[iDim].iLength);
		}

	//	Otherwise we recurse

	else
		{
		int iPos1 = iDataStart1;
		int iPos2 = iDataStart2;

		int iCount = Min(Dims1[iDim].iLength, Dims2[iDim].iLength);
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = CompareTensor(Dims1, iPos1, dData1, Dims2, iPos2, dData2, iDim + 1);
			if (iCompare != 0)
				return iCompare;

			iPos1 += Dims1[iDim].iStride;
			iPos2 += Dims2[iDim].iStride;
			}

		//	If elements are equal, then we compare the size of the array

		return KeyCompare(Dims1[iDim].iLength, Dims2[iDim].iLength);
		}
	}

int CAEONTensor::CompareTensorExact (const TArray<SDimDesc>& Dims1, int iDataStart1, CDatum dData1, const TArray<SDimDesc>& Dims2, int iDataStart2, CDatum dData2, int iDim)

//	CompareTensorExact
//
//	Recursively compares each dimension of the tensor.

	{
	if (Dims1.GetCount() != Dims2.GetCount())
		throw CException(errFail);

	else if (Dims1.GetCount() == 0)
		return 0;

	//	If we're at the last dimension then we compare the data

	else if (iDim == Dims1.GetCount() - 1)
		{
		int iPos1 = iDataStart1;
		int iPos2 = iDataStart2;

		int iCount = Min(Dims1[iDim].iLength, Dims2[iDim].iLength);
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = dData1.GetElement(iPos1).OpCompareExact(dData2.GetElement(iPos2));
			if (iCompare != 0)
				return iCompare;

			iPos1 += Dims1[iDim].iStride;
			iPos2 += Dims2[iDim].iStride;
			}

		//	If elements are equal, then we compare the size of the array

		return KeyCompare(Dims1[iDim].iLength, Dims2[iDim].iLength);
		}

	//	Otherwise we recurse

	else
		{
		int iPos1 = iDataStart1;
		int iPos2 = iDataStart2;

		int iCount = Min(Dims1[iDim].iLength, Dims2[iDim].iLength);
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = CompareTensor(Dims1, iPos1, dData1, Dims2, iPos2, dData2, iDim + 1);
			if (iCompare != 0)
				return iCompare;

			iPos1 += Dims1[iDim].iStride;
			iPos2 += Dims2[iDim].iStride;
			}

		//	If elements are equal, then we compare the size of the array

		return KeyCompare(Dims1[iDim].iLength, Dims2[iDim].iLength);
		}
	}

CDatum CAEONTensor::Create2DTensor (int iRows, int iCols, CDatum dElementType, CDatum dData)

//	Create2DTensor
//
//	Creates a new 2D tensor with the given number of rows and columns and element type.

	{
	TArray<int> Dims;
	Dims.InsertEmpty(2);
	Dims[0] = iRows;
	Dims[1] = iCols;

	return CreateNTensor(Dims, dElementType, dData);
	}

CDatum CAEONTensor::CreateNTensor (const TArray<int>& Dims, CDatum dElementType, CDatum dData)

//	CreateNTensor
//
//	Creates a new tensor with the given dimensions and element type. Dimension 
//	lengths must be >= 1.

	{
	if (Dims.GetCount() == 0)
		throw CException(errFail);

	//	Create the type of each of the dimensions.

	TArray<CDatum> DimTypes;
	DimTypes.InsertEmpty(Dims.GetCount());
	for (int i = 0; i < Dims.GetCount(); i++)
		{
		if (Dims[i] < 1)
			throw CException(errFail);

		DimTypes[i] = CAEONTypes::CreateInt32SubRange(NULL_STR, 0, Dims[i] - 1);
		}

	//	Create the tensor type.

	CDatum dType = CAEONTypes::CreateTensor(NULL_STR, dElementType, DimTypes);

	//	And now create the tensor.

	return CDatum(new CAEONTensor(dType, dData));
	}

CDatum CAEONTensor::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	CAEONTensor *pTensor = new CAEONTensor;
	CDatum dValue(pTensor);
	Serialized.Add(dwID, dValue);

	//	Flags

	DWORD dwFlags = Stream.ReadDWORD();
	pTensor->m_bColumnMajor = ((dwFlags & 0x00000001) ? true : false);

	//	Read the datatype

	pTensor->m_dDatatype = CDatum::DeserializeAEON(Stream, Serialized);
	pTensor->m_dElementType = CalcElementType(pTensor->m_dDatatype);
	pTensor->m_Dims = CalcDims(pTensor->m_dDatatype, pTensor->m_bColumnMajor);
	pTensor->m_iDataStart = Stream.ReadInt();

	pTensor->m_bStandardDims = pTensor->CalcIsStandardDims();

	//	Read the data array

	pTensor->m_dData = CDatum::DeserializeAEON(Stream, Serialized);

	return dValue;
	}

bool CAEONTensor::EnumElements (DWORD dwFlags, std::function<bool(CDatum)> fn) const

//	EnumElements
//
//	Enumerate all elements and call the given function.

	{
	if (dwFlags & CDatum::FLAG_RECURSIVE)
		{
		CRecursionGuard Guard(*this);
		if (Guard.InRecursion())
			return true;

		for (Iterator i = begin(); i != end(); ++i)
			{
			if (!(*i).EnumElements(dwFlags, fn))
				return false;
			}
		}
	else
		{
		for (Iterator i = begin(); i != end(); ++i)
			{
			CDatum dElement = *i;
			if (dElement.IsIdenticalToNil() && !(dwFlags & CDatum::FLAG_ALLOW_NULLS))
				continue;

			if (!fn(dElement))
				return false;
			}
		}

	return true;
	}

CDatum CAEONTensor::ExpandIndexRange (CDatum dArray)

//	ExpandIndexRange
//
//	Takes an array of indices or ranges and expands them to an array of indices.
//	NOTE: We do not check to see if the indices are in range.

	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < dArray.GetCount(); i++)
		{
		CDatum dIndex = dArray.GetElement(i);
		if (dIndex.IsNil())
			continue;
		else if (dIndex.GetBasicType() == CDatum::typeRange)
			{
			const IAEONRange* pRange = dIndex.GetRangeInterface();
			if (pRange != NULL)
				{
				int iStart = pRange->GetStart();
				int iEnd = pRange->GetEnd();
				int iStep = pRange->GetStep();

				int iIndex = iStart;
				for (int i = 0; i < pRange->GetLength(); i++)
					{
					dResult.Append(iIndex);
					iIndex += iStep;
					}
				}
			}
		else
			dResult.Append(dIndex);
		}

	return dResult;
	}

bool CAEONTensor::Find (CDatum dValue, int *retiIndex) const
	{
	for (Iterator i = begin(); i != end(); ++i)
		{
		if (dValue.OpIsEqual(*i))
			{
			if (retiIndex)
				*retiIndex = i.AsFlatIndex();
			return true;
			}
		}

	return false;
	}

CDatum CAEONTensor::FindAll (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	for (Iterator i = begin(); i != end(); ++i)
		{
		if (dValue.OpIsEqual(*i))
			dResult.Append(i.AsFlatIndex());
		}

	return dResult;
	}

CDatum CAEONTensor::FindAllExact (CDatum dValue) const
	{
	CDatum dResult(CDatum::typeArray);
	for (Iterator i = begin(); i != end(); ++i)
		{
		if (dValue.OpIsIdentical(*i))
			dResult.Append(i.AsFlatIndex());
		}

	return dResult;
	}

bool CAEONTensor::FindExact (CDatum dValue, int *retiIndex) const
	{
	for (Iterator i = begin(); i != end(); ++i)
		{
		if (dValue.OpIsIdentical(*i))
			{
			if (retiIndex)
				*retiIndex = i.AsFlatIndex();
			return true;
			}
		}

	return false;
	}

CString CAEONTensor::Format (const CStringFormat& Format) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	if (m_Dims.GetCount() == 0)
		return NULL_STR;

	CStringBuffer Output;
	FormatOutput(Output, 0, begin(), Format);

	return CString(std::move(Output));
	}

void CAEONTensor::FormatOutput (CStringBuffer& Buffer, int iLevel, const Iterator& Pos, const CStringFormat& Format) const

//	FormatOutput
//
//	Outputs the tensor to a string buffer.
//
//	iLevel = 0: Output the entire tensor
//	iLevel = 1: Output the slice at the position
//	...

	{
	if (iLevel == m_Dims.GetCount())
		{
		Buffer.Write((*Pos).Format(Format));
		}
	else
		{
		Buffer.Write("[", 1);
		if (iLevel < m_Dims.GetCount() - 2)
			{
			Buffer.Write("\n ", 2);
			for (int i = 0; i < iLevel; i++)
				Buffer.WriteChar(' ');
			}

		for (Iterator i = Pos; i != end(); i.IncDim(iLevel))
			{
			if (i != Pos)
				{
				if (iLevel == m_Dims.GetCount() - 1)
					Buffer.Write(", ", 2);
				else
					{
					Buffer.Write(",\n ", 3);
					for (int i = 0; i < iLevel; i++)
						Buffer.WriteChar(' ');
					}
				}

			FormatOutput(Buffer, iLevel + 1, i, Format);
			}

		if (iLevel < m_Dims.GetCount() - 2)
			{
			Buffer.WriteChar('\n');
			for (int i = 0; i < iLevel - 1; i++)
				Buffer.WriteChar(' ');
			}
		Buffer.Write("]", 1);
		}
	}

CDatum CAEONTensor::FromDatum (CDatum dValue) const
	{
	const IDatatype &Type = dValue.GetDatatype();
	if (Type.IsA(m_dElementType))
		return dValue;

	return CDatum::CreateAsType(m_dElementType, dValue);
	}

bool CAEONTensor::IsSameSize (const CAEONTensor& Src) const
	{
	if (m_Dims.GetCount() != Src.m_Dims.GetCount())
		return false;

	for (int i = 0; i < m_Dims.GetCount(); i++)
		if (m_Dims[i].iLength != Src.m_Dims[i].iLength)
			return false;

	return true;
	}

int CAEONTensor::GetCount (void) const

//	GetCount
//
//	We treat this as an array of arrays (if multidimensional) so we only return
//	the size of the first dimension.
//
//	NOTE: We return the number of elements, but callers cannot assume that this
//	defines the range of indices, because we could start at a different origin.

	{
	if (m_Dims.GetCount() == 0)
		return 0;
	else
		return m_Dims[0].iLength;
	}

CDatum CAEONTensor::GetElement (int iIndex) const
	{
	return GetElementAt(iIndex);
	}

CDatum CAEONTensor::GetElementAt (int iIndex) const
	{
	//	If no dimensions, always null

	if (m_Dims.GetCount() == 0)
		return MakeNullElement();

	//	If one dimension, just return the value.

	else if (m_Dims.GetCount() == 1)
		{
		int iPos = CalcIndex1D(iIndex);
		if (iPos == -1)
			return MakeNullElement();

		return m_dData.raw_GetArrayElement(iPos);
		}
	else
		{
		//	We expect this to be just a index into the first dimension and 
		//	we return a slice of the tensor.

		if (iIndex < m_Dims[0].iOrigin || iIndex >= m_Dims[0].iOrigin + m_Dims[0].iLength)
			return CDatum();	//	LATER: Maybe an empty tensor of the right shape?

		CAEONTensor* pTensor = new CAEONTensor();
		CDatum dResult(pTensor);

		pTensor->m_dData = m_dData;
		pTensor->m_iDataStart = m_iDataStart + m_Dims[0].iStride * (iIndex - m_Dims[0].iOrigin);
		pTensor->m_bColumnMajor = m_bColumnMajor;

		//	Create a new tensor datatype

		pTensor->m_dDatatype = ((const IDatatype&)m_dDatatype).GetSliceType();
		pTensor->m_Dims.InsertEmpty(m_Dims.GetCount() - 1);
		for (int i = 1; i < m_Dims.GetCount(); i++)
			pTensor->m_Dims[i - 1] = m_Dims[i];

		pTensor->m_dElementType = m_dElementType;
		pTensor->m_bStandardDims = pTensor->CalcIsStandardDims();

		return dResult;
		}
	}

CDatum CAEONTensor::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const
	{
	SSliceDesc Slice = CalcSlice(dIndex);
	if (Slice.IsEmpty())
		return MakeNullElement();
	else if (Slice.IsSingle())
		return m_dData.raw_GetArrayElement(Slice.iPos);
	else
		{
		CAEONTensor* pResult = new CAEONTensor(Slice.dSliceType);
		GetSliceElements(*pResult, Slice.SliceIndex, 0, begin(), 0, pResult->begin());
		return CDatum(pResult);
		}
	}

CDatum CAEONTensor::GetElementAt2DA (CDatum dIndex1, CDatum dIndex2) const
	{
	SSliceDesc Slice = CalcSlice2D(dIndex1, dIndex2);
	if (Slice.IsEmpty())
		return MakeNullElement();
	else if (Slice.IsSingle())
		return m_dData.raw_GetArrayElement(Slice.iPos);
	else
		{
		CAEONTensor* pResult = new CAEONTensor(Slice.dSliceType);
		GetSliceElements(*pResult, Slice.SliceIndex, 0, begin(), 0, pResult->begin());
		return CDatum(pResult);
		}
	}

CDatum CAEONTensor::GetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3) const
	{
	SSliceDesc Slice = CalcSlice3D(dIndex1, dIndex2, dIndex3);
	if (Slice.IsEmpty())
		return MakeNullElement();
	else if (Slice.IsSingle())
		return m_dData.raw_GetArrayElement(Slice.iPos);
	else
		{
		CAEONTensor* pResult = new CAEONTensor(Slice.dSliceType);
		GetSliceElements(*pResult, Slice.SliceIndex, 0, begin(), 0, pResult->begin());
		return CDatum(pResult);
		}
	}

CDatum CAEONTensor::GetElementAt2DI (int iIndex1, int iIndex2) const
	{
	if (m_Dims.GetCount() != 2)
		return MakeNullElement();

	else if (m_bStandardDims)
		{
		if (iIndex1 < 0 || iIndex1 >= m_Dims[0].iLength || iIndex2 < 0 || iIndex2 >= m_Dims[1].iLength)
			return MakeNullElement();

		//	Safe, since we check bounds above.
		return m_dData.raw_GetArrayElement(m_Dims[0].iStride * iIndex1 + iIndex2);
		}

	else
		{
		int iPos = CalcIndex2D(iIndex1, iIndex2);
		if (iPos == -1)
			return MakeNullElement();

		//	Safe, since we check bounds in CalcIndex2D
		return m_dData.raw_GetArrayElement(iPos);
		}
	}

CDatum CAEONTensor::GetElementAt3DI (int iIndex1, int iIndex2, int iIndex3) const
	{
	if (m_Dims.GetCount() == 3)
		{
		int iPos = CalcIndex3D(iIndex1, iIndex2, iIndex3);
		if (iPos == -1)
			return MakeNullElement();

		return m_dData.raw_GetArrayElement(iPos);
		}
	else
		return MakeNullElement();
	}

bool CAEONTensor::IsSquareMatrix () const
	{
	//	Must be 2D square matrix

	if (m_Dims.GetCount() != 2 || m_Dims[0].iLength != m_Dims[1].iLength)
		return false;

	//	Must be able to support a float type.

	if (!((const IDatatype&)CAEONTypes::Get(IDatatype::FLOAT)).IsA(m_dElementType))
		return false;

	return true;
	}

CDatum CAEONTensor::IteratorBegin () const
	{
	if (m_Dims.GetCount() == 0)
		return CDatum();
	else if (m_Dims.GetCount() == 1)
		{
		if (m_Dims[0].bEnum)
			return CDatum::CreateEnum(0, m_Dims[0].dType);
		else
			return CDatum(m_Dims[0].iOrigin);
		}
	else
		{
		CDatum dResult(CDatum::typeArray);
		for (int i = 0; i < m_Dims.GetCount(); i++)
			{
			if (m_Dims[i].bEnum)
				dResult.Append(CDatum::CreateEnum(m_Dims[i].iOrigin, m_Dims[i].dType));
			else
				dResult.Append(m_Dims[i].iOrigin);
			}

		return dResult;
		}
	}

CDatum CAEONTensor::IteratorNext (CDatum dIterator) const
	{
	if (m_Dims.GetCount() == 0)
		return CDatum();
	else if (m_Dims.GetCount() == 1)
		{
		int iIndex = dIterator;
		if (iIndex + 1 < m_Dims[0].iOrigin + m_Dims[0].iLength)
			{
			if (m_Dims[0].bEnum)
				return CDatum::CreateEnum(iIndex + 1, m_Dims[0].dType);
			else
				return CDatum(iIndex + 1);
			}
		else
			return CDatum();
		}
	else
		{
		CDatum dResult = dIterator.Clone();

		//	We increment the last index and then check to see if it is in bounds.
		//	If it is, then we return the new iterator. Otherwise, we return nil.

		for (int i = m_Dims.GetCount() - 1; i >= 0; i--)
			{
			int iIndex = dIterator.GetElement(i);
			if (iIndex + 1 < m_Dims[i].iOrigin + m_Dims[i].iLength)
				{
				if (m_Dims[i].bEnum)
					dResult.SetElement(i, CDatum::CreateEnum(iIndex + 1, m_Dims[i].dType));
				else
					dResult.SetElement(i, iIndex + 1);

				return dResult;
				}
			else
				{
				if (m_Dims[i].bEnum)
					dResult.SetElement(i, CDatum::CreateEnum(0, m_Dims[i].dType));
				else
					dResult.SetElement(i, m_Dims[i].iOrigin);
				}
			}

		return CDatum();
		}
	}

CDatum CAEONTensor::MathAbs () const
	{
	CAEONTensor *pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);

	//	NOTE: This new array might be a different shape than the original if
	//	we are a slice of a larger tensor. Setting elements the long way will
	//	ensure that we get the right shape.

	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dAbs = (*i).MathAbs();
		int iDest = pResult->CalcFlatIndex(i.AsIndices());
		pResult->m_dData.raw_SetArrayElement(iDest, dAbs);
		}

	return dResult;
	}

void CAEONTensor::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	for (Iterator i = begin(); i != end(); ++i)
		(*i).MathAccumulateStats(Stats);
	}

CDatum CAEONTensor::MathCeil () const
	{
	CAEONTensor *pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);

	//	NOTE: This new array might be a different shape than the original if
	//	we are a slice of a larger tensor. Setting elements the long way will
	//	ensure that we get the right shape.

	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dAbs = (*i).MathCeil();
		int iDest = pResult->CalcFlatIndex(i.AsIndices());
		pResult->m_dData.raw_SetArrayElement(iDest, dAbs);
		}

	return dResult;
	}

CDatum CAEONTensor::MathFloor () const
	{
	CAEONTensor *pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);

	//	NOTE: This new array might be a different shape than the original if
	//	we are a slice of a larger tensor. Setting elements the long way will
	//	ensure that we get the right shape.

	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dAbs = (*i).MathFloor();
		int iDest = pResult->CalcFlatIndex(i.AsIndices());
		pResult->m_dData.raw_SetArrayElement(iDest, dAbs);
		}

	return dResult;
	}

CDatum CAEONTensor::MathInvert () const
	{
	if (!IsSquareMatrix())
		return CDatum();

	int iSize = m_Dims[0].iLength;

	//	We start with the identity matrix

	CDatum dIdentity = OpIdentity();

	//	Create an augmented matrix

	TArray<CDatum> AugDims;
	AugDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, iSize - 1));
	AugDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, 2 * iSize - 1));
	CDatum dAugmentDatatype = CAEONTypes::CreateTensor(NULL_STR, m_dElementType, AugDims);
	CAEONTensor* pAugmented = new CAEONTensor(dAugmentDatatype);
	CDatum dAugmented(pAugmented);

	//	Initialize the augmented matrix

	for (int i = 0; i < iSize; i++)
		{
		for (int j = 0; j < iSize; j++)
			pAugmented->SetElementAt2DI(i, j, GetElementAt2DI(i, j));

		for (int j = iSize; j < 2 * iSize; j++)
			pAugmented->SetElementAt2DI(i, j, dIdentity.GetElementAt2DI(i, j - iSize));
		}

	//	Now we row reduce

	for (int i = 0; i < iSize; i++)
		{
		//	Make sure pivot row is non-zero

		double rPivot = pAugmented->GetElementAt2DI(i, i);
		if (rPivot == 0.0)
			{
			if (!SwapNonZeroRow(*pAugmented, i))
				return CDatum();
			}

		//	Normalize the row

		rPivot = pAugmented->GetElementAt2DI(i, i);
		for (int j = 0; j < 2 * iSize; j++)
			pAugmented->SetElementAt2DI(i, j, (double)pAugmented->GetElementAt2DI(i, j) / rPivot);

		//	Eliminate other rows

		for (int k = 0; k < iSize; k++)
			{
			if (k == i)
				continue;

			double rFactor = pAugmented->GetElementAt2DI(k, i);
			for (int j = 0; j < 2 * iSize; j++)
				pAugmented->SetElementAt2DI(k, j, (double)pAugmented->GetElementAt2DI(k, j) - rFactor * (double)pAugmented->GetElementAt2DI(i, j));
			}
		}

	//	Copy the result

	CAEONTensor* pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);
	for (int i = 0; i < iSize; i++)
		{
		for (int j = 0; j < iSize; j++)
			dResult.SetElementAt2DI(i, j, pAugmented->GetElementAt2DI(i, j + iSize));
		}

	return dResult;
	}

CDatum CAEONTensor::MathMatMul (CDatum dValue) const
	{
	//	Argument must be a tensor

	if (dValue.GetBasicType() != CDatum::typeTensor)
		return CDatum::CreateError(strPattern(ERR_TENSOR_EXPECTED, dValue.AsString()));

	const CAEONTensor& Left = *this;
	const CAEONTensor& Right = AsTensor(dValue);

	//	We don't support matmul for tensors that don't start at 0.

	for (int i = 0; i < Left.m_Dims.GetCount(); i++)
		if (Left.m_Dims[i].iOrigin != 0)
			return CDatum::CreateError(ERR_TENSOR_MATMUL_ORIGIN);

	for (int i = 0; i < Right.m_Dims.GetCount(); i++)
		if (Right.m_Dims[i].iOrigin != 0)
			return CDatum::CreateError(ERR_TENSOR_MATMUL_ORIGIN);

	//	If 0 dimensions, then error.

	if (Left.m_Dims.GetCount() == 0 || Right.m_Dims.GetCount() == 0)
		return CDatum::CreateError(strPattern(ERR_TENSOR_EXPECTED, dValue.AsString()));

	//	If 2 1D tensors, then we do a dot product.

	else if (Left.m_Dims.GetCount() == 1 && Right.m_Dims.GetCount() == 1)
		{
		if (Left.m_Dims[0].iLength != Right.m_Dims[0].iLength)
			return CDatum::CreateError(ERR_TENSOR_DOT_MISMATCH);

		double rResult = 0.0;
		for (int i = 0; i < Left.GetCount(); i++)
			rResult += (double)Left.GetElement(i) * (double)Right.GetElement(i);

		return CDatum(rResult);
		}

	//	If left is 1D and right is 2D, then we do a matrix-vector multiplication.

	else if (Left.m_Dims.GetCount() == 1 && Right.m_Dims.GetCount() >= 2)
		{
		//	Convert left into a 2D tensor with one row.

		CDatum dA = Create2DTensor(1, Left.m_Dims[0].iLength, Left.m_dElementType, CDatum::raw_AsComplex(this));
		CDatum dResult = dA.MathMatMul(dValue);
		if (dResult.IsError())
			return dResult;

		//	Remove any leading dimensions that are 1.

		const CAEONTensor& Result = AsTensor(dResult);
		return Result.SqueezeLeadingDims();
		}

	//	If left is 2D and right is 1D, then we do a matrix-vector multiplication.

	else if (Left.m_Dims.GetCount() >= 2 && Right.m_Dims.GetCount() == 1)
		{
		//	Convert right into a 2D tensor with one column.

		CDatum dB = Create2DTensor(Right.m_Dims[0].iLength, 1, Right.m_dElementType, dValue);
		CDatum dResult = MathMatMul(dB);
		if (dResult.IsError())
			return dResult;

		//	Remove any trailing dimensions that are 1.

		const CAEONTensor& Result = AsTensor(dResult);
		return Result.SqueezeTrailingDims();
		}

	//	If both are at least 2D, then we do a matrix multiplication.

	else if (Left.m_Dims.GetCount() >= 2 && Right.m_Dims.GetCount() >= 2)
		{
		int iRows = Left.m_Dims[Left.m_Dims.GetCount() - 2].iLength;
		int iCols = Right.m_Dims[Right.m_Dims.GetCount() - 1].iLength;
		int K1 = Left.m_Dims[Left.m_Dims.GetCount() - 1].iLength;
		int K2 = Right.m_Dims[Right.m_Dims.GetCount() - 2].iLength;

		if (K1 != K2)
			return CDatum::CreateError(ERR_TENSOR_MATMUL_MISMATCH);

		//	If we have more than 2 dimensions on either side then we figure out 
		//	leading dimensions so we can broadcast.

		TArray<int> LeftBatch;
		for (int i = 0; i < Left.m_Dims.GetCount() - 2; i++)
			LeftBatch.Insert(Left.m_Dims[i].iLength);

		TArray<int> RightBatch;
		for (int i = 0; i < Right.m_Dims.GetCount() - 2; i++)
			RightBatch.Insert(Right.m_Dims[i].iLength);

		TArray<int> BatchDims = CalcBroadcastDims(LeftBatch, RightBatch);
		if (BatchDims.GetCount() == 0 && (LeftBatch.GetCount() > 0 || RightBatch.GetCount() > 0))
			return CDatum::CreateError(ERR_TENSOR_MATMUL_CANT_BROADCAST);

		//	Create a destination tensor with the right dimensions.

		TArray<int> NewDims;
		NewDims.Insert(BatchDims);
		NewDims.Insert(iRows);
		NewDims.Insert(iCols);

		CDatum dResult = CreateNTensor(NewDims, Left.m_dElementType);
		CAEONTensor& Result = *(CAEONTensor*)dResult.GetComplex();

		//	Now generate all index permutations.

		TArray<TArray<int>> BatchIndices = CalcIndexPermutations(BatchDims);

		//	Now multiply each batch.

		for (int i = 0; i < BatchIndices.GetCount(); i++)
			{
			const TArray<int>& Indices = BatchIndices[i];

			//	Set the cursor for left tensor.

			Iterator LeftCursor = Left.begin();
			ApplyBatchIndices(LeftCursor, Indices, 2);

			//	Set the cursor for right tensor.

			Iterator RightCursor = Right.begin();
			ApplyBatchIndices(RightCursor, Indices, 2);

			//	And finally, set the cursor for the result tensor.

			Iterator ResultCursor = Result.begin();
			ApplyBatchIndices(ResultCursor, Indices, 2);

			//	Loop through the rows and columns of the result tensor

			for (int iRow = 0; iRow < iRows; iRow++)
				{
				ResultCursor.SetDim(Result.m_Dims.GetCount() - 1, 0);

				RightCursor.SetDim(Right.m_Dims.GetCount() - 2, 0);
				RightCursor.SetDim(Right.m_Dims.GetCount() - 1, 0);

				for (int iCol = 0; iCol < iCols; iCol++)
					{
					double rSum = 0.0;

					//	Loop through the K1 dimension and calculate the dot product

					Iterator L1 = LeftCursor;
					Iterator R1 = RightCursor;
					for (int k = 0; k < K1; k++)
						{
						double rLeft = *L1;
						double rRight = *R1;
						rSum += rLeft * rRight;

						//	Advance the cursors

						L1.IncDim(Left.m_Dims.GetCount() - 1);
						R1.IncDim(Right.m_Dims.GetCount() - 2);
						}

					Result.m_dData.SetElement(ResultCursor.GetDataIndex(), rSum);

					if (iCol + 1 < iCols)
						{
						ResultCursor.IncDim(Result.m_Dims.GetCount() - 1);
						RightCursor.IncDim(Right.m_Dims.GetCount() - 1);
						}
					}

				ResultCursor.IncDim(Result.m_Dims.GetCount() - 2);
				LeftCursor.IncDim(Left.m_Dims.GetCount() - 2);
				}
			}

		return dResult;
		}
	else
		return CDatum();	//	Should never get here.
	}

CDatum CAEONTensor::MathMax () const
	{
	bool bFound = false;
	CDatum dMax;
	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dValue = (*i).MathMax();
		if (dValue.IsNil())
			continue;

		else if (dValue.IsIdenticalToNaN())
			return dValue;

		else if (!bFound)
			{
			dMax = dValue;
			bFound = true;
			}

		else
			{
			int iCompare = dValue.OpCompare(dMax);
			if (iCompare == 1)
				dMax = dValue;
			else if (iCompare == 2 || iCompare == -2)
				return CDatum::CreateNaN();
			}
		}

	return dMax;
	}

CDatum CAEONTensor::MathMedian () const
	{
	TArray<CNumberValue> Sorted;
	Sorted.GrowToFit(CalcDataCount());
	for (Iterator i = begin(); i != end(); ++i)
		{
		Sorted.Insert(CNumberValue((*i).MathMedian()));
		if (!Sorted[Sorted.GetCount() - 1].IsValidNumber())
			return CDatum::CreateNaN();
		}

	Sorted.Sort();
	if (Sorted.GetCount() % 2 == 0)
		{
		CNumberValue Result(Sorted[Sorted.GetCount() / 2 - 1]);
		Result.Add(Sorted[Sorted.GetCount() / 2].GetDatum());
		Result.Divide(2.0);
		return Result.GetDatum();
		}
	else
		return Sorted[Sorted.GetCount() / 2].GetDatum();
	}

CDatum CAEONTensor::MathMin () const
	{
	bool bFound = false;
	CDatum dMin;
	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dValue = (*i).MathMin();
		if (dValue.IsNil())
			continue;

		else if (dValue.IsIdenticalToNaN())
			return dValue;

		else if (!bFound)
			{
			dMin = dValue;
			bFound = true;
			}

		else
			{
			int iCompare = dValue.OpCompare(dMin);
			if (iCompare == -1)
				dMin = dValue;
			else if (iCompare == 2 || iCompare == -2)
				return CDatum::CreateNaN();
			}
		}

	return dMin;
	}

CDatum CAEONTensor::MathRound () const
	{
	CAEONTensor *pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);

	//	NOTE: This new array might be a different shape than the original if
	//	we are a slice of a larger tensor. Setting elements the long way will
	//	ensure that we get the right shape.

	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dRound = (*i).MathRound();
		int iDest = pResult->CalcFlatIndex(i.AsIndices());
		pResult->m_dData.raw_SetArrayElement(iDest, dRound);
		}

	return dResult;
	}

CDatum CAEONTensor::MathSign () const
	{
	CAEONTensor *pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);

	//	NOTE: This new array might be a different shape than the original if
	//	we are a slice of a larger tensor. Setting elements the long way will
	//	ensure that we get the right shape.

	for (Iterator i = begin(); i != end(); ++i)
		{
		CDatum dSign = (*i).MathSign();
		int iDest = pResult->CalcFlatIndex(i.AsIndices());
		pResult->m_dData.raw_SetArrayElement(iDest, dSign);
		}

	return dResult;
	}

CDatum CAEONTensor::MathAddToElements (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathAddToElements(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathAddElementsTo (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathAddElementsTo(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathDivideElementsBy (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathDivideElementsBy(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathDivideByElements (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathDivideByElements(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathExpElementsTo (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathExpElementsTo(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathExpToElements (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathExpToElements(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathModElementsBy (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathModElementsBy(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathModByElements (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathModByElements(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathMultiplyElements (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathMultiplyElements(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathNegateElements () const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathNegateElements();
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathSubtractFromElements (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathSubtractFromElements(dValue);
	return CDatum(pResult);
	}

CDatum CAEONTensor::MathSubtractElementsFrom (CDatum dValue) const
	{
	CAEONTensor* pResult = new CAEONTensor(*this);
	pResult->m_dData = m_dData.MathSubtractElementsFrom(dValue);
	return CDatum(pResult);
	}

size_t CAEONTensor::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	size_t TotalSize = 2 + CalcDataCount();

	for (Iterator i = begin(); i != end(); ++i)
		TotalSize += (*i).CalcSerializeSize(iFormat);

	return TotalSize;
	}

void CAEONTensor::OnMarked (void)
	{
	m_dDatatype.Mark();
	m_dElementType.Mark();
	m_dData.Mark();

	for (int i = 0; i < m_Dims.GetCount(); i++)
		m_Dims[i].dType.Mark();
	}

int CAEONTensor::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string and int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string and int)

	{
	switch (iValueType)
		{
		case CDatum::typeTensor:
			{
			//	Start by comparing the number of dimensions.

			const CAEONTensor& Tensor2 = AsTensor(dValue);
			if (m_Dims.GetCount() < Tensor2.m_Dims.GetCount())
				return -1;
			else if (m_Dims.GetCount() > Tensor2.m_Dims.GetCount())
				return 1;

			//	Compare tensors of the same dimensions

			return CompareTensor(m_Dims, m_iDataStart, m_dData, Tensor2.m_Dims, Tensor2.m_iDataStart, Tensor2.m_dData, 0);
			}

		case CDatum::typeArray:
			{
			//	Must have only one dimension

			if (m_Dims.GetCount() < 1)
				return -1;
			else if (m_Dims.GetCount() > 1)
				return 1;

			//	Compare the elements in sequence

			int iCount = Min(m_Dims[0].iLength, dValue.GetCount());
			for (int i = 0; i < iCount; i++)
				{
				int iCompare = GetElementAt(i).OpCompare(dValue.GetElement(i));
				if (iCompare != 0)
					return iCompare;
				}

			//	Compare the size of the array

			return KeyCompare(m_Dims[0].iLength, dValue.GetCount());
			}

		default:
			{
			int iCompare = KeyCompareNoCase(AsString(), dValue.AsString());
			if (iCompare < 0)
				return -2;
			else if (iCompare > 0)
				return 2;
			else
				return 0;
			}
		}
	}

int CAEONTensor::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const

//	OpCompareExact
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string and int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string and int)

	{
	switch (iValueType)
		{
		case CDatum::typeTensor:
			{
			//	LATER: For now we assume that all tensor types are implemented 
			//	with this class.
			//
			//	Start by comparing the number of dimensions.

			const CAEONTensor& Tensor2 = AsTensor(dValue);
			if (m_Dims.GetCount() < Tensor2.m_Dims.GetCount())
				return -1;
			else if (m_Dims.GetCount() > Tensor2.m_Dims.GetCount())
				return 1;

			//	Compare tensors of the same dimensions

			return CompareTensorExact(m_Dims, m_iDataStart, m_dData, Tensor2.m_Dims, Tensor2.m_iDataStart, Tensor2.m_dData, 0);
			}

		case CDatum::typeArray:
			{
			//	Must have only one dimension

			if (m_Dims.GetCount() < 1)
				return -1;
			else if (m_Dims.GetCount() > 1)
				return 1;

			//	Compare the elements in sequence

			int iCount = Min(m_Dims[0].iLength, dValue.GetCount());
			for (int i = 0; i < iCount; i++)
				{
				int iCompare = GetElementAt(i).OpCompareExact(dValue.GetElement(i));
				if (iCompare != 0)
					return iCompare;
				}

			//	Compare the size of the array

			return KeyCompare(m_Dims[0].iLength, dValue.GetCount());
			}

		default:
			{
			int iCompare = KeyCompare(AsString(), dValue.AsString());
			if (iCompare < 0)
				return -2;
			else if (iCompare > 0)
				return 2;
			else
				return 0;
			}
		}
	}

CDatum CAEONTensor::OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const
	{
	if (iAxis < 0 || iAxis >= m_Dims.GetCount())
		return CDatum::CreateError(ERR_INVALID_AXIS);

	CDatum dSrcShape = dSrc.GetProperty(FIELD_SHAPE);
	if (dSrcShape.GetCount() >= 1 && dSrcShape.GetCount() == m_Dims.GetCount() - 1)
		return OpConcatenatedMinus1(Ctx, dSrc, iAxis);

	else if (dSrcShape.GetCount() != m_Dims.GetCount())
		return CDatum::CreateError(strPattern(ERR_CONCAT_INVALID_DIMENSIONS, dSrcShape.GetCount(), m_Dims.GetCount()));

	//	Create types for each dimension of the result.

	TArray<CDatum> ResultDims;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		if (i == iAxis)
			ResultDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, m_Dims[i].iLength + (int)dSrcShape.GetElement(i) - 1));
		else
			{
			ResultDims.Insert(m_Dims[i].dType);
			if ((int)dSrcShape.GetElement(i) != m_Dims[i].iLength)
				return CDatum::CreateError(strPattern(ERR_CONCAT_INVALID_DIMENSION_SIZE, i, m_Dims[i].iLength, (int)dSrcShape.GetElement(i)));
			}
		}

	//	Create the resulting tensor

	CDatum dResultType = Ctx.GetTypeSystem().AddAnonymousTensor(m_dElementType, ResultDims);
	CAEONTensor* pResult = new CAEONTensor(dResultType);

	//	Copy the original tensor to the result.

	Iterator src = begin();
	while (src != end())
		{
		Iterator dest = Iterator(*pResult, src.AsIndices());

		pResult->m_dData.SetElement(dest.GetDataIndex(), *src);
		++src;
		}

	//	Copy the source tensor to the result.

	CDatum dSrcPos = dSrc.IteratorBegin();
	Iterator dest = Iterator(*pResult);
	while (!dSrcPos.IsNil())
		{
		for (int i = 0; i < dSrcPos.GetCount(); i++)
			{
			if (i == iAxis)
				dest.SetDim(i, m_Dims[i].iLength + (int)dSrcPos.GetElement(i));
			else
				dest.SetDim(i, dSrcPos.GetElement(i));
			}

		pResult->m_dData.SetElement(dest.GetDataIndex(), dSrc.GetElementAt(Ctx.GetTypeSystem(), dSrcPos));
		dSrcPos = dSrc.IteratorNext(dSrcPos);
		}

	return CDatum(pResult);
	}

CDatum CAEONTensor::OpConcatenatedMinus1 (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const
	{
	//	We assume that the source has 1 fewer dimensions than the original 
	//	tensor. We extend it to the same number of dimensions by adding a
	//	single dimension at the concatenation axis.

	CDatum dSrcShape = dSrc.GetProperty(FIELD_SHAPE);

	int iSrcDim = 0;
	TArray<CDatum> ResultDims;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		if (i == iAxis)
			ResultDims.Insert(CAEONTypes::CreateInt32SubRange(NULL_STR, 0, m_Dims[i].iLength + 1 - 1));
		else
			{
			ResultDims.Insert(m_Dims[i].dType);
			if ((int)dSrcShape.GetElement(iSrcDim) != m_Dims[i].iLength)
				return CDatum::CreateError(strPattern(ERR_CONCAT_INVALID_DIMENSION_SIZE, i, m_Dims[i].iLength, (int)dSrcShape.GetElement(i)));

			iSrcDim++;
			}
		}

	//	Create the resulting tensor

	CDatum dResultType = Ctx.GetTypeSystem().AddAnonymousTensor(m_dElementType, ResultDims);
	CAEONTensor* pResult = new CAEONTensor(dResultType);

	//	Copy the original tensor to the result.

	Iterator src = begin();
	while (src != end())
		{
		Iterator dest = Iterator(*pResult, src.AsIndices());

		pResult->m_dData.SetElement(dest.GetDataIndex(), *src);
		++src;
		}

	//	Copy the source tensor to the result.

	CDatum dSrcPos = dSrc.IteratorBegin();
	Iterator dest = Iterator(*pResult);
	while (!dSrcPos.IsNil())
		{
		int iSrcDim = 0;
		for (int i = 0; i < pResult->m_Dims.GetCount(); i++)
			{
			if (i == iAxis)
				dest.SetDim(i, m_Dims[i].iLength);
			else
				{
				dest.SetDim(i, dSrcPos.GetElement(iSrcDim));
				iSrcDim++;
				}
			}

		pResult->m_dData.SetElement(dest.GetDataIndex(), dSrc.GetElementAt(Ctx.GetTypeSystem(), dSrcPos));
		dSrcPos = dSrc.IteratorNext(dSrcPos);
		}

	return CDatum(pResult);
	}

CDatum CAEONTensor::OpIdentity () const
	{
	ASSERT(IsSquareMatrix());

	CAEONTensor* pResult = new CAEONTensor(m_dDatatype);
	CDatum dResult(pResult);

	//	We start with the identity matrix

	int iSize = m_Dims[0].iLength;
	for (int i = 0; i < iSize; i++)
		{
		for (int j = 0; j < iSize; j++)
			{
			if (i == j)
				dResult.SetElementAt2DI(i, j, CDatum(1));
			else
				dResult.SetElementAt2DI(i, j, CDatum(0));
			}
		}

	return dResult;
	}

bool CAEONTensor::SwapNonZeroRow (CAEONTensor& Augmented, int iRow) const
	{
	int iSize = m_Dims[0].iLength;

	for (int i = iRow + 1; i < iSize; i++)
		{
		if ((double)Augmented.GetElementAt2DI(i, iRow) != 0.0)
			{
			//	Swap the rows
			for (int j = 0; j < 2 * iSize; j++)
				{
				CDatum dTemp = Augmented.GetElementAt2DI(iRow, j);
				Augmented.SetElementAt2DI(iRow, j, Augmented.GetElementAt2DI(i, j));
				Augmented.SetElementAt2DI(i, j, dTemp);
				}
			return true;
			}
		}

	return false;
	}

bool CAEONTensor::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
	{
	switch (iValueType)
		{
		case CDatum::typeTensor:
			{
			//	Must have same number of dimensions.

			const CAEONTensor& Tensor2 = AsTensor(dValue);
			if (m_Dims.GetCount() != Tensor2.m_Dims.GetCount())
				return false;

			//	Dimensions must be the same size.

			for (int i = 0; i < m_Dims.GetCount(); i++)
				if (m_Dims[i].iLength != Tensor2.m_Dims[i].iLength)
					return false;

			//	Now iterate over all elements and compare.

			Iterator i1 = begin();
			Iterator i2 = Tensor2.begin();

			while (i1 != end())
				{
				if (!(*i1).OpIsEqual(*i2))
					return false;

				++i1;
				++i2;
				}

			return true;
			}

		case CDatum::typeArray:
			{
			//	Must have only one dimension and be the same size

			if (m_Dims.GetCount() != 1 || m_Dims[0].iLength != dValue.GetCount())
				return false;

			//	Compare the elements in sequence

			for (int i = 0; i < m_Dims[0].iLength; i++)
				{
				if (!GetElementAt(i).OpIsEqual(dValue.GetElement(i)))
					return false;
				}

			return true;
			}

		default:
			return false;
		}
	}

bool CAEONTensor::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const
	{
	switch (iValueType)
		{
		case CDatum::typeTensor:
			{
			//	Must have same number of dimensions.

			const CAEONTensor& Tensor2 = AsTensor(dValue);
			if (m_Dims.GetCount() != Tensor2.m_Dims.GetCount())
				return false;

			//	Dimensions must be the same size.

			for (int i = 0; i < m_Dims.GetCount(); i++)
				if (m_Dims[i].iLength != Tensor2.m_Dims[i].iLength)
					return false;

			//	Now iterate over all elements and compare.

			Iterator i1 = begin();
			Iterator i2 = Tensor2.begin();

			while (i1 != end())
				{
				if (!(*i1).OpIsIdentical(*i2))
					return false;

				++i1;
				++i2;
				}

			return true;
			}

		case CDatum::typeArray:
			{
			//	Must have only one dimension and be the same size

			if (m_Dims.GetCount() != 1 || m_Dims[0].iLength != dValue.GetCount())
				return false;

			//	Compare the elements in sequence

			for (int i = 0; i < m_Dims[0].iLength; i++)
				{
				if (!GetElementAt(i).OpIsIdentical(dValue.GetElement(i)))
					return false;
				}

			return true;
			}

		default:
			return false;
		}
	}

CDatum CAEONTensor::raw_IteratorGetElement (CBuffer& Buffer) const
	{
	Iterator* pIterator = (Iterator*)Buffer.GetPointer();
	return m_dData.GetElement(pIterator->GetDataIndex());
	}

CDatum CAEONTensor::raw_IteratorGetKey (CBuffer& Buffer) const
	{
	Iterator* pIterator = (Iterator*)Buffer.GetPointer();
	return pIterator->AsIndexDatum();
	}

bool CAEONTensor::raw_IteratorHasMore (CBuffer& Buffer) const
	{
	Iterator* pIterator = (Iterator*)Buffer.GetPointer();
	return *pIterator != end();
	}

void CAEONTensor::raw_IteratorNext (CBuffer& Buffer) const
	{
	Iterator* pIterator = (Iterator*)Buffer.GetPointer();
	++(*pIterator);
	}

void CAEONTensor::raw_IteratorSetElement (CBuffer& Buffer, CDatum dValue)
	{
	Iterator* pIterator = (Iterator*)Buffer.GetPointer();

	//	Compute the data index in THIS tensor based on the indices in the given
	//	iterator. NOTE: We do not assume that the iterator is valid for this tensor
	//	(only that it is the same shape).

	int iDataIndex = CalcIndex(pIterator->AsIndices());
	if (iDataIndex == -1)
		return;

	m_dData.SetElement(iDataIndex, dValue);
	}

CBuffer CAEONTensor::raw_IteratorStart () const
	{
	Iterator i = begin();
	CBuffer Buffer(sizeof(i));
	new(Buffer.GetPointer()) Iterator(i);
	return Buffer;
	}

void CAEONTensor::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			m_dData.Serialize(iFormat, Stream);
			break;
			}

		case CDatum::EFormat::GridLang:
			{
			//	LATER: Output as nested arrays.
			m_dData.Serialize(iFormat, Stream);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			m_dData.Serialize(iFormat, Stream);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

void CAEONTensor::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_TENSOR))
		return;

	//	Flags

	DWORD dwFlags = 0;
	dwFlags |= (m_bColumnMajor ? 0x00000001 : 0);
	Stream.Write(dwFlags);

	//	Serialize the datatype

	m_dDatatype.SerializeAEON(Stream, Serialized);

	//	Data

	Stream.Write(m_iDataStart);
	m_dData.SerializeAEON(Stream, Serialized);
	}

void CAEONTensor::SetElement (int iIndex, CDatum dDatum)
	{
	if (m_Dims.GetCount() == 0)
		return;
	else if (m_Dims.GetCount() == 1)
		{
		int iPos = CalcIndex1D(iIndex);
		if (iPos == -1)
			return;

		m_dData.raw_SetArrayElement(iPos, dDatum);
		}
	else if (m_Dims.GetCount() == 2)
		{
		//	We expect an array of values to place into the slice.

		int iPos = CalcIndex2D(iIndex, m_Dims[1].iOrigin);
		if (iPos == -1)
			return;

		for (int i = 0; i < m_Dims[1].iLength; i++)
			{
			m_dData.raw_SetArrayElement(iPos, dDatum.GetElement(i));
			iPos += m_Dims[1].iStride;
			}
		}
	else
		{
		//	Otherwise, we expect dDatum to be a tensor that matches the 
		//	dimensions of a slice of the first dimension.

		if (dDatum.GetBasicType() != CDatum::typeTensor)
			return;

		const CAEONTensor& Tensor = AsTensor(dDatum);
		if (Tensor.m_Dims.GetCount() != m_Dims.GetCount() - 1)
			return;

		//	We expect the tensor to have the same dimensions as a slice of the
		//	first dimension.

		for (int i = 1; i < m_Dims.GetCount(); i++)
			if (Tensor.m_Dims[i - 1].iLength != m_Dims[i].iLength)
				return;

		//	Loop over all elements in the tensor and copy them to the appropriate

		TArray<int> Cursor;
		Cursor.InsertEmpty(m_Dims.GetCount());
		Cursor[0] = iIndex;

		for (Iterator i = Tensor.begin(); i != Tensor.end(); ++i)
			{
			for (int j = 1; j < m_Dims.GetCount(); j++)
				Cursor[j] = i.AsIndices()[j - 1];

			int iPos = CalcIndex(Cursor);
			if (iPos == -1)
				return;

			m_dData.raw_SetArrayElement(iPos, *i);
			}
		}
	}
			
void CAEONTensor::SetElementAt (CDatum dIndex, CDatum dDatum)
	{
	SSliceDesc Slice = CalcSlice(dIndex);
	if (Slice.IsEmpty())
		return;
	else if (Slice.IsSingle())
		{
		m_dData.raw_SetArrayElement(Slice.iPos, dDatum);
		}
	else
		{
		//	Convert the value to a slice of the appropriate type.

		CDatum dNewValue = CDatum::CreateAsType(Slice.dSliceType, dDatum);
		if (dNewValue.IsNil())
			return;

		const CAEONTensor& Src = AsTensor(dNewValue);
		SetSliceElements(Src, 0, Src.begin(), Slice.SliceIndex, 0, begin());
		}
	}

void CAEONTensor::SetElementAt2DA (CDatum dIndex1, CDatum dIndex2, CDatum dValue)
	{
	SSliceDesc Slice = CalcSlice2D(dIndex1, dIndex2);
	if (Slice.IsEmpty())
		return;
	else if (Slice.IsSingle())
		{
		m_dData.raw_SetArrayElement(Slice.iPos, dValue);
		}
	else
		{
		//	Convert the value to a slice of the appropriate type.

		CDatum dNewValue = CDatum::CreateAsType(Slice.dSliceType, dValue);
		if (dNewValue.IsNil())
			return;

		const CAEONTensor& Src = AsTensor(dNewValue);
		SetSliceElements(Src, 0, Src.begin(), Slice.SliceIndex, 0, begin());
		}
	}

void CAEONTensor::SetElementAt2DI (int iIndex1, int iIndex2, CDatum dValue)
	{
	if (m_Dims.GetCount() != 2)
		return;

	else if (m_bStandardDims)
		{
		if (iIndex1 < 0 || iIndex1 >= m_Dims[0].iLength || iIndex2 < 0 || iIndex2 >= m_Dims[1].iLength)
			return;

		m_dData.raw_SetArrayElement(m_Dims[0].iStride * iIndex1 + iIndex2, dValue);
		}

	else
		{
		int iPos = CalcIndex2D(iIndex1, iIndex2);
		if (iPos == -1)
			return;

		m_dData.raw_SetArrayElement(iPos, dValue);
		}
	}

void CAEONTensor::SetElementAt3DA (CDatum dIndex1, CDatum dIndex2, CDatum dIndex3, CDatum dValue)
	{
	SSliceDesc Slice = CalcSlice3D(dIndex1, dIndex2, dIndex3);
	if (Slice.IsEmpty())
		return;
	else if (Slice.IsSingle())
		{
		m_dData.raw_SetArrayElement(Slice.iPos, dValue);
		}
	else
		{
		//	Convert the value to a slice of the appropriate type.

		CDatum dNewValue = CDatum::CreateAsType(Slice.dSliceType, dValue);
		if (dNewValue.IsNil())
			return;

		const CAEONTensor& Src = AsTensor(dNewValue);
		SetSliceElements(Src, 0, Src.begin(), Slice.SliceIndex, 0, begin());
		}
	}

void CAEONTensor::SetElementAt3DI (int iIndex1, int iIndex2, int iIndex3, CDatum dValue)
	{
	if (m_Dims.GetCount() == 3)
		{
		int iPos = CalcIndex3D(iIndex1, iIndex2, iIndex3);
		if (iPos == -1)
			return;

		m_dData.raw_SetArrayElement(iPos, dValue);
		}
	}

void CAEONTensor::GetSliceElements (CAEONTensor& Dest, const TArray<CDatum>& SrcIndex, int iSrcLevel, const Iterator& SrcPos, int iDestLevel, const Iterator& DestPos) const

//	GetSliceElements
//
//	This is a recursive call in which extract a slice from the tensor and return it
//	in Dest.

	{
	if (iSrcLevel == m_Dims.GetCount())
		{
		int iDestIndex = Dest.CalcIndex(DestPos.AsIndices());
		if (iDestIndex != -1)
			Dest.m_dData.raw_SetArrayElement(iDestIndex, *SrcPos);
		}
	else
		{
		CDatum dSrcDimIndex = (iSrcLevel < SrcIndex.GetCount() ? SrcIndex[iSrcLevel] : CDatum());

		//	True (*) means that we take the entire dimension.
		//	Nil means we did not supply enough indices and again we take the 
		//	entire dimension.

		if (dSrcDimIndex.IsNil() || dSrcDimIndex.IsIdenticalToTrue())
			{
			Iterator Dst = DestPos;
			for (Iterator Src = SrcPos; Src != end(); Src.IncDim(iSrcLevel), Dst.IncDim(iDestLevel))
				{
				GetSliceElements(Dest, SrcIndex, iSrcLevel + 1, Src, iDestLevel + 1, Dst);
				}
			}
		else if (dSrcDimIndex.GetBasicType() == CDatum::typeRange)
			{
			const IAEONRange* pRange = dSrcDimIndex.GetRangeInterface();
			if (pRange == NULL)
				return;

			int iStart = pRange->GetStart();
			int iEnd = pRange->GetEnd();
			int iStep = pRange->GetStep();

			int iIndex = iStart;
			Iterator Src = SrcPos;
			Iterator Dst = DestPos;

			for (int i = 0; i < pRange->GetLength(); i++)
				{
				//	OK if Src == end (which means we are out of bounds).

				Src.SetDim(iSrcLevel, iIndex);
				if (Src != end())
					GetSliceElements(Dest, SrcIndex, iSrcLevel + 1, Src, iDestLevel + 1, Dst);

				Dst.IncDim(iDestLevel);

				iIndex += iStep;
				}
			}
		else if (dSrcDimIndex.IsArray())
			{
			Iterator Src = SrcPos;
			Iterator Dst = DestPos;

			for (int i = 0; i < dSrcDimIndex.GetCount(); i++)
				{
				//	OK if Src == end (which means we are out of bounds).

				Src.SetDim(iSrcLevel, (int)dSrcDimIndex.GetElement(i));
				if (Src != end())
					GetSliceElements(Dest, SrcIndex, iSrcLevel + 1, Src, iDestLevel + 1, Dst);

				Dst.IncDim(iDestLevel);
				}
			}
		else
			{
			int iSrcIndex = (int)dSrcDimIndex;
			Iterator Src = SrcPos;
			Src.SetDim(iSrcLevel, iSrcIndex);

			GetSliceElements(Dest, SrcIndex, iSrcLevel + 1, Src, iDestLevel, DestPos);
			}
		}
	}

void CAEONTensor::SetSliceElements (const CAEONTensor& Src, int iSrcLevel, const Iterator& SrcPos, const TArray<CDatum>& DestIndex, int iDestLevel, const Iterator& DestPos)

//	SetSliceElements
//
//	This is a recursive call in which extract a slice from the tensor and return it
//	in Dest.

	{
	if (iDestLevel == m_Dims.GetCount())
		{
		int iDestIndex = CalcIndex(DestPos.AsIndices());
		if (iDestIndex != -1)
			{
			m_dData.raw_SetArrayElement(iDestIndex, *SrcPos);
			}
		}
	else
		{
		CDatum dDestDimIndex = (iDestLevel < DestIndex.GetCount() ? DestIndex[iDestLevel] : CDatum());

		//	True (*) means that we take the entire dimension.
		//	Nil means we did not supply enough indices and again we take the 
		//	entire dimension.

		if (dDestDimIndex.IsNil() || dDestDimIndex.IsIdenticalToTrue())
			{
			Iterator Sr = SrcPos;
			for (Iterator Dst = DestPos; Dst != end(); Sr.IncDim(iSrcLevel), Dst.IncDim(iDestLevel))
				{
				SetSliceElements(Src, iSrcLevel + 1, Sr, DestIndex, iDestLevel + 1, Dst);
				}
			}
		else if (dDestDimIndex.GetBasicType() == CDatum::typeRange)
			{
			const IAEONRange* pRange = dDestDimIndex.GetRangeInterface();
			if (pRange == NULL)
				return;

			int iStart = pRange->GetStart();
			int iEnd = pRange->GetEnd();
			int iStep = pRange->GetStep();

			int iIndex = iStart;
			Iterator Sr = SrcPos;
			Iterator Dst = DestPos;

			for (int i = 0; i < pRange->GetLength(); i++)
				{
				//	OK if Src == end (which means we are out of bounds).

				Dst.SetDim(iDestLevel, iIndex);
				if (Dst != end())
					SetSliceElements(Src, iSrcLevel + 1, Sr, DestIndex, iDestLevel + 1, Dst);

				Sr.IncDim(iSrcLevel);

				iIndex += iStep;
				}
			}
		else if (dDestDimIndex.IsArray())
			{
			Iterator Sr = SrcPos;
			Iterator Dst = DestPos;

			for (int i = 0; i < dDestDimIndex.GetCount(); i++)
				{
				//	OK if Dst == end (which means we are out of bounds).

				Dst.SetDim(iDestLevel, (int)dDestDimIndex.GetElement(i));
				if (Dst != end())
					SetSliceElements(Src, iSrcLevel + 1, Sr, DestIndex, iDestLevel + 1, Dst);

				Sr.IncDim(iDestLevel);
				}
			}
		else
			{
			int iDestIndex = (int)dDestDimIndex;
			Iterator Dst = DestPos;
			Dst.SetDim(iDestLevel, iDestIndex);

			SetSliceElements(Src, iSrcLevel + 1, SrcPos, DestIndex, iDestLevel + 1, Dst);
			}
		}
	}

CDatum CAEONTensor::SqueezeLeadingDims () const

//	SqueezeLeadingDims
//
//	If this tensor has leading dimensions that are 1, then we remove them
//	and return a new tensor with the remaining dimensions. Otherwise, we return
//	the original tensor.

	{
	bool bSqueezed = false;
	TArray<int> NewDims;
	for (int i = 0; i < m_Dims.GetCount(); i++)
		{
		if (m_Dims[i].iLength == 1 && m_Dims[i].iOrigin == 0)
			bSqueezed = true;
		else
			NewDims.Insert(m_Dims[i].iLength);
		}

	if (!bSqueezed)
		return CDatum::raw_AsComplex(this);

	//	Create a new tensor with the new dimensions

	return CreateNTensor(NewDims, m_dElementType, m_dData);
	}

CDatum CAEONTensor::SqueezeTrailingDims () const

//	SqueezeTrailingDims
//
//	If this tensor has trailing dimensions that are 1, then we remove them
//	and return a new tensor with the remaining dimensions. Otherwise, we return
//	the original tensor.

	{
	int r = m_Dims.GetCount();
	int keep = r; // number of dims to keep (from the front)

	// Move left while trailing dims are exactly 1 (origin 0).

	while (keep > 0 
			&& m_Dims[keep - 1].iOrigin == 0 
			&& m_Dims[keep - 1].iLength == 1) 
		{
		--keep;
		}

	if (keep == r)
		{
		// nothing to squeeze
		return CDatum::raw_AsComplex(this);
		}

	TArray<int> NewDims;
	for (int i = 0; i < keep; ++i) 
		NewDims.Insert(m_Dims[i].iLength);

	return CreateNTensor(NewDims, m_dElementType, m_dData);
	}
