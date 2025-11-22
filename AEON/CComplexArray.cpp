//	CComplexArray.cpp
//
//	CComplexArray class
//	Copyright (c) 2010 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ARRAY,					"array");

TDatumPropertyHandler<CComplexArray> CComplexArray::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			CDatum dType = ((const IDatatype&)Obj.GetDatatype()).GetElementType();
			return (!dType.IsNil() ? dType : CAEONTypeSystem::GetCoreType(IDatatype::ANY));
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return CAEONTypes::Get(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.Append(Obj.GetCount());
			return dResult;
			},
		NULL,
		},
	{
		"size",
		"I",
		"Returns the number of element in the array.",
		[](const CComplexArray &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CComplexArray::m_pMethodsExt = NULL;

const CString &CComplexArray::GetTypename (void) const { return TYPENAME_ARRAY; }

CComplexArray::CComplexArray (CDatum dSrc)

//	ComplexArray constructor

	{
	int i;

	if (dSrc.IsStruct())
		{
		InsertEmpty(1);
		SetElement(0, dSrc);
		}
	else
		{
		int iCount = dSrc.GetCount();

		//	Clone from another complex array

		if (iCount > 0)
			{
			InsertEmpty(iCount);

			for (i = 0; i < iCount; i++)
				SetElement(i, dSrc.GetElement(i));
			}
		}
	}

CComplexArray::CComplexArray (const TArray<CString> &Src)

//	CComplexArray constructor

	{
	int i;
	int iCount = Src.GetCount();

	if (iCount > 0)
		{
		InsertEmpty(iCount);

		for (i = 0; i < iCount; i++)
			SetElement(i, Src[i]);
		}
	}

CComplexArray::CComplexArray (const TArray<CDatum> &Src)

//	CComplexArray constructor

	{
	int i;
	int iCount = Src.GetCount();

	if (iCount > 0)
		{
		InsertEmpty(iCount);

		for (i = 0; i < iCount; i++)
			SetElement(i, Src[i]);
		}
	}

CDatum CComplexArray::CalcMax (const TArray<CDatum>& Array)

//	CalcMax
//
//	Compute the maximum value.

	{
	if (Array.GetCount() == 0)
		return CDatum();

	bool bFound = false;
	CDatum dMax;
	for (int i = 0; i < Array.GetCount(); i++)
		{
		CDatum dValue = Array[i].MathMax();
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

CDatum CComplexArray::CalcMedian (const TArray<CDatum>& Array)

//	CalcMedian
//
//	Compute the median value.

	{
	if (Array.GetCount() == 0)
		return CDatum();

	TArray<CNumberValue> Sorted;
	Sorted.InsertEmpty(Array.GetCount());
	for (int i = 0; i < Array.GetCount(); i++)
		{
		Sorted[i] = CNumberValue(Array[i].MathMedian());
		if (!Sorted[i].IsValidNumber())
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

size_t CComplexArray::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory being used.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return 0;

	size_t dwSize = 0;

	for (int i = 0; i < m_Array.GetCount(); i++)
		dwSize += m_Array[i].CalcMemorySize();

	return dwSize;
	}

CDatum CComplexArray::CalcMin (const TArray<CDatum>& Array)

//	CalcMin
//
//	Compute the minimum value.

	{
	if (Array.GetCount() == 0)
		return CDatum();

	bool bFound = false;
	CDatum dMin;
	for (int i = 0; i < Array.GetCount(); i++)
		{
		CDatum dValue = Array[i].MathMin();
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

IComplexDatum *CComplexArray::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clones a copy

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CComplexArray(m_Array);

		case CDatum::EClone::CopyOnWrite:
			//	Default handling
			return NULL;

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CComplexArray(m_Array);
			pClone->CloneContents();
			return pClone;
			}

		case CDatum::EClone::Isolate:
			return NULL;

		default:
			throw CException(errFail);
		}
	}

void CComplexArray::CloneContents ()

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		//	LATER: This breaks the circular reference in the clone.
		//	Need to do better.
		return;

	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i] = m_Array[i].Clone(CDatum::EClone::DeepCopy);
	}

bool CComplexArray::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE if we contain dValue.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return false;

	for (int i = 0; i < m_Array.GetCount(); i++)
		if (m_Array[i].Contains(dValue))
			return true;

	return false;
	}

void CComplexArray::DeleteElement (int iIndex)

//	DeleteElement
//
//	Deletes the given element.

	{
	if (iIndex < 0 || iIndex >= GetCount())
		return;

	m_Array.Delete(iIndex);
	}

bool CComplexArray::Find (CDatum dValue, int *retiIndex) const

//	Find
//
//	Finds an element by value.

	{
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsEqual(GetElement(i)))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CDatum CComplexArray::FindAll (CDatum dValue) const

//	FindAll
//
//	Find all elements that match the value.

	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsEqual(GetElement(i)))
			dResult.Append(i);
		}

	return dResult;
	}

CDatum CComplexArray::FindAllExact (CDatum dValue) const

//	FindAllExact
//
//	Find all elements identical to the value.

	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsIdentical(GetElement(i)))
			dResult.Append(i);
		}

	return dResult;
	}

bool CComplexArray::FindExact (CDatum dValue, int *retiIndex) const

//	FindExact
//
//	Find an element that is identical to the value.

	{
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsIdentical(GetElement(i)))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

bool CComplexArray::FindElement (CDatum dValue, int *retiIndex) const

//	FindElement
//
//	Finds the given element

	{
	int i;

	for (i = 0; i < m_Array.GetCount(); i++)
		if (dValue.IsEqualCompatible(m_Array[i]))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}

	return false;
	}

int CComplexArray::FindMaxElementInArray (const TArray<CDatum>& Array)

//	FindMaxElementInArray
//
//	Returns the index of the maximum element in the array. Return -1 if there
//	are no elements in the array or if there are any NaN values.

	{
	int iMaxIndex = -1;

	CDatum dMax;
	for (int i = 0; i < Array.GetCount(); i++)
		{
		CDatum dValue = Array[i].MathMax();
		if (dValue.IsNil())
			continue;

		else if (dValue.IsIdenticalToNaN())
			return -1;

		else if (iMaxIndex == -1)
			{
			dMax = dValue;
			iMaxIndex = i;
			}
		else
			{
			int iCompare = dValue.OpCompare(dMax);
			if (iCompare == 1)
				{
				dMax = dValue;
				iMaxIndex = i;
				}
			else if (iCompare == 2 || iCompare == -2)
				return -1;
			}
		}

	return iMaxIndex;
	}

int CComplexArray::FindMinElementInArray (const TArray<CDatum>& Array)

//	FindMinElementInArray
//
//	Returns the index of the minimum element in the array. Return -1 if there
//	are no elements in the array or if there are any NaN values.

	{
	int iMinIndex = -1;

	CDatum dMin;
	for (int i = 0; i < Array.GetCount(); i++)
		{
		CDatum dValue = Array[i].MathMax();
		if (dValue.IsNil())
			continue;

		else if (dValue.IsIdenticalToNaN())
			return -1;

		else if (iMinIndex == -1)
			{
			dMin = dValue;
			iMinIndex = i;
			}
		else
			{
			int iCompare = dValue.OpCompare(dMin);
			if (iCompare == -1)
				{
				dMin = dValue;
				iMinIndex = i;
				}
			else if (iCompare == 2 || iCompare == -2)
				return -1;
			}
		}

	return iMinIndex;
	}

CString CComplexArray::Format (const CStringFormat& Fmt) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	return Format(m_Array, Fmt);
	}

CString CComplexArray::Format (const TArray<CDatum>& Array, const CStringFormat& Fmt)
	{
	CStringBuffer Output;

	Output.Write("[", 1);

	for (int i = 0; i < Array.GetCount(); i++)
		{
		if (i != 0)
			Output.Write(", ", 2);

		Output.Write(Array[i].Format(Fmt));
		}

	Output.Write("]", 1);

	return CString(std::move(Output));
	}

CDatum CComplexArray::GetElementAt (int iIndex) const
	{
	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		return m_Array[iIndex];
	else if (iIndex < 0)
		{
		int iNewIndex = iIndex + m_Array.GetCount();
		if (iNewIndex >= 0 && iNewIndex < m_Array.GetCount())
			return m_Array[iNewIndex];
		else
			return CDatum();
		}
	else
		return CDatum();
	}

CDatum CComplexArray::GetIndices (CDatum dArray)

//	GetIndices
//
//	Returns an array of indices for the given array.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(dArray.GetCount());
	
	for (int i = 0; i < dArray.GetCount(); i++)
		dResult.Append(CDatum(i));
	
	return dResult;
	}

void CComplexArray::InsertElementAt (CDatum dIndex, CDatum dDatum)

//	InsertElementAt
//
//	Inserts an element at the given index.

	{
	OnCopyOnWrite();

	bool bFromEnd;
	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount(), &bFromEnd);

	//	If from the end, we insert AFTER the specified position (because
	//	we want -1 to mean AFTER the last entry).

	if (bFromEnd && iIndex >= 0)
		iIndex++;

	//	Handle different cases

	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		{
		m_Array.Insert(dDatum, iIndex);
		}
	else if (iIndex >= 0)
		{
		int iNew = iIndex - GetCount();
		GrowToFit(iNew + 1);
		for (int i = 0; i < iNew; i++)
			Append(CDatum());

		Append(dDatum);
		}
	else if (dIndex.IsContainer())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			InsertElementAt(dIndex.GetElement(i), dDatum);
			}
		}
	}

CDatum CComplexArray::MathAbs () const

//	MathAbs
//
//	Returns a new array of absolute values.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		dResult.Append(m_Array[i].MathAbs());
		}
	return dResult;
	}

void CComplexArray::MathAccumulateStats (CDatum::SStatsCtx& Stats) const

//	MathAccumulateStats
//
//	Accumulate statistical values. We recurse into any nested arrays.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].MathAccumulateStats(Stats);
	}

CDatum CComplexArray::MathRound () const

//	MathRound
//
//	Returns a new array of absolute values.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum();

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		dResult.Append(m_Array[i].MathRound());
		}
	return dResult;
	}

size_t CComplexArray::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	int i;
	size_t TotalSize = 2 + m_Array.GetCount();

	for (i = 0; i < m_Array.GetCount(); i++)
		TotalSize += m_Array[i].CalcSerializeSize(iFormat);

	return TotalSize;
	}

void CComplexArray::OnCopyOnWrite ()

//	OnCopyOnWrite
//
//	We're about to be modified, so see if we need to make a copy.

	{
	if (m_bCopyOnWrite)
		{
		CloneContents();
		m_bCopyOnWrite = false;
		}
	}

void CComplexArray::OnMarked (void)

//	OnMarked
//
//	Mark any elements that we own

	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].Mark();
	}

CDatum CComplexArray::OpConcatenated (IInvokeCtx& Ctx, CDatum dSrc, int iAxis) const
	{
	return CAEONOp::ExecConcatenateArray_Array(Ctx, CDatum::raw_AsComplex(this), dSrc, iAxis);
	}

int CComplexArray::CompareArray (CDatum dSrc, CDatum::Types iValueType, CDatum dValue)

//	CompareArray
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string and int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string and int)

	{
	switch (iValueType)
		{
		case CDatum::typeArray:
			{
			CRecursionGuard Guard(*dSrc.raw_GetComplex());
			if (Guard.InRecursion())
				return 0;

			//	NOTE: We use a Lexicographical comparison here, which is slower
			//	than a simple comparison, but it is more general.

			int iCount = Min(dSrc.GetCount(), dValue.GetCount());
			for (int i = 0; i < iCount; i++)
				{
				int iCompare = dSrc.GetElement(i).OpCompare(dValue.GetElement(i));
				if (iCompare != 0)
					return iCompare;
				}

			return KeyCompare(dSrc.GetCount(), dValue.GetCount());
			}

		case CDatum::typeTensor:
			{
			//	Let the tensor handle the comparison.

			int iCompare = dValue.OpCompare(dSrc);
			if (iCompare == 0)
				return 0;
			else if (iCompare == -1)
				return 1;
			else
				return -1;
			}

		default:
			{
			int iCompare = KeyCompareNoCase(dSrc.AsString(), dValue.AsString());
			if (iCompare < 0)
				return -2;
			else if (iCompare > 0)
				return 2;
			else
				return 0;
			}
		}
	}

int CComplexArray::CompareArrayExact (CDatum dSrc, CDatum::Types iValueType, CDatum dValue)

//	CompareArrayExact
//
//  -2:		If dKey < dKey2 (but not comparable, e.g., string and int)
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2
//	2:		If dKey > dKey2 (but not comparable, e.g., string and int)

	{
	switch (iValueType)
		{
		case CDatum::typeArray:
			{
			CRecursionGuard Guard(*dSrc.raw_GetComplex());
			if (Guard.InRecursion())
				return 0;

			//	NOTE: We use a Lexicographical comparison here, which is slower
			//	than a simple comparison, but it is more general.

			int iCount = Min(dSrc.GetCount(), dValue.GetCount());
			for (int i = 0; i < iCount; i++)
				{
				int iCompare = dSrc.GetElement(i).OpCompareExact(dValue.GetElement(i));
				if (iCompare != 0)
					return iCompare;
				}

			return KeyCompare(dSrc.GetCount(), dValue.GetCount());
			}

		case CDatum::typeTensor:
			{
			//	Let the tensor handle the comparison.

			int iCompare = dValue.OpCompareExact(dSrc);
			if (iCompare == 0)
				return 0;
			else if (iCompare == -1)
				return 1;
			else
				return -1;
			}

		default:
			{
			int iCompare = KeyCompare(dSrc.AsString(), dValue.AsString());
			if (iCompare < 0)
				return -2;
			else if (iCompare > 0)
				return 2;
			else
				return 0;
			}
		}
	}

bool CComplexArray::IsArrayEqual (CDatum dSrc, CDatum::Types iValueType, CDatum dValue)
	{
	switch (iValueType)
		{
		case CDatum::typeArray:
			{
			CRecursionGuard Guard(*dSrc.raw_GetComplex());
			if (Guard.InRecursion())
				return true;

			if (dSrc.GetCount() != dValue.GetCount())
				return false;

			for (int i = 0; i < dSrc.GetCount(); i++)
				if (!dSrc.GetElement(i).OpIsEqual(dValue.GetElement(i)))
					return false;

			return true;
			}

		case CDatum::typeTensor:
			return dValue.OpIsEqual(dSrc);

		default:
			return false;
		}
	}

bool CComplexArray::IsArrayIdentical (CDatum dSrc, CDatum::Types iValueType, CDatum dValue)
	{
	switch (iValueType)
		{
		case CDatum::typeArray:
			{
			CRecursionGuard Guard(*dSrc.raw_GetComplex());
			if (Guard.InRecursion())
				return true;

			if (dSrc.GetCount() != dValue.GetCount())
				return false;

			for (int i = 0; i < dSrc.GetCount(); i++)
				if (!dSrc.GetElement(i).OpIsIdentical(dValue.GetElement(i)))
					return false;

			return true;
			}

		case CDatum::typeTensor:
			return dValue.OpIsIdentical(dSrc);

		default:
			return false;
		}
	}

bool CComplexArray::RemoveElementAt (CDatum dIndex)

//	RemoveElementAt
//
//	Removes the element by index.

	{
	OnCopyOnWrite();

	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount());

	//	Handle different cases

	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		{
		m_Array.Delete(iIndex);
		return true;
		}
	else if (dIndex.IsContainer())
		{
		int iOldSize = m_Array.GetCount();

		TSortMap<int, bool> Indices;
		for (int i = 0; i < dIndex.GetCount(); i++)
			Indices.SetAt(dIndex.GetElement(i).AsArrayIndex(m_Array.GetCount()), true);

		m_Array.Delete(Indices);

		return (iOldSize != m_Array.GetCount());
		}
	else
		return false;
	}

void CComplexArray::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResolveDatatypes
//
//	Resolve datatypes after deserialization.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].ResolveDatatypes(TypeSystem);
	}

void CComplexArray::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize to the given format.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		Stream.Write("null", 4);

	int i;

	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("(", 1);

			for (i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write(")", 1);
			break;
			}

		case CDatum::EFormat::GridLang:
			{
			Stream.Write("[ ", 2);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(", ", 2);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write(" ]", 2);
			break;
			}

		case CDatum::EFormat::JSON:
			{
			Stream.Write("[", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(",", 1);

				m_Array[i].Serialize(iFormat, Stream);
				}

			Stream.Write("]", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

CDatum CComplexArray::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CComplexArray *pArray = new CComplexArray;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		pArray->m_Array.Insert(CDatum::DeserializeAEON(Stream, Serialized));
		}

	return dValue;
	}

void CComplexArray::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_ARRAY))
		return;

	//	Otherwise, write out the full array
	//	NOTE: This handles recursively nested arrays.

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		m_Array[i].SerializeAEON(Stream, Serialized);
		}
	}

