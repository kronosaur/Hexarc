//	CAEONVectorString.cpp
//
//	CAEONVectorString class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_VECTOR_STRING,			"vector_String");

TDatumPropertyHandler<CAEONVectorString> CAEONVectorString::m_Properties = {
	{
		"datatype",
		"%",
		"Returns the type of the array.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"dimensions",
		"I",
		"Returns number of dimensions in the tensor.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return 1;
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the array.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::STRING);
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the array.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return CAEONTypeSystem::GetCoreType(IDatatype::INTEGER);
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfInt32",
		"Returns an array of valid indices.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return CComplexArray::GetIndices(CDatum::raw_AsComplex(&Obj));
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of element in the array.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	{
		"shape",
		"$ArrayOfInt32",
		"Returns an array of sizes for each dimension.",
		[](const CAEONVectorString &Obj, const CString &sProperty)
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
		[](const CAEONVectorString &Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONVectorString::m_pMethodsExt = NULL;

CAEONVectorString::CAEONVectorString (const TArray<CString>& Src)

//	CAEONVectorString constructor

	{
	m_Array.InsertEmpty(Src.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i] = CDatum(Src[i]);
	}

CDatum CAEONVectorString::AsNumberArray () const

//	AsNumberArray
//
//	Returns an array of numbers (or nan, if the values cannot be converted to
//	a number).

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(m_Array.GetCount());

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		//	We always include null values (because callers might want to deal 
		//	with them).

		if (m_Array[i].IsNil())
			dResult.Append(CDatum());
		else
			{
			CDatum dNumberValue;
			if (!CDatum::CreateFromStringValue(m_Array[i], &dNumberValue)
					|| !dNumberValue.IsNumber())
				return CDatum::CreateNaN();

			dResult.Append(dNumberValue);
			}
		}

	return dResult;
	}

IComplexDatum *CAEONVectorString::Clone (CDatum::EClone iMode) const
	{
	switch (iMode)
		{
		case CDatum::EClone::CopyOnWrite:
			//	Default handler
			return NULL;

		default:
			return new CAEONVectorString(m_Array);
		}
	}

int CAEONVectorString::FindMaxElement () const
	{
	CDatum dNumberArray = AsNumberArray();
	if (!dNumberArray.IsNaN())
		return dNumberArray.FindMaxElement();

	//	If we cannot convert to a number then try case-insensitive string
	//	comparison.

	if (m_Array.GetCount() == 0)
		return -1;

	CStringView sMax = m_Array[0];
	int iIndex = 0;
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (KeyCompareNoCase(m_Array[i].AsStringView(), sMax) > 0)
			{
			sMax = m_Array[i];
			iIndex = i;
			}
		}

	return iIndex;
	}

int CAEONVectorString::FindMinElement () const
	{
	CDatum dNumberArray = AsNumberArray();
	if (!dNumberArray.IsNaN())
		return dNumberArray.FindMinElement();

	//	If we cannot convert to a number then try case-insensitive string
	//	comparison.

	if (m_Array.GetCount() == 0)
		return -1;

	CStringView sBest = m_Array[0];
	int iIndex = 0;
	for (int i = 1; i < m_Array.GetCount(); i++)
		{
		if (KeyCompareNoCase(m_Array[i].AsStringView(), sBest) < 0)
			{
			sBest = m_Array[i];
			iIndex = i;
			}
		}

	return iIndex;
	}

const CString &CAEONVectorString::GetTypename (void) const
	{
	return TYPENAME_VECTOR_STRING;
	}

CString CAEONVectorString::AsString (void) const
	{
	CStringBuffer Output;

	Output.Write("[", 1);

	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (i != 0)
			Output.Write(", ", 2);

		Output.Write(m_Array[i].AsStringView());
		}

	Output.Write("]", 1);

	return CString(std::move(Output));
	}

size_t CAEONVectorString::CalcMemorySize (void) const
	{
	size_t dwSize = 0;

	for (int i = 0; i < m_Array.GetCount(); i++)
		dwSize += sizeof(m_Array[i]) + ::AlignUp((size_t)m_Array[i].AsStringView().GetLength(), sizeof(DWORD)) + sizeof(DWORD);

	return dwSize;
	}

bool CAEONVectorString::Find (CDatum dValue, int *retiIndex) const
	{
	CString ValueToFind = dValue.AsString();
	for (int i = 0; i < GetCount(); i++)
		{
		if (strEqualsNoCase(ValueToFind, m_Array[i].AsStringView()))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CDatum CAEONVectorString::FindAll (CDatum dValue) const 
	{
	CString ValueToFind = dValue.AsString();
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (strEqualsNoCase(ValueToFind, m_Array[i].AsStringView()))
			dResult.Append(i);
		}

	return dResult;
	}

CDatum CAEONVectorString::FindAllExact (CDatum dValue) const
	{
	CString ValueToFind = dValue.AsString();
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (strEquals(ValueToFind, m_Array[i].AsStringView()))
			dResult.Append(i);
		}

	return dResult;
	}

bool CAEONVectorString::FindElement (CDatum dValue, int *retiIndex) const
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		{
		if (dValue == m_Array[i])
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

bool CAEONVectorString::FindExact (CDatum dValue, int *retiIndex) const
	{
	CString ValueToFind = dValue.AsString();
	for (int i = 0; i < GetCount(); i++)
		{
		if (strEquals(ValueToFind, m_Array[i].AsStringView()))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CDatum CAEONVectorString::GetElementAt (int iIndex) const
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

void CAEONVectorString::InsertElementAt (CDatum dIndex, CDatum dDatum)
	{
	bool bFromEnd;
	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount(), &bFromEnd);

	//	If from the end, we insert AFTER the specified position (because
	//	we want -1 to mean AFTER the last entry).

	if (bFromEnd && iIndex >= 0)
		iIndex++;

	//	Handle different cases

	if (iIndex >= 0 && iIndex < m_Array.GetCount())
		{
		Insert(dDatum, iIndex);
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
		TSortMap<int, bool> Indices;
		for (int i = 0; i < dIndex.GetCount(); i++)
			Indices.SetAt(dIndex.GetElement(i).AsArrayIndex(m_Array.GetCount()), true);

		m_Array.Delete(Indices);
		}
	}

CDatum CAEONVectorString::MathAbs () const
	{
	return AsNumberArray().MathAbs();
	}

void CAEONVectorString::MathAccumulateStats (CDatum::SStatsCtx& Stats) const
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		Stats.iNaNCount++;
	}

CDatum CAEONVectorString::MathAverage () const
	{
	return AsNumberArray().MathAverage();
	}

CDatum CAEONVectorString::MathCeil () const
	{
	return AsNumberArray().MathCeil();
	}

CDatum CAEONVectorString::MathFloor () const
	{
	return AsNumberArray().MathFloor();
	}

CDatum CAEONVectorString::MathMax () const
	{
	CDatum dResult = AsNumberArray().MathMax();

	//	If we cannot convert to a number then try case-insensitive string
	//	comparison.

	if (dResult.IsNaN())
		{
		//	NOTE: We know array has at lease one element because AsNumberArray
		//	only returns NaN if it has checked at lease one element.

		CStringView sMax = m_Array[0];
		for (int i = 1; i < m_Array.GetCount(); i++)
			{
			if (KeyCompareNoCase(m_Array[i].AsStringView(), sMax) > 0)
				sMax = m_Array[i];
			}

		dResult = CDatum(sMax);
		}

	return dResult;
	}

CDatum CAEONVectorString::MathMin () const
	{
	CDatum dResult = AsNumberArray().MathMin();

	//	If we cannot convert to a number then try case-insensitive string
	//	comparison.

	if (dResult.IsNaN())
		{
		//	NOTE: We know array has at lease one element because AsNumberArray
		//	only returns NaN if it has checked at lease one element.

		CStringView sMin = m_Array[0];
		for (int i = 1; i < m_Array.GetCount(); i++)
			{
			if (KeyCompareNoCase(m_Array[i].AsStringView(), sMin) < 0)
				sMin = m_Array[i];
			}

		dResult = CDatum(sMin);
		}

	return dResult;
	}

CDatum CAEONVectorString::MathRound () const
	{
	return AsNumberArray().MathRound();
	}

CDatum CAEONVectorString::MathSign () const
	{
	return AsNumberArray().MathSign();
	}

CDatum CAEONVectorString::MathSum () const
	{
	return AsNumberArray().MathSum();
	}

CDatum CAEONVectorString::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorString *pArray = new CAEONVectorString;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the string table.

	int iStringCount = Stream.ReadInt();
	TArray<CDatum> StringTable;
	StringTable.InsertEmpty(iStringCount);
	for (int i = 0; i < iStringCount; i++)
		StringTable[i] = CDatum(CString::Deserialize(Stream));

	//	Now load the array of indices into the string table.

	int iArrayCount = Stream.ReadInt();
	pArray->m_Array.InsertEmpty(iArrayCount);
	for (int i = 0; i < iArrayCount; i++)
		{
		int iIndex = Stream.ReadInt();
		if (iIndex < 0 || iIndex >= StringTable.GetCount())
			throw CException(errFail);

		pArray->m_Array[i] = StringTable[iIndex];
		}

#ifdef DEBUG_STRING_TABLE
	printf("%d unique strings out of %d elements. Saved %d%%\n", iStringCount, iArrayCount, (iArrayCount > 0 ? 100 * (iArrayCount - iStringCount) / iArrayCount : 0));
#endif

	return dValue;
	}

CDatum CAEONVectorString::DeserializeAEON_v1 (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new array and add it to the map.

	CAEONVectorString *pArray = new CAEONVectorString;
	CDatum dValue(pArray);
	Serialized.Add(dwID, dValue);

	//	Read the elements of the array.

	DWORD dwCount = Stream.ReadDWORD();
	pArray->m_Array.GrowToFit((int)dwCount);
	for (int i = 0; i < (int)dwCount; i++)
		{
		pArray->m_Array.Insert(CDatum(CString::Deserialize(Stream)));
		}

	return dValue;
	}

size_t CAEONVectorString::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const
	{
	size_t TotalSize = 2 + m_Array.GetCount();

	for (int i = 0; i < m_Array.GetCount(); i++)
		TotalSize += m_Array[i].CalcSerializeSize(iFormat);

	return TotalSize;
	}

void CAEONVectorString::OnMarked (void)
	{
	for (int i = 0; i < m_Array.GetCount(); i++)
		m_Array[i].Mark();
	}

bool CAEONVectorString::RemoveElementAt (CDatum dIndex)
	{
	int iIndex = dIndex.AsArrayIndex(m_Array.GetCount());
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

void CAEONVectorString::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("(", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
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
					Stream.Write(", ", 2);

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

void CAEONVectorString::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_VECTOR_STRING_V2))
		return;

	//	Generate a table of unique strings and build an array of indices into that table.

	CDatumStringTable StringTable(GetCount());

	TArray<DWORD> IndexArray;
	IndexArray.InsertEmpty(m_Array.GetCount());
	for (int i = 0; i < m_Array.GetCount(); i++)
		IndexArray[i] = StringTable.AddString(m_Array[i]);

	//	Write out the string table

	Stream.Write(StringTable.GetCount());
	for (int i = 0; i < StringTable.GetCount(); i++)
		StringTable.GetStringByIndex(i).AsStringView().Serialize(Stream);

	//	Write out the array of indices into the string table.

	Stream.Write(IndexArray.GetCount());
	for (int i = 0; i < IndexArray.GetCount(); i++)
		Stream.Write(IndexArray[i]);
	}
