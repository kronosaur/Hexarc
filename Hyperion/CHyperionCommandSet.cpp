//	CHyperionCommandSet.cpp
//
//	CHyperionCommandSet class
//	Copyright (c) 2012 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(EXPORT_PUBLIC,						"public")

DECLARE_CONST_STRING(FIELD_ATTRIBUTES,					"attributes")
DECLARE_CONST_STRING(FIELD_CODE,						"code")
DECLARE_CONST_STRING(FIELD_EXPORT,						"export")
DECLARE_CONST_STRING(FIELD_HELP,						"help")
DECLARE_CONST_STRING(FIELD_NAME,						"name")

DECLARE_CONST_STRING(ERR_NO_NAME,						"Function must have a name.")

CHyperionCommandSet::~CHyperionCommandSet (void)

//	CHyperionCommandSet destructor

	{
	DeleteAll();
	}

bool CHyperionCommandSet::AddCommand (const CString &sName, CDatum dDesc, SCommandInfo **retpEntry, CString *retsError)

//	AddCommand
//
//	Adds a command to the set

	{
	int i;

	if (sName.IsEmpty())
		{
		*retsError = ERR_NO_NAME;
		return false;
		}

	//	Create the new command entry

	SCommandInfo *pNewCommand = new SCommandInfo;
	pNewCommand->dDesc = dDesc;
	pNewCommand->sName = sName;
	pNewCommand->dHelp = dDesc.GetElement(FIELD_HELP);
	pNewCommand->dCode = dDesc.GetElement(FIELD_CODE);
	pNewCommand->bPublic = strEquals((CStringView)dDesc.GetElement(FIELD_EXPORT), EXPORT_PUBLIC);

	//	Add attributes

	CDatum dAttribs = dDesc.GetElement(FIELD_ATTRIBUTES);
	for (i = 0; i < dAttribs.GetCount(); i++)
		{
		CStringView sAttrib = dAttribs.GetElement(i);
		if (!sAttrib.IsEmpty())
			{
			//	Add the attribute to the command entry

			pNewCommand->Attribs.Insert(sAttrib);

			//	Add the command to the index

			CCommandIndex *pIndex = m_ByAttrib.SetAt(sAttrib);
			pIndex->Insert(sName, pNewCommand);
			}
		}

	//	Add to all commands

	m_AllCommands.Insert(pNewCommand);

	//	If this command has a leading namespace (namespace+name) then index it
	//	by the name without the namespace and add the namespace as an attribute.

	CString sNamespace;
	CString sLocalName;
	IHyperionService::ParseObjectName(pNewCommand->sName, &sNamespace, &sLocalName);
	if (!sNamespace.IsEmpty())
		{
		pNewCommand->Attribs.Insert(sNamespace);

		CCommandIndex *pIndex = m_ByAttrib.SetAt(sNamespace);
		pIndex->Insert(sLocalName, pNewCommand);
		}

	//	Done

	if (retpEntry)
		*retpEntry = pNewCommand;

	return true;
	}

void CHyperionCommandSet::DeleteAll (void)

//	DeleteAll
//
//	Delete all commands

	{
	int i;

	for (i = 0; i < m_AllCommands.GetCount(); i++)
		delete m_AllCommands[i];

	m_AllCommands.DeleteAll();
	m_ByAttrib.DeleteAll();
	}

bool CHyperionCommandSet::FindCommand (const CString &sAttrib, const CString &sName, SCommandInfo *retCommand)

//	FindCommand
//
//	Finds the given command out of the set of commands with the given attribute.

	{
	CCommandIndex *pIndex = m_ByAttrib.GetAt(sAttrib);
	if (pIndex == NULL)
		return false;
	
	SCommandInfo *pCommand;
	if (!pIndex->Find(sName, &pCommand))
		return false;

	if (retCommand)
		*retCommand = *pCommand;

	return true;
	}

void CHyperionCommandSet::GetCommands (const CString &sAttrib, TArray<CHyperionCommandSet::SCommandInfo> *retList)

//	GetCommands
//
//	Adds all of the commands with the given attribute to the list.

	{
	int i;

	//	If no attribute, then we return all commands

	if (sAttrib.IsEmpty())
		{
		retList->GrowToFit(m_AllCommands.GetCount());
		for (i = 0; i < m_AllCommands.GetCount(); i++)
			retList->Insert(*m_AllCommands[i]);
		}

	//	Otherwise, use the index

	else
		{
		CCommandIndex *pIndex = m_ByAttrib.GetAt(sAttrib);
		if (pIndex == NULL)
			return;

		retList->GrowToFit(pIndex->GetCount());
		for (i = 0; i < pIndex->GetCount(); i++)
			retList->Insert(*pIndex->GetValue(i));
		}
	}

void CHyperionCommandSet::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	//	NOTE: dDesc contains all the other data, so we don't need to mark them
	//	explicitly.

	for (i = 0; i < m_AllCommands.GetCount(); i++)
		m_AllCommands[i]->dDesc.Mark();
	}
