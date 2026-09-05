//	CAEONRecord.cpp
//
//	CAEONRecord class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE_SPECIAL,			"__datatype__");
DECLARE_CONST_STRING(TYPENAME_OBJECT,					"object");

TDatumMethodHandler<IComplexDatum> *CAEONRecord::m_pMethodsExt = NULL;

TDatumPropertyHandler<CAEONRecord> CAEONRecord::m_Properties = {
	{
		"columns",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CAEONRecord& Record, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			const IDatatype& Type = Record.GetDatatype();
			dResult.GrowToFit(Type.GetMemberCount());
			for (int i = 0; i < Type.GetMemberCount(); i++)
				dResult.Append(Type.GetMember(i).sID);

			return dResult;
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the struct.",
		[](const CAEONRecord& Record, const CString &sProperty)
			{
			return Record.GetDatatype();
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CAEONRecord& Record, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Record.GetCount());
			for (int i = 0; i < Record.GetCount(); i++)
				dResult.Append(Record.GetKey(i));

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of entries in the struct.",
		[](const CAEONRecord& Record, const CString &sProperty)
			{
			return CDatum(Record.GetCount());
			},
		NULL,
		},
	};

const CString& CAEONRecord::GetTypename () const { return TYPENAME_OBJECT; }

CAEONRecord::CAEONRecord (CDatum dType, const CDatum* pValues, int iCount) : m_dType(dType)
	{
	m_bInlineSlots = (iCount <= INLINE_SLOT_COUNT);
	if (m_bInlineSlots)
		{
		m_iInlineSlotCount = iCount;
		for (int i = 0; i < iCount; i++)
			m_InlineValues[i] = pValues[i];
		}
	else
		{
		m_iInlineSlotCount = 0;
		m_Values.InsertEmpty(iCount);
		for (int i = 0; i < iCount; i++)
			m_Values[i] = pValues[i];
		}
	}

void CAEONRecord::AppendStruct (CDatum dDatum)
	{
	if (dDatum.IsStruct())
		{
		OnCopyOnWrite();

		for (int i = 0; i < dDatum.GetCount(); i++)
			SetElement(dDatum.GetKey(i), dDatum.GetElement(i));
		}
	}

CString CAEONRecord::AsString () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	CStringBuffer Output;
	Output.Write("{", 1);

	for (int i = 0; i < GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(GetKey(i));
		Output.Write(":", 1);
		Output.Write(GetElement(i).AsString());
		}

	Output.Write("}", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

size_t CAEONRecord::CalcMemorySize () const
	{
	size_t dwSize = 0;
	for (int i = 0; i < GetSlotValueCount(); i++)
		dwSize += GetSlotValueRef(i).CalcMemorySize();

	return dwSize;
	}

IComplexDatum *CAEONRecord::Clone (CDatum::EClone iMode) const
	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CAEONRecord(*this);

		case CDatum::EClone::CopyOnWrite:
			return NULL;

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CAEONRecord(*this);
			pClone->CloneContents();
			return pClone;
			}

		case CDatum::EClone::Isolate:
			return NULL;

		default:
			throw CException(errFail);
		}
	}

CDatum CAEONRecord::Cleaned () const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return CDatum::raw_AsComplex(this);

	CDatum dResult(CDatum::typeStruct);
	dResult.GrowToFit(GetCount());

	for (int i = 0; i < GetCount(); i++)
		dResult.SetElement(GetKey(i), GetElement(i).Cleaned());

	return dResult;
	}

void CAEONRecord::CloneContents ()
	{
	for (int i = 0; i < GetSlotValueCount(); i++)
		SetSlotValue(i, GetSlotValueRef(i).Clone(CDatum::EClone::DeepCopy));
	}

bool CAEONRecord::Contains (CDatum dValue) const
	{
	for (int i = 0; i < GetSlotValueCount(); i++)
		if (GetSlotValueRef(i).Contains(dValue))
			return true;

	return false;
	}

void CAEONRecord::DeleteAllSlotValues ()
	{
	if (m_bInlineSlots)
		{
		for (int i = 0; i < m_iInlineSlotCount; i++)
			m_InlineValues[i] = CDatum();

		m_iInlineSlotCount = 0;
		}
	else
		m_Values.DeleteAll();
	}

void CAEONRecord::DeleteElement (int iIndex)
	{
	OnCopyOnWrite();

	if (iIndex >= 0 && iIndex < GetSlotValueCount())
		SetSlotValue(iIndex, CDatum());
	}

void CAEONRecord::EnsureSlotValueCount (int iCount)
	{
	if (iCount <= GetSlotValueCount())
		return;

	if (m_bInlineSlots && iCount <= INLINE_SLOT_COUNT)
		{
		m_iInlineSlotCount = iCount;
		return;
		}

	if (m_bInlineSlots)
		{
		m_Values.InsertEmpty(iCount);
		for (int i = 0; i < m_iInlineSlotCount; i++)
			{
			m_Values[i] = m_InlineValues[i];
			m_InlineValues[i] = CDatum();
			}

		m_iInlineSlotCount = 0;
		m_bInlineSlots = false;
		}
	else
		m_Values.InsertEmpty(iCount - m_Values.GetCount());
	}

bool CAEONRecord::FindElement (const CString& sKey, CDatum *retpValue) const
	{
	int iSlot = FindSlot(sKey);
	if (iSlot == -1)
		return false;

	if (retpValue)
		*retpValue = GetSlot(iSlot);

	return true;
	}

int CAEONRecord::FindSlot (const CString& sKey) const
	{
	const IDatatype& Type = m_dType;
	return Type.FindMember(sKey);
	}

CDatum CAEONRecord::GetElement (int iIndex) const
	{
	if (iIndex < 0)
		return CDatum();

	return GetSlot(iIndex);
	}

CDatum CAEONRecord::GetElement (const CString& sKey) const
	{
	int iSlot = FindSlot(sKey);
	return (iSlot == -1 ? CDatum() : GetSlot(iSlot));
	}

CDatum CAEONRecord::GetElementAt (CAEONTypeSystem& TypeSystem, CDatum dIndex) const
	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		return GetElement(iIndex);
	else if (dIndex.IsContainer())
		{
		CDatum dResult(dIndex.IsStruct() ? CDatum::typeStruct : CDatum::typeArray);

		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			CDatum dEntry = (dIndex.IsStruct() ? CDatum(dIndex.GetKey(i)) : dIndex.GetElement(i));
			if (dEntry.IsNil())
				continue;
			else if (dEntry.IsNumberInt32(&iIndex))
				{
				if (iIndex >= 0 && iIndex < GetCount())
					{
					if (dIndex.IsStruct())
						dResult.SetElement(GetKey(iIndex), GetElement(iIndex));
					else
						dResult.Append(GetElement(iIndex));
					}
				}
			else
				{
				CString sKey = dEntry.AsString();
				CDatum dValue;
				if (FindElement(sKey, &dValue))
					{
					if (dIndex.IsStruct())
						dResult.SetElement(sKey, dValue);
					else
						dResult.Append(dValue);
					}
				}
			}

		return dResult;
		}
	else
		return GetElement(dIndex.AsString());
	}

CString CAEONRecord::GetKey (int iIndex) const
	{
	if (iIndex < 0)
		return NULL_STR;

	const IDatatype& Type = m_dType;
	if (iIndex >= Type.GetMemberCount())
		return NULL_STR;

	return Type.GetMember(iIndex).sID;
	}

CDatum CAEONRecord::GetMethod (const CString& sMethod) const
	{
	CDatum dValue = GetElement(sMethod);
	if (dValue.CanInvoke())
		return dValue;

	if (m_pMethodsExt)
		return m_pMethodsExt->GetMethod(sMethod);
	else
		return CDatum();
	}

CDatum CAEONRecord::GetProperty (const CString& sProperty) const
	{
	CDatum dValue = GetElement(sProperty);
	if (!dValue.IsNil())
		return dValue;

	return m_Properties.GetProperty(*this, sProperty);
	}

CDatum CAEONRecord::GetSlot (int iSlot) const
	{
	if (iSlot >= 0 && iSlot < GetSlotValueCount())
		return GetSlotValueRef(iSlot);
	else
		return CDatum();
	}

bool CAEONRecord::raw_GetRecordSlot (int iIndex, CDatum* retdValue) const
	{
	if (iIndex < 0 || iIndex >= GetSlotValueCount())
		return false;

	if (retdValue)
		*retdValue = GetSlotValueRef(iIndex);

	return true;
	}

int CAEONRecord::GetSlotValueCount () const
	{
	return (m_bInlineSlots ? m_iInlineSlotCount : m_Values.GetCount());
	}

CDatum& CAEONRecord::GetSlotValueRef (int iSlot)
	{
	ASSERT(iSlot >= 0 && iSlot < GetSlotValueCount());
	return (m_bInlineSlots ? m_InlineValues[iSlot] : m_Values[iSlot]);
	}

const CDatum& CAEONRecord::GetSlotValueRef (int iSlot) const
	{
	ASSERT(iSlot >= 0 && iSlot < GetSlotValueCount());
	return (m_bInlineSlots ? m_InlineValues[iSlot] : m_Values[iSlot]);
	}

void CAEONRecord::GrowToFit (int iCount)
	{
	OnCopyOnWrite();
	EnsureSlotValueCount(iCount);
	}

void CAEONRecord::InitStorage ()
	{
	const IDatatype& Type = m_dType;
	InitStorage(Type.GetMemberCount());
	}

void CAEONRecord::InitStorage (int iMemberCount)
	{
	const bool bWasInlineSlots = m_bInlineSlots;
	const int iOldInlineSlotCount = m_iInlineSlotCount;

	m_bInlineSlots = (iMemberCount <= INLINE_SLOT_COUNT);

	if (m_bInlineSlots)
		{
		if (m_iInlineSlotCount < iMemberCount)
			m_iInlineSlotCount = iMemberCount;
		}
	else
		{
		if (bWasInlineSlots && iOldInlineSlotCount > 0)
			{
			if (m_Values.GetCount() < Max(iMemberCount, iOldInlineSlotCount))
				m_Values.InsertEmpty(Max(iMemberCount, iOldInlineSlotCount) - m_Values.GetCount());

			for (int i = 0; i < iOldInlineSlotCount; i++)
				{
				m_Values[i] = m_InlineValues[i];
				m_InlineValues[i] = CDatum();
				}

			m_iInlineSlotCount = 0;
			}

		if (m_Values.GetCount() < iMemberCount)
			m_Values.InsertEmpty(iMemberCount - m_Values.GetCount());
		}
	}

void CAEONRecord::OnCopyOnWrite ()
	{
	if (m_bCopyOnWrite)
		{
		CloneContents();
		m_bCopyOnWrite = false;
		}
	}

void CAEONRecord::OnMarked ()
	{
	m_dType.Mark();

	for (int i = 0; i < GetSlotValueCount(); i++)
		GetSlotValueRef(i).Mark();
	}

int CAEONRecord::OpCompare (CDatum::Types iValueType, CDatum dValue) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return 0;

	if (!dValue.IsStruct())
		return KeyCompareNoCase(AsString(), dValue.AsString());

	if (GetBasicType() == iValueType)
		{
		int iCount = Min(GetCount(), dValue.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = KeyCompareNoCase(GetKey(i), dValue.GetKey(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = GetElement(i).OpCompare(dValue.GetElement(i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(GetCount(), dValue.GetCount());
		}
	else if (OpIsEqual(iValueType, dValue))
		return 0;
	else if (GetBasicType() == CDatum::typeStruct)
		return -1;
	else
		return 1;
	}

int CAEONRecord::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return 0;

	if (!dValue.IsStruct())
		return KeyCompare(AsString(), dValue.AsString());

	if (GetBasicType() == iValueType)
		{
		int iCount = Min(GetCount(), dValue.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = KeyCompare(GetKey(i), dValue.GetKey(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = GetElement(i).OpCompareExact(dValue.GetElement(i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(GetCount(), dValue.GetCount());
		}
	else if (OpIsIdentical(iValueType, dValue))
		return 0;
	else if (GetBasicType() == CDatum::typeStruct)
		return -1;
	else
		return 1;
	}

bool CAEONRecord::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return true;

	if (!dValue.IsStruct())
		return false;
	else if (GetCount() != dValue.GetCount())
		return false;
	else if (GetBasicType() == iValueType)
		{
		for (int i = 0; i < GetCount(); i++)
			if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
					|| !GetElement(i).OpIsEqual(dValue.GetElement(i)))
				return false;

		return true;
		}
	else
		{
		for (int i = 0; i < GetCount(); i++)
			{
			CDatum dValue2 = dValue.GetElement(GetKey(i));
			if (!GetElement(i).OpIsEqual(dValue2))
				return false;
			}

		return true;
		}
	}

bool CAEONRecord::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return true;

	if ((const IDatatype&)GetDatatype() != (const IDatatype&)dValue.GetDatatype())
		return false;

	if (GetCount() != dValue.GetCount())
		return false;

	for (int i = 0; i < GetCount(); i++)
		if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
				|| !GetElement(i).OpIsIdentical(dValue.GetElement(i)))
			return false;

	return true;
	}

bool CAEONRecord::RemoveAll ()
	{
	OnCopyOnWrite();
	DeleteAllSlotValues();
	return true;
	}

bool CAEONRecord::RemoveElementAt (CDatum dIndex)
	{
	OnCopyOnWrite();

	if (dIndex.IsStruct() || dIndex.IsArray())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			SetElement(dIndex.IsStruct() ? dIndex.GetKey(i) : dIndex.GetElement(i).AsString(), CDatum());
		}
	else
		SetElement(dIndex.AsString(), CDatum());

	return true;
	}

void CAEONRecord::ResolveDatatypes (const CAEONTypeSystem& TypeSystem)
	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	m_dType = TypeSystem.ResolveType(m_dType);
	InitStorage();

	for (int i = 0; i < GetSlotValueCount(); i++)
		GetSlotValueRef(i).ResolveDatatypes(TypeSystem);
	}

void CAEONRecord::Serialize (CDatum::EFormat iFormat, IByteStream& Stream) const
	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("[", 1);
			Stream.Write(GetTypename());
			Stream.Write(":", 1);

			GetDatatype().Serialize(iFormat, Stream);
			Stream.Write(":", 1);

			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);
				Stream.Write(":", 1);
				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}]", 2);
			break;
			}

		case CDatum::EFormat::GridLang:
			SerializeAsStruct(iFormat, Stream);
			break;

		case CDatum::EFormat::AEONJSON:
		case CDatum::EFormat::AEONJSONJavaScript:
		case CDatum::EFormat::JSON:
			{
			Stream.Write("{\"", 2);

			FIELD_DATATYPE_SPECIAL.SerializeJSON(Stream);
			Stream.Write("\": ", 3);
			GetDatatype().Serialize(iFormat, Stream);

			for (int i = 0; i < GetCount(); i++)
				{
				Stream.Write(", ", 2);
				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);
				Stream.Write(": ", 2);
				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}", 1);
			break;
			}

		default:
			IComplexDatum::Serialize(iFormat, Stream);
			break;
		}
	}

CDatum CAEONRecord::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap& Serialized)
	{
	CDatum dType = CDatum::DeserializeAEON(Stream, Serialized);
	CDatum dValue = CDatum::CreateRecord(dType);
	CAEONRecord* pRecord = static_cast<CAEONRecord*>(dValue.raw_GetComplex());
	Serialized.Add(dwID, dValue);

	int iCount = (int)Stream.ReadDWORD();
	pRecord->GrowToFit(iCount);
	for (int i = 0; i < iCount; i++)
		{
		CString sKey = CString::Deserialize(Stream);
		CDatum dElement = CDatum::DeserializeAEON(Stream, Serialized);
		pRecord->SetElement(sKey, dElement);
		}

	return dValue;
	}

void CAEONRecord::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_OBJECT))
		return;

	m_dType.SerializeAEON(Stream, Serialized);

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		GetKey(i).Serialize(Stream);
		GetElement(i).SerializeAEON(Stream, Serialized);
		}
	}

void CAEONRecord::SetArrayElementUnchecked (int iIndex, CDatum dDatum)
	{
	if (iIndex < 0)
		return;

	OnCopyOnWrite();
	SetSlotValue(iIndex, dDatum);
	}

void CAEONRecord::SetElement (int iIndex, CDatum dDatum)
	{
	const IDatatype& Type = m_dType;
	if (iIndex < 0 || iIndex >= Type.GetMemberCount())
		return;

	OnCopyOnWrite();

	CDatum dMemberType = Type.GetMemberType(iIndex);
	if (((const IDatatype&)dDatum.GetDatatype()).IsA(dMemberType))
		SetSlotValue(iIndex, dDatum);
	else
		SetSlotValue(iIndex, CDatum::CreateAsType(dMemberType, dDatum));
	}

void CAEONRecord::SetElement (const CString& sKey, CDatum dDatum)
	{
	int iSlot = FindSlot(sKey);
	if (iSlot == -1)
		return;

	SetElement(iSlot, dDatum);
	}

void CAEONRecord::SetElementAt (CDatum dIndex, CDatum dDatum)
	{
	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32())
		{
		int iIndex = dIndex;
		if (iIndex >= 0 && iIndex < GetCount())
			SetElement(iIndex, dDatum);
		}
	else
		SetElement(dIndex.AsString(), dDatum);
	}

void CAEONRecord::SetSlotValue (int iSlot, CDatum dDatum)
	{
	if (iSlot < 0)
		return;

	EnsureSlotValueCount(iSlot + 1);
	GetSlotValueRef(iSlot) = dDatum;
	}
