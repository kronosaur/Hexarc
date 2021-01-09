//	CGLNamespaceCtx.cpp
//
//	CGLNamespaceCtx Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

CGLNamespaceCtx::CGLNamespaceCtx (CGLTypeSystem &Types) :
		m_Types(Types)

//	CGLNamespaceCtx constructor

	{
	Push(m_Types.GetGlobals());
	}

IGLType *CGLNamespaceCtx::Find (const CString &sName) const

//	Find
//
//	Looks for the identifier by name. Returns NULL if it could not be found.

	{
	CString sID = strToLower(sName);

	for (int i = m_Scopes.GetCount() - 1; i >= 0; i--)
		{
		IGLType *pType = m_Scopes[i]->GetAt(sID);
		if (pType)
			return pType;
		}

	return NULL;
	}

