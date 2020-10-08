//	CPortSet.cpp
//
//	CPortSet class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

DECLARE_CONST_STRING(FIELD_DOLLAR_SEQ,					"$Seq");

DECLARE_CONST_STRING(PORT_TYPE_CONSOLE,					"console");
DECLARE_CONST_STRING(PORT_TYPE_NULL,					"null");
DECLARE_CONST_STRING(PORT_TYPE_VALUE,					"value");

DECLARE_CONST_STRING(ERR_INVALID_PORT_ID,				"Invalid port ID: %s.");
DECLARE_CONST_STRING(ERR_PORT_ALREADY_EXISTS,			"Port %s already exists.");
DECLARE_CONST_STRING(ERR_UNKNOWN_PORT_TYPE,				"Unknown port type: %s.");

bool CPortSet::AddPort (const CString &sID, const CString &sType, CString *retsError)

//	AddPort
//
//	Adds a new port of the given name.

	{
	CSmartLock Lock(m_cs);

	if (sID.IsEmpty())
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_PORT_ID, sID);
		return false;
		}

	bool bNew;
	auto pPort = m_Ports.SetAt(sID, &bNew);
	if (!bNew)
		{
		if (retsError) *retsError = strPattern(ERR_PORT_ALREADY_EXISTS, sID);
		return false;
		}

	if (strEquals(sType, PORT_TYPE_CONSOLE))
		pPort->Set(new CConsolePort(*this, sID));
	else if (strEquals(sType, PORT_TYPE_NULL))
		pPort->Set(new CNullPort(*this, sID));
	else if (strEquals(sType, PORT_TYPE_VALUE))
		pPort->Set(new CValuePort(*this, sID));
	else
		{
		if (retsError) *retsError = strPattern(ERR_UNKNOWN_PORT_TYPE, sType);
		return false;
		}

	return true;
	}

CDatum CPortSet::GetView (SequenceNumber Seq) const

//	GetView
//
//	Returns a view of all ports from the given sequence number on.

	{
	CSmartLock Lock(m_cs);

	CDatum dResult(CDatum::typeStruct);

	for (int i = 0; i < m_Ports.GetCount(); i++)
		{
		CDatum dView = m_Ports[i]->GetView(Seq);
		if (!dView.IsNil())
			dResult.SetElement(m_Ports[i]->GetID(), dView);
		}

	dResult.SetElement(FIELD_DOLLAR_SEQ, m_Seq);
	return dResult;
	}

void CPortSet::Mark ()

//	Mark
//
//	Mark data in use

	{
	for (int i = 0; i < m_Ports.GetCount(); i++)
		m_Ports[i]->Mark();
	}

void CPortSet::Output (const CString &sID, CDatum dValue)

//	Output
//
//	Outputs to the port.

	{
	CSmartLock Lock(m_cs);

	auto pPort = m_Ports.GetAt(sID);
	if (!pPort)
		return;

	(*pPort)->Output(dValue);
	}
