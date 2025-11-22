//	CComplexStruct.cpp
//
//	CComplexStruct class
//	Copyright (c) 2010 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_STRUCT,					"struct");

TDatumPropertyHandler<CComplexStruct> CComplexStruct::m_Properties = {
	{
		"columns",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CComplexStruct& Obj, const CString &sProperty)
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
		[](const CComplexStruct &Obj, const CString &sProperty)
			{
			return Obj.GetDatatype();
			},
		NULL,
		},
	{
		"keys",
		"$ArrayOfString",
		"Returns an array of keys.",
		[](const CComplexStruct& Obj, const CString &sProperty)
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
		[](const CComplexStruct& Obj, const CString &sProperty)
			{
			return CDatum(Obj.GetCount());
			},
		NULL,
		},
	};

TDatumMethodHandler<IComplexDatum> *CComplexStruct::m_pMethodsExt = NULL;

CComplexStruct::CComplexStruct (CDatum dSrc)

//	CComplexStruct constructor

	{
	//	Clone from another complex structure

	for (int i = 0; i < dSrc.GetCount(); i++)
		{
		CString sKey = dSrc.GetKey(i);
		if (!sKey.IsEmpty())
			SetElement(sKey, dSrc.GetElement(i));
		}
	}

CComplexStruct::CComplexStruct (const TSortMap<CString, CString> &Src)

//	CComplexStruct construtor

	{
	int i;

	for (i = 0; i < Src.GetCount(); i++)
		{
		const CString &sKey = Src.GetKey(i);
		if (!sKey.IsEmpty())
			SetElement(sKey, Src.GetValue(i));
		}
	}

CComplexStruct::CComplexStruct (const TSortMap<CString, CDatum> &Src)

//	CComplexStruct constructor

	{
	m_Map = Src;
	}

const CString &CComplexStruct::GetTypename (void) const { return TYPENAME_STRUCT; }

void CComplexStruct::AppendStruct (CDatum dDatum)

//	AppendStruct
//
//	Appends the element of the given structure

	{
	if (dDatum.IsStruct())
		{
		OnCopyOnWrite();

		for (int i = 0; i < dDatum.GetCount(); i++)
			SetElement(dDatum.GetKey(i), dDatum.GetElement(i));
		}
	}

CString CComplexStruct::AsString (void) const

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

		Output.Write(m_Map.GetKey(i));
		Output.Write(":", 1);

		Output.Write(m_Map[i].AsString());
		}

	Output.Write("}", 1);

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

size_t CComplexStruct::CalcMemorySize (void) const

//	CalcMemorySize
//
//	Computes the amount of memory being used.

	{
	size_t dwSize = 0;

	for (int i = 0; i < m_Map.GetCount(); i++)
		{
		dwSize += m_Map.GetKey(i).GetLength() + sizeof(DWORD) + 1;
		dwSize += m_Map[i].CalcMemorySize();
		}

	return dwSize;
	}

IComplexDatum *CComplexStruct::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Clones a copy

	{
	switch (iMode)
		{
		case CDatum::EClone::ShallowCopy:
			return new CComplexStruct(m_Map);

		case CDatum::EClone::CopyOnWrite:
			//	Default handling
			return NULL;

		case CDatum::EClone::DeepCopy:
			{
			auto pClone = new CComplexStruct(m_Map);
			pClone->CloneContents();
			return pClone;
			}

		case CDatum::EClone::Isolate:
			return NULL;

		default:
			throw CException(errFail);
		}
	}

void CComplexStruct::CloneContents ()

//	CloneContents
//
//	Clones all content so that it is a copy.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i] = m_Map[i].Clone(CDatum::EClone::DeepCopy);
	}

bool CComplexStruct::Contains (CDatum dValue) const

//	Contains
//
//	Returns TRUE if we contain dValue.

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		if (m_Map[i].Contains(dValue))
			return true;

	return false;
	}

bool CComplexStruct::FindElement (const CString &sKey, CDatum *retpValue) const

//	FindElement
//
//	Find element by key.

	{
	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue == NULL)
		return false;

	if (retpValue)
		*retpValue = *pValue;

	return true;
	}

CDatum CComplexStruct::GetElement (const CString &sKey) const

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

CDatum CComplexStruct::GetElementAt (CAEONTypeSystem &TypeSystem, CDatum dIndex) const

//	GetElement
//
//	Returns the element.

	{
	int iIndex;

	if (dIndex.IsNil())
		return CDatum();
	else if (dIndex.IsNumberInt32(&iIndex))
		{
		if (iIndex >= 0 && iIndex < m_Map.GetCount())
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
				CDatum* pValue = m_Map.GetAt(sKey);
				if (pValue)
					dResult.SetElement(sKey, *pValue);
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
					if (iIndex >= 0 && iIndex < m_Map.GetCount())
						dResult.Append(m_Map[iIndex]);
					}
				else
					{
					CString sKey = dEntry.AsString();
					CDatum* pValue = m_Map.GetAt(sKey);
					if (pValue)
						dResult.Append(*pValue);
					}
				}

			return dResult;
			}
		}
	else
		{
		CDatum* pValue = m_Map.GetAt(dIndex.AsString());
		if (pValue)
			return *pValue;
		else
			return CDatum();
		}
	}

CDatum CComplexStruct::GetMethod (const CString &sMethod) const

//	GetMethod
//
//	Returns a method

	{
	CDatum *pValue = m_Map.GetAt(sMethod);
	if (pValue && pValue->CanInvoke())
		return *pValue;

	if (m_pMethodsExt) 
		return m_pMethodsExt->GetMethod(sMethod);
	else
		return CDatum();
	}

CDatum CComplexStruct::GetProperty (const CString &sKey) const

//	GetProperty
//
//	Returns the property.

	{
	CDatum *pValue = m_Map.GetAt(sKey);
	if (pValue)
		return *pValue;

	return m_Properties.GetProperty(*this, sKey);
	}

void CComplexStruct::OnCopyOnWrite ()

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

void CComplexStruct::OnMarked (void)

//	OnMarked
//
//	Mark

	{
	for (int i = 0; i < m_Map.GetCount(); i++)
		m_Map[i].Mark();
	}

int CComplexStruct::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return 0;

	if (!dValue.IsStruct())
		return KeyCompareNoCase(AsString(), dValue.AsString());

	//	If we're the same type, then we can just compare by member because we
	//	guarantee that the order will be the same.

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

int CComplexStruct::OpCompareExact (CDatum::Types iValueType, CDatum dValue) const

//	OpCompareExact
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return 0;

	if (!dValue.IsStruct())
		return KeyCompare(AsString(), dValue.AsString());

	//	If we're the same type, then we can just compare by member because we
	//	guarantee that the order will be the same.

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

bool CComplexStruct::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if the two values are equivalent.

	{
	CRecursionGuard Guard(*this);
	if (Guard.InRecursion())
		return true;

	if (!dValue.IsStruct())
		return false;

	else if (GetCount() != dValue.GetCount())
		return false;

	//	If we're the same type, then we can just do a member-by-member compare.

	else if (GetBasicType() == iValueType)
		{
		for (int i = 0; i < GetCount(); i++)
			if (!strEqualsNoCase(GetKey(i), dValue.GetKey(i))
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

bool CComplexStruct::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const

//	OpIsIdentical
//
//	Returns TRUE if the two values are identical.

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

bool CComplexStruct::RemoveElementAt (CDatum dIndex)

//	RemoveElementAt
//
//	Removes an element by key.

	{
	OnCopyOnWrite();

	if (dIndex.IsStruct())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			m_Map.DeleteAt(dIndex.GetKey(i));
		}
	else if (dIndex.IsArray())
		{
		for (int i = 0; i < dIndex.GetCount(); i++)
			m_Map.DeleteAt(dIndex.GetElement(i).AsString());
		}
	else
		m_Map.DeleteAt(dIndex.AsString());

	return true;
	}

void CComplexStruct::ResolveDatatypes (const CAEONTypeSystem &TypeSystem)

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

CDatum CComplexStruct::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
	//	Create a new object and add it to the map.

	CComplexStruct *pObj = new CComplexStruct;
	CDatum dValue(pObj);
	Serialized.Add(dwID, dValue);

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

void CComplexStruct::SetElementAt (CDatum dIndex, CDatum dDatum)

//	SetElement
//
//	Sets the element.

	{
	OnCopyOnWrite();

	if (dIndex.IsNil())
		{ }
	else if (dIndex.IsNumberInt32())
		{
		int iIndex = dIndex;
		if (iIndex >= 0 && iIndex < m_Map.GetCount())
			m_Map[iIndex] = dDatum;
		}
	else
		{
		m_Map.SetAt(dIndex.AsString(), dDatum);
		}
	}

bool CComplexStruct::SetElementIfNew (const CString& sKey, CDatum dValue)

//	SetElementIfNew
//
//	Sets the element but only if it is not already set.

	{
	int iPos;
	if (m_Map.FindPos(sKey, &iPos))
		return false;

	OnCopyOnWrite();
	m_Map.InsertSorted(sKey, dValue, iPos);
	return true;
	}
