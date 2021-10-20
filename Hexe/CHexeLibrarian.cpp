//	CHexeLibrarian.cpp
//
//	CHexeLibrarian class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(LIBRARY_CORE,						"core")

CHexeLibrarian g_HexeLibrarian;

CDatum CHexeLibrarian::FindFunction (const CString &sLibrary, const CString &sFunction) const

//	FindFunction
//
//	Returns a function for the given library (or nil).

	{
	auto *pLibrary = GetLibrary(sLibrary);
	if (!pLibrary)
		return CDatum();

	auto *pFunc = pLibrary->Functions.GetAt(sFunction);
	if (!pFunc)
		return CDatum();

	return *pFunc;
	}

bool CHexeLibrarian::FindLibrary (const CString &sName, DWORD *retdwLibraryID) const

//	FindLibrary
//
//	Finds the ID of the given library

	{
	int iPos;
	if (!m_Catalog.FindPos(sName, &iPos))
		return false;

	//	Done

	if (retdwLibraryID)
		*retdwLibraryID = (DWORD)iPos;

	return true;
	}

const CString &CHexeLibrarian::GetEntry (DWORD dwLibrary, int iIndex, CDatum *retdFunction)

//	GetEntry
//
//	Returns the given entry

	{
	ASSERT((int)dwLibrary >= 0 && (int)dwLibrary < m_Catalog.GetCount());

	SLibrary *pLibrary = &m_Catalog[dwLibrary];

	if (retdFunction)
		*retdFunction = pLibrary->Functions[iIndex];

	return pLibrary->Functions.GetKey(iIndex);
	}

int CHexeLibrarian::GetEntryCount (DWORD dwLibrary)

//	GetEntryCount
//
//	Returns the number of entries in the given library

	{
	ASSERT((int)dwLibrary >= 0 && (int)dwLibrary < m_Catalog.GetCount());

	SLibrary *pLibrary = &m_Catalog[dwLibrary];
	return pLibrary->Functions.GetCount();
	}

const CHexeLibrarian::SLibrary *CHexeLibrarian::GetLibrary (const CString &sName) const

//	GetLibrary
//
//	Returns the library with the given name (or NULL).

	{
	return m_Catalog.GetAt(sName);
	}

CHexeLibrarian::SLibrary *CHexeLibrarian::GetLibrary (const CString &sName)

//	GetLibrary
//
//	Returns the library with the given name. If none exists, a new one is created.

	{
	ASSERT(!sName.IsEmpty());

	SLibrary *pLibrary;
	int iPos;
	if (m_Catalog.FindPos(sName, &iPos))
		pLibrary = &m_Catalog[iPos];
	else
		pLibrary = m_Catalog.Insert(sName);

	return pLibrary;
	}

void CHexeLibrarian::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i, j;

	for (i = 0; i < m_Catalog.GetCount(); i++)
		{
		SLibrary *pLibrary = &m_Catalog[i];
		for (j = 0; j < pLibrary->Functions.GetCount(); j++)
			pLibrary->Functions[j].Mark();
		}
	}

void CHexeLibrarian::RegisterCoreLibraries (void)

//	RegisterCoreLibraries
//
//	Registers built-in libraries
	
	{
	RegisterLibrary(LIBRARY_CORE, g_iCoreLibraryDefCount, g_CoreLibraryDef);
	RegisterLibrary(LIBRARY_CORE, g_iCoreVectorLibraryDefCount, g_CoreVectorLibraryDef);
	}

void CHexeLibrarian::RegisterLibrary (const CString &sName, int iCount, SLibraryFuncDef *pNewLibrary)

//	RegisterLibrary
//
//	Registers an external library

	{
	int i;

	SLibrary *pLibrary = GetLibrary(sName);

	for (i = 0; i < iCount; i++)
		{
		CDatum dFunc;
		CHexeLibraryFunction::Create(pNewLibrary[i], &dFunc);

		pLibrary->Functions.SetAt(pNewLibrary[i].sName, dFunc);
		}
	}

//	CHexe ----------------------------------------------------------------------

bool CHexe::m_bInitialized = false;

bool CHexe::Boot (void)

//	Boot
//
//	Boot the Hexe system. Must be called before other functions are called.

	{
	if (!m_bInitialized)
		{
		g_HexeLibrarian.RegisterCoreLibraries();

		if (!CHexeProcess::Boot())
			return false;

		//	Register some AEON datatypes

		CHexeCode::RegisterFactory();
		CHexeError::RegisterFactory();
		CHexeFunction::RegisterFactory();
		CHexeLocalEnvironment::RegisterFactory();
		CHexeGlobalEnvironment::RegisterFactory();

		//	Register our mark handler

		CDatum::RegisterMarkProc(CHexe::Mark);

		//	Done

		m_bInitialized = true;
		}

	return true;
	}

void CHexe::Mark (void)

//	Mark
//
//	Mark data in use

	{
	g_HexeLibrarian.Mark();
	}
