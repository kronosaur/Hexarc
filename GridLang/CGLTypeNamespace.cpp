//	CGLTypeNamespace.cpp
//
//	CGLTypeNamespace Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(ERR_UNDECLARED_IDENTIFIER,			"Undeclared identifier: %s");
DECLARE_CONST_STRING(ERR_NOT_AN_OBJECT,					"%s is not an Object. Cannot inherit from this type.");
DECLARE_CONST_STRING(ERR_INVALID_TYPE_REF,				"Invalid type reference.");

bool CGLTypeNamespace::DeclareClass (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError)

//	DeclareClass
//
//	Declares an object class.

	{
	//	Get the parent class

	const CString &sBaseName = Def.GetBaseName();
	CString sBaseID = strToLower(sBaseName);

	//	First look for the base class in our AST (essentially, our local scope).

	if (!DeclareIfNeeded(Ctx, AST, sBaseID, retsError))
		return false;

	//	Look up the base type

	IGLType *pParentObj = Ctx.Find(sBaseName);
	if (!pParentObj)
		return Def.ComposeError(strPattern(ERR_UNDECLARED_IDENTIFIER, sBaseName));

	//	Parent must be an object

	if (!pParentObj->IsA(Ctx.GetCoreType(GLCoreType::Object)))
		return Def.ComposeError(strPattern(ERR_NOT_AN_OBJECT, sBaseName));

	//	Now create the object

	TSharedPtr<IGLType> pType = IGLType::CreateObject(*pParentObj, Def.GetName(), Def);
	if (!Insert(pType))
		//	Should never fail because we've already check that this is
		//	not a duplicate.
		throw CException(errFail);

	//	Success!

	return true;
	}

bool CGLTypeNamespace::DeclareIfNeeded (CGLNamespaceCtx &Ctx, const IASTNode &AST, const CString &sTypeID, CString *retsError)

//	DeclareIfNeeded
//
//	If sTypeID is defined in the AST, make sure it is declared.

	{
	//	First look for the type in our AST (essentially, our local scope).

	const IASTNode *pASTDef = AST.FindDefinition(sTypeID);

	//	If the type is defined in the AST but we haven't yet declared it, then 
	//	we need to recurse.

	if (pASTDef && !m_Types.GetAt(sTypeID))
		{
		if (!DeclareType(Ctx, AST, *pASTDef, retsError))
			return false;
		}

	//	Done!

	return true;
	}

bool CGLTypeNamespace::DeclareOrdinal (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError)

//	DeclareOrdinal
//
//	Declares an ordinal type.

	{
	TArray<CString> Ordinals;
	Ordinals.InsertEmpty(Def.GetDefinitionCount());
	for (int i = 0; i < Def.GetDefinitionCount(); i++)
		Ordinals[i] = Def.GetDefinitionString(i);

	//	Create the property type.

	TSharedPtr<IGLType> pType = IGLType::CreateOrdinals(Ctx.GetCoreType(GLCoreType::Ordinal), Def.GetName(), Ordinals);
	if (!Insert(pType))
		//	Should never fail because we've already check that this is
		//	not a duplicate.
		throw CException(errFail);

	//	Success!

	return true;
	}

bool CGLTypeNamespace::DeclareProperty (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError)

//	DeclareProperty
//
//	Declares a property.

	{
	//	Get the property type.

	IGLType *pTypeRef;
	if (!InterpretTypeRef(Ctx, AST, Def.GetTypeRef(), pTypeRef, retsError) || !pTypeRef)
		return false;

	//	Create the property type.

	TSharedPtr<IGLType> pType = IGLType::CreateProperty(Ctx.GetCoreType(GLCoreType::Property), *pTypeRef, Def.GetName(), Def);
	if (!Insert(pType))
		//	Should never fail because we've already check that this is
		//	not a duplicate.
		throw CException(errFail);

	//	Success!

	return true;
	}

bool CGLTypeNamespace::DeclareType (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &Def, CString *retsError)

//	DeclareType
//
//	Declares the given type. This function may recurse if we need to declare

	{
	switch (Def.GetType())
		{
		case EASTType::ClassDef:
			if (!DeclareClass(Ctx, AST, Def, retsError))
				return false;
			break;

#if 0
		case EASTType::FunctionDef:
			if (!DeclareFunction(Ctx, AST, Def, retsError))
				return false;
			break;
#endif

		case EASTType::ConstDef:
		case EASTType::GlobalDef:
		case EASTType::PropertyDef:
		case EASTType::VarDef:
			if (!DeclareProperty(Ctx, AST, Def, retsError))
				return false;
			break;

		case EASTType::OrdinalDef:
			if (!DeclareOrdinal(Ctx, AST, Def, retsError))
				return false;
			break;

		default:
			throw CException(errFail);
		}

	return true;
	}

bool CGLTypeNamespace::DeclareTypes (CGLNamespaceCtx &Ctx, const IASTNode &AST, CString *retsError)

//	DeclareTypes
//
//	Looks over all definitions in the given AST and declares them as appropriate
//	in the namespace.

	{
	for (int i = 0; i < AST.GetDefinitionCount(); i++)
		{
		auto &Def = AST.GetDefinition(i);

		//	If this type has already been defined, then we're done. This can
		//	happen when a type relies on another type and we need to recurse.

		if (IsDeclared(Def.GetName()))
			continue;

		//	Create the type as appropriate

		if (!DeclareType(Ctx, AST, Def, retsError))
			return false;
		}

	return true;
	}

void CGLTypeNamespace::Dump () const

//	Dump
//
//	Dump the types

	{
	CGLTypeTree Tree;
	for (int i = 0; i < m_Types.GetCount(); i++)
		Tree.AddType(*m_Types[i]);

	Tree.Dump();
	}

bool CGLTypeNamespace::InterpretTypeRef (CGLNamespaceCtx &Ctx, const IASTNode &AST, const IASTNode &TypeRef, IGLType *&retpType, CString *retsError)

//	InterpretTypeRef
//
//	Returns the type that corresponds to TypeRef. If we haven't yet declared the
//	type, we do it. If we cannot interpret the reference, we return NULL.

	{
	if (TypeRef.GetType() == EASTType::TypeRef)
		{
		CString sTypeID = strToLower(TypeRef.GetName());

		//	Make sure the type is declared

		if (!DeclareIfNeeded(Ctx, AST, sTypeID, retsError))
			return false;

		//	Now find the type.

		retpType = Ctx.Find(sTypeID);
		if (!retpType)
			return TypeRef.ComposeError(strPattern(ERR_UNDECLARED_IDENTIFIER, TypeRef.GetName()));

		return true;
		}
	else
		return TypeRef.ComposeError(ERR_INVALID_TYPE_REF, retsError);
	}

IGLType *CGLTypeNamespace::GetAt (const CString &sID) const

//	GetAt
//
//	Returns a type by canonical (lowercase) ID. If not found, we return NULL.

	{
	auto pType = m_Types.GetAt(sID);
	if (!pType)
		return NULL;

	return *pType;
	}

bool CGLTypeNamespace::Insert (TSharedPtr<IGLType> pType)

//	Insert
//
//	Adds the given type to the namespace. If a type of the same name already
//	exists, we return FALSE.

	{
	bool bInserted;
	auto pSlot = m_Types.SetAt(::strToLower(pType->GetName()), &bInserted);
	if (!bInserted)
		return false;

	*pSlot = pType;
	return true;
	}
