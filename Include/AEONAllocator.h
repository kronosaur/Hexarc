//	AEONAllocator.h
//
//	AEON header file
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by AEON.h

#pragma once

#ifdef DEBUG
//#define DEBUG_ALLOCATOR
#endif

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
		TGCAllocator (void)
			{
			ASSERT(sizeof(VALUE) >= sizeof(DWORD));
			}

		TGCAllocator (const TGCAllocator &Src) = delete;
		TGCAllocator (TGCAllocator &&Src) = delete;

		~TGCAllocator ()
			{
			for (int i = 0; i < m_Backbone.GetCount(); i++)
				delete m_Backbone[i];
			}

		TGCAllocator &operator= (const TGCAllocator &Src) = delete;
		TGCAllocator &operator= (TGCAllocator &&Src) = delete;

		DWORD New (VALUE pValue)
			{
			CSmartLock Lock(m_cs);

			//	Get the new allocation

			DWORD dwNewID = GetNextFree();
			SSegment& Seg = GetSegment(dwNewID);
			VALUE *pNewValue = GetValue(dwNewID);

			//	Set the free list. The value at dwNewID is an index
			//	to the next free item in the list.

			Seg.dwCount++;
			Seg.dwFirstFree = ValueToIndex(*pNewValue);

			//	If this segment has run out of free entries, we look for another
			//	segment with free entries.

			if (Seg.dwFirstFree == END_OF_FREE_LIST)
				m_iNextFreeSegment = CalcNextFreeSegment();

			//	Set the value

			*pNewValue = pValue;

			//	Done

			return dwNewID;
			}

		void Sweep (void)
			{
			CSmartLock Lock(m_cs);

#ifdef DEBUG_ALLOCATOR
			int iMarked = 0;
			int iDeleted = 0;
			int iFree = 0;
#endif
			for (int i = 0; i < m_Backbone.GetCount(); i++)
				{
				if (m_Backbone[i] == NULL)
					continue;

				VALUE *pStart = m_Backbone[i]->Values;
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

							*pPos = IndexToValue(m_Backbone[i]->dwFirstFree);

							//	First free is now this item

							m_Backbone[i]->dwFirstFree = (i * SEGMENT_SIZE) + (DWORD)(pPos - pStart);
							m_Backbone[i]->dwCount--;
#ifdef DEBUG_ALLOCATOR
							iDeleted++;
#endif
							}
						else
							{
							ACTUAL::ClearMark(*pPos);

#ifdef DEBUG_ALLOCATOR
							iMarked++;
#endif
							}
						}
					else
						{
#ifdef DEBUG_ALLOCATOR
						iFree++;
#endif							
						}

					pPos++;
					}

				//	If the segment is free, then we need to delete it

				if (m_Backbone[i]->dwCount == 0)
					{
#ifdef DEBUG_ALLOCATOR
					printf("[%llx]: Deleting segment %d\n", (DWORD_PTR)this, i);
#endif
					delete m_Backbone[i];
					m_Backbone[i] = NULL;
					m_iNextFreeSegment = -1;
					}
				}

			if (m_iNextFreeSegment == -1)
				m_iNextFreeSegment = CalcNextFreeSegment();

#ifdef DEBUG_ALLOCATOR
			printf("[%llx] Sweep: %s marked; %s deleted; %s free\n", 
					(DWORD_PTR)this,
					(LPCSTR)strFormatInteger(iMarked, -1, FORMAT_THOUSAND_SEPARATOR), 
					(LPCSTR)strFormatInteger(iDeleted, -1, FORMAT_THOUSAND_SEPARATOR),
					(LPCSTR)strFormatInteger(iFree, -1, FORMAT_THOUSAND_SEPARATOR));
#endif
			}

		void DumpStats () const
			{
			CSmartLock Lock(m_cs);

			int iTotalAlloc = 0;
			int iTotalFree = 0;
			typename ACTUAL::SStats Stats;

			for (int i = 0; i < m_Backbone.GetCount(); i++)
				{
				if (m_Backbone[i] == NULL)
					continue;

				VALUE *pStart = m_Backbone[i]->Values;
				VALUE *pPos = pStart;
				VALUE *pEnd = pPos + SEGMENT_SIZE;

				while (pPos < pEnd)
					{
					if (!IsFree(*pPos))
						ACTUAL::AccumulateStats(Stats, *pPos);
					else
						iTotalFree++;

					pPos++;
					}
				}

			ACTUAL::DumpStats(Stats);
			}

	private:

		static constexpr int SEGMENT_SIZE = 8096;
		static constexpr DWORD END_OF_FREE_LIST = 0xFFFFFFFF;

		struct SSegment
			{
			DWORD dwCount = 0;
			DWORD dwFirstFree = END_OF_FREE_LIST;
			VALUE Values[SEGMENT_SIZE];
			};

		void AllocSeg (void)
			{
			ASSERT(m_iNextFreeSegment == -1);

			//	Figure out the Segment index of this segment. It will either be 
			//	at a previously freed segment or at the end of the backbone.

			int iSegIndex = -1;
			for (int i = 0; i < m_Backbone.GetCount(); i++)
				if (m_Backbone[i] == NULL)
					{
					iSegIndex = i;
					break;
					}

			if (iSegIndex == -1)
				{
				iSegIndex = m_Backbone.GetCount();
				m_Backbone.Insert(NULL);
				}

			DWORD dwBase = iSegIndex * SEGMENT_SIZE;

			//	Allocate a new segment 

			SSegment *pNewSeg = new SSegment;

			//	Chain all the entries together into a free list.

			pNewSeg->dwFirstFree = dwBase;
			for (int i = 0; i < SEGMENT_SIZE - 1; i++)
				pNewSeg->Values[i] = IndexToValue(dwBase + i + 1);

			//	The last entry points to the end of the free list

			pNewSeg->Values[SEGMENT_SIZE - 1] = IndexToValue(END_OF_FREE_LIST);

			//	Insert

			m_iNextFreeSegment = iSegIndex;
			m_Backbone[iSegIndex] = pNewSeg;

#ifdef DEBUG_ALLOCATOR
			printf("[%llx]: Allocating segment %d\n", (DWORD_PTR)this, iSegIndex);
#endif
			}

		int CalcNextFreeSegment () const
			{
			for (int i = 0; i < m_Backbone.GetCount(); i++)
				if (m_Backbone[i] && m_Backbone[i]->dwFirstFree != END_OF_FREE_LIST)
					return i;

			return -1;
			}

		DWORD GetNextFree ()
			{
			if (m_iNextFreeSegment == -1)
				AllocSeg();

			ASSERT(m_Backbone[m_iNextFreeSegment]->dwFirstFree != END_OF_FREE_LIST);
			return m_Backbone[m_iNextFreeSegment]->dwFirstFree;
			}

		SSegment& GetSegment (DWORD dwID) { return *m_Backbone[dwID / SEGMENT_SIZE]; }
		VALUE *GetValue (DWORD dwID) { return m_Backbone[dwID / SEGMENT_SIZE]->Values + (dwID % SEGMENT_SIZE); }
		VALUE IndexToValue (DWORD dwID) { return (dwID == END_OF_FREE_LIST ? (VALUE)(DWORD_PTR)END_OF_FREE_LIST : ((VALUE)(DWORD_PTR)((dwID << 1) | 0x01))); }
		bool IsFree (VALUE pPointer) const { return (((DWORD)(DWORD_PTR)pPointer) & 0x01); }
		DWORD ValueToIndex (VALUE pPointer) { return (((DWORD)(DWORD_PTR)pPointer) == END_OF_FREE_LIST ? END_OF_FREE_LIST : (((DWORD)(DWORD_PTR)pPointer) >> 1)); }

		CCriticalSection m_cs;
		TArray<SSegment *> m_Backbone;
		int m_iNextFreeSegment = -1;
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

		struct SStats
			{
			TSortMap<CString, int> TypeCount;
			TSortMap<int, int> ArraySizeHistogram;
			};

		void Mark (IComplexDatum *pValue) { pValue->Mark(); }

	protected:

		void ClearMark (IComplexDatum *pValue) { pValue->ClearMark(); }
		void FreeValue (IComplexDatum *pValue) { delete pValue; }
		bool IsMarked (IComplexDatum *pValue) { return pValue->IsMarked(); }

		void AccumulateStats (SStats &Stats, IComplexDatum *pValue) const
			{
			CString sType = pValue->GetTypename();
			bool bNew;
			int *pCount = Stats.TypeCount.SetAt(sType, &bNew);
			if (bNew)
				*pCount = 1;
			else
				(*pCount)++;

			if (strEquals(sType, CString("array")))
				{
				bool bNew;
				int* pSize = Stats.ArraySizeHistogram.SetAt(pValue->GetCount(), &bNew);
				if (bNew)
					*pSize = 1;
				else
					(*pSize)++;

				if (pValue->GetCount() == 3 && mathRandom(1, 1000) == 1)
					printf("%s\n", (LPCSTR)pValue->AsString());
				}
			}

		void DumpStats (const SStats &Stats) const
			{
			for (int i = 0; i < Stats.TypeCount.GetCount(); i++)
				printf("%s: %s\n", (LPCSTR)Stats.TypeCount.GetKey(i), (LPCSTR)strFormatInteger(Stats.TypeCount[i], -1, FORMAT_THOUSAND_SEPARATOR));

			for (int i = 0; i < Stats.ArraySizeHistogram.GetCount(); i++)
				printf("Array size: %d: %d\n", Stats.ArraySizeHistogram.GetKey(i), Stats.ArraySizeHistogram[i]);
			}
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

#ifdef DEBUG_ALLOCATOR
			int iMarked = 0;
			int iDeleted = 0;
			int iFree = 0;
#endif

			for (i = 0; i < m_MetaBack.GetCount(); i++)
				{
				BYTE *pMetaArray = m_MetaBack[i];

				for (j = 0; j < SEGMENT_SIZE; j++)
					if (!(pMetaArray[j] & FLAG_FREE))
						{
						if (pMetaArray[j] & FLAG_MARKED)
							{
							pMetaArray[j] = FLAG_NONE;
#ifdef DEBUG_ALLOCATOR
							iMarked++;
#endif
							}
						else
							{
							Delete(i * SEGMENT_SIZE + j);
#ifdef DEBUG_ALLOCATOR
							iDeleted++;
#endif
							}
						}
					else
						{
#ifdef DEBUG_ALLOCATOR
						iFree++;
#endif
						}
				}

#ifdef DEBUG_ALLOCATOR
			printf("Sweep: %d marked; %d deleted; %d free\n", iMarked, iDeleted, iFree);
#endif
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

class CAEONTableTable
	{
	public:

		static constexpr DWORD INVALID_INDEX = 0xFFFFFFFF;

		CAEONTableTable () { }
		~CAEONTableTable ()
			{
			for (int i = 0; i < m_Backbone.GetCount(); i++)
				delete [] m_Backbone[i];
			}

		CAEONTableTable (const CAEONTableTable& Src) = delete;
		CAEONTableTable (CAEONTableTable&& Src) = delete;
		CAEONTableTable& operator= (const CAEONTableTable& Src) = delete;
		CAEONTableTable& operator= (CAEONTableTable&& Src) = delete;

		DWORD AddTable (CDatum dTable)
			{
			CSmartLock Lock(m_cs);

			if (m_FreeList.GetCount() == 0)
				{
				if (!AllocSegment())
					return INVALID_INDEX;
				}

			int iIndex = m_FreeList[m_FreeList.GetCount() - 1];
			m_FreeList.Delete(m_FreeList.GetCount() - 1);

			DWORD dwID = (DWORD)iIndex;
			GetEntry(dwID)->dTable = dTable;
			return dwID;
			}

		void DeleteTable (DWORD dwID)
			{
			//	No need to lock because this only happens during GC.

			int iIndex = (int)dwID;
			if (iIndex < 0 || iIndex >= GetAllocSize())
				return;

			GetEntry(dwID)->dTable = CDatum();
			m_FreeList.Insert(iIndex);
			}

		CDatum GetTable (DWORD dwID) const
			{
			//	No need to lock because segments can't change and are never deleted.

			if ((int)dwID >= GetAllocSize())
				return CDatum();

			return GetEntry(dwID)->dTable;
			}

	private:

		static constexpr DWORD MAX_ID = 0xFFFF;
		static constexpr int SEGMENT_SIZE = 256;

		struct SEntry
			{
			CDatum dTable;
			};

		bool AllocSegment ()
			{
			if (m_Backbone.GetCount() >= (MAX_ID / SEGMENT_SIZE))
				return false;

			SEntry* pNewSeg = new SEntry[SEGMENT_SIZE];
			m_Backbone.Insert(pNewSeg);
			int iBase = (m_Backbone.GetCount() - 1) * SEGMENT_SIZE;
			for (int i = 0; i < SEGMENT_SIZE; i++)
				m_FreeList.Insert(iBase + i);

			return true;
			}

		int GetAllocSize () const { return m_Backbone.GetCount() * SEGMENT_SIZE; }
		SEntry* GetEntry (DWORD dwID) { return &m_Backbone[dwID / SEGMENT_SIZE][dwID % SEGMENT_SIZE]; }
		const SEntry* GetEntry (DWORD dwID) const { return &m_Backbone[dwID / SEGMENT_SIZE][dwID % SEGMENT_SIZE]; }

		CCriticalSection m_cs;
		TArray<SEntry*> m_Backbone;
		TArray<int> m_FreeList;
	};

class CAEONStore
	{
	public:

		static DWORD Alloc (IComplexDatum* pValue) { return m_ComplexAlloc.New(pValue); }
		static DWORD Alloc (LPSTR pValue) { return m_StringAlloc.New(pValue); }

		static DWORD AllocTableID (CDatum dTable) { return m_TableTable.AddTable(dTable); }
		static void FreeTableID (DWORD dwID) { m_TableTable.DeleteTable(dwID); }
		static CDatum GetTableByID (DWORD dwID) { return m_TableTable.GetTable(dwID); }

		static void MarkComplex (IComplexDatum* pValue) { m_ComplexAlloc.Mark(pValue); }
		static void MarkString (LPSTR Value) { m_StringAlloc.Mark(Value); }

		static void RegisterMarkProc (MARKPROC fnProc) { m_MarkList.Insert(fnProc); }

		static void Sweep ();

	private:

		static CGCStringAllocator m_StringAlloc;
		static CGCComplexAllocator m_ComplexAlloc;

		static TArray<MARKPROC> m_MarkList;
		static CAEONTableTable m_TableTable;
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
