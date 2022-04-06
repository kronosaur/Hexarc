//	AEONAllocator.h
//
//	AEON header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by AEON.h

#pragma once

//	TGCAllocator
//
//	This template keeps track of a set of VALUEs.
//
//	USAGE
//
//	1. Allocate a new VALUE
//	2. TGCAllocator.New(VALUE) to keep track of it
//	3. TGCAllocator.Mark(VALUE) to mark it
//	4. TGCAllocator.Sweep() to free unuzed value (and to unmark).

template <class VALUE, class ACTUAL> class TGCAllocator : public ACTUAL
	{
	public:
		TGCAllocator (void) : m_dwFirstFree(END_OF_FREE_LIST)
			{
			ASSERT(sizeof(VALUE) >= sizeof(DWORD));
			}

		DWORD New (VALUE pValue)
			{
			CSmartLock Lock(m_cs);

			//	If necessary allocate more segments

			if (m_dwFirstFree == END_OF_FREE_LIST)
				AllocSeg();

			//	Get the new allocation

			DWORD dwNewID = m_dwFirstFree;
			VALUE *pNewValue = GetValue(dwNewID);

			//	Set the free list. The value at dwNewID is an index
			//	to the next free item in the list.

			m_dwFirstFree = ValueToIndex(*pNewValue);

			//	Set the value

			*pNewValue = pValue;

			//	Done

			return dwNewID;
			}

		void Sweep (void)
			{
			CSmartLock Lock(m_cs);
			int i;

			for (i = 0; i < m_Backbone.GetCount(); i++)
				{
				VALUE *pStart = m_Backbone[i];
				VALUE *pPos = pStart;
				VALUE *pEnd = pPos + SEGMENT_SIZE;

				while (pPos < pEnd)
					{
					if (!IsFree(*pPos))
						{
						if (!ACTUAL::IsMarked(*pPos))
							{
							ACTUAL::FreeValue(*pPos);

							//	We store an index to the next free item.
							//	(This is not really a pointer, but we cast it into a pointer)

							*pPos = IndexToValue(m_dwFirstFree);

							//	First free is now this item

							m_dwFirstFree = (i * SEGMENT_SIZE) + (DWORD)(pPos - pStart);
							}
						else
							ACTUAL::ClearMark(*pPos);
						}

					pPos++;
					}
				}
			}

	private:
		enum Constants
			{
			SEGMENT_SIZE =		8096,
			END_OF_FREE_LIST =	0xFFFFFFFF,
			};

		void AllocSeg (void)
			{
			DWORD dwBase = m_Backbone.GetCount() * SEGMENT_SIZE;

			//	Allocate a new segment

			VALUE *pNewSeg = new VALUE [SEGMENT_SIZE];

			//	Add all entries to the free list

			VALUE *pPos = pNewSeg;
			VALUE *pEnd = pNewSeg + (SEGMENT_SIZE - 1);
			DWORD dwOffset = 1;
			while (pPos < pEnd)
				{
				*pPos = IndexToValue(dwBase + dwOffset);
				dwOffset++;
				pPos++;
				}

			*pEnd = IndexToValue(END_OF_FREE_LIST);
			m_dwFirstFree = dwBase;

			//	Insert

			m_Backbone.Insert(pNewSeg);
			}

		inline VALUE *GetValue (DWORD dwID) { return m_Backbone[dwID / SEGMENT_SIZE] + (dwID % SEGMENT_SIZE); }
		inline VALUE IndexToValue (DWORD dwID) { return (dwID == END_OF_FREE_LIST ? (VALUE)(DWORD_PTR)END_OF_FREE_LIST : ((VALUE)(DWORD_PTR)((dwID << 1) | 0x01))); }
		inline bool IsFree (VALUE pPointer) { return (((DWORD)(DWORD_PTR)pPointer) & 0x01); }
		inline DWORD ValueToIndex (VALUE pPointer) { return (((DWORD)(DWORD_PTR)pPointer) == END_OF_FREE_LIST ? END_OF_FREE_LIST : (((DWORD)(DWORD_PTR)pPointer) >> 1)); }

		CCriticalSection m_cs;
		TArray<VALUE *> m_Backbone;
		DWORD m_dwFirstFree;
	};

//	CGCStringAllocatorBase
//
//	Used to implement CGStringAllocator

class CGCStringAllocatorBase
	{
	public:
		inline void Mark (LPSTR pValue)
			{
			int iLength = *((int *)pValue - 1);
			//	Negative lengths are literal values that don't need to be marked/freed
			if (iLength >= 0)
				pValue[iLength] = '\xff';
			}

	protected:
		inline void ClearMark (LPSTR pValue)
			{
			int iLength = *((int *)pValue - 1);
			pValue[iLength] = '\0';
			}

		inline void FreeValue (LPSTR pPointer) { delete (pPointer - sizeof(int)); }

		inline bool IsMarked (LPSTR pPointer)
			{
			int iLength = *((int *)pPointer - 1);
			return pPointer[iLength] == '\xff';
			}
	};

typedef TGCAllocator<LPSTR, CGCStringAllocatorBase> CGCStringAllocator;

//	CGCComplexAllocatorBase
//
//	Used to implement IComplexDatum allocator

class CGCComplexAllocatorBase
	{
	public:
		inline void Mark (IComplexDatum *pValue) { pValue->Mark(); }

	protected:
		inline void ClearMark (IComplexDatum *pValue) { pValue->ClearMark(); }
		inline void FreeValue (IComplexDatum *pValue) { delete pValue; }
		inline bool IsMarked (IComplexDatum *pValue) { return pValue->IsMarked(); }
	};

typedef TGCAllocator<IComplexDatum *, CGCComplexAllocatorBase> CGCComplexAllocator;

template <class VALUE> class TAllocatorGC
	{
	public:
		TAllocatorGC (void) : m_dwFirstFree(END_OF_FREE_LIST)
			{
			ASSERT(sizeof(VALUE) >= sizeof(DWORD));
			}

		~TAllocatorGC (void)
			{
			int i;
			for (i = 0; i < m_ValueBack.GetCount(); i++)
				delete m_ValueBack[i];

			for (i = 0; i < m_MetaBack.GetCount(); i++)
				delete m_MetaBack[i];
			}

		void ClearMark (DWORD dwID)
			{
			BYTE *pMeta = GetMeta(dwID);
			*pMeta &= ~FLAG_MARKED;
			}

		void Delete (DWORD dwID)
			{
			VALUE *pValue = GetValue(dwID);
			(*(DWORD *)pValue) = m_dwFirstFree;
			m_dwFirstFree = dwID;

			BYTE *pMeta = GetMeta(dwID);
			*pMeta = FLAG_FREE;
			}

		inline const VALUE &Get (DWORD dwID) const
			{
			return *GetValue(dwID);
			}

		bool IsMarked (DWORD dwID)
			{
			BYTE *pMeta = GetMeta(dwID);
			return ((*pMeta & FLAG_MARKED) ? true : false);
			}

		void Mark (DWORD dwID)
			{
			BYTE *pMeta = GetMeta(dwID);
			*pMeta |= FLAG_MARKED;
			}

		DWORD New (VALUE &NewValue)
			{
			CSmartLock Lock(m_cs);

			//	If necessary allocate more segments

			if (m_dwFirstFree == END_OF_FREE_LIST)
				AllocSeg();

			//	Get the new allocation

			DWORD dwNewID = m_dwFirstFree;
			VALUE *pNewValue = GetValue(dwNewID);
			BYTE *pNewMeta = GetMeta(dwNewID);

			//	Set the free list

			ASSERT(*pNewMeta == FLAG_FREE);
			m_dwFirstFree = *(DWORD *)pNewValue;

			//	Set the value

			*pNewValue = NewValue;
			*pNewMeta = 0;

			//	Done

			return dwNewID;
			}

		void Sweep (void)
			{
			CSmartLock Lock(m_cs);
			int i, j;

			for (i = 0; i < m_MetaBack.GetCount(); i++)
				{
				BYTE *pMetaArray = m_MetaBack[i];

				for (j = 0; j < SEGMENT_SIZE; j++)
					if (!(pMetaArray[j] & FLAG_FREE))
						{
						if (pMetaArray[j] & FLAG_MARKED)
							pMetaArray[j] = FLAG_NONE;
						else
							Delete(i * SEGMENT_SIZE + j);
						}
				}
			}

	private:
		enum Constants
			{
			SEGMENT_SIZE =		8096,

			FLAG_NONE =			0x00,
			FLAG_FREE =			0x01,
			FLAG_MARKED =		0x02,

			END_OF_FREE_LIST =	0xFFFFFFFF,
			};

		void AllocSeg (void)
			{
			DWORD dwBase = m_ValueBack.GetCount() * SEGMENT_SIZE;

			//	Allocate a new segment

			VALUE *pNewSeg = new VALUE [SEGMENT_SIZE];

			//	Add all entries to the free list

			VALUE *pPos = pNewSeg;
			VALUE *pEnd = pNewSeg + (SEGMENT_SIZE - 1);
			DWORD dwOffset = 1;
			while (pPos < pEnd)
				{
				(*(DWORD *)pPos) = dwBase + dwOffset;
				dwOffset++;
				pPos++;
				}

			(*(DWORD *)pEnd) = END_OF_FREE_LIST;
			m_dwFirstFree = dwBase;

			//	Allocate meta data

			BYTE *pNewMeta = new BYTE [SEGMENT_SIZE];

			//	Set free

			BYTE *pMetaPos = pNewMeta;
			BYTE *pMetaEnd = pNewMeta + SEGMENT_SIZE;
			while (pMetaPos < pMetaEnd)
				*pMetaPos++ = FLAG_FREE;

			//	Insert

			m_ValueBack.Insert(pNewSeg);
			m_MetaBack.Insert(pNewMeta);
			}

		inline BYTE *GetMeta (DWORD dwID) const { return m_MetaBack[dwID / SEGMENT_SIZE] + (dwID % SEGMENT_SIZE); }
		inline VALUE *GetValue (DWORD dwID) const { return m_ValueBack[dwID / SEGMENT_SIZE] + (dwID % SEGMENT_SIZE); }

		CCriticalSection m_cs;
		TArray<VALUE *> m_ValueBack;
		TArray<BYTE *> m_MetaBack;
		DWORD m_dwFirstFree;
	};

class CAEONStore
	{
	public:
		static DWORD Alloc (IComplexDatum* pValue) { return m_ComplexAlloc.New(pValue); }
		static DWORD Alloc (double rValue) { return m_DoubleAlloc.New(rValue); }
		static DWORD Alloc (LPSTR pValue) { return m_StringAlloc.New(pValue); }

		static double GetDouble (DWORD dwID) { return m_DoubleAlloc.Get(dwID); }

		static void MarkComplex (IComplexDatum* pValue) { m_ComplexAlloc.Mark(pValue); }
		static void MarkDouble (DWORD dwID) { m_DoubleAlloc.Mark(dwID); }
		static void MarkString (LPSTR Value) { m_StringAlloc.Mark(Value); }

		static void RegisterMarkProc (MARKPROC fnProc) { m_MarkList.Insert(fnProc); }

		static void Sweep ();

	private:
		static TAllocatorGC<double> m_DoubleAlloc;
		static CGCStringAllocator m_StringAlloc;
		static CGCComplexAllocator m_ComplexAlloc;

		static TArray<MARKPROC> m_MarkList;
	};

class CAEONFactoryList
	{
	public:
		~CAEONFactoryList (void);

		bool FindFactory (const CString &sTypename, IComplexFactory **retpFactory);
		inline void RegisterFactory (const CString &sTypename, IComplexFactory *pFactory) { m_List.Insert(sTypename, pFactory); }

	private:
		TSortMap<CString, IComplexFactory *> m_List;
	};
