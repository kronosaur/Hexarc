//	CHexeLibrarian.cpp
//
//	CHexeLibrarian class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_MSG,							"msg");
DECLARE_CONST_STRING(FIELD_PAYLOAD,						"payload");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");

DECLARE_CONST_STRING(LIBRARY_CORE,						"core");

DECLARE_CONST_STRING(TYPE_HEXARC_MSG,					"hexarcMsg");

CHexeLibrarian g_HexeLibrarian;

//template <class OBJ> TArray<struct TDatumMethodHandler<OBJ>::SEntry> TDatumMethodHandler<OBJ>::m_Methods;

CDatum CHexeLibrarian::CreateFunction (const SFunctionDef &Def)

//	CreateFunction
//
//	Creates a library function.

	{
	CHexeLibraryFunction *pFunc = new CHexeLibraryFunction;
	pFunc->SetName(CString(Def.pName));
	pFunc->SetArgList(CString(Def.pArgList));
	pFunc->SetHelp(CString(Def.pHelp));
	pFunc->SetFunction(Def.pfFunc, Def.dwData);
	pFunc->SetExecFlags(Def.dwExecFlags);

	return CDatum(pFunc);
	}

CDatum CHexeLibrarian::FindFunction (CStringView sLibrary, CStringView sFunction) const

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

	return m_Functions[*pFunc];
	}

bool CHexeLibrarian::FindLibrary (CStringView sName, DWORD* retdwLibraryID) const

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

CStringView CHexeLibrarian::GetEntry (DWORD dwLibrary, int iIndex, CDatum* retdFunction)

//	GetEntry
//
//	Returns the given entry

	{
	ASSERT((int)dwLibrary >= 0 && (int)dwLibrary < m_Catalog.GetCount());

	SLibrary* pLibrary = &m_Catalog[dwLibrary];

	if (retdFunction)
		*retdFunction = m_Functions[pLibrary->Functions[iIndex]];

	return pLibrary->Functions.GetKey(iIndex);
	}

int CHexeLibrarian::GetEntryCount (DWORD dwLibrary)

//	GetEntryCount
//
//	Returns the number of entries in the given library

	{
	ASSERT((int)dwLibrary >= 0 && (int)dwLibrary < m_Catalog.GetCount());

	SLibrary* pLibrary = &m_Catalog[dwLibrary];
	return pLibrary->Functions.GetCount();
	}

const CHexeLibrarian::SLibrary *CHexeLibrarian::GetLibrary (CStringView sName) const

//	GetLibrary
//
//	Returns the library with the given name (or NULL).

	{
	return m_Catalog.GetAt(sName);
	}

CHexeLibrarian::SLibrary* CHexeLibrarian::GetLibrary (CStringView sName)

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
	for (int i = 0; i < m_Functions.GetCount(); i++)
		m_Functions[i].Mark();
	}

void CHexeLibrarian::RegisterCoreLibraries (void)

//	RegisterCoreLibraries
//
//	Registers built-in libraries
	
	{
	RegisterLibrary(LIBRARY_CORE, g_iCoreLibraryDefCount, g_CoreLibraryDef);
	RegisterLibrary(LIBRARY_CORE, g_iCoreVectorLibraryDefCount, g_CoreVectorLibraryDef);
	}

void CHexeLibrarian::RegisterLibrary (CStringView sName, int iCount, const SLibraryFuncDef* pNewLibrary)

//	RegisterLibrary
//
//	Registers an external library

	{
	SLibrary *pLibrary = GetLibrary(sName);

	for (int i = 0; i < iCount; i++)
		{
		CDatum dFunc;
		CHexeLibraryFunction::Create(pNewLibrary[i], &dFunc);

		int iID = m_Functions.GetCount();
		m_Functions.Insert(dFunc);

		pLibrary->Functions.SetAt(pNewLibrary[i].sName, iID);
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

bool CHexe::InvokeHexarcMsg (const CString &sMsg, CDatum dPayload, CDatum &retdResult)

//	InvokeHexarcMsg
//
//	Composes a response from a library function to invoke the given Hexarc
//	message.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_TYPE, TYPE_HEXARC_MSG);
	dResult.SetElement(FIELD_MSG, sMsg);
	dResult.SetElement(FIELD_PAYLOAD, dPayload);

	retdResult = dResult;

	//	FALSE means special result.

	return false;
	}

void CHexe::Mark (void)

//	Mark
//
//	Mark data in use

	{
	g_HexeLibrarian.Mark();
	}

bool CHexe::RunFunction (CDatum dFunc, TArray<CDatum>&& Args, CDatum dContext, SAEONInvokeResult& retResult)

//	RunFunction
//
//	Composes a response from a library function to call an function with 
//	arguments.

	{
	retResult.iResult = CDatum::InvokeResult::functionCall;
	retResult.dResult = dFunc;
	retResult.Args = std::move(Args);
	retResult.dContext = dContext;

	//	FALSE means special result.

	return false;
	}

bool CHexe::RunFunction1Arg (CDatum dFunc, CDatum dArg, CDatum dContext, SAEONInvokeResult& retResult)
	{
	TArray<CDatum> Args;
	Args.InsertEmpty(1);
	Args[0] = dArg;

	return RunFunction(dFunc, std::move(Args), dContext, retResult);
	}

bool CHexe::RunFunction2Args (CDatum dFunc, CDatum dArg1, CDatum dArg2, CDatum dContext, SAEONInvokeResult& retResult)
	{
	TArray<CDatum> Args;
	Args.InsertEmpty(2);
	Args[0] = dArg1;
	Args[1] = dArg2;

	return RunFunction(dFunc, std::move(Args), dContext, retResult);
	}

