//	CGLObjectType.cpp
//
//	CGLObjectType Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

bool CGLObjectType::OnDeclare (CGLNamespaceCtx &Ctx, const IASTNode &Def, CString *retsError)

//	OnDeclare
//
//	Declare any symbols inside our scope.

	{
	CGLPushNamespace SmartScope(Ctx, m_Types);

	//	Generate our symbol now, because the types we declare will need to use
	//	it.

	GenerateSymbol();

	//	Declare types

	if (!m_Types.DeclareTypes(Ctx, this, Def, retsError))
		return false;

	return true;
	}

bool CGLObjectType::OnDefine (CGLNamespaceCtx &Ctx, CString *retsError)

//	OnDefine
//
//	Defines the object type.

	{
	return true;
	}
