//	CProgramSet.cpp
//
//	CProgramSet class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

CProgramSet::CProgramSet ()

//	CProgramSet constructor

	{
	m_HasWork.Create();
	m_HasWork.Reset();
	}

bool CProgramSet::CreateInstance (CDatum dCode, const SRunOptions &Options, DWORD *retdwID, CString *retsError)

//	CreateInstance
//
//	Creates a new program instance. Returns FALSE if we get an error.

	{
	CSmartLock Lock(m_cs);

	TSharedPtr<IProgramInstance> pNewInstance = IProgramInstance::Create(m_dwNextID++, dCode, Options, retsError);
	if (!pNewInstance)
		{
		return false;
		}

	if (retdwID)
		*retdwID = pNewInstance->GetID();

	m_Instances.SetAt(pNewInstance->GetID(), pNewInstance);

	//	Wake up the threads to start processing.

	m_HasWork.Set();

	return true;
	}

bool CProgramSet::GetInstanceView (DWORD dwID, const CString &sUsername, SequenceNumber Seq, CDatum *retdResult) const

//	GetInstanceView
//
//	Returns a list of view entries after the given sequence number.

	{
	m_cs.Lock();
	auto ppInstance = m_Instances.GetAt(dwID);
	m_cs.Unlock();
	if (ppInstance == NULL)
		{
		if (retdResult) *retdResult = CDatum(strPattern("Unknown program instance: %x", dwID));
		return false;
		}

	//	Make sure the given user is allowed to access the running program.

	if (!(*ppInstance)->CanView(sUsername))
		{
		if (retdResult) *retdResult = CDatum(strPattern("You are not authorized to access that program: %x", dwID));
		return false;
		}

	if (retdResult)
		*retdResult = (*ppInstance)->GetView(Seq);

	return true;
	}

IProgramInstance *CProgramSet::LockNextInstance ()

//	LockNextInstance
//
//	Looks for the next program to run. If we can't find any program to run, then
//	we return NULL and reset the work event.

	{
	CSmartLock Lock(m_cs);

	if (m_Instances.GetCount() == 0)
		{
		m_HasWork.Reset();
		return NULL;
		}

	m_iNextInstance = (m_iNextInstance % m_Instances.GetCount());
	const int iStartInstance = m_iNextInstance;
	do
		{
		auto pInstance = m_Instances[m_iNextInstance];
		m_iNextInstance = (m_iNextInstance + 1) % m_Instances.GetCount();

		if (pInstance->RunLock())
			return pInstance;
		}
	while (m_iNextInstance != iStartInstance);

	//	Nothing is ready to run.

	m_HasWork.Reset();
	return NULL;
	}

void CProgramSet::Mark ()

//	Mark
//
//	Mark all data in use.

	{
	CSmartLock Lock(m_cs);

	for (int i = 0; i < m_Instances.GetCount(); i++)
		m_Instances[i]->Mark();
	}

void CProgramSet::OnAsyncResult (const SArchonMessage &Msg)

//	OnAsyncResult
//
//	Handle an async result.

	{
	CSmartLock Lock(m_cs);

	//	Find the program

	IProgramInstance **ppInstance = m_TicketToInstance.GetAt(Msg.dwTicket);
	if (!ppInstance)
		return;

	(*ppInstance)->SetAsyncResult(CSimpleEngine::MessageToHexeResult(Msg));

	m_TicketToInstance.DeleteAt(Msg.dwTicket);
	m_HasWork.Set();
	}

SRunResult CProgramSet::Run ()

//	Run
//
//	Run the next available program. Returns in one of three situations:
//
//	1. The program has completed.
//	2. The program needs an async result.
//	3. The program has exceeded its time quantum.

	{
	//	Figure out which program we're going to run next

	IProgramInstance *pInstance = LockNextInstance();
	if (pInstance == NULL)
		return SRunResult(NULL, CHexeProcess::runOK, CDatum());

	//	Run the instance

	return pInstance->Run();
	}

void CProgramSet::SetAsyncTicket (IProgramInstance &Program, DWORD dwTicket)

//	SetAsyncTicket
//
//	Associates an async result ticket with the given program.

	{
	CSmartLock Lock(m_cs);
	m_TicketToInstance.SetAt(dwTicket, &Program);
	}
