//	CIODiagnostics.cpp
//
//	CIODiagnostics class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CString CIODiagnostics::AsString () const

//	AsString
//
//	Returns a string.

	{
	if (m_Log.GetCount() == 0)
		return NULL_STR;

	CStringBuffer Result;
	for (int i = 0; i < m_Log.GetCount(); i++)
		{
		if (i != 0)
			Result.WriteChar('\n');
		Result.Write(m_Log[i]);
		}

	return CString(std::move(Result));
	}

void CIODiagnostics::Log (CStringView sLine)

//	Log
//
//	Logs a line.

	{
	if (!m_bEnabled)
		return;

	m_Log.Insert(strPattern("%x [%x]: %s", ::sysGetTickCount(), ::GetCurrentThreadId(), sLine));
	}
