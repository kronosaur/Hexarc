//	CAEONObject.cpp
//
//	CAEONObject class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE_SPECIAL,			"__datatype__");

DECLARE_CONST_STRING(TYPENAME_OBJECT,					"object");

const CString &CAEONObject::GetTypename () const { return TYPENAME_OBJECT; }

TDatumPropertyHandler<CAEONObject> CAEONObject::m_Properties = {
	{
		"columns",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CAEONObject& Obj, const CString &sProperty)
			{
			const IDatatype& Type = Obj.GetDatatype();

			if (Type.GetMemberCount() == 0)
				{
				CDatum dResult(CDatum::typeArray);
				dResult.GrowToFit(Obj.m_Map.GetCount());
				for (int i = 0; i < Obj.m_Map.GetCount(); i++)
					dResult.Append(Obj.m_Map.GetKey(i));

				return dResult;
				}
			else
				{
				CDatum dResult(CDatum::typeArray);
				for (int i = 0; i < Type.GetMemberCount(); i++)
					{
					auto MemberDesc = Type.GetMember(i);
					if (MemberDesc.iType == IDatatype::EMemberType::InstanceKeyVar || MemberDesc.iType == IDatatype::EMemberType::InstanceVar)
						dResult.Append(MemberDesc.sID);
					}

				return dResult;
				}
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the struct.",
		[](const CAEONObject &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CAEONObject& Obj, const CString &sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.GetCount());
			for (int i = 0; i < Obj.GetCount(); i++)
				dResult.Append(Obj.GetKey(i));

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of entries in the struct.",
		[](const CAEONObject& Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CAEONObject::m_pMethodsExt = NULL;

CAEONObject::CAEONObject (CDatum dType, CDatum dSrc) : m_dType(dType)
	{
	InitStorage();

	for (int i = 0; i < dSrc.GetCount(); i++)
		{
		CString sKey = dSrc.GetKey(i);
		if (!sKey.IsEmpty())
			SetElement(sKey, dSrc.GetElement(i));
		}
	}

CAEONObject::CAEONObject (CDatum dType, const TSortMap<CString, CDatum> &Src) : 
		m_dType(dType)
	{
	InitStorage();

	for (int i = 0; i < Src.GetCount(); i++)
		SetElement(Src.GetKey(i), Src[i]);
	}

CAEONObject::CAEONObject (CDatum dType, const TSortMap<CString, CString> &Src) : 
		m_dType(dType)
	{
	InitStorage();

	for (int i = 0; i < Src.GetCount(); i++)
		{
		const CString &sKey = Src.GetKey(i);
		if (!sKey.IsEmpty())
			SetElement(sKey, Src.GetValue(i));
		}
	}

void CAEONObject::AppendStruct (CDatum dDatum)
	{
	if (dDatum.IsStruct())
		{
		OnCopyOnWrite();

		for (int i = 0; i < dDatum.GetCount(); i++)
			SetElement(dDatum.GetKey(i), dDatum.GetElement(i));
		}
	}

int CAEONObject::CalcSlotCount () const
	{
	const IDatatype& Type = m_dType;
	int iCount = 0;
	for (int i = 0; i < Type.GetMemberCount(); i++)
		if (IsInstanceMember(Type.GetMember(i)))
			iCount++;

	return iCount;
	}

CString CAEONObject::AsString () const
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

size_t CAEONObject::CalcMemorySize () const
	{
	size_t dwSize = 0;

	for (int i = 0; i < m_Values.GetCount(); i++)
		dwSize += m_Values[i].CalcMemorySize();

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		dwSize += m_Map.GetKey(i).GetLength() + sizeof(DWORD) + 1;
		dwSize += m_Map[i].CalcMemorySize();
		}

	return dwSize;
	}

IComplexDatum *CAEONObject::Clone (CDatum::EClone iMode) const
	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CAEONObject(*this);

		case CDatum::EClone::CopyOnWrite:
			return NULL;

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CAEONObject(*this);
			pClone->CloneContents();
			return pClone;
			}

		case CDatum::EClone::Isolate:
			return NULL;

		default:
			throw CException(errFail);
		}
	}

CDatum CAEONObject::Cleaned () const
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

void CAEONObject::CloneContents ()
	{
	for (int i = 0; i < m_Values.GetCount(); i++)
		m_Values[i] = m_Values[i].Clone(CDatum::EClone::DeepCopy);

	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i] = m_Map[i].Clone(CDatum::EClone::DeepCopy);
	}

bool CAEONObject::Contains (CDatum dValue) const
	{
	for (int i = 0; i < m_Values.GetCount(); i++)
		if (m_Values[i].Contains(dValue))
			return true;

	for (int i = 0; i < m_Map.GetCount(); i++)
		if (m_Map[i].Contains(dValue))
			return true;

	return false;
	}

void CAEONObject::DeleteElement (int iIndex)
	{
	OnCopyOnWrite();

	if (HasSlots())
		{
		if (iIndex >= 0 && iIndex < m_Values.GetCount())
			m_Values[iIndex] = CDatum();
		}
	else if (iIndex >= 0 && iIndex < m_Map.GetCount())
		m_Map.DeleteAt(m_Map.GetKey(iIndex));
	}

void CAEONObject::EnsureSlots ()
	{
	const IDatatype& Type = m_dType;
	int iMemberCount = Type.GetMemberCount();
	if (iMemberCount > m_Values.GetCount())
		m_Values.InsertEmpty(iMemberCount - m_Values.GetCount());
	}

bool CAEONObject::FindElement (const CString &sKey, CDatum *retpValue) const
	{
	int iSlot;
	if (FindSlot(sKey, &iSlot))
		{
		if (retpValue)
			*retpValue = GetSlot(iSlot);

		return true;
		}

	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue == NULL)
		return false;

	if (retpValue)
		*retpValue = *pValue;

	return true;
	}

bool CAEONObject::FindSlot (const CString &sKey, int *retiSlot, IDatatype::SMemberDesc *retMember) const
	{
	const IDatatype& Type = m_dType;
	if (Type.GetMemberCount() == 0)
		return false;

	int iMember = Type.FindMember(sKey);
	if (iMember == -1)
		return false;

	IDatatype::SMemberDesc Member = Type.GetMember(iMember);
	if (!IsInstanceMember(Member))
		return false;

	if (retiSlot)
		*retiSlot = iMember;

	if (retMember)
		*retMember = Member;

	return true;
	}

int CAEONObject::GetCount () const
	{
	if (!HasSlots())
		return m_Map.GetCount();
	else
		return CalcSlotCount();
	}

CDatum CAEONObject::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element.

	{
	if (iIndex < 0)
		return CDatum();

	const IDatatype& Type = m_dType;
	for (int i = 0; i < Type.GetMemberCount(); i++)
		{
		auto MemberDesc = Type.GetMember(i);
		if (IsInstanceMember(MemberDesc))
			{
			if (iIndex == 0)
				return GetSlot(i);
			else
				iIndex--;
			}
		}

	return CDatum();
	}

CDatum CAEONObject::GetElement (const CString &sKey) const
	{
	int iSlot;
	if (FindSlot(sKey, &iSlot))
		return GetSlot(iSlot);

	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue)
		return *pValue;
	else
		return CDatum();
	}

CDatum CAEONObject::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const
	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		{
		if (HasSlots())
			return GetElement(iIndex);
		else if (iIndex >= 0 && iIndex < m_Map.GetCount())
			return m_Map[iIndex];
		else
			return CDatum();
		}
	else if (dIndex.IsContainer())
		{
		if (dIndex.IsStruct())
			{
			CDatum dResult(CDatum::typeStruct);

			for (int i = 0; i < dIndex.GetCount(); i++)
				{
				CString sKey = dIndex.GetKey(i);
				CDatum dValue;
				if (FindElement(sKey, &dValue))
					dResult.SetElement(sKey, dValue);
				}

			return dResult;
			}
		else
			{
			CDatum dResult(CDatum::typeArray);

			for (int i = 0; i < dIndex.GetCount(); i++)
				{
				CDatum dEntry = dIndex.GetElement(i);
				if (dEntry.IsNil())
					{ }
				else if (dEntry.IsNumberInt32(&iIndex))
					{
					if (HasSlots())
						{
						if (iIndex >= 0 && iIndex < GetCount())
							dResult.Append(GetElement(iIndex));
						}
					else if (iIndex >= 0 && iIndex < m_Map.GetCount())
						dResult.Append(m_Map[iIndex]);
					}
				else
					{
					CString sKey = dEntry.AsString();
					CDatum dValue;
					if (FindElement(sKey, &dValue))
						dResult.Append(dValue);
					}
				}

			return dResult;
			}
		}
	else
		{
		return GetElement(dIndex.AsString());
		}
	}

CString CAEONObject::GetKey (int iIndex) const

//	GetKey
//
//	Get the key at the given index.

	{
	if (iIndex < 0)
		return NULL_STR;

	const IDatatype& Type = m_dType;
	for (int i = 0; i < Type.GetMemberCount(); i++)
		{
		auto MemberDesc = Type.GetMember(i);
		if (IsInstanceMember(MemberDesc))
			{
			if (iIndex == 0)
				return MemberDesc.sID;
			else
				iIndex--;
			}
		}

	return NULL_STR;
	}

CDatum CAEONObject::GetSlot (int iSlot) const
	{
	if (iSlot >= 0 && iSlot < m_Values.GetCount())
		return m_Values[iSlot];
	else
		return CDatum();
	}

void CAEONObject::GrowToFit (int iCount)
	{
	OnCopyOnWrite();

	if (HasSlots())
		m_Values.GrowToFit(iCount);
	else
		m_Map.GrowToFit(iCount);
	}

bool CAEONObject::HasSlots () const
	{
	return m_bHasSlots;
	}

void CAEONObject::InitStorage ()
	{
	const IDatatype& Type = m_dType;
	const int iMemberCount = Type.GetMemberCount();

	m_bHasSlots = (iMemberCount > 0);
	}

bool CAEONObject::IsInstanceMember (const IDatatype::SMemberDesc &Member)
	{
	return (Member.iType == IDatatype::EMemberType::InstanceKeyVar
			|| Member.iType == IDatatype::EMemberType::InstanceVar);
	}

CDatum CAEONObject::GetMethod (const CString &sMethod) const
	{
	int iSlot;
	if (FindSlot(sMethod, &iSlot))
		{
		CDatum dValue = GetSlot(iSlot);
		if (dValue.CanInvoke())
			return dValue;
		}

	CDatum *pValue = m_Map.GetAt(sMethod);
	if (pValue && pValue->CanInvoke())
		return *pValue;

	if (m_pMethodsExt) 
		return m_pMethodsExt->GetMethod(sMethod);
	else
		return CDatum();
	}

CDatum CAEONObject::GetProperty (const CString &sKey) const
	{
	int iSlot;
	if (FindSlot(sKey, &iSlot))
		return GetSlot(iSlot);

	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue)
		return *pValue;

	return m_Properties.GetProperty(*this, sKey);
	}

void CAEONObject::OnCopyOnWrite ()
	{
	if (m_bCopyOnWrite)
		{
		CloneContents();
		m_bCopyOnWrite = false;
		}
	}

void CAEONObject::OnMarked ()
	{
	m_dType.Mark();

	for (int i = 0; i < m_Values.GetCount(); i++)
		m_Values[i].Mark();

	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].Mark();
	}

int CAEONObject::OpCompare (CDatum::Types iValueType, CDatum dValue) const
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

int CAEONObject::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const
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

bool CAEONObject::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
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

bool CAEONObject::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const
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

bool CAEONObject::RemoveAll ()
	{
	OnCopyOnWrite();

	m_Values.DeleteAll();
	m_Map.DeleteAll();

	return true;
	}

bool CAEONObject::RemoveElementAt (CDatum dIndex)
	{
	OnCopyOnWrite();

	if (dIndex.IsStruct())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			if (HasSlots())
				SetElement(dIndex.GetKey(i), CDatum());
			else
				m_Map.DeleteAt(dIndex.GetKey(i));
			}
		}
	else if (dIndex.IsArray())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			{
			if (HasSlots())
				SetElement(dIndex.GetElement(i).AsString(), CDatum());
			else
				m_Map.DeleteAt(dIndex.GetElement(i).AsString());
			}
		}
	else if (HasSlots())
		SetElement(dIndex.AsString(), CDatum());
	else
		m_Map.DeleteAt(dIndex.AsString());

	return true;
	}

void CAEONObject::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

//	ResolveDatatypes
//
//	Resolve datatypes after deserialization.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	m_dType = TypeSystem.ResolveType(m_dType);
	InitStorage();

	for (int i = 0; i < m_Values.GetCount(); i++)
		m_Values[i].ResolveDatatypes(TypeSystem);

	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].ResolveDatatypes(TypeSystem);
	}

void CAEONObject::Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	Serialize
//
//	Serialize.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONScript:
		case CDatum::EFormat::AEONLocal:
			{
			Stream.Write("[", 1);
			Stream.Write(GetTypename());
			Stream.Write(":", 1);

			//	Write out the datatype

			GetDatatype().Serialize(iFormat, Stream);
			Stream.Write(":", 1);

			//	Now write out each member variable as a structure.

			Stream.Write("{", 1);

			for (int i = 0; i < GetCount(); i++)
				{
				if (i != 0)
					Stream.Write(" ", 1);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(":", 1);

				//	Write the value

				GetElement(i).Serialize(iFormat, Stream);
				}

			Stream.Write("}]", 2);
			break;
			}

		case CDatum::EFormat::GridLang:
			SerializeAsStruct(iFormat, Stream);
			break;

		//	For JSON format we just serialize as a plain Struct but write out 
		//	the datatype in the special __datatype__ field.

		case CDatum::EFormat::AEONJSON:
		case CDatum::EFormat::JSON:
			{
			Stream.Write("{\"", 2);

			FIELD_DATATYPE_SPECIAL.SerializeJSON(Stream);
			Stream.Write("\": ", 3);
			GetDatatype().Serialize(iFormat, Stream);

			for (int i = 0; i < GetCount(); i++)
				{
				Stream.Write(", ", 2);

				//	Write the key

				CDatum Key(GetKey(i));
				Key.Serialize(iFormat, Stream);

				//	Separator

				Stream.Write(": ", 2);

				//	Write the value

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

CDatum CAEONObject::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Load the datatype so that we can choose the appropriate implementation.

	CDatum dType = CDatum::DeserializeAEON(Stream, Serialized);
	const IDatatype& Type = dType;

	if (Type.GetClass() == IDatatype::ECategory::Schema)
		{
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

	//	Create a class object and add it to the map before loading members, so
	//	recursive member references can resolve.

	CAEONObject *pObj = new CAEONObject(dType);
	CDatum dValue(pObj);
	Serialized.Add(dwID, dValue);

	//	Load each member.

	int iCount = (int)Stream.ReadDWORD();
	pObj->GrowToFit(iCount);
	for (int i = 0; i < iCount; i++)
		{
		CString sKey = CString::Deserialize(Stream);
		CDatum dElement = CDatum::DeserializeAEON(Stream, Serialized);
		
		pObj->SetElement(sKey, dElement);
		}

	return dValue;
	}

void CAEONObject::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_OBJECT))
		return;

	//	Write out the datatype

	m_dType.SerializeAEON(Stream, Serialized);

	//	Now write out each member variable

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		GetKey(i).Serialize(Stream);
		GetElement(i).SerializeAEON(Stream, Serialized);
		}
	}

void CAEONObject::SetArrayElementUnchecked (int iIndex, CDatum dDatum)

//	SetArrayElementUnchecked
//
//	Sets a typed-object slot by ordinal without checking/coercing the value type.

	{
	if (!HasSlots())
		{
		SetElement(iIndex, dDatum);
		return;
		}

	if (iIndex < 0)
		return;

	OnCopyOnWrite();

	if (iIndex >= m_Values.GetCount())
		m_Values.InsertEmpty(iIndex + 1 - m_Values.GetCount());

	m_Values[iIndex] = dDatum;
	}

void CAEONObject::SetElement (int iIndex, CDatum dDatum)

//	SetElement
//
//	Sets the element by schema ordinal.

	{
	const IDatatype& Type = m_dType;
	if (iIndex < 0 || iIndex >= Type.GetMemberCount())
		return;

	IDatatype::SMemberDesc Member = Type.GetMember(iIndex);
	if (Member.iType != IDatatype::EMemberType::InstanceKeyVar
			&& Member.iType != IDatatype::EMemberType::InstanceVar)
		return;

	OnCopyOnWrite();
	SetSlot(iIndex, Member, dDatum);
	}

void CAEONObject::SetElement (const CString &sKey, CDatum dDatum)

//	SetElement
//
//	Sets the element.

	{
	const IDatatype& Type = m_dType;
	if (Type.GetMemberCount() == 0)
		{
		OnCopyOnWrite();
		m_Map.SetAt(sKey, dDatum);
		}
	else
		{
		int iMember;
		IDatatype::SMemberDesc Member;
		if (!FindSlot(sKey, &iMember, &Member))
			return;

		OnCopyOnWrite();
		SetSlot(iMember, Member, dDatum);
		}
	}

void CAEONObject::SetSlot (int iSlot, const IDatatype::SMemberDesc &Member, CDatum dDatum)
	{
	EnsureSlots();

	if (iSlot < 0 || iSlot >= m_Values.GetCount())
		return;

	if (((const IDatatype&)dDatum.GetDatatype()).IsA(Member.dType))
		m_Values[iSlot] = dDatum;
	else
		m_Values[iSlot] = CDatum::CreateAsType(Member.dType, dDatum);
	}

void CAEONObject::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElement
//
//	Sets the element.

	{
	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32())
		{
		int iIndex = dIndex;
		if (HasSlots())
			{
			if (iIndex >= 0 && iIndex < GetCount())
				{
				OnCopyOnWrite();
				SetElement(GetKey(iIndex), dDatum);
				}
			}
		else if (iIndex >= 0 && iIndex < m_Map.GetCount())
			{
			OnCopyOnWrite();
			SetElement(GetKey(iIndex), dDatum);
			}
		}
	else
		{
		OnCopyOnWrite();
		SetElement(dIndex.AsString(), dDatum);
		}

	}
