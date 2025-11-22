//	IHyperionService.cpp
//
//	IHyperionService class
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ALLOW_ACCESS,				"allowAccess");
DECLARE_CONST_STRING(FIELD_PORT,						"port");
DECLARE_CONST_STRING(FIELD_PROTOCOL,					"protocol");
DECLARE_CONST_STRING(FIELD_RIGHTS,						"rights");

DECLARE_CONST_STRING(PROTOCOL_AI1,						"ai1");
DECLARE_CONST_STRING(PROTOCOL_HTTP,						"http");
DECLARE_CONST_STRING(PROTOCOL_HEXARC_MSG,				"hexarcMsg");

DECLARE_CONST_STRING(RIGHT_ARC_ADMIN,					"Arc.admin");
DECLARE_CONST_STRING(RIGHT_ARC_DEVELOPER,				"Arc.developer");

DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Invalid port: %s.");
DECLARE_CONST_STRING(ERR_UNKNOWN_PROTOCOL,				"Unknown protocol: %s.");

bool IHyperionService::CreateService (const CString &sName,
									  CDatum dServiceDef, 
									  const CHexeDocument &Package, 
									  const CString &sPackageName,
									  IHyperionService **retpService, 
									  CString *retsError)

//	CreateService
//
//	Creates a service from a service descriptor

	{
	//	Create the appropriate service

	CStringView sProtocol = dServiceDef.GetElement(FIELD_PROTOCOL);
	IHyperionService *pService;
	if (strEquals(sProtocol, PROTOCOL_HTTP))
		{
		if (!CHTTPService::CreateServiceClass(dServiceDef, Package, &pService, retsError))
			return false;
		}
	else if (strEquals(sProtocol, PROTOCOL_AI1))
		{
		if (!CAI1Service::CreateServiceClass(dServiceDef, Package, &pService, retsError))
			return false;
		}
	else if (strEquals(sProtocol, PROTOCOL_HEXARC_MSG))
		{
		if (!CHexarcMsgService::CreateServiceClass(dServiceDef, Package, &pService, retsError))
			return false;
		}
	else
		{
		if (retsError)
			*retsError = strPattern(ERR_UNKNOWN_PROTOCOL, sProtocol);
		return false;
		}

	//	Initialize some basic information

	pService->m_sPackageName = sPackageName;
	pService->m_sName = sName;
	pService->m_sProtocol = sProtocol;

	//	Load the port, if necessary

	if (pService->IsListener())
		{
		CString sPort = dServiceDef.GetElement(FIELD_PORT).AsString();
		if (!ValidatePort(sPort))
			{
			if (retsError)
				*retsError = strPattern(ERR_INVALID_PORT, sPort);
			return false;
			}

		pService->m_sPort = sPort;
		}

	//	Load the rights from the service desc

	pService->m_SecurityCtx.SetServiceRights(dServiceDef.GetElement(FIELD_RIGHTS));
	if (pService->m_SecurityCtx.HasNoServiceRights())
		pService->m_SecurityCtx.InsertServiceRight(RIGHT_ARC_DEVELOPER);

	//	We use the sandbox, unless we have admin rights

	if (!pService->m_SecurityCtx.HasServiceRight(RIGHT_ARC_ADMIN))
		{
		CString sSandbox = strPattern("%s.", sPackageName);
		pService->m_SecurityCtx.SetSandbox(sSandbox);
		}

	//	Load the access granted to other packages

	if (!pService->m_Access.InitFromDatum(sPackageName, dServiceDef.GetElement(FIELD_ALLOW_ACCESS), retsError))
		return false;

	//	Allow subclasses to initialize

	if (!pService->OnInit(dServiceDef, Package, retsError))
		return false;

	//	Done

	*retpService = pService;
	return true;
	}

bool IHyperionService::IsAccessible (const CString &sName)

//	IsAccessible
//
//	Returns TRUE if the given name is accessible from the sandbox
//
//	sID may be:
//
//	An AI1 interface
//	A table
//	A user right

	{
	return m_SecurityCtx.IsNamespaceAccessible(sName);
	}

void IHyperionService::OnGetListeners (TArray<SListenerDesc> &Listeners) const

//	OnGetListeners
//
//	Returns the list of listeners for this service.

	{
	Listeners.DeleteAll();

	if (!m_sPort.IsEmpty())
		{
		SListenerDesc *pDesc = Listeners.Insert();
		pDesc->sProtocol = NULL_STR;	//	raw protocol is the default
		pDesc->sPort = m_sPort;
		}
	}

bool IHyperionService::ParseObjectName (const CString &sName, CString *retsSandbox, CString *retsLocalName)

//	ParseObjectName
//
//	Parse name of the form:
//
//	{sandbox}+{localName}

	{
	char *pPos = sName.GetParsePointer();
	char *pPosEnd = pPos + sName.GetLength();

	char *pStart = pPos;
	while (pPos < pPosEnd && *pPos != '+')
		pPos++;

	if (pPos == pPosEnd)
		{
		if (retsSandbox)
			*retsSandbox = NULL_STR;

		if (retsLocalName)
			*retsLocalName = sName;
		}
	else
		{
		if (retsSandbox)
			*retsSandbox = CString(pStart, pPos - pStart);

		pPos++;
		pStart = pPos;
		while (pPos < pPosEnd)
			pPos++;

		if (retsLocalName)
			*retsLocalName = CString(pStart, pPos - pStart);
		}

	return true;
	}

bool IHyperionService::ValidatePort (const CString &sPort)

//	ValidatePort
//
//	Makes sure the port is valid

	{
	//	Make sure it is a valid number

	if (strToInt(sPort, 0) == 0)
		return false;

	return true;
	}
