//	IGLType.cpp
//
//	IGLType Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(TYPENAME_TYPE,						"Type");

TSharedPtr<IGLType> IGLType::CreateAbstract (IGLType &Parent, const CString &sName)

//	CreateAbstract
//
//	Creates an abstract type.

	{
	CGLSimpleType *pType = new CGLSimpleType(&Parent, sName);
	pType->SetConcrete(false);

	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateConcrete (IGLType &Parent, const CString &sName)

//	CreateConcrete
//
//	Creates a simple concrete type.

	{
	CGLSimpleType *pType = new CGLSimpleType(&Parent, sName);
	pType->SetConcrete(true);

	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateConcreteNumber (IGLType &Parent, const CString &sName, GLCoreType iType)

//	CreateConcreteNumber
//
//	Creates a concrete number type.

	{
	CGLNumberType *pType = new CGLNumberType(&Parent, sName, iType);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateObject (IGLType &Parent, const CString &sName, const IASTNode &Def)

//	CreateObject
//
//	Creates an object definition.

	{
	CGLObjectType *pType = new CGLObjectType(&Parent, sName, Def);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateOrdinals (IGLType &Parent, const CString &sName, const TArray<CString> &Ordinals)

//	CreateOrdinals
//
//	Creates an ordinals definition.

	{
	CGLOrdinalType *pType = new CGLOrdinalType(&Parent, sName, Ordinals);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateProperty (IGLType &PropertyType, IGLType &Type, const CString &sName, const IASTNode &Def)

//	CreateProperty
//
//	Creates a property definition.

	{
	CGLPropertyType *pType = new CGLPropertyType(&PropertyType, sName, Type, Def);
	return TSharedPtr<IGLType>(pType);
	}

TSharedPtr<IGLType> IGLType::CreateRoot ()

//	CreateRoot
//
//	Creates the root type (the only type with no parent).

	{
	CGLSimpleType *pType = new CGLSimpleType(NULL, TYPENAME_TYPE);
	pType->SetConcrete(false);

	return TSharedPtr<IGLType>(pType);
	}

bool IGLType::IsA (const IGLType &Type) const

//	IsA
//
//	Returns TRUE if we are the given Type or a descendant of the given Type.

	{
	return (&Type == this) || (m_pParent && m_pParent->IsA(Type));
	}
