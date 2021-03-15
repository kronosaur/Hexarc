//	CGridLangTypeLoader.cpp
//
//	CGridLangTypeLoaded Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(SCOPE_ROOT,					":");
DECLARE_CONST_STRING(SCOPE_SEPARATOR,				":");

DECLARE_CONST_STRING(ERR_CANNOT_CREATE,				"Unable to create type.");

bool CGridLangTypeLoader::Load (CString *retsError)

//	Load
//
//	Processes the AST and declares types.

	{
	//	First initialize the type system with all default/core types.

	if (!m_Types.Init(retsError))
		return false;

	//	Declare all types. This will identify all type definitions in the AST
	//	and will:
	//
	//	1.	Add the new type to the type system
	//	2.	Add a pointer from the AST to the new type.

	if (!DeclareTypes(m_AST.GetRoot(), SCOPE_ROOT, retsError))
		return false;

#if 0
	for (int i = 0; i < AST.GetDefinitionCount(); i++)
		{
		auto &Def = AST.GetDefinition(i);

		//	If this type has already been defined, then we're done. This can
		//	happen when a type relies on another type and we need to recurse.

		if (IsDeclared(Def.GetName()))
			continue;

		//	Create the type as appropriate

		if (!DeclareType(Ctx, pScope, AST, Def, retsError))
			return false;
		}
#endif

	return true;
	}

bool CGridLangTypeLoader::DeclareClass (IASTNode &Node, const CString &sScopePrefix, CString *retsError)

//	DeclareClass
//
//	Declare a class type.

	{
	CString sCanonical = CreateSymbol(sScopePrefix, Node.GetName());
	TSharedPtr<IGLType> pNewType = IGLType::CreateObject(Node.GetName(), sCanonical);
	if (!pNewType)
		{
		if (retsError) *retsError = ERR_CANNOT_CREATE;
		return false;
		}

	//	Add to types and connect to AST node.

	m_Types.Insert(pNewType);
	Node.SetTypeDef(*pNewType);

	//	Now we need to recurse and define any types inside the class.

	CString sNewScopePrefix = strPattern("%s%s:", sScopePrefix, Node.GetName());
	for (int i = 0; i < Node.GetChildCount(); i++)
		{
		IASTNode &ChildNode = Node.GetChild(i);
		if (!DeclareTypes(ChildNode, sNewScopePrefix, retsError))
			return false;
		}

	//	Done

	return true;
	}

bool CGridLangTypeLoader::DeclareOrdinal (IASTNode &Node, const CString &sScopePrefix, CString *retsError)

//	DeclareOrdinal
//
//	Declares an ordinal definition.

	{
	//	LATER: ...
	return true;
	}

bool CGridLangTypeLoader::DeclareTypes (IASTNode &Node, const CString &sScopePrefix, CString *retsError)

//	DeclareTypes
//
//	Iterates over all elements of the Node and declares any types defined in 
//	its scope.

	{
	switch (Node.GetType())
		{
		case EASTType::ClassDef:
			return DeclareClass(Node, sScopePrefix, retsError);

		case EASTType::OrdinalDef:
			return DeclareOrdinal(Node, sScopePrefix, retsError);

		default:
			return true;
		}
	}

