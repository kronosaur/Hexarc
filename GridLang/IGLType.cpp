//	IGLType.cpp
//
//	IGLType Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPENAME_TYPE,						"Type");

TSharedPtr<IGLType> IGLType::CreateAbstract (const IGLType &IsA, const IGLType *pScope, const CString &sName)

//	CreateAbstract
//
//	Creates an abstract type.

	{
	CGLSimpleType *pType = new CGLSimpleType(&IsA, pScope, sName);
	pType->SetConcrete(false);

	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateConcrete (const IGLType &IsA, const IGLType *pScope, const CString &sName)

//	CreateConcrete
//
//	Creates a simple concrete type.

	{
	CGLSimpleType *pType = new CGLSimpleType(&IsA, pScope, sName);
	pType->SetConcrete(true);

	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateConcreteNumber (const IGLType &IsA, const IGLType *pScope, const CString &sName, GLCoreType iType)

//	CreateConcreteNumber
//
//	Creates a concrete number type.

	{
	CGLNumberType *pType = new CGLNumberType(&IsA, pScope, sName, iType);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateFunction (const IGLType &IsA, const IGLType *pScope, const CString &sName)

//	CreateFunction
//
//	Creates a function definition.

	{
	CGLFunctionType *pType = new CGLFunctionType(&IsA, pScope, sName);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateObject (const CString &sName, const CString &sCanonical)

//	CreateObject
//
//	Creates an object definition.

	{
	CGLObjectType *pType = new CGLObjectType(sName, sCanonical);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateOrdinals (const IGLType &IsA, const IGLType *pScope, const CString &sName, const TArray<CString> &Ordinals)

//	CreateOrdinals
//
//	Creates an ordinals definition.

	{
	CGLOrdinalType *pType = new CGLOrdinalType(&IsA, pScope, sName, Ordinals);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateProperty (const IGLType &IsA, const IGLType *pScope, const IGLType &Type, const CString &sName, const IASTNode &Def)

//	CreateProperty
//
//	Creates a property definition.

	{
	CGLPropertyType *pType = new CGLPropertyType(&IsA, pScope, sName, Type, Def);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateRoot ()

//	CreateRoot
//
//	Creates the root type (the only type with no parent).

	{
	CGLSimpleType *pType = new CGLSimpleType(NULL, NULL, TYPENAME_TYPE);
	pType->SetConcrete(false);

	return TSharedPtr<IGLType>(pType);
	}

bool IGLType::Declare (CGLNamespaceCtx &Ctx, const IASTNode &Def, CString *retsError)

//	Declare
//
//	Give the type a chance to declare any symbols in its scope.

	{
	if (!OnDeclare(Ctx, Def, retsError))
		return false;

	//	Generate our global symbol, if necessary

	if (m_sSymbol.IsEmpty())
		GenerateSymbol();

	return true;
	}

bool IGLType::Define (CGLNamespaceCtx &Ctx, CString *retsError)

//	Define
//
//	Finish the definition based on the stored AST

	{
	if (m_bDefined)
		return true;

	if (!OnDefine(Ctx, retsError))
		return false;

	m_bDefined = true;
	return true;
	}

bool IGLType::IsA (const IGLType &Type) const

//	IsA
//
//	Returns TRUE if we are the given Type or a descendant of the given Type.

	{
	return (&Type == this) || (m_pParent && m_pParent->IsA(Type));
	}

void IGLType::GenerateSymbol ()

//	GenerateSymbol
//
//	Initializes m_sSymbol.

	{
	if (m_pScope)
		m_sSymbol = strPattern("%s:%s", m_pScope->GetSymbol(), OnGenerateLocalSymbol());
	else
		m_sSymbol = OnGenerateLocalSymbol();
	}
