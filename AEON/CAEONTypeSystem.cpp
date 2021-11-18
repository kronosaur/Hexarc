//	CAEONTypeSystem.cpp
//
//	CAEONTypeSystem class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ANY,						"Any");
DECLARE_CONST_STRING(TYPENAME_ARRAY,					"Array");
DECLARE_CONST_STRING(TYPENAME_BINARY,					"Binary");
DECLARE_CONST_STRING(TYPENAME_BOOL,						"Bool");
DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"Datatype");
DECLARE_CONST_STRING(TYPENAME_DATE_TIME,				"DateTime");
DECLARE_CONST_STRING(TYPENAME_FLOAT_64,					"Float64");
DECLARE_CONST_STRING(TYPENAME_FUNCTION,					"Function");
DECLARE_CONST_STRING(TYPENAME_INT_32,					"Int32");
DECLARE_CONST_STRING(TYPENAME_INT_64,					"Int64");
DECLARE_CONST_STRING(TYPENAME_INT_IP,					"IntIP");
DECLARE_CONST_STRING(TYPENAME_INTEGER,					"Integer");
DECLARE_CONST_STRING(TYPENAME_NULL,						"Null");
DECLARE_CONST_STRING(TYPENAME_NUMBER,					"Number");
DECLARE_CONST_STRING(TYPENAME_OBJECT,					"Object");
DECLARE_CONST_STRING(TYPENAME_REAL,						"Real");
DECLARE_CONST_STRING(TYPENAME_SIGNED,					"Signed");
DECLARE_CONST_STRING(TYPENAME_STRING,					"String");
DECLARE_CONST_STRING(TYPENAME_STRUCT,					"Struct");
DECLARE_CONST_STRING(TYPENAME_UINT_32,					"UInt32");
DECLARE_CONST_STRING(TYPENAME_UINT_64,					"UInt64");
DECLARE_CONST_STRING(TYPENAME_UNSIGNED,					"Unsigned");

TArray<CDatum> CAEONTypeSystem::m_CoreTypes;

void CAEONTypeSystem::AddCoreType (IDatatype *pNewDatatype)

//	AddCoreType
//
//	Adds the given core type.

	{
	CDatum dType(new CComplexDatatype(pNewDatatype));
	DWORD dwType = pNewDatatype->GetCoreType();
	if (dwType == IDatatype::UNKNOWN)
		throw CException(errFail);

	if ((int)dwType + 1 > m_CoreTypes.GetCount())
		{
		m_CoreTypes.InsertEmpty(dwType + 1 - m_CoreTypes.GetCount());
		}

	m_CoreTypes[dwType] = dType;
	}

bool CAEONTypeSystem::AddType (CDatum dType)

//	AddType
//
//	Adds a type and returns FALSE if we failed.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		return false;

	const IDatatype &Datatype = dType;
	m_Types.SetAt(Datatype.GetFullyQualifiedName(), dType);
	return true;
	}

CDatum CAEONTypeSystem::CreateDatatypeClass (const CString &sFullyQualifiedName, const CDatatypeList &Implements, IDatatype **retpNewType)

//	CreateDatatypeClass
//
//	Creates a new datatype.

	{
	IDatatype *pNewType = new CDatatypeClass({ sFullyQualifiedName, Implements });
	CDatum dNewType(new CComplexDatatype(pNewType));

	if (retpNewType)
		*retpNewType = pNewType;

	return dNewType;
	}

CDatum CAEONTypeSystem::FindType (const CString &sFullyQualifiedName, const IDatatype **retpDatatype) const

//	FindType
//
//	Looks for the datatype by name. Returns NULL if not found.

	{
	auto pEntry = m_Types.GetAt(sFullyQualifiedName);
	if (!pEntry)
		{
		if (retpDatatype)
			*retpDatatype = NULL;

		return CDatum();
		}

	if (retpDatatype)
		*retpDatatype = &(const IDatatype &)(*pEntry);

	return *pEntry;
	}

CDatum CAEONTypeSystem::GetCoreType (DWORD dwType)

//	GetCoreType
//
//	Returns the given core type.

	{
	//	Initialize, if not already.

	if (m_CoreTypes.GetCount() == 0)
		InitCoreTypes();

	if (dwType == 0 || (int)dwType >= m_CoreTypes.GetCount())
		throw CException(errFail);

	return m_CoreTypes[dwType];
	}

void CAEONTypeSystem::InitCoreTypes ()

//	InitCoreType
//
//	Initializes all core types.

	{
	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ANY), 
			IDatatype::ANY,
			{ },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_BOOL), 
			IDatatype::BOOL,
			{ },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_NULL), 
			IDatatype::NULL_T,
			{ },
			false
			})
		);

	//	Numbers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_NUMBER), 
			IDatatype::NUMBER,
			{ },
			true
			})
		);

	//	Integers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INTEGER), 
			IDatatype::INTEGER,
			{ GetCoreType(IDatatype::NUMBER) },
			true
			})
		);

	//	Signed Integers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_SIGNED), 
			IDatatype::SIGNED,
			{ GetCoreType(IDatatype::INTEGER) },
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_32), 
			IDatatype::INT_32,
			{ GetCoreType(IDatatype::SIGNED) },
			32,
			false,
			false
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_64), 
			IDatatype::INT_64,
			{ GetCoreType(IDatatype::SIGNED) },
			64,
			false,
			false
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_IP), 
			IDatatype::INT_IP,
			{ GetCoreType(IDatatype::SIGNED) },
			0,
			false,
			false
			})
		);

	//	Unsigned Integers

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_UNSIGNED), 
			IDatatype::UNSIGNED,
			{ GetCoreType(IDatatype::INTEGER) },
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_32), 
			IDatatype::UINT_32,
			{ GetCoreType(IDatatype::UNSIGNED) },
			32,
			false,
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_64), 
			IDatatype::UINT_64,
			{ GetCoreType(IDatatype::UNSIGNED) },
			32,
			false,
			true
			})
		);

	//	Reals

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_REAL), 
			IDatatype::REAL,
			{ GetCoreType(IDatatype::NUMBER) },
			true
			})
		);

	AddCoreType(new CDatatypeNumber(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT_64), 
			IDatatype::FLOAT_64,
			{ GetCoreType(IDatatype::REAL) },
			64,
			true,
			false
			})
		);

	//	Other

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_STRING), 
			IDatatype::STRING,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeArray(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY), 
			IDatatype::ARRAY,
			GetCoreType(IDatatype::ANY)
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_STRUCT), 
			IDatatype::STRUCT,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_DATE_TIME), 
			IDatatype::DATE_TIME,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_BINARY), 
			IDatatype::BINARY,
			{  },
			false
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_FUNCTION), 
			IDatatype::FUNCTION,
			{  },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_OBJECT), 
			IDatatype::OBJECT,
			{  },
			true
			})
		);

	AddCoreType(new CDatatypeSimple(
		{
			MakeFullyQualifiedName(NULL_STR, TYPENAME_DATATYPE), 
			IDatatype::DATATYPE,
			{  },
			false
			})
		);
	}

CString CAEONTypeSystem::MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName)

//	MakeFullyQualifiedName
//
//	Returns a fully qualified name from a scope and name. A scope of NULL means
//	global scope.

	{
	return strPattern("%s$%s", sFullyQualifiedScope, sName);
	}

void CAEONTypeSystem::Mark ()

//	Mark
//
//	Mark types in use.

	{
	for (int i = 0; i < m_Types.GetCount(); i++)
		m_Types[i].Mark();
	}

void CAEONTypeSystem::MarkCoreTypes ()

//	MarkCoreTypes
//
//	Marks all core types.

	{
	for (int i = 0; i < m_CoreTypes.GetCount(); i++)
		m_CoreTypes[i].Mark();
	}

CString CAEONTypeSystem::ParseNameFromFullyQualifiedName (const CString &sValue)

//	ParseNameFromFullyQualifiedName
//
//	A fully-qualified name has the form [$SCOPE]$NAME. In some cases $SCOPE is null
//	(because its a global scope). Also, sometimes SCOPE is itself a fully-
//	qualified name.

	{
	//	Find the last '$' sign.

	const char *pPos = sValue.GetParsePointer();
	const char *pEnd = pPos + sValue.GetLength();
	while (pEnd > pPos && *pEnd != '$')
		pEnd--;

	if (*pEnd == '$')
		return CString(pEnd + 1);
	else
		return NULL_STR;
	}
