//	CAEONTypeSystem.cpp
//
//	CAEONTypeSystem class
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATATYPE,					"datatype");
DECLARE_CONST_STRING(FIELD_DESCRIPTION,					"description");
DECLARE_CONST_STRING(FIELD_LABEL,						"label");
DECLARE_CONST_STRING(FIELD_NAME,						"name");
DECLARE_CONST_STRING(FIELD_ORDINAL,						"ordinal");

CAEONTypeSystem CAEONTypeSystem::m_Null;

CDatum CAEONTypeSystem::AddAnonymousArray (CDatum dElementType)

//	AddAnonymousArray
//
//	Adds a new array type (or returns an existing one).

	{
	CDatum dNewType = CAEONTypes::CreateArray(NULL_STR, dElementType);
	const IDatatype& NewType = dNewType;

	//	See if this type already exists.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if (NewType == m_Types[i])
			return m_Types[i];
		}
	
	if (!AddType(dNewType))
		return CDatum();
	
	return dNewType;
	}

CDatum CAEONTypeSystem::AddAnonymousDictionary (CDatum dKeyType, CDatum dElementType)

//	AddAnonymousDictionary
//
//	Adds a new dictionary type (or returns an existing one).

	{
	CDatum dNewType = CAEONTypes::CreateDictionary(NULL_STR, dKeyType, dElementType);
	const IDatatype& NewType = dNewType;

	//	See if this type already exists.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if (NewType == m_Types[i])
			return m_Types[i];
		}
	
	if (!AddType(dNewType))
		return CDatum();
	
	return dNewType;
	}

CDatum CAEONTypeSystem::AddAnonymousRange (int iMin, int iMax)

//	AddAnonymousRange
//
//	Adds a new range type (or returns an existing one).

	{
	CDatum dNewType = CAEONTypes::CreateInt32SubRange(NULL_STR, iMin, iMax);
	const IDatatype& NewType = dNewType;

	//	See if this type already exists.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if (NewType == m_Types[i])
			return m_Types[i];
		}

	if (!AddType(dNewType))
		return CDatum();

	return dNewType;
	}

CDatum CAEONTypeSystem::AddAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns)

//	AddAnonymousSchema
//
//	Adds a new schema (or returns an existing one).

	{
	CDatum dNewSchema = CAEONTypes::CreateSchema(NULL_STR, Columns);
	const IDatatype& NewSchema = dNewSchema;

	//	See if this schema already exists.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if (NewSchema == m_Types[i])
			return m_Types[i];
		}

	if (!AddType(dNewSchema))
		return CDatum();

	return dNewSchema;
	}

CDatum CAEONTypeSystem::AddAnonymousTensor (CDatum dElementType, TArray<CDatum>&& Dimensions)

//	AddAnonymousTensor
//
//	Adds a new tensor type (or returns an existing one).

	{
	CDatum dNewType = CAEONTypes::CreateTensor(NULL_STR, dElementType, std::move(Dimensions));

	//	See if this type already exists.

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if ((const IDatatype&)dNewType == (const IDatatype&)m_Types[i])
			return m_Types[i];
		}

	if (!AddType(dNewType))
		return CDatum();

	return dNewType;
	}

bool CAEONTypeSystem::AddType (CDatum dType)

//	AddType
//
//	Adds a type and returns FALSE if we failed.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		return false;

	const IDatatype &Datatype = dType;
	bool bNew;
	DWORD* pAtom = m_Index.SetAt(strToLower(Datatype.GetFullyQualifiedName()), &bNew);
	if (!bNew)
		return false;

	*pAtom = m_Types.GetCount();
	m_Types.Insert(dType);

	return true;
	}

DWORD CAEONTypeSystem::Atomize (CStringView sFullyQualifiedName)

//	Atomize
//
//	Gets the atom for the given type.

	{
	auto* pAtom = m_Index.GetAt(strToLower(sFullyQualifiedName));
	if (pAtom == NULL)
		return NULL_ATOM;

	return *pAtom;
	}

CDatum CAEONTypeSystem::FindType (const CString& sFullyQualifiedName, const IDatatype **retpDatatype) const

//	FindType
//
//	Looks for the datatype by name. Returns NULL if not found.

	{
	auto pEntry = m_Index.GetAt(strToLower(sFullyQualifiedName));
	if (!pEntry)
		{
		if (retpDatatype)
			*retpDatatype = NULL;

		return CDatum();
		}

	CDatum dType = m_Types[*pEntry];

	if (retpDatatype)
		*retpDatatype = &(const IDatatype &)dType;

	return dType;
	}

CDatum CAEONTypeSystem::FindType (CDatum dType) const

//	FindType
//
//	Finds a type that is equal to the given type and returns it. If none is
//	found, we return Nil.

	{
	const IDatatype& TypeToFind = dType;

	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		if ((const IDatatype&)m_Types[i] == TypeToFind)
			return m_Types[i];
		}

	return CDatum();
	}

CDatum CAEONTypeSystem::GetTypeList () const

//	GetTypeList
//
//	Returns all types defined.

	{
	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(m_Types.GetCount());

	for (int i = 0; i < m_Types.GetCount(); i++)
		dResult.Append(m_Types[i]);

	return dResult;
	}

bool CAEONTypeSystem::InitFrom (CDatum dSerialized, CString *retsError)

//	InitFrom
//
//	Initialize from a serialized struct.

	{
	return true;
	}

CString CAEONTypeSystem::MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName)

//	MakeFullyQualifiedName
//
//	Returns a fully qualified name from a scope and name. A scope of NULL means
//	global scope.

	{
	return CAEONTypes::MakeFullyQualifiedName(sFullyQualifiedScope, sName);
	}

CString CAEONTypeSystem::MakeFullyQualifiedName (const TArray<CString>& Names)

//	MakeFullyQualifiedName
//
//	Returns a fully qualified name from a list of names.

	{
	if (Names.GetCount() == 0)
		return NULL_STR;

	CString sScope;
	for (int i = 0; i < Names.GetCount() - 1; i++)
		{
		sScope = MakeFullyQualifiedName(sScope, Names[i]);
		}

	return MakeFullyQualifiedName(sScope, Names[Names.GetCount() - 1]);
	}

void CAEONTypeSystem::Mark ()

//	Mark
//
//	Mark types in use.

	{
	DEBUG_TRY

	for (int i = 0; i < m_Types.GetCount(); i++)
		m_Types[i].Mark();

	DEBUG_CATCH
	}

CDatum CAEONTypeSystem::ResolveType (CDatum dType) const

//	ResolveType
//
//	Compares dType against our list of types. If this refers to one of our types
//	then we return it unchanged. If the type is equal to one of our types, then
//	we return our type (this helps when we deserialize a type). Otherwise, we
//	return the type unchanged.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		return dType;

	bool bDuplicateName = false;
	const IDatatype &Type = dType;
	for (int i = 0; i < m_Types.GetCount(); i++)
		{
		const IDatatype &OurType = m_Types[i];

		if (&Type == &OurType)
			return dType;

		//	If we have the same name and we're a subset of the registered type,
		//	then it means that the type got upgraded.

		else if (strEquals(Type.GetFullyQualifiedName(), OurType.GetFullyQualifiedName()))
			{
			if (OurType.IsSupersetOf(Type))
				return m_Types[i];

			//	Otherwise, we continue, but we remember that we have a duplicate
			//	name.

			bDuplicateName = true;
			}

		//	If the type is equal to one of our types, then we return our type.
		//	We cannot use == because that short-circuits if it is the same
		//	fully qualified name (but we can't rely on that because we might
		//	be loading an older version of the datatype).

		else if (Type.IsEqualEx(OurType))
			return m_Types[i];
		}

	return dType;
	}

CDatum CAEONTypeSystem::Serialize () const

//	Serialize
//
//	Serializes all types.

	{
	return CDatum();
	}

