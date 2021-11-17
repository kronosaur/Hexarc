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
			false
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

void CAEONTypeSystem::MarkCoreTypes ()

//	MarkCoreTypes
//
//	Marks all core types.

	{
	for (int i = 0; i < m_CoreTypes.GetCount(); i++)
		m_CoreTypes[i].Mark();
	}

