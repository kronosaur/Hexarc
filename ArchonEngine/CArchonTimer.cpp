//	CArchonTimer.cpp
//
//	CArchonTimer class
//	Copyright (c) 2018 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")

void CArchonTimer::LogTime (IArchonProcessCtx *pProcess, const CString &sText, DWORDLONG dwMinTime) const

//	LogTime
//
//	Logs a time

	{
	DWORDLONG dwTime = ::sysGetTicksElapsed(m_dwStart);
	if (dwTime >= dwMinTime)
		pProcess->Log(MSG_LOG_INFO, strPattern("%s (%s ms)", sText, strFormatInteger((int)dwTime, -1, FORMAT_THOUSAND_SEPARATOR)));
	}
