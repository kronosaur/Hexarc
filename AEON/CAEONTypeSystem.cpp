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
	CDatum dNewType = CreateAnonymousArray(NULL_STR, dElementType);
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
	CDatum dNewType = CreateAnonymousDictionary(NULL_STR, dKeyType, dElementType);
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
	CDatum dNewSchema = CreateAnonymousSchema(Columns);
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

CDatum CAEONTypeSystem::AddAnonymousTensor (CDatum dElementType, const TArray<CDatum>& Dimensions)

//	AddAnonymousTensor
//
//	Adds a new tensor type (or returns an existing one).

	{
	IDatatype* pNewType = new CDatatypeTensor(CDatatypeTensor::SCreate({ NULL_STR, 0, dElementType, Dimensions }));
	CDatum dNewType(new CComplexDatatype(pNewType));

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

CDatum CAEONTypeSystem::CreateAnonymousArray (const CString& sFullyQualifiedName, CDatum dElementType)

//	CreateAnonymousArray
//
//	Creates an anonymous array given parameters.

	{
	if (dElementType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	IDatatype *pNewType = new CDatatypeArray(CDatatypeArray::SCreate({ sFullyQualifiedName, 0, dElementType, false }));

	CDatum dType(new CComplexDatatype(pNewType));
	return dType;
	}

CDatum CAEONTypeSystem::CreateAnonymousDictionary (const CString& sFullyQualifiedName, CDatum dKeyType, CDatum dElementType)

//	CreateAnonymousDictionary
//
//	Creates an anonymous dictionary given parameters.

	{
	if (dElementType.GetBasicType() != CDatum::typeDatatype || dKeyType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	IDatatype *pNewType = new CDatatypeArray(CDatatypeArray::SCreate({ sFullyQualifiedName, 0, dElementType, false, true, dKeyType }));

	CDatum dType(new CComplexDatatype(pNewType));
	return dType;
	}

CDatum CAEONTypeSystem::CreateAnonymousSchema (const TArray<IDatatype::SMemberDesc> &Columns)

//	CreateAnonymousSchema
//
//	Create an anonymous schema.

	{
	CString sFullyQualifiedName = CAEONTypeSystem::MakeFullyQualifiedName(NULL_STR, strPattern("AnonymousSchema%08x", mathRandom()));
	return CAEONTypes::CreateSchema(sFullyQualifiedName, Columns, 0);
	}

CDatum CAEONTypeSystem::CreateAnonymousTable (const CString& sFullyQualifiedName, CDatum dSchema)

//	CreateAnonymousTable
//
//	Creates an anonymous table of the given schema.

	{
	if (((const IDatatype&)dSchema).GetClass() != IDatatype::ECategory::Schema)
		throw CException(errFail);

	IDatatype *pNewType = new CDatatypeArray(CDatatypeArray::SCreate({ sFullyQualifiedName, 0, dSchema, true }));

	CDatum dType(new CComplexDatatype(pNewType));
	return dType;
	}

CDatum CAEONTypeSystem::CreateNullableType (const CString& sFullyQualifiedName, CDatum dVariantType)

//	CreateNullableType
//
//	Creates a nullable type.

	{
	const IDatatype& VariantType = dVariantType;

	//	Certain types are intrinsically nullable, which means we can just return
	//	them.

	if (VariantType.CanBeNull())
		return dVariantType;

	CString sID;
	if (sFullyQualifiedName.IsEmpty())
		{
		if (VariantType.GetCoreType())
			sID = MakeFullyQualifiedName(NULL_STR, strPattern("%s?", VariantType.GetName()));
		else
			sID = MakeFullyQualifiedName(NULL_STR, strPattern("NullableType%08x", mathRandom()));
		}
	else
		sID = sFullyQualifiedName;

	IDatatype *pNewType = new CDatatypeNullable(CDatatypeNullable::SCreate({ sID, 0, dVariantType }));

	CDatum dType(new CComplexDatatype(pNewType));
	return dType;
	}

CDatum CAEONTypeSystem::CreateDatatypeClass (const CString &sFullyQualifiedName, IDatatype **retpNewType)

//	CreateDatatypeClass
//
//	Creates a new datatype.

	{
	IDatatype *pNewType = new CDatatypeClass({ sFullyQualifiedName, CDatatypeList({ CAEONTypes::Get(IDatatype::CLASS_T) }) });
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

CDatum CAEONTypeSystem::CreateDatatypeFunction (const CString& sFullyQualifiedName, const IDatatype::SReturnTypeDesc& Return, const TArray<IDatatype::SArgDesc>& Args, IDatatype** retpNewType, CString* retsError)

//	CreateDatatypeFunction
//
//	Creates a new function.

	{
	CString sID;
	if (sFullyQualifiedName.IsEmpty())
		sID = MakeFullyQualifiedName(NULL_STR, strPattern("AnonymousFunction%08x", mathRandom()));
	else
		sID = sFullyQualifiedName;

	IDatatype* pNewType = new CDatatypeFunction(CDatatypeFunction::SCreate({ sID, Return, Args }));
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

CDatum CAEONTypeSystem::CreateDatatypeSchema (const CString& sFullyQualifiedName, IDatatype **retpNewType)

//	CreateDatatypeSchema
//
//	Creates a new datatype.

	{
	IDatatype *pNewType = new CDatatypeSchema({ sFullyQualifiedName, CDatatypeList({ CAEONTypes::Get(IDatatype::SCHEMA) }), 0 });
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

CDatum CAEONTypeSystem::FindCoreType (const CString& sFullyQualifiedName, const IDatatype** retpDatatype)

//	FindCoreType
//
//	Looks for a core type. Return nil if not found.

	{
	return CAEONTypes::FindCoreType(sFullyQualifiedName, retpDatatype);
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

CDatum CAEONTypeSystem::GetCoreType (DWORD dwType)

//	GetCoreType
//
//	Returns the given core type.

	{
	return CAEONTypes::Get(dwType);
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
	for (int i = 0; i < m_Types.GetCount(); i++)
		m_Types[i].Mark();
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

