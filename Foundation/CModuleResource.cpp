//	CModuleResource.cpp
//
//	CModuleResource class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CModuleResource::Close (void)

//	Close
//
//	Closes the resource

	{
	if (m_hRes)
		{
		::FreeResource(m_hRes);
		m_hRes = NULL;
		}
	}

bool CModuleResource::Open (const CString &sType, const CString &sName)

//	Open
//
//	Opens the given resource

	{
	ASSERT(m_hRes == NULL);

	HRSRC hFind = ::FindResource(NULL, CString16(sName), CString16(sType));
	if (hFind == NULL)
		return false;

	m_iLength = ::SizeofResource(NULL, hFind);
	if (m_iLength == 0)
		return false;

	m_hRes = ::LoadResource(NULL, hFind);
	if (m_hRes == NULL)
		return false;

	//	Done

	return true;
	}
