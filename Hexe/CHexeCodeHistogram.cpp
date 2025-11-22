//	CHexeCodeHistogram.cpp
//
//	CHexeCodeHistogram class
//	Copyright (c) 2024 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CHexeCodeHistogram::AddEntry (DWORD dwOpCode, DWORD dwTime)

//	AddEntry
//
//	Adds an entry to the histogram.

	{
	SHistogramEntry &Entry = m_Histogram[GetOpCode(dwOpCode) >> 24];
	Entry.iCount++;
	Entry.dwTotalTime += dwTime;
	}

void CHexeCodeHistogram::DebugOutput (void)

//	DebugOutput
//
//	Prints the histogram.

	{
	//	Sort by total time.

	TSortMap<DWORD, int> Sorted;
	for (int i = 0; i < opCodeCount; i++)
		Sorted.Insert(m_Histogram[i].dwTotalTime, i);

	//	Output

	DWORD dwTotalTime = 0;
	DWORDLONG dwTotalOps = 0;
	COpCodeDatabase Db;
	for (int i = 0; i < Sorted.GetCount(); i++)
		{
		int iEntry = Sorted[i];
		if (m_Histogram[iEntry].iCount > 0)
			{
			SOpCodeInfo* pInfo = Db.GetInfo((DWORD)iEntry << 24);
			CString sName = (pInfo && pInfo->dwOpCode != opNoOp ? pInfo->sOpCode : strPattern("UNK %02x", iEntry));

			dwTotalTime += m_Histogram[iEntry].dwTotalTime;
			dwTotalOps += m_Histogram[iEntry].iCount;

			printf("%02x %s: %s calls, %d ms [%d ns]\n", 
					iEntry,
					(LPSTR)sName, 
					(LPSTR)strFormatInteger(m_Histogram[iEntry].iCount, -1, FORMAT_THOUSAND_SEPARATOR), 
					m_Histogram[iEntry].dwTotalTime,
					(DWORD)(1000000.0 * (double)m_Histogram[iEntry].dwTotalTime / (double)m_Histogram[iEntry].iCount));
			}
		}

	printf("Total time: %d ms\n", dwTotalTime);
	double rTotalOps = (double)dwTotalOps / 1000000.0;
	printf("Total ops: %f million ops\n", rTotalOps);
	}
