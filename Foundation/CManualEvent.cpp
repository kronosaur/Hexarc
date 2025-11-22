//	CManualEvent.cpp
//
//	CManualEvent class
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CManualEvent::Create (const CString &sName, bool *retbExists)

//	Create
//
//	Initializes the semaphore at 0 and with the given maximum

	{
	Close();

	//	Create the semaphore

	m_hHandle = ::CreateEvent(NULL, 
			TRUE, 
			FALSE, 
			(sName.IsEmpty() ? NULL : CString16(sName)));
	if (m_hHandle == NULL)
		throw CException(errFail);

	//	See if the semaphore already exists

	if (retbExists)
		*retbExists = (::GetLastError() == ERROR_ALREADY_EXISTS);
	}

