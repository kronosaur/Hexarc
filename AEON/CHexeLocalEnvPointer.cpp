//	CHexeLocalEnvPointer.cpp
//
//	CHexeLocalEnvPointer class
//	Copyright (c) 2023 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CHexeLocalEnvPointer::CHexeLocalEnvPointer (CDatum dEnv) :
		m_dEnv(dEnv),
		m_pEnv(CHexeLocalEnvironment::Upconvert(dEnv))

//	CHexeLocalEnvPointer constructor

	{
	if (!m_pEnv && !m_dEnv.IsIdenticalToNil())
		throw CException(errFail);
	}

CHexeLocalEnvPointer::CHexeLocalEnvPointer (int iArgCount)

//	CHexeLocalEnvPointer constructor

	{
	m_pEnv = new CHexeLocalEnvironment(iArgCount);
	}

CHexeLocalEnvPointer::CHexeLocalEnvPointer (CHexeLocalEnvPointer &&Src) noexcept

//	CHexeLocalEnvPointer constructor

	{
	m_dEnv = Src.m_dEnv;
	m_pEnv = Src.m_pEnv;

	//	If the source owns the environment, then we take ownership (and we will
	//	delete it or add it as a datum).

	if (!Src.TrackedByGC())
		Src.m_pEnv = NULL;
	}

CHexeLocalEnvPointer &CHexeLocalEnvPointer::operator= (CHexeLocalEnvPointer &&Src) noexcept

//	CHexeLocalEnvPointer operator =

	{
	CleanUp();

	m_dEnv = Src.m_dEnv;
	m_pEnv = Src.m_pEnv;

	if (!Src.TrackedByGC())
		Src.m_pEnv = NULL;

	return *this;
	}

void CHexeLocalEnvPointer::CleanUp ()

//	CleanUp
//
//	Clean up

	{
	//	If we own the environment, then we need to delete it.

	if (!TrackedByGC() && m_pEnv)
		{
		delete m_pEnv;
		m_pEnv = NULL;
		}
	}

CDatum CHexeLocalEnvPointer::GetClosure () const

//	GetClosure
//
//	Converts to a GC-enabled datum.

	{
	if (!TrackedByGC() && m_pEnv)
		{
		m_dEnv = CDatum(m_pEnv);

		//	Clear mark so that the next garbage collection pass we mark it
		//	and its children.

		m_pEnv->ClearMark();
		}

	return m_dEnv;
	}

void CHexeLocalEnvPointer::Mark ()

//	Mark
//
//	Mark data in use.

	{
	if (TrackedByGC())
		m_dEnv.Mark();
	else if (m_pEnv)
		{
		//	Since we're not part of the CDatum GC, we need to mark; we clear 
		//	the mark first because otherwise no one will clear it.

		m_pEnv->ClearMark();
		m_pEnv->Mark();
		}
	}
