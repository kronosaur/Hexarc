//	CMutex.cpp
//
//	CMutex class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

void CMutex::Create (const CString &sName, bool bInitialOwner, bool *retbExists)

//	Create
//
//	Creates a new mutex

	{
	Close();

	//	Create the semaphore

	m_hHandle = ::CreateMutex(NULL, (bInitialOwner ? TRUE : FALSE), (sName.IsEmpty() ? NULL : CString16(sName)));
	if (m_hHandle == NULL)
		throw CException(errFail);

	//	See if the semaphore already exists

	if (retbExists)
		*retbExists = (::GetLastError() == ERROR_ALREADY_EXISTS);
	}
