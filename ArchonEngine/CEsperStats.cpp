//	CEsperStats.cpp
//
//	CEsperStats class
//	Copyright (c) 20173 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

CEsperStats::CEsperStats (void) :
		m_StartTicks(::sysGetTickCount64()),
		m_StartTime(CDateTime::Now),
		m_StartTimePoint((DWORD)(m_StartTicks / (DWORDLONG)SAMPLE_TIME)),
		m_LastHourlySample(m_StartTime)

//	CEsperStats constructor

	{
	}

void CEsperStats::GetCurrentStat (EStats iStat, TAggregatedSample<DWORDLONG> &Value) const

//	GetCurrentStat
//
//	Returns the current stat

	{
	CSmartLock Lock(m_cs);
	ASSERT(iStat >= 0 && iStat < statCount);

	m_Stat[iStat].GetAggregated(Value);
	}

void CEsperStats::GetHistory (int iHours, TArray<SAllStats> &History) const

//	GetHistory
//
//	Returns hourly history.

	{
	CSmartLock Lock(m_csHistory);
	int i;

	ASSERT(iHours > 0);
	History.DeleteAll();

	//	Can't return more data than we have.

	iHours = Min(iHours, m_HourlyStats[statConnectionsIn].GetCount());
	if (iHours < 1)
		return;

	//	End at the last hourly sample date (aligned to the hour).

	CDateTime LastDate = m_LastHourlySample;
	LastDate.SetTime(LastDate.Hour(), 0, 0);

	//	Compute the date of the first sample

	DWORDLONG dwBack = (DWORDLONG)(iHours - 1) * 60 * 60 * 1000;
	CTimeSpan Back(dwBack);
	CDateTime SampleDate = timeSubtractTime(LastDate, Back);

	//	We advance by one hour each sample

	CTimeSpan OneHour(60 * 60 * 1000);

	//	Peak samples are the highest value of all minutes in the hour.
	//	Thus to convert to per-second rates we need to divide by 60.

	DWORDLONG ToSeconds = (SAMPLE_TIME / 1000);
	DWORDLONG To100K = 100000;

	//	Return data

	History.InsertEmpty(iHours);
	for (i = 0; i < iHours; i++)
		{
		int iIndex = m_HourlyStats[statConnectionsIn].GetCount() - iHours + i;

		SAllStats &Results = History[i];
		Results.Time = SampleDate;

		TAggregatedSample<DWORDLONG> &Connections = m_HourlyStats[statConnectionsIn][iIndex];
		Results.dwConnectionsIn = (DWORD)Connections.GetTotal();
		Results.dwPeakConnectionsIn = Max((DWORD)(Results.dwConnectionsIn > 0 ? 1 : 0), (DWORD)(Connections.GetPeak() / ToSeconds));

		TAggregatedSample<DWORDLONG> &Reads = m_HourlyStats[statBytesRead][iIndex];
		Results.dwTotalRead = (DWORD)(Reads.GetTotal() / To100K);
		Results.dwPeakRead = (DWORD)(Reads.GetPeak() / ToSeconds);

		TAggregatedSample<DWORDLONG> &Writes = m_HourlyStats[statBytesWritten][iIndex];
		Results.dwTotalWritten = (DWORD)(Writes.GetTotal() / To100K);
		Results.dwPeakWritten = (DWORD)(Writes.GetPeak() / ToSeconds);

		//	Next

		SampleDate = timeAddTime(SampleDate, OneHour);
		}
	}

void CEsperStats::IncStat (EStats iStat, DWORD dwValue)

//	IncStat
//
//	Increment the given stat

	{
	CSmartLock Lock(m_cs);
	ASSERT(iStat >= 0 && iStat < statCount);

	m_Stat[iStat].SetAt(GetTimePoint(), (DWORDLONG)dwValue);
	}

bool CEsperStats::IsHistoryTime (void)

//	IsHistoryTime
//
//	Returns TRUE if it is time to aggregate history

	{
	CSmartLock Lock(m_csHistory);

	CDateTime Now(CDateTime::Now);
	if (Now.Hour() != m_LastHourlySample.Hour())
		{
		//	We update the time so that a subsequent thread can't come in.

		m_LastHourlySample = Now;
		return true;
		}

	return false;
	}

void CEsperStats::Update (void)

//	Update
//
//	This should be called periodically to update the hourly array.

	{
	int i;

	if (IsHistoryTime())
		{
		//	First get the stats

		CSmartLock Lock(m_cs);
		TAggregatedSample<DWORDLONG> Stats[statCount];
		for (i = 0; i < statCount; i++)
			m_Stat[i].GetAggregated(Stats[i]);

		Lock.Unlock();

		//	Now lock the history and add the data. We use two separate locks because
		//	we don't want to slow down network traffic while we accumulate history.

		CSmartLock HistoryLock(m_csHistory);

		for (i = 0; i < statCount; i++)
			m_HourlyStats[i].Insert(Stats[i]);
		}
	}