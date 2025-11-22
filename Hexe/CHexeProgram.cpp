//	CHexeProgram.cpp
//
//	CHexeProgram class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CDatum CHexeProgram::GetEntryPoint (const CString& sName) const

//	GetEntryPoint
//
//	Returns the function type of the given entry point (or Nil if not found).

	{
	auto pEntry = m_EntryPoints.GetAt(strToLower(sName));
	if (!pEntry)
		return CDatum();

	return pEntry->dType;
	}

TArray<CHexeDocument::SEntryPoint> CHexeProgram::GetMainCode () const

//	GetMainCode
//
//	Returns the main code block.

	{
	TArray<CHexeDocument::SEntryPoint> Result;
	if (m_sMain.IsEmpty() || m_dMainCode.IsNil())
		return Result;

	Result.InsertEmpty(1);

	Result[0].sFunction = m_sMain;
	Result[0].dCode = m_dMainCode;

	return Result;
	}

void CHexeProgram::Mark ()

//	Mark
//
//	Mark data in use.

	{
	m_dMainCode.Mark();
	m_Types.Mark();

	for (int i = 0; i < m_EntryPoints.GetCount(); i++)
		{
		m_EntryPoints[i].dCode.Mark();
		m_EntryPoints[i].dType.Mark();
		}
	}
