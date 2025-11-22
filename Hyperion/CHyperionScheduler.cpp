//	CHyperionScheduler.cpp
//
//	CHyperionScheduler class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_LAST_RAN_ON,					"lastRanOn")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_RUN_FREQUENCY,				"runFrequency")
DECLARE_CONST_STRING(FIELD_STATUS,						"status")
DECLARE_CONST_STRING(FIELD_WILL_RUN_ON,					"willRunOn")

DECLARE_CONST_STRING(STATUS_READY,						"ready")
DECLARE_CONST_STRING(STATUS_RUNNING,					"running")

DECLARE_CONST_STRING(ERR_UNKNOWN_TASK,					"Unknown task: %s.")
DECLARE_CONST_STRING(ERR_RUN_STOPPED,					"User stopped task: %s")
DECLARE_CONST_STRING(ERR_RUN_TOO_LONG,					"Task exceeded maximum running time: %s")

const int MAX_RUN_TIME =								60 * 60 * 1000;	//	60 minutes is long enough.

CDatum CHyperionScheduler::GetTaskList (void)

//	GetTaskList
//
//	Returns a list of tasks

	{
	CSmartLock Lock(m_cs);
	int i;

	CComplexArray *pResult = new CComplexArray;
	for (i = 0; i < m_Tasks.GetCount(); i++)
		{
		CComplexStruct *pTask = new CComplexStruct;
		pTask->SetElement(FIELD_NAME, m_Tasks[i].sName);
		pTask->SetElement(FIELD_STATUS, (m_Tasks[i].bRunning ? STATUS_RUNNING : STATUS_READY));
		pTask->SetElement(FIELD_LAST_RAN_ON, m_Tasks[i].LastRun);
		pTask->SetElement(FIELD_WILL_RUN_ON, m_Tasks[i].NextRun);
		pTask->SetElement(FIELD_RUN_FREQUENCY, m_Tasks[i].iInterval);

		pResult->Append(CDatum(pTask));
		}

	return CDatum(pResult);
	}

void CHyperionScheduler::GetTasksToRun (TArray<CString> *retTasks)

//	GetTasksToRun
//
//	Returns a list of the tasks that need to run now.

	{
	CSmartLock Lock(m_cs);
	int i;

	retTasks->DeleteAll();

	CDateTime Now(CDateTime::Now);
	for (i = 0; i < m_Tasks.GetCount(); i++)
		if (m_Tasks[i].NextRun <= Now)
			retTasks->Insert(m_Tasks[i].sName);
	}

bool CHyperionScheduler::IsSignalStop (const CString &sTask, CString *retsMessage)

//	IsSignalStop
//
//	Returns TRUE if the task has been signalled to stop.

	{
	CSmartLock Lock(m_cs);
	STask *pTask = m_Tasks.GetAt(sTask);
	if (pTask == NULL || !pTask->bRunning)
		return false;

	//	If the user signalled stop, then we should stop

	if (pTask->bSignalStop)
		{
		if (retsMessage) *retsMessage = strPattern(ERR_RUN_STOPPED, sTask);
		return true;
		}

	//	If the task has exceeded its max run time, then stop it.

	CDateTime Now(CDateTime::Now);
	CDateTime StopAt = timeAddTime(pTask->LastRun, CTimeSpan(0, MAX_RUN_TIME));
	if (StopAt <= Now)
		{
		if (retsMessage) *retsMessage = strPattern(ERR_RUN_TOO_LONG, sTask);
		return true;
		}

	//	Otherwise, stop is not signalled

	return false;
	}

void CHyperionScheduler::Mark (void)

//	Mark
//
//	Mark data in use

	{
	int i;

	for (i = 0; i < m_Tasks.GetCount(); i++)
		m_Tasks[i].dCode.Mark();
	}

void CHyperionScheduler::SetRunComplete (const CString &sTask)

//	SetRunComplete
//
//	Mark the task as having run.

	{
	CSmartLock Lock(m_cs);
	STask *pTask = m_Tasks.GetAt(sTask);
	if (pTask == NULL)
		return;

	//	If we're supposed to delete this when done, do it now

	if (pTask->bDelete)
		{
		m_Tasks.DeleteAt(sTask);
		return;
		}

	//	Otherwise, mark it as done

	pTask->bRunning = false;
	pTask->bSignalStop = false;
	pTask->NextRun = timeAddTime(pTask->NextRun, CTimeSpan(pTask->iInterval * 1000));
	}

CHyperionScheduler::EResults CHyperionScheduler::SetRunning (const CString &sTask, CDatum *retdCode)

//	SetRunning
//
//	Sets the task as running and returns the code. If success, we return resultOK.
//	If the task is currently running, we return resultRunning.

	{
	CSmartLock Lock(m_cs);
	STask *pTask = m_Tasks.GetAt(sTask);
	if (pTask == NULL)
		return resultError;

	//	If we're already running, can't start again

	if (pTask->bRunning)
		return resultRunning;

	//	Set state and return the code

	pTask->bRunning = true;
	pTask->LastRun = CDateTime(CDateTime::Now);

	if (retdCode)
		*retdCode = pTask->dCode;

	return resultOK;
	}

bool CHyperionScheduler::SetSignalStop (const CString &sTask, CString *retsError)

//	SetSignalStop
//
//	Tells the task to stop running.

	{
	CSmartLock Lock(m_cs);
	STask *pTask = m_Tasks.GetAt(sTask);
	if (pTask == NULL)
		{
		if (retsError) *retsError = strPattern(ERR_UNKNOWN_TASK, sTask);
		return false;
		}

	//	If we're not running, then nothing to do.

	if (!pTask->bRunning)
		return true;

	//	Set the state

	pTask->bSignalStop = true;
	return true;
	}

void CHyperionScheduler::SetTaskList (const TArray<CHyperionPackageList::STaskInfo> &Tasks)

//	SetTaskList
//
//	Sets the list of tasks to run

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Clear all marks

	for (i = 0; i < m_Tasks.GetCount(); i++)
		m_Tasks[i].bMarked = false;

	//	Add or mark any tasks that we are supposed to have

	for (i = 0; i < Tasks.GetCount(); i++)
		{
		STask *pTask = m_Tasks.GetAt(Tasks[i].sName);
		if (pTask == NULL)
			{
			//	Insert the task

			pTask = m_Tasks.Insert(Tasks[i].sName);

			//	Initialize the time for a next run based on the interval.

			pTask->sName = Tasks[i].sName;
			int iNextRun = 1000 * Max(1, mathRandom(1, Tasks[i].iInterval));
			pTask->NextRun = timeAddTime(CDateTime(CDateTime::Now), CTimeSpan(iNextRun));
			pTask->bRunning = false;
			pTask->bDelete = false;
			pTask->bSignalStop = false;
			}

		pTask->bMarked = true;
		pTask->iInterval = Tasks[i].iInterval;
		pTask->dCode = Tasks[i].dCode;
		}

	//	Delete any tasks that we no longer need

	TArray<CString> ToDelete;
	for (i = 0; i < m_Tasks.GetCount(); i++)
		if (!m_Tasks[i].bMarked)
			{
			//	If we're not running, delete it now

			if (!m_Tasks[i].bRunning)
				ToDelete.Insert(m_Tasks[i].sName);

			//	Otherwise, we mark it for deletion

			else
				m_Tasks[i].bDelete = true;
			}

	//	Delete

	for (i = 0; i < ToDelete.GetCount(); i++)
		m_Tasks.DeleteAt(ToDelete[i]);
	}

bool CHyperionScheduler::SetTaskRunOn (const CString &sTask, const CDateTime &RunOn, CString *retsError)

//	SetTaskRunOn
//
//	Sets the date time when the task should run

	{
	CSmartLock Lock(m_cs);
	STask *pTask = m_Tasks.GetAt(sTask);
	if (pTask == NULL)
		{
		if (retsError) *retsError = strPattern(ERR_UNKNOWN_TASK, sTask);
		return false;
		}

	//	Set the time

	pTask->NextRun = RunOn;
	return true;
	}
