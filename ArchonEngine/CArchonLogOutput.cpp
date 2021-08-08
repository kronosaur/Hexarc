//	CArchonLogOutput.cpp
//
//	CArchonLogOutput class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");

void CArchonLogOutput::Write (const CString &sLine, ELogClasses iClass)
	{
	m_ProcessCtx.Log(MSG_LOG_DEBUG, sLine);
	}
