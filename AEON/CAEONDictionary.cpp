//	CAEONDictionary.cpp
//
//	CAEONDictionary class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_DICTIONARY,				"dictionary");

TDatumPropertyHandler<CAEONDictionary> CAEONDictionary::m_Properties = {
	{
		"columns",
		"a",
		"Returns an array of keys.",
		[](const CAEONDictionary& Obj, const CString& sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.m_Map.GetCount());
			for (int i = 0; i < Obj.m_Map.GetCount(); i++)
				dResult.Append(Obj.m_Map.GetKey(i));

			return dResult;
			},
		NULL,
		},
	{
		"datatype",
		"%",
		"Returns the type of the dictionary.",
		[](const CAEONDictionary& Obj, const CString& sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"elementtype",
		"%",
		"Returns the element type of the dictionary.",
		[](const CAEONDictionary& Obj, const CString& sProperty)
			{
			return ((const IDatatype&)Obj.GetDatatype()).GetElementType();
			},
		NULL,
		},
	{
		"keytype",
		"%",
		"Returns the key type of the dictionary.",
		[](const CAEONDictionary& Obj, const CString& sProperty)
			{
			return ((const IDatatype&)Obj.GetDatatype()).GetKeyType();
			},
		NULL,
		},
	{
		"keys",
		"a",
		"Returns an array of keys.",
		[](const CAEONDictionary& Obj, const CString& sProperty)
			{
			CDatum dResult(CDatum::typeArray);
			dResult.GrowToFit(Obj.m_Map.GetCount());
			for (int i = 0; i < Obj.m_Map.GetCount(); i++)
				dResult.Append(Obj.m_Map.GetKey(i));

			return dResult;
			},
		NULL,
		},
	{
		"length",
		"I",
		"Returns the number of entries in the dictionary.",
		[](const CAEONDictionary& Obj, const CString& sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum>* CAEONDictionary::m_pMethodsExt = NULL;

CAEONDictionary::CAEONDictionary (CDatum dDatatype, CDatum dSrc)

//	CAEONDictionary constructor

	{
	InitDatatype(dDatatype);

	//	If this is a structure, then create from the structure

	if (dSrc.IsStruct())
		{
		GrowToFit(dSrc.GetCount());

		for (int i = 0; i < dSrc.GetCount(); i++)
			SetElementAt(dSrc.GetKeyEx(i), dSrc.GetElement(i));
		}

	//	If a table, then convert to dictionary.

	else if (const IAEONTable* pTable = dSrc.GetTableInterface())
		{
		GrowToFit(pTable->GetRowCount());

		for (int i = 0; i < pTable->GetRowCount(); i++)
			{
			SetElementAt(pTable->GetRowID(i), dSrc.GetElement(i));
			}
		}
	
	//	Otherwise, we expect an array of key/value pairs

	else if (dSrc.IsArray())
		{
		GrowToFit(dSrc.GetCount());

		for (int i = 0; i < dSrc.GetCount(); i++)
			{
			CDatum dPair = dSrc.GetElement(i);
			if (dPair.GetCount() < 2)
				continue;
					
			SetElementAt(dPair.GetElement(0), dPair.GetElement(1));
			}
		}

	//	Else nothing
	}

CAEONDictionary::CAEONDictionary (CDatum dDatatype, const TSortMap<CDatum, CDatum>& Src)

//	CAEONDictionary constructor

	{
	InitDatatype(dDatatype);

	m_Map = Src;
	}

const CString& CAEONDictionary::GetTypename () const { return TYPENAME_DICTIONARY; }

void CAEONDictionary::AppendStruct (CDatum dDatum)

//	AppendStruct
//
//	Appends the element of the given structure

	{
	if (dDatum.IsStruct())
		{
		OnCopyOnWrite();

		for (int i = 0; i < dDatum.GetCount(); i++)
			SetElementAt(dDatum.GetKeyEx(i), dDatum.GetElement(i));
		}
	}

CString CAEONDictionary::AsString () const

//	AsString
//
//	Represent as a string

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return AsAddress();

	CStringBuffer Output;

	Output.Write("{", 1);

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		if (i != 0)
			Output.Write(" ", 1);

		Output.Write(m_Map.GetKey(i).AsString());
		Output.Write(":", 1);

		Output.Write(m_Map[i].AsString());
		}

	Output.Write("}", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

size_t CAEONDictionary::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory being used.

	{
	size_t dwSize = 0;

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		dwSize += m_Map.GetKey(i).CalcMemorySize();
		dwSize += m_Map[i].CalcMemorySize();
		}

	return dwSize;
	}

IComplexDatum *CAEONDictionary::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clones a copy

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CAEONDictionary(m_dDatatype, m_Map);

		case CDatum::EClone::CopyOnWrite:
			//	Default handling
			return NULL;

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CAEONDictionary(m_dDatatype, m_Map);
			pClone->CloneContents();
			return pClone;
			}

		case CDatum::EClone::Isolate:
			return NULL;

		default:
			throw CException(errFail);
		}
	}

void CAEONDictionary::CloneContents ()

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i] = m_Map[i].Clone(CDatum::EClone::DeepCopy);
	}

bool CAEONDictionary::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE if we contain dValue.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		if (m_Map[i].Contains(dValue))
			return true;

	return false;
	}

CDatum CAEONDictionary::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap& Serialized)
	{
	//	Create a new object and add it to the map.

	CAEONDictionary *pObj = new CAEONDictionary;
	CDatum dValue(pObj);
	Serialized.Add(dwID, dValue);

	//	Load types

	pObj->m_dDatatype = CDatum::DeserializeAEON(Stream, Serialized);
	pObj->m_dKeyType = ((const IDatatype&)pObj->m_dDatatype).GetKeyType();
	pObj->m_dElementType = ((const IDatatype&)pObj->m_dDatatype).GetElementType();

	//	Load each member.

	int iCount = (int)Stream.ReadDWORD();
	pObj->m_Map.GrowToFit(iCount);
	for (int i = 0; i < iCount; i++)
		{
		CDatum dKey = CDatum::DeserializeAEON(Stream, Serialized);
		CDatum dElement = CDatum::DeserializeAEON(Stream, Serialized);
		
		pObj->SetElementAt(dKey, dElement);
		}

	return dValue;
	}

bool CAEONDictionary::Find (CDatum dValue, int *retiIndex) const

//	Find
//
//	Finds an element by value.

	{
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsEqual(m_Map[i]))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

CDatum CAEONDictionary::FindAll (CDatum dValue) const

//	FindAll
//
//	Find all elements that match the value.

	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsEqual(m_Map[i]))
			dResult.Append(m_Map.GetKey(i));
		}

	return dResult;
	}

CDatum CAEONDictionary::FindAllExact (CDatum dValue) const

//	FindAllExact
//
//	Find all elements identical to the value.

	{
	CDatum dResult(CDatum::typeArray);
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsIdentical(m_Map[i]))
			dResult.Append(m_Map.GetKey(i));
		}

	return dResult;
	}

bool CAEONDictionary::FindExact (CDatum dValue, int *retiIndex) const

//	FindExact
//
//	Find an element that is identical to the value.

	{
	for (int i = 0; i < GetCount(); i++)
		{
		if (dValue.OpIsIdentical(m_Map[i]))
			{
			if (retiIndex)
				*retiIndex = i;
			return true;
			}
		}

	return false;
	}

bool CAEONDictionary::FindElement (const CString& sKey, CDatum* retpValue) const

//	FindElement
//
//	Find element by key.

	{
	CDatum* pValue = m_Map.GetAt(sKey);
	if (pValue == NULL)
		return false;

	if (retpValue)
		*retpValue = *pValue;

	return true;
	}

CDatum CAEONDictionary::DatumToElement (CDatum dValue) const
	{
	const IDatatype &Type = dValue.GetDatatype();
	if (Type.IsA(m_dElementType))
		return dValue;

	return CDatum::CreateAsType(m_dElementType, dValue);
	}

CDatum CAEONDictionary::DatumToKey (CDatum dValue) const
	{
	const IDatatype &Type = dValue.GetDatatype();
	if (Type.IsA(m_dKeyType))
		return dValue;

	return CDatum::CreateAsType(m_dKeyType, dValue);
	}

CDatum CAEONDictionary::GetElement (const CString& sKey) const

//	GetElement
//
//	Returns the element.

	{
	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue)
		return *pValue;
	else
		return CDatum();
	}

CDatum CAEONDictionary::GetElementAt (CAEONTypeSystem& TypeSystem, CDatum dIndex) const

//	GetElement
//
//	Returns the element.

	{
	CDatum* pValue = m_Map.GetAt(dIndex);
	if (pValue)
		return *pValue;
	else
		return CDatum();
	}

CDatum CAEONDictionary::GetMethod (const CString& sMethod) const

//	GetMethod
//
//	Returns a method

	{
	if (m_pMethodsExt) 
		return m_pMethodsExt->GetMethod(sMethod);
	else
		return CDatum();
	}

CDatum CAEONDictionary::GetProperty (const CString& sKey) const

//	GetProperty
//
//	Returns the property.

	{
	return m_Properties.GetProperty(*this, sKey);
	}

void CAEONDictionary::InitDatatype (CDatum dDatatype)

//	InitDatatype
//
//	Initializes the datatype.

	{
	if (dDatatype.IsNil())
		dDatatype = CAEONTypes::Get(IDatatype::DICTIONARY);

	const IDatatype& Datatype = dDatatype;
	if (!Datatype.IsA(IDatatype::DICTIONARY))
		throw CException(errFail);

	m_dDatatype = dDatatype;
	m_dKeyType = Datatype.GetKeyType();
	if (m_dKeyType.IsNil())
		throw CException(errFail);

	m_dElementType = Datatype.GetElementType();
	if (m_dElementType.IsNil())
		throw CException(errFail);
	}

void CAEONDictionary::OnCopyOnWrite ()

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

void CAEONDictionary::OnMarked (void)

//	OnMarked
//
//	Mark

	{
	m_dDatatype.Mark();
	m_dKeyType.Mark();
	m_dElementType.Mark();

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		const_cast<CDatum&>(m_Map.GetKey(i)).Mark();
		m_Map[i].Mark();
		}
	}

int CAEONDictionary::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	if (!dValue.IsStruct())
		return KeyCompare(AsString(), dValue.AsString());

	//	If we're the same type, then we can just compare by member because we
	//	guarantee that the order will be the same.

	if (GetBasicType() == iValueType)
		{
		int iCount = Min(GetCount(), dValue.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = GetKeyEx(i).OpCompare(dValue.GetKeyEx(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = GetElement(i).OpCompare(dValue.GetElement(i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(GetCount(), dValue.GetCount());
		}

	//	Otherwise, we're trying to compare a struct with an object, in which
	//	the order of members might be different. We define equality as having
	//	all the same members equal, regardless of order.

	else if (OpIsEqual(iValueType, dValue))
		return 0;

	//	Otherwise, structs always come before objects

	else if (GetBasicType() == CDatum::typeStruct)
		return -1;
	else
		return 1;
	}

int CAEONDictionary::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const

//	OpCompareExact
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	if (!dValue.IsStruct())
		return KeyCompare(AsString(), dValue.AsString());

	//	If we're the same type, then we can just compare by member because we
	//	guarantee that the order will be the same.

	if (GetBasicType() == iValueType)
		{
		int iCount = Min(GetCount(), dValue.GetCount());
		for (int i = 0; i < iCount; i++)
			{
			int iCompare = GetKeyEx(i).OpCompareExact(dValue.GetKeyEx(i));
			if (iCompare != 0)
				return iCompare;

			iCompare = GetElement(i).OpCompareExact(dValue.GetElement(i));
			if (iCompare != 0)
				return iCompare;
			}

		return KeyCompare(GetCount(), dValue.GetCount());
		}

	//	Otherwise, we're trying to compare a struct with an object, in which
	//	the order of members might be different. We define equality as having
	//	all the same members equal, regardless of order.

	else if (OpIsIdentical(iValueType, dValue))
		return 0;

	//	Otherwise, structs always come before objects

	else if (GetBasicType() == CDatum::typeStruct)
		return -1;
	else
		return 1;
	}

bool CAEONDictionary::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if the two values are equivalent.

	{
	if (!dValue.IsStruct())
		return false;

	else if (GetCount() != dValue.GetCount())
		return false;

	//	If we're the same type, then we can just do a member-by-member compare.

	else if (GetBasicType() == iValueType)
		{
		for (int i = 0; i < GetCount(); i++)
			if (!GetKeyEx(i).OpIsEqual(dValue.GetKeyEx(i))
					|| !GetElement(i).OpIsEqual(dValue.GetElement(i)))
				return false;

		return true;
		}

	//	Otherwise, we're probably comparing a struct to an object and we cannot
	//	guarantee that the order of members is the same.

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

bool CAEONDictionary::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const

//	OpIsIdentical
//
//	Returns TRUE if the two values are identical.

	{
	if ((const IDatatype&)GetDatatype() != (const IDatatype&)dValue.GetDatatype())
		return false;

	if (GetCount() != dValue.GetCount())
		return false;

	for (int i = 0; i < GetCount(); i++)
		if (!GetKeyEx(i).OpIsIdentical(dValue.GetKeyEx(i))
				|| !GetElement(i).OpIsIdentical(dValue.GetElement(i)))
			return false;

	return true;
	}

bool CAEONDictionary::RemoveElementAt (CDatum dIndex)

//	RemoveElementAt
//
//	Removes an element by key.

	{
	OnCopyOnWrite();
	m_Map.DeleteAt(DatumToKey(dIndex));

	return true;
	}

void CAEONDictionary::ResolveDatatypes (const CAEONTypeSystem& TypeSystem)

//	ResolveDatatypes
//
//	Resolve datatypes after deserialization.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return;

	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].ResolveDatatypes(TypeSystem);
	}

void CAEONDictionary::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const

//	SerializeAEON
//
//	Serializes the object to a stream.

	{
	//	See if we've already serialized this. If so, then we just write out the
	//	reference.

	if (!Serialized.WriteID(Stream, this, CDatum::SERIALIZE_TYPE_DICTIONARY))
		return;

	//	Write out the datatype

	m_dDatatype.SerializeAEON(Stream, Serialized);

	//	Now write out each member variable

	Stream.Write(GetCount());
	for (int i = 0; i < GetCount(); i++)
		{
		GetKeyEx(i).SerializeAEON(Stream, Serialized);
		GetElement(i).SerializeAEON(Stream, Serialized);
		}
	}


void CAEONDictionary::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElement
//
//	Sets the element.

	{
	OnCopyOnWrite();
	m_Map.SetAt(DatumToKey(dIndex), DatumToElement(dDatum));
	}

bool CAEONDictionary::SetElementIfNew (CDatum dKey, CDatum dValue)

//	SetElementIfNew
//
//	Sets the element but only if it is not already set.

	{
	dKey = DatumToKey(dKey);

	int iPos;
	if (m_Map.FindPos(dKey, &iPos))
		return false;

	OnCopyOnWrite();
	m_Map.InsertSorted(dKey, DatumToElement(dValue), iPos);
	return true;
	}
