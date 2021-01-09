//	CGLPropertyType.cpp
//
//	CGLPropertyType Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

CGLPropertyType::CGLPropertyType (IGLType *pParent, const CString &sName, const IGLType &Type, const IASTNode &Node) : IGLType(pParent, sName),
		m_Type(Type),
		m_pDef(const_cast<IASTNode &>(Node).AddRef())

//	CGLPropertyType constructor

	{
	switch (Node.GetType())
		{
		case EASTType::ConstDef:
			m_bConstant = true;
			m_bReadOnly = true;
			break;

		case EASTType::GlobalDef:
			m_bGlobal = true;
			break;

		case EASTType::VarDef:
			break;

		case EASTType::PropertyDef:
			m_bPublic = true;
			break;

		default:
			throw CException(errFail);
		}
	}
