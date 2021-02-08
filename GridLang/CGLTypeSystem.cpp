//	CGLTypeSystem.cpp
//
//	CGLTypeSystem Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPENAME_BOOL,						"Bool");
DECLARE_CONST_STRING(TYPENAME_FLOAT,					"Float");
DECLARE_CONST_STRING(TYPENAME_FLOAT_64,					"Float64");
DECLARE_CONST_STRING(TYPENAME_FUNCTION,					"Function");
DECLARE_CONST_STRING(TYPENAME_INTEGER,					"Integer");
DECLARE_CONST_STRING(TYPENAME_INT_32,					"Int32");
DECLARE_CONST_STRING(TYPENAME_INT_64,					"Int64");
DECLARE_CONST_STRING(TYPENAME_INT_IP,					"IntIP");
DECLARE_CONST_STRING(TYPENAME_NULL_TYPE,				"NullType");
DECLARE_CONST_STRING(TYPENAME_NUMBER,					"Number");
DECLARE_CONST_STRING(TYPENAME_OBJECT,					"Object");
DECLARE_CONST_STRING(TYPENAME_ORDINAL,					"Ordinal");
DECLARE_CONST_STRING(TYPENAME_PROPERTY,					"Property");
DECLARE_CONST_STRING(TYPENAME_REAL,						"Real");
DECLARE_CONST_STRING(TYPENAME_SIGNED,					"Signed");
DECLARE_CONST_STRING(TYPENAME_STRING,					"string");
DECLARE_CONST_STRING(TYPENAME_TYPE,						"Type");
DECLARE_CONST_STRING(TYPENAME_UNSIGNED,					"Unsigned");
DECLARE_CONST_STRING(TYPENAME_UINT_32,					"UInt32");
DECLARE_CONST_STRING(TYPENAME_UINT_64,					"UInt64");

void CGLTypeSystem::AddCoreTypes ()

//	AddCoreTypes
//
//	Adds all core types.

	{
	//	Type

	m_Core[(int)GLCoreType::Type] = InsertOrThrow(IGLType::CreateRoot());

	//	Null

	m_Core[(int)GLCoreType::NullType] = InsertOrThrow(IGLType::CreateConcrete(GetCoreType(GLCoreType::Type), NULL, TYPENAME_NULL_TYPE));

	//	Bool

	m_Core[(int)GLCoreType::Bool] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Type), NULL, TYPENAME_BOOL));

	//	Numbers

	m_Core[(int)GLCoreType::Number] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Type), NULL, TYPENAME_NUMBER));
	m_Core[(int)GLCoreType::Real] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Number), NULL, TYPENAME_REAL));

	m_Core[(int)GLCoreType::Float] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Real), NULL, TYPENAME_FLOAT));
	m_Core[(int)GLCoreType::Float64] = InsertOrThrow(IGLType::CreateConcreteNumber(GetCoreType(GLCoreType::Float), NULL, TYPENAME_FLOAT_64, GLCoreType::Float64));

	m_Core[(int)GLCoreType::Integer] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Real), NULL, TYPENAME_INTEGER));

	m_Core[(int)GLCoreType::Signed] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Integer), NULL, TYPENAME_SIGNED));
	m_Core[(int)GLCoreType::Int32] = InsertOrThrow(IGLType::CreateConcreteNumber(GetCoreType(GLCoreType::Signed), NULL, TYPENAME_INT_32, GLCoreType::Int32));
	m_Core[(int)GLCoreType::Int64] = InsertOrThrow(IGLType::CreateConcreteNumber(GetCoreType(GLCoreType::Signed), NULL, TYPENAME_INT_64, GLCoreType::Int64));
	m_Core[(int)GLCoreType::IntIP] = InsertOrThrow(IGLType::CreateConcreteNumber(GetCoreType(GLCoreType::Signed), NULL, TYPENAME_INT_IP, GLCoreType::IntIP));

	m_Core[(int)GLCoreType::Unsigned] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Integer), NULL, TYPENAME_UNSIGNED));
	m_Core[(int)GLCoreType::UInt32] = InsertOrThrow(IGLType::CreateConcreteNumber(GetCoreType(GLCoreType::Unsigned), NULL, TYPENAME_UINT_32, GLCoreType::UInt32));
	m_Core[(int)GLCoreType::UInt64] = InsertOrThrow(IGLType::CreateConcreteNumber(GetCoreType(GLCoreType::Unsigned), NULL, TYPENAME_UINT_64, GLCoreType::UInt64));

	m_Core[(int)GLCoreType::Ordinal] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Unsigned), NULL, TYPENAME_ORDINAL));

	//	Other Core Types

	m_Core[(int)GLCoreType::String] = InsertOrThrow(IGLType::CreateConcrete(GetCoreType(GLCoreType::Type), NULL, TYPENAME_STRING));
	m_Core[(int)GLCoreType::Object] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Type), NULL, TYPENAME_OBJECT));
	m_Core[(int)GLCoreType::Property] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Type), NULL, TYPENAME_PROPERTY));
	m_Core[(int)GLCoreType::Function] = InsertOrThrow(IGLType::CreateAbstract(GetCoreType(GLCoreType::Type), NULL, TYPENAME_FUNCTION));
	}

bool CGLTypeSystem::InitFromAST (const CGridLangAST &AST, CString *retsError)

//	InitFromAST
//
//	Initialize from an AST. This will load all custom types. We return TRUE if
//	we succeed.

	{
	m_Types.DeletAll();

	AddCoreTypes();

	//	Define all core library functions

	CGridLangCoreLibrary::Define(GetCoreType(GLCoreType::Function), m_Types);

	//	Load the AST

	if (AST.IsEmpty())
		return true;

	//	Declare all types (not yet defined, however).

	CGLNamespaceCtx Ctx(*this);
	if (!m_Types.DeclareTypes(Ctx, NULL, AST.GetRoot(), retsError))
		return false;

	//	Now finish defining all the types.

	if (!m_Types.DefineTypes(Ctx, retsError))
		return false;

	//	Success!

	return true;
	}

bool CGLTypeSystem::IsDefined (const CString &sName) const

//	IsDefined
//
//	Returns TRUE if the given type is defined.

	{
	auto pType = m_Types.GetAt(strToLower(sName));
	return pType;
	}
