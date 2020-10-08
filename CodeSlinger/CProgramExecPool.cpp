//	CProgramExecPool.cpp
//
//	CProgramExecPool class
//	Copyright (c) 2020 Kronosaur Productions, LLC. All Rights Reserved.

#include "PreComp.h"

bool CProgramExecPool::Boot (IArchonProcessCtx &ArchonCtx, CCodeSlingerEngine &Engine, int iThreadCount)

//	Boot
//
//	Start up all thread. Returns FALSE if we failed.

	{
	m_Pool.DeleteAll();
	m_Pool.InsertEmpty(iThreadCount);

	for (int i = 0; i < m_Pool.GetCount(); i++)
		{
		m_Pool[i].Set(new CProgramExecThread(ArchonCtx, Engine, m_ProgramSet, Engine.GetPauseEvent(), Engine.GetRunEvent(), Engine.GetQuitEvent()));
		m_Pool[i]->Start();
		}

	return true;
	}

void CProgramExecPool::WaitForPause ()

//	WaitForPause
//
//	Waits for all threads to pause and returns when they have paused.

	{
	for (int i = 0; i < m_Pool.GetCount(); i++)
		{
		m_Pool[i]->WaitForPause();
		}
	}

void CProgramExecPool::WaitForShutdown ()

//	WaitForShutdown
//
//	Waits for all threads to shut down.

	{
	for (int i = 0; i < m_Pool.GetCount(); i++)
		m_Pool[i]->Wait();

	m_Pool.DeleteAll();
	}
