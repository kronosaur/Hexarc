//	CAEONObject.cpp
//
//	CAEONObject class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE_SPECIAL,			"__datatype__");

DECLARE_CONST_STRING(TYPENAME_OBJECT,					"object");

const CString &CAEONObject::GetTypename () const { return TYPENAME_OBJECT; }

CDatum CAEONObject::GetElement (int iIndex) const

//	GetElement
//
//	Returns the element.

	{
	if (iIndex < 0)
		return CDatum();

	//	We keep the order by the schema. Later we should store in that order
	//	too.

	const IDatatype& Type = m_dType;
	for (int i = 0; i < Type.GetMemberCount(); i++)
		{
		auto MemberDesc = Type.GetMember(i);
		if (MemberDesc.iType == IDatatype::EMemberType::InstanceKeyVar || MemberDesc.iType == IDatatype::EMemberType::InstanceVar)
			{
			if (iIndex == 0)
				return CComplexStruct::GetElement(MemberDesc.sID);
			else
				iIndex--;
			}
		}

	return CDatum();
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
		if (MemberDesc.iType == IDatatype::EMemberType::InstanceKeyVar || MemberDesc.iType == IDatatype::EMemberType::InstanceVar)
			{
			if (iIndex == 0)
				return MemberDesc.sID;
			else
				iIndex--;
			}
		}

	return NULL_STR;
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
	//	Create a new object and add it to the map.

	CAEONObject *pObj = new CAEONObject;
	CDatum dValue(pObj);
	Serialized.Add(dwID, dValue);

	//	Load the datatype.

	pObj->m_dType = CDatum::DeserializeAEON(Stream, Serialized);

	//	Load each member.

	int iCount = (int)Stream.ReadDWORD();
	pObj->m_Map.GrowToFit(iCount);
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
		int iMember = Type.FindMember(sKey);
		if (iMember == -1)
			return;

		OnCopyOnWrite();

		IDatatype::SMemberDesc Member = Type.GetMember(iMember);
		if (((const IDatatype&)dDatum.GetDatatype()).IsA(Member.dType))
			{
			m_Map.SetAt(sKey, dDatum);
			}
		else
			{
			CDatum dNewValue = CDatum::CreateAsType(Member.dType, dDatum);
			m_Map.SetAt(sKey, dNewValue);
			}
		}
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
		if (iIndex >= 0 && iIndex < m_Map.GetCount())
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
