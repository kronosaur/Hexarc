//	CAEONTypes.cpp
//
//	CAEONTypes class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPENAME_ANY,						"Any");
DECLARE_CONST_STRING(TYPENAME_ARRAY,					"Array");
DECLARE_CONST_STRING(TYPENAME_ARRAY_DATE_TIME,			"ArrayOfDateTime");
DECLARE_CONST_STRING(TYPENAME_ARRAY_FLOAT_64,			"ArrayOfFloat64");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_32,				"ArrayOfInt32");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_64,				"ArrayOfInt64");
DECLARE_CONST_STRING(TYPENAME_ARRAY_INT_IP,				"ArrayOfIntIP");
DECLARE_CONST_STRING(TYPENAME_ARRAY_STRING,				"ArrayOfString");
DECLARE_CONST_STRING(TYPENAME_BINARY,					"Binary");
DECLARE_CONST_STRING(TYPENAME_BITMAP_RGBA8,				"BitmapOfRGBA8");
DECLARE_CONST_STRING(TYPENAME_BOOL,						"Bool");
DECLARE_CONST_STRING(TYPENAME_CANVAS,					"Canvas");
DECLARE_CONST_STRING(TYPENAME_DATATYPE,					"Datatype");
DECLARE_CONST_STRING(TYPENAME_DATE_TIME,				"DateTime");
DECLARE_CONST_STRING(TYPENAME_ENUM,						"Enum");
DECLARE_CONST_STRING(TYPENAME_FLOAT,					"Float");
DECLARE_CONST_STRING(TYPENAME_FLOAT_64,					"Float64");
DECLARE_CONST_STRING(TYPENAME_FUNCTION,					"FunctionType");
DECLARE_CONST_STRING(TYPENAME_INT_32,					"Int32");
DECLARE_CONST_STRING(TYPENAME_INT_64,					"Int64");
DECLARE_CONST_STRING(TYPENAME_INT_IP,					"IntIP");
DECLARE_CONST_STRING(TYPENAME_INTEGER,					"Integer");
DECLARE_CONST_STRING(TYPENAME_MATRIX_F64,				"MatrixOfFloat64");
DECLARE_CONST_STRING(TYPENAME_MATRIX_3X3_F64,			"Matrix3X3OfFloat64");
DECLARE_CONST_STRING(TYPENAME_MATRIX_4X4_F64,			"Matrix4X4OfFloat64");
DECLARE_CONST_STRING(TYPENAME_MEMBER_TABLE,				"MemberTable");
DECLARE_CONST_STRING(TYPENAME_NULL,						"NullType");
DECLARE_CONST_STRING(TYPENAME_NUMBER,					"Number");
DECLARE_CONST_STRING(TYPENAME_OBJECT,					"Object");
DECLARE_CONST_STRING(TYPENAME_REAL,						"Real");
DECLARE_CONST_STRING(TYPENAME_SCHEMA_TABLE,				"SchemaTable");
DECLARE_CONST_STRING(TYPENAME_SIGNED,					"Signed");
DECLARE_CONST_STRING(TYPENAME_STRING,					"String");
DECLARE_CONST_STRING(TYPENAME_STRUCT,					"Struct");
DECLARE_CONST_STRING(TYPENAME_TABLE,					"Table");
DECLARE_CONST_STRING(TYPENAME_TEXT_LINES,				"TextLines");
DECLARE_CONST_STRING(TYPENAME_TIME_SPAN,				"TimeSpan");
DECLARE_CONST_STRING(TYPENAME_UINT_32,					"UInt32");
DECLARE_CONST_STRING(TYPENAME_UINT_64,					"UInt64");
DECLARE_CONST_STRING(TYPENAME_UNSIGNED,					"Unsigned");
DECLARE_CONST_STRING(TYPENAME_VECTOR_2D_F64,			"Vector2DOfFloat64");
DECLARE_CONST_STRING(TYPENAME_VECTOR_3D_F64,			"Vector3DOfFloat64");

CCriticalSection CAEONTypes::m_cs;
TArray<CDatum> CAEONTypes::m_Types;
TArray<int> CAEONTypes::m_FreeTypes;
bool CAEONTypes::m_bInitDone = false;
DWORD CAEONTypes::m_dwNextAnonymousID = 1;

DWORD CAEONTypes::Add (CDatum dType)

//	Add
//
//	Add a new type and return an ID.

	{
	if (dType.GetBasicType() != CDatum::typeDatatype)
		throw CException(errFail);

	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	if (m_FreeTypes.GetCount() > 0)
		{
		int iNextFree = m_FreeTypes.GetCount() - 1;
		int iID = m_FreeTypes[iNextFree];
		m_FreeTypes.Delete(iNextFree);

		m_Types[iID] = dType;
		return iID;
		}
	else
		{
		int iID = m_Types.GetCount();
		m_Types.Insert(dType);
		return iID;
		}
	}

CDatum CAEONTypes::CreateAny ()

//	CreateAny
//
//	Creates the Any datatype.

	{
	return CDatum(new CComplexDatatype(new CDatatypeAny(MakeFullyQualifiedName(NULL_STR, TYPENAME_ANY))));
	}

CDatum CAEONTypes::CreateArray (const CString& sFullyQualifiedName, CDatum dElementType, DWORD dwCoreType)

//	CreateArray
//
//	Creates an array type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeArray(
		{
			sFullyQualifiedName, 
			dwCoreType,
			dElementType
			})
		));
	}

CDatum CAEONTypes::CreateMatrix (const CString& sFullyQualifiedName, CDatum dElementType, int iRows, int iCols, DWORD dwCoreType)

//	CreateMatrix
//
//	Creates a matrix type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeMatrix(
		{
			sFullyQualifiedName, 
			dwCoreType,
			dElementType,
			iRows,
			iCols
			})
		));
	}

CDatum CAEONTypes::CreateMemberTable ()

//	CreateMemberTable
//
//	Creates the datatype for the member table.

	{
	//	LATER: Move code here.
	return CDatum(new CComplexDatatype(CAEONTypeSystem::CreateMemberTable()));
	}

CDatum CAEONTypes::CreateNumber (const CString& sFullyQualifiedName, const CDatatypeList& Implements, int iBits, bool bFloat, bool bUnsigned, DWORD dwCoreType)

//	CreateNumber
//
//	Creates a number type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeNumber(
		{
			sFullyQualifiedName, 
			dwCoreType,
			Implements,
			iBits,
			bFloat,
			bUnsigned
			})
		));
	}

CDatum CAEONTypes::CreateSchemaTable ()

//	CreateSchemaTable
//
//	Creates the datatype for a schema table.

	{
	//	LATER: Move code here.
	return CDatum(new CComplexDatatype(CAEONTypeSystem::CreateSchemaTable()));
	}

CDatum CAEONTypes::CreateSimple (const CString& sFullyQualifiedName, const CDatatypeList& Implements, bool bAbstract, DWORD dwCoreType)

//	CreateSimple
//
//	Create a simple type.

	{
	return CDatum(new CComplexDatatype(new CDatatypeSimple(
		{
			sFullyQualifiedName, 
			dwCoreType,
			Implements,
			bAbstract
			})
		));
	}

CDatum CAEONTypes::Get (DWORD dwID)

//	Get
//
//	Return a type from an ID.

	{
	CSmartLock Lock(m_cs);

	if (!m_bInitDone)
		InitCoreTypes();

	if (dwID <= 0 || dwID >= (DWORD)m_Types.GetCount())
		throw CException(errFail);

	return m_Types[dwID];
	}

void CAEONTypes::InitCoreTypes ()

//	InitCoreTypes
//
//	Registers all core types.

	{
#ifdef DEBUG
	if (m_Types.GetCount() > 0 || m_bInitDone)
		throw CException(errFail);
#endif

	m_bInitDone = true;

	//	+1 because the count is always one more than the max index.

	m_Types.InsertEmpty(IDatatype::MAX_CORE_TYPE + 1);

	//	Add all core types.

	SetCoreType(IDatatype::ANY, CreateAny());
	SetCoreType(IDatatype::DATATYPE, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_DATATYPE), { }, false, IDatatype::DATATYPE));
	SetCoreType(IDatatype::BOOL, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_BOOL), { }, true, IDatatype::BOOL));
	SetCoreType(IDatatype::NULL_T, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_NULL), { }, false, IDatatype::NULL_T));
	SetCoreType(IDatatype::NUMBER, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_NUMBER), { }, true, IDatatype::NUMBER));
	SetCoreType(IDatatype::REAL, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_REAL), { Get(IDatatype::NUMBER) }, true, IDatatype::REAL));
	SetCoreType(IDatatype::INTEGER, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_INTEGER), { Get(IDatatype::NUMBER) }, true, IDatatype::INTEGER));
	SetCoreType(IDatatype::FLOAT, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT), { Get(IDatatype::REAL) }, true, IDatatype::FLOAT));
	SetCoreType(IDatatype::SIGNED, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_SIGNED), { Get(IDatatype::INTEGER) }, true, IDatatype::SIGNED));
	SetCoreType(IDatatype::UNSIGNED, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_UNSIGNED), { Get(IDatatype::INTEGER) }, true, IDatatype::UNSIGNED));
	SetCoreType(IDatatype::INT_32, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_32), { Get(IDatatype::SIGNED) }, 32, false, false, IDatatype::INT_32));
	SetCoreType(IDatatype::INT_64, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_64), { Get(IDatatype::SIGNED) }, 64, false, false, IDatatype::INT_64));
	SetCoreType(IDatatype::INT_IP, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_INT_IP), { Get(IDatatype::SIGNED) }, 0, false, false, IDatatype::INT_IP));
	SetCoreType(IDatatype::UINT_32, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_32), { Get(IDatatype::UNSIGNED) }, 32, false, true, IDatatype::UINT_32));
	SetCoreType(IDatatype::UINT_64, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_UINT_64), { Get(IDatatype::UNSIGNED) }, 64, false, true, IDatatype::UINT_64));
	SetCoreType(IDatatype::FLOAT_64, CreateNumber(MakeFullyQualifiedName(NULL_STR, TYPENAME_FLOAT_64), { Get(IDatatype::FLOAT) }, 64, true, false, IDatatype::FLOAT_64));
	SetCoreType(IDatatype::STRING, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_STRING), { }, false, IDatatype::STRING));
	SetCoreType(IDatatype::ARRAY, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY), Get(IDatatype::ANY), IDatatype::ARRAY));
	SetCoreType(IDatatype::STRUCT, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_STRUCT), { }, false, IDatatype::STRUCT));
	SetCoreType(IDatatype::DATE_TIME, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_DATE_TIME), { }, false, IDatatype::DATE_TIME));
	SetCoreType(IDatatype::TIME_SPAN, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_TIME_SPAN), { }, false, IDatatype::TIME_SPAN));
	SetCoreType(IDatatype::BINARY, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_BINARY), { }, false, IDatatype::BINARY));
	SetCoreType(IDatatype::FUNCTION, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_FUNCTION), { }, true, IDatatype::FUNCTION));
	SetCoreType(IDatatype::OBJECT, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_OBJECT), { }, true, IDatatype::OBJECT));
	SetCoreType(IDatatype::TABLE, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_TABLE), { }, true, IDatatype::TABLE));
	SetCoreType(IDatatype::ARRAY_INT_32, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_32), Get(IDatatype::INT_32), IDatatype::ARRAY_INT_32));
	SetCoreType(IDatatype::ARRAY_FLOAT_64, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_FLOAT_64), Get(IDatatype::FLOAT_64), IDatatype::ARRAY_FLOAT_64));
	SetCoreType(IDatatype::ARRAY_STRING, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_STRING), Get(IDatatype::STRING), IDatatype::ARRAY_STRING));
	SetCoreType(IDatatype::ARRAY_DATE_TIME, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_DATE_TIME), Get(IDatatype::DATE_TIME), IDatatype::ARRAY_DATE_TIME));
	SetCoreType(IDatatype::ARRAY_INT_64, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_64), Get(IDatatype::INT_64), IDatatype::ARRAY_INT_64));
	SetCoreType(IDatatype::ARRAY_INT_IP, CreateArray(MakeFullyQualifiedName(NULL_STR, TYPENAME_ARRAY_INT_IP), Get(IDatatype::INT_IP), IDatatype::ARRAY_INT_IP));
	SetCoreType(IDatatype::SCHEMA_TABLE, CreateSchemaTable());
	SetCoreType(IDatatype::VECTOR_2D_F64, CreateMatrix(MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_2D_F64), Get(IDatatype::FLOAT_64), 1, 2, IDatatype::VECTOR_2D_F64));
	SetCoreType(IDatatype::VECTOR_3D_F64, CreateMatrix(MakeFullyQualifiedName(NULL_STR, TYPENAME_VECTOR_3D_F64), Get(IDatatype::FLOAT_64), 1, 3, IDatatype::VECTOR_3D_F64));
	SetCoreType(IDatatype::MATRIX_F64, CreateMatrix(MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_F64), Get(IDatatype::FLOAT_64), 0, 0, IDatatype::MATRIX_F64));
	SetCoreType(IDatatype::MATRIX_3X3_F64, CreateMatrix(MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_3X3_F64), Get(IDatatype::FLOAT_64), 3, 3, IDatatype::MATRIX_3X3_F64));
	SetCoreType(IDatatype::MATRIX_4X4_F64, CreateMatrix(MakeFullyQualifiedName(NULL_STR, TYPENAME_MATRIX_4X4_F64), Get(IDatatype::FLOAT_64), 4, 4, IDatatype::MATRIX_4X4_F64));
	SetCoreType(IDatatype::CANVAS, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_CANVAS), { }, false, IDatatype::CANVAS));
	SetCoreType(IDatatype::BITMAP_RGBA8, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_BITMAP_RGBA8), { }, false, IDatatype::BITMAP_RGBA8));
	SetCoreType(IDatatype::ENUM, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_ENUM), { }, true, IDatatype::ENUM));
	SetCoreType(IDatatype::MEMBER_TABLE, CreateMemberTable());
	SetCoreType(IDatatype::TEXT_LINES, CreateSimple(MakeFullyQualifiedName(NULL_STR, TYPENAME_TEXT_LINES), { }, false, IDatatype::TEXT_LINES));

#ifdef DEBUG
	if (!m_Types[IDatatype::UNKNOWN].IsIdenticalToNil())
		throw CException(errFail);

	for (int i = 1; i <= IDatatype::MAX_CORE_TYPE; i++)
		if (m_Types[i].GetBasicType() != CDatum::typeDatatype)
			throw CException(errFail);
#endif
	}

CString CAEONTypes::MakeAnonymousName (const CString& sType)

//	MakeAnonymousName
//
//	Returns a new name.

	{
	CSmartLock Lock(m_cs);
	DWORD dwID = m_dwNextAnonymousID++;
	Lock.Unlock();

	return strPattern("Anonymous%s%08x", sType, dwID);
	}

CString CAEONTypes::MakeAnonymousName (const CString& sFullyQualifiedScope, const CString& sType)

//	MakeAnonymousName
//
//	Returns a fully qualified anonymous type name.

	{
	CSmartLock Lock(m_cs);
	DWORD dwID = m_dwNextAnonymousID++;
	Lock.Unlock();

	return strPattern("%s$Anonymous%s%08x", sFullyQualifiedScope, sType, dwID);
	}

CString CAEONTypes::MakeFullyQualifiedName (const CString &sFullyQualifiedScope, const CString &sName)

//	MakeFullyQualifiedName
//
//	Returns a fully qualified name from a scope and name. A scope of NULL means
//	global scope.

	{
	return strPattern("%s$%s", sFullyQualifiedScope, sName);
	}

void CAEONTypes::MarkAndSweep ()

//	MarkAndSweep
//
//	Remove any types that are no longer being used. We assume this is called 
//	after all datums are marked but before sweep.

	{
	if (m_Types.GetCount() == 0)
		return;

	//	No need to lock because garbage collection is always single-threaded.

	//	Mark all core types.
	//	NOTE: We start at 1 because 0 is reserved for IDatatype::UNKNOWN, which
	//	is always nil.

	for (int i = 1; i <= IDatatype::MAX_CORE_TYPE; i++)
		m_Types[i].Mark();

	//	For all other types, if not marked, then we can discard them.

	for (int i = IDatatype::MAX_CORE_TYPE + 1; i < m_Types.GetCount(); i++)
		{
		if (!m_Types[i].IsIdenticalToNil())
			{
			auto pComplex = m_Types[i].GetComplex();
			if (pComplex && !pComplex->IsMarked())
				{
				m_Types[i] = CDatum();
				m_FreeTypes.Insert(i);
				}
			}
		}
	}

CString CAEONTypes::ParseNameFromFullyQualifiedName (const CString &sValue)

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

void CAEONTypes::SetCoreType (DWORD dwCoreType, CDatum dType)

//	SetCoreType
//
//	Sets the appropriate core type.

	{
#ifdef DEBUG
	if (dwCoreType <= 0 || dwCoreType >= (DWORD)m_Types.GetCount())
		throw CException(errFail);

	if (!m_Types[dwCoreType].IsIdenticalToNil())
		throw CException(errFail);

	const IDatatype& Datatype = dType;
	if (Datatype.GetCoreType() != dwCoreType)
		throw CException(errFail);
#endif

	m_Types[dwCoreType] = dType;
	}
