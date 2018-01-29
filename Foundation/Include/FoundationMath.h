//	FoundationMath.h
//
//	Foundation header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

//	Math functions

int mathDoubleToIntStochastic (double rValue);
DWORD mathRandom (void);
int mathRandom (int iFrom, int iTo);
double mathRandom (double rFrom, double rTo);
inline double mathRandomDouble (void) { return (mathRandom() / 2147483648.0); }
double mathRandomGaussian (void);
void mathRandomUnique (int iFrom, int iTo, int iCount, TArray<int> *retResult);
double mathRound (double rValue, double rMultiple = 0.0);
DWORD mathSeededRandom (DWORD *ioSeed);
int mathSeededRandom (DWORD *ioSeed, int iFrom, int iTo);
inline double mathSeededRandomDouble (DWORD *ioSeed) { return (mathSeededRandom(ioSeed) / 2147483648.0); }
void mathShuffledRange (int iFrom, int iTo, TArray<int> *retResult);

//	CIPInteger -----------------------------------------------------------------

class CIPInteger
	{
	public:
		CIPInteger (void);
		CIPInteger (const CIPInteger &Src);
		CIPInteger (int iSrc);
		CIPInteger (DWORDLONG ilSrc);
		CIPInteger (double rSrc);
		~CIPInteger (void);

		CIPInteger &operator= (const CIPInteger &Src);
		bool operator== (const CIPInteger &Src) const;
		bool operator!= (const CIPInteger &Src) const;

		CIPInteger &operator+= (const CIPInteger &Src);
		CIPInteger operator + (const CIPInteger &Src) const;
		CIPInteger &operator-= (const CIPInteger &Src);
		CIPInteger operator - (const CIPInteger &Src) const;
		CIPInteger operator - (void) const;
		CIPInteger &operator*= (const CIPInteger &Src);
		CIPInteger operator * (const CIPInteger &Src) const;
		CIPInteger &operator/= (const CIPInteger &Src);
		CIPInteger operator / (const CIPInteger &Src) const;

		int AsByteArray (TArray<BYTE> *retValue) const;
		int AsInteger32Signed (void) const;
		DWORDLONG AsInteger64Unsigned (void) const;
		CString AsString (void) const;
		int Compare (const CIPInteger &Src) const;
		bool FitsAsInteger64Unsigned (void) const;
		void InitFromBytes (IMemoryBlock &Data);
		void InitFromString (const CString &sString);
		bool IsEmpty (void) const;
		inline bool IsNegative (void) const { return m_bNegative; }
		static bool Deserialize (IByteStream &Stream, CIPInteger *retpValue);
		bool Serialize (IByteStream &Stream) const;
		void TakeHandoff (CIPInteger &Src);
		void WriteBytes (IByteStream &Stream) const;

	private:
		CIPInteger (void *Value, bool bNegative) : m_Value(Value), m_bNegative(bNegative) { }

		void *m_Value;
		bool m_bNegative;
	};

inline int KeyCompare (const CIPInteger &Key1, const CIPInteger &Key2) { return Key1.Compare(Key2); }

extern CIPInteger NULL_IPINTEGER;

//	Statistics -----------------------------------------------------------------

template <class VALUE> class TStatistics
	{
	public:
		void Calc (const TQueue<VALUE> &Data)
			{
			int i;

			m_iCount = Data.GetCount();
			m_Average = 0;
			m_Max = 0;
			m_Min = 0;
			m_Median = 0;

			VALUE Total = 0;
			for (i = 0; i < Data.GetCount(); i++)
				{
				VALUE Value = Data.GetAt(i);

				//	Average

				Total += Value;

				//	Min/Max

				if (i == 0)
					{
					m_Max = Value;
					m_Min = Value;
					}
				else if (Value > m_Max)
					m_Max = Value;
				else if (Value < m_Min)
					m_Min = Value;
				}

			//	Compute average

			if (m_iCount > 0)
				m_Average = Total / m_iCount;
			}

		inline VALUE GetAverage (void) const { return m_Average; }
		inline int GetCount (void) const { return m_iCount; }
		inline VALUE GetMax (void) const { return m_Max; }
		inline VALUE GetMedian (void) const { return m_Median; }
		inline VALUE GetMin (void) const { return m_Min; }

	private:
		int m_iCount;
		VALUE m_Average;
		VALUE m_Max;
		VALUE m_Min;

		VALUE m_Median;
	};
