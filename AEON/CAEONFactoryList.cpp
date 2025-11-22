//	CAEONFactoryList.cpp
//
//	CAEONFactoryList class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CAEONFactoryList::~CAEONFactoryList (void)

//	CAEONFactoryList destructor

	{
	}

bool CAEONFactoryList::FindFactory (const CString &sTypename, IComplexFactory **retpFactory)

//	FindFactory
//
//	Finds the factory of the given typename

	{
	int iPos;

	if (!m_List.FindPos(sTypename, &iPos))
		return false;

	if (retpFactory)
		*retpFactory = m_List[iPos];

	return true;
	}
