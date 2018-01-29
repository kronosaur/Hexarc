//	FoundationCounters.h
//
//	Foundation header file
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

template <class SAMPLE> class TAggregatedSample
	{
	public:
		TAggregatedSample (SAMPLE dwTotal = 0, SAMPLE dwAverage = 0, SAMPLE dwPeak = 0) : 
				m_dwTotal(dwTotal),
				m_dwAverage(dwAverage),
				m_dwPeak(dwPeak)
			{ }

		inline SAMPLE GetAverage (void) const { return m_dwAverage; }
		inline SAMPLE GetPeak (void) const { return m_dwPeak; }
		inline SAMPLE GetTotal (void) const { return m_dwTotal; }

	private:
		SAMPLE m_dwTotal;
		SAMPLE m_dwAverage;
		SAMPLE m_dwPeak;
	};

template <class SAMPLE, int SIZE> class TSampleArray
	{
	public:
		TSampleArray (void) :
				m_dwLast(0xffffffff)
			{
			for (int i = 0; i < SIZE; i++)
				m_Array[i] = 0;
			}

		void GetAggregated (TAggregatedSample<SAMPLE> &Result) const
			{
			SAMPLE dwTotal = 0;
			SAMPLE dwMax = 0;
			for (int i = 0; i < SIZE; i++)
				{
				dwTotal += m_Array[i];
				if (m_Array[i] > dwMax)
					dwMax = m_Array[i];
				}

			Result = TAggregatedSample<SAMPLE>(dwTotal, dwTotal / SIZE, dwMax);
			}

		DWORD GetLastTime (void) const { return (IsEmpty() ? 0 : m_dwLast); }

		void GetSamples (TArray<SAMPLE> &Result) const
			{
			Result.InsertEmpty(SIZE);
			DWORD dwStart = (IsEmpty() ? 0 : m_dwLast);

			for (int i = 0; i < SIZE; i++)
				Result[SIZE - (i + 1)] = m_Array[(dwStart + SIZE - i) % SIZE];
			}

		bool IsEmpty (void) const { return (m_dwLast == 0xffffffff); }

		void SetAt (DWORD dwTime, SAMPLE Value)
			{
			DWORD dwIndex = dwTime % SIZE;

			if (dwTime == m_dwLast)
				m_Array[dwIndex] += Value;
			else if (dwTime == m_dwLast + 1 || IsEmpty())
				{
				m_Array[dwIndex] = Value;
				m_dwLast = dwTime;
				}
			else
				{
				while (m_dwLast < dwTime - 1)
					{
					m_dwLast++;
					m_Array[m_dwLast % SIZE] = 0;
					}
				m_Array[dwIndex] = Value;
				m_dwLast = dwTime;
				}
			}

	private:
		inline DWORD Next (DWORD dwIndex) const { return (dwIndex + 1) % SIZE; }

		SAMPLE m_Array[SIZE];
		DWORD m_dwLast;
	};
