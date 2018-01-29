//	FoundationArrays.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

enum ESortOptions
	{
	DescendingSort = 1,
	AscendingSort = -1,
	};

//	Because we are overriding new, we can't use the
//	new defined while debugging memory leaks

#ifdef DEBUG_MEMORY_LEAKS
#undef new
#endif

//	Explicit placement operator
#ifndef __PLACEMENT_NEW_INLINE
inline void *operator new (size_t, void *p) { return p; }
//	Avoid conflict with MS include
#define __PLACEMENT_NEW_INLINE
#endif

const int DEFAULT_ARRAY_GRANULARITY = 10;

class CArrayBase
	{
	protected:
		CArrayBase (HANDLE hHeap, int iGranularity);
		~CArrayBase (void);

		void CopyOptions (const CArrayBase &Src);
		void DeleteBytes (int iOffset, int iLength);
		inline char *GetBytes (void) const { return (m_pBlock ? (char *)(&m_pBlock[1]) : NULL); }
		inline int GetGranularity (void) const { return (m_pBlock ? m_pBlock->m_iGranularity : DEFAULT_ARRAY_GRANULARITY); }
		inline HANDLE GetHeap (void) const { return (m_pBlock ? m_pBlock->m_hHeap : ::GetProcessHeap()); }
		inline int GetSize (void) const { return (m_pBlock ? m_pBlock->m_iSize : 0); }
		void InsertBytes (int iOffset, void *pData, int iLength, int iAllocQuantum);
		void Resize (int iNewSize, bool bPreserve, int iAllocQuantum);
		void TakeHandoffBase (CArrayBase &Src);

	protected:
		struct SHeader
			{
			HANDLE m_hHeap;				//	Heap on which block is allocated
			int m_iSize;				//	Size of data portion (as seen by callers)
			int m_iAllocSize;			//	Current size of block
			int m_iGranularity;			//	Used by descendants to resize block
			};

		CArrayBase (SHeader *pBlock) : m_pBlock(pBlock)
			{ }

		SHeader *m_pBlock;
	};

#pragma warning(disable:4291)			//	No need for a delete because we're placing object

template <class VALUE> class TArray : public CArrayBase
	{
	public:
		typedef int (*COMPAREPROC)(void *pCtx, const VALUE &Key1, const VALUE &Key2);

		explicit TArray (HANDLE hHeap = NULL) : CArrayBase(hHeap, DEFAULT_ARRAY_GRANULARITY) { }
		explicit TArray (int iGranularity) : CArrayBase(NULL, iGranularity) { }
		TArray (const TArray<VALUE> &Obj) : CArrayBase(Obj.GetHeap(), Obj.GetGranularity())
			{
			InsertBytes(0, NULL, Obj.GetCount() * sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(GetBytes() + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}
			}
		TArray (TArray<VALUE> &&Src) : CArrayBase(Src.m_pBlock)
			{
			Src.m_pBlock = NULL;
			}

		~TArray (void) { DeleteAll(); }

		TArray<VALUE> &operator= (const TArray<VALUE> &Obj)
			{
			DeleteAll();

			CopyOptions(Obj);
			InsertBytes(0, NULL, Obj.GetCount() * sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			for (int i = 0; i < Obj.GetCount(); i++)
				{
				VALUE *pElement = new(GetBytes() + (i * sizeof(VALUE))) VALUE(Obj[i]);
				}

			return *this;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void Delete (int iIndex, int iCount = 1)
			{
			ASSERT(iIndex + iCount <= GetCount());

			VALUE *pElement = (VALUE *)(GetBytes() + iIndex * sizeof(VALUE));
			for (int i = 0; i < iCount; i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(iIndex * sizeof(VALUE), iCount * sizeof(VALUE));
			}

		void DeleteAll (void)
			{
			VALUE *pElement = (VALUE *)GetBytes();
			for (int i = 0; i < GetCount(); i++, pElement++)
				pElement->VALUE::~VALUE();

			DeleteBytes(0, GetSize());
			}

		int DeleteValue (const VALUE &ToDelete)
			{
			int iDeleted = 0;

			for (int i = 0; i < GetCount(); i++)
				if (GetAt(i) == ToDelete)
					{
					Delete(i);
					i--;
					iDeleted++;
					}

			return iDeleted;
			}

		void DeleteAllAndFreeValues (void)
			{
			for (int i = 0; i < GetCount(); i++)
				delete GetAt(i);

			DeleteAll();
			}

		bool Find (const VALUE &ToFind, int *retiIndex = NULL) const
			{
			int iCount = GetCount();

			for (int i = 0; i < iCount; i++)
				if (KeyCompare(GetAt(i), ToFind) == 0)
					{
					if (retiIndex)
						*retiIndex = i;
					return true;
					}

			return false;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			VALUE *pElement = (VALUE *)(GetBytes() + iIndex * sizeof(VALUE));
			return *pElement;
			}

		inline int GetCount (void) const
			{
			return GetSize() / sizeof(VALUE);
			}

		void GrowToFit (int iCount)
			{
			if (iCount > 0)
				Resize(GetSize() + iCount * sizeof(VALUE), true, GetGranularity() * sizeof(VALUE));
			}

		void Insert (const VALUE &Value, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			VALUE *pElement = new(GetBytes() + iOffset) VALUE(Value);
			}

		void Insert (const TArray<VALUE> &Array, int iIndex = -1)
			{
			if (Array.GetCount() > 0)
				{
				int iPos = (iIndex == -1 ? GetCount() : iIndex);
				InsertEmpty(Array.GetCount(), iIndex);

				for (int i = 0; i < Array.GetCount(); i++)
					GetAt(iPos + i) = Array[i];
				}
			}

		VALUE *Insert (void)
			{
			int iOffset = GetCount() * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			return new(GetBytes() + iOffset) VALUE;
			}

		VALUE *InsertAt (int iIndex)
			{
			if (iIndex == -1)
				iIndex = GetCount();
			else if (iIndex > GetCount())
				InsertEmpty(iIndex - GetCount());

			int iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			return new(GetBytes() + iOffset) VALUE;
			}

		void InsertEmpty (int iCount = 1, int iIndex = -1)
			{
			int iOffset;
			if (iIndex == -1) iIndex = GetCount();
			iOffset = iIndex * sizeof(VALUE);
			InsertBytes(iOffset, NULL, iCount * sizeof(VALUE), GetGranularity() * sizeof(VALUE));

			for (int i = 0; i < iCount; i++)
				{
				VALUE *pElement = new(GetBytes() + iOffset + (i * sizeof(VALUE))) VALUE;
				}
			}

		inline VALUE &Random (void) const
			{
			int iCount = GetCount();
			if (iCount == 0)
				return m_NullValue;

			VALUE *pElement = (VALUE *)(GetBytes() + mathRandom(0, iCount - 1) * sizeof(VALUE));
			return *pElement;
			}

		void Reverse (void)
			{
			int iSrc, iDest;
			int iCount = GetCount();

			TArray<VALUE> ReverseArray;
			ReverseArray.InsertEmpty(GetCount());
			for (iSrc = 0, iDest = iCount - 1; iSrc < iCount; iSrc++, iDest--)
				ReverseArray[iDest] = GetAt(iSrc);

			TakeHandoff(ReverseArray);
			}

		void Shuffle (void)
			{
			if (GetCount() < 2)
				return;

			//	Fisher-Yates algorithm

			int i = GetCount() - 1;
			while (i > 0)
				{
				int x = mathRandom(0, i);

				VALUE Temp = GetAt(x);
				GetAt(x) = GetAt(i);
				GetAt(i) = Temp;

				i--;
				}
			}

		void Sort (void *pCtx, COMPAREPROC pfCompare, ESortOptions Order = AscendingSort)
			{
			if (GetCount() < 2)
				return;

			TArray<int> Result;

			//	Binary sort the contents into an indexed array

			Result.GrowToFit(GetCount());
			SortRange(Order, 0, GetCount() - 1, pfCompare, pCtx, Result);

			//	Create a new sorted array

			TArray<VALUE> SortedArray;
			SortedArray.InsertEmpty(GetCount());
			for (int i = 0; i < GetCount(); i++)
				SortedArray[i] = GetAt(Result[i]);

			TakeHandoff(SortedArray);
			}

		void Sort (ESortOptions Order = AscendingSort)
			{
			Sort(NULL, DefaultCompare, Order);
			}

		void TakeHandoff (TArray<VALUE> &Obj)
			{
			DeleteAll();
			TakeHandoffBase(Obj);
			}

	private:
		static int DefaultCompare (void *pCtx, const VALUE &Key1, const VALUE &Key2)
			{
			return KeyCompare(Key1, Key2);
			}

		void SortRange (ESortOptions Order, int iLeft, int iRight, COMPAREPROC pfCompare, void *pCtx, TArray<int> &Result)
			{
			if (iLeft == iRight)
				Result.Insert(iLeft);
			else if (iLeft + 1 == iRight)
				{
				int iCompare = Order * pfCompare(pCtx, GetAt(iLeft), GetAt(iRight));
				if (iCompare == 1)
					{
					Result.Insert(iLeft);
					Result.Insert(iRight);
					}
				else
					{
					Result.Insert(iRight);
					Result.Insert(iLeft);
					}
				}
			else
				{
				int iMid = iLeft + ((iRight - iLeft) / 2);

				TArray<int> Buffer1;
				TArray<int> Buffer2;

				SortRange(Order, iLeft, iMid, pfCompare, pCtx, Buffer1);
				SortRange(Order, iMid+1, iRight, pfCompare, pCtx, Buffer2);

				//	Merge

				int iPos1 = 0;
				int iPos2 = 0;
				bool bDone = false;
				while (!bDone)
					{
					if (iPos1 < Buffer1.GetCount() && iPos2 < Buffer2.GetCount())
						{
						int iCompare = Order * pfCompare(pCtx, GetAt(Buffer1[iPos1]), GetAt(Buffer2[iPos2]));
						if (iCompare == 1)
							Result.Insert(Buffer1[iPos1++]);
						else if (iCompare == -1)
							Result.Insert(Buffer2[iPos2++]);
						else
							{
							Result.Insert(Buffer1[iPos1++]);
							Result.Insert(Buffer2[iPos2++]);
							}
						}
					else if (iPos1 < Buffer1.GetCount())
						Result.Insert(Buffer1[iPos1++]);
					else if (iPos2 < Buffer2.GetCount())
						Result.Insert(Buffer2[iPos2++]);
					else
						bDone = true;
					}
				}
			}

		static VALUE m_NullValue;
	};
#pragma warning(default:4291)

template<class VALUE> VALUE TArray<VALUE>::m_NullValue;

//	Simple array classes

class CStringArray : public TArray<CString> { };
class CIntArray : public TArray<int> { };

#ifdef DEBUG_MEMORY_LEAKS
#define new DEBUG_NEW
#endif

//	Queues

template <class VALUE> class TQueue
	{
	public:
		TQueue (int iSize = 100)
			{
			m_pArray = NULL;
			Init(iSize);
			}

		TQueue (const TQueue<VALUE> &Src)
			{
			int i;

			m_iSize = Src.m_iSize;
			m_pArray = new VALUE [m_iSize];
			m_iHead = Src.m_iHead;
			m_iTail = Src.m_iTail;

			for (i = 0; i < GetCount(); i++)
				GetAt(i) = Src.GetAt(i);
			}

		~TQueue (void)
			{
			if (m_pArray)
				delete [] m_pArray;
			}

		TQueue<VALUE> &operator= (const TQueue<VALUE> &Obj)
			{
			int i;

			Init(Obj.GetCapacity());

			m_iHead = Obj.m_iHead;
			m_iTail = Obj.m_iTail;

			for (i = 0; i < Obj.GetCount(); i++)
				GetAt(i) = Obj.GetAt(i);

			return *this;
			}

		inline VALUE &operator [] (int iIndex) const { return GetAt(iIndex); }

		void DeleteAll (void)
			{
			Dequeue(GetCount(), true);
			}

		void Dequeue (bool bDelete = false)
			{
			ASSERT(m_iHead != m_iTail);
			if (bDelete)
				m_pArray[m_iHead] = VALUE();

			m_iHead = (m_iHead + 1) % m_iSize;
			}

		void Dequeue (int iCount, bool bDelete = false)
			{
			if (iCount == 0)
				return;

			int i;
			ASSERT(iCount <= GetCount());
			if (bDelete)
				{
				for (i = 0; i < iCount; i++)
					GetAt(i) = VALUE();
				}

			m_iHead = (m_iHead + iCount) % m_iSize;
			}

		void Enqueue (void)
			{
			if (IsFull())
				throw CException(errOutOfMemory);

			m_iTail = (m_iTail + 1) % m_iSize;
			}

		void Enqueue (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				throw CException(errOutOfMemory);

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			}

		void EnqueueAndGrow (const VALUE &Value)
			{
			if (IsFull())
				GrowToFit(Max(1, Min(m_iSize, 1000)));

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			}

		void EnqueueAndOverwrite (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				m_iHead = (m_iHead + 1) % m_iSize;

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			}

		bool Find (const VALUE &ToFind) const
			{
			int i;
			for (i = 0; i < GetCount(); i++)
				if (KeyCompare(GetAt(i), ToFind) == 0)
					return true;

			return false;
			}

		inline VALUE &GetAt (int iIndex) const
			{
			ASSERT(iIndex < GetCount());
			return m_pArray[(m_iHead + iIndex) % m_iSize];
			}

		int GetCapacity (void) const
			{
			return m_iSize - 1;
			}

		int GetCount (void) const
			{
			if (m_iTail >= m_iHead)
				return (m_iTail - m_iHead);
			else
				return (m_iTail + m_iSize - m_iHead);
			}

		void GrowToFit (int iCount)
			{
			int i;

			if (m_iSize == 0)
				Init(iCount);
			else if (m_iTail >= m_iHead)
				{
				int iAvailable = (m_iSize - 1) - (m_iTail - m_iHead);
				if (iAvailable < iCount)
					{
					int iNeeded = iCount - iAvailable;
					int iNewSize = m_iSize + iNeeded;

					VALUE *pNewArray = new VALUE [iNewSize];
					for (i = m_iHead; i < m_iTail; i++)
						pNewArray[i] = m_pArray[i];

					delete [] m_pArray;
					m_pArray = pNewArray;
					m_iSize = iNewSize;
					}
				}
			else
				{
				int iAvailable = (m_iSize - 1) - (m_iTail + m_iSize - m_iHead);
				if (iAvailable < iCount)
					{
					int iNeeded = iCount - iAvailable;
					int iNewSize = m_iSize + iNeeded;

					VALUE *pNewArray = new VALUE [iNewSize];
					for (i = 0; i < m_iTail; i++)
						pNewArray[i] = m_pArray[i];

					for (i = m_iHead; i < m_iSize; i++)
						pNewArray[i + iNeeded] = m_pArray[i];

					delete [] m_pArray;
					m_iHead += iNeeded;
					m_pArray = pNewArray;
					m_iSize = iNewSize;
					}
				}
			}

		inline VALUE &Head (void) const
			{
			ASSERT(m_iHead != m_iTail);
			return m_pArray[m_iHead];
			}

		void Init (int iSize)
			{
			if (m_pArray)
				delete [] m_pArray;

			if (iSize > 0)
				{
				m_iSize = iSize + 1;
				m_pArray = new VALUE [m_iSize];
				}
			else
				{
				m_iSize = 0;
				m_pArray = NULL;
				}

			m_iHead = 0;
			m_iTail = 0;
			}

		inline bool IsEmpty (void) const
			{
			return (m_iHead == m_iTail);
			}

		inline bool IsFull (void) const
			{
			if (m_iSize == 0)
				return true;

			return (((m_iTail + 1) % m_iSize) == m_iHead);
			}

		inline VALUE &Tail (void) const
			{
			ASSERT(m_iHead != m_iTail);
			return m_pArray[(m_iTail + m_iSize - 1) % m_iSize];
			}

		bool TryEnqueue (const VALUE &Value)
			{
			if (((m_iTail + 1) % m_iSize) == m_iHead)
				return false;

			m_pArray[m_iTail] = Value;
			m_iTail = (m_iTail + 1) % m_iSize;
			return true;
			}

	private:
		VALUE *m_pArray;
		int m_iSize;
		int m_iHead;
		int m_iTail;
	};

template <class VALUE> class TSharedQueue
	{
	public:
		TSharedQueue (int iSize = 100) : m_Queue(iSize) { m_Event.Create(); }

		VALUE Dequeue (void)
			{
			CSmartLock Lock(m_cs);

			VALUE Result;

			if (m_Queue.GetCount())
				{
				Result = m_Queue.Head();
				m_Queue.Dequeue();

				//	If we pulled the last value, reset the event

				if (m_Queue.IsEmpty())
					m_Event.Reset();
				}

			return Result;
			}

		int Dequeue (int iCount, TArray<VALUE> *pList)
			{
			CSmartLock Lock(m_cs);
			int i;

			iCount = Min(iCount, m_Queue.GetCount());
			for (i = 0; i < iCount; i++)
				pList->Insert(m_Queue.GetAt(i));

			m_Queue.Dequeue(iCount);

			//	Reset event if we're empty

			if (m_Queue.IsEmpty())
				m_Event.Reset();

			return iCount;
			}

		void DequeueHandoff (VALUE *retpValue)
			{
			CSmartLock Lock(m_cs);

			if (m_Queue.GetCount())
				{
				retpValue->TakeHandoff(m_Queue.Head());
				m_Queue.Dequeue();

				if (m_Queue.GetCount() == 0)
					m_Event.Reset();
				}
			}

		bool Enqueue (const VALUE &Value)
			{
			CSmartLock Lock(m_cs);

			if (m_Queue.IsFull())
				return false;

			m_Queue.Enqueue(Value);
			if (m_Queue.GetCount() == 1)
				m_Event.Set();

			return true;
			}

		bool EnqueueHandoff (VALUE &Value)
			{
			CSmartLock Lock(m_cs);

			if (m_Queue.IsFull())
				return false;

			m_Queue.Enqueue();
			m_Queue.Tail().TakeHandoff(Value);
			if (m_Queue.GetCount() == 1)
				m_Event.Set();

			return true;
			}

		inline CManualEvent &GetEvent (void) { return m_Event; }

		inline bool IsEmpty (void) { return (::WaitForSingleObject(m_Event.GetEvent(), 0) == WAIT_TIMEOUT); }
		inline bool IsFull (void) { return m_Queue.IsFull(); }

	private:
		CCriticalSection m_cs;
		CManualEvent m_Event;
		TQueue<VALUE> m_Queue;
	};

//	Sets

struct SLargeSetEnumerator
	{
	DWORD dwDWORDIndex;						//	0xffffffff = no more
	DWORD dwBitIndex;
	DWORD dwBitMask;
	};

class CLargeSet
	{
	public:
		CLargeSet (int iSize = -1);

		void Clear (DWORD dwValue);
		void Clear (const CLargeSet &Src);
		void ClearAll (void);
		int GetCount (void) const;
		DWORD GetNext (SLargeSetEnumerator &i) const;
		void GetValues (TArray<DWORD> *retResult);
		bool HasMore (SLargeSetEnumerator &i) const;
		bool IsEmpty (void) const;
		bool IsSet (DWORD dwValue) const;
		void Reset (SLargeSetEnumerator &i) const;
		void Set (DWORD dwValue);
		void Set (const CLargeSet &Set);

	private:
		TArray<DWORD> m_Set;
	};

//	Sparse Array ---------------------------------------------------------------

template <class VALUE> class TSparseArray
	{
	public:
		TSparseArray (int iTotalSize, int iSubUnitSize, VALUE rInitialValue) :
				m_iTotalSize(iTotalSize),
				m_iSubUnitSize(iSubUnitSize),
				m_rInitialValue(rInitialValue)
			{
			int iBackboneSize = AlignUp(m_iTotalSize, m_iSubUnitSize) / m_iSubUnitSize;
			m_Array.InsertEmpty(iBackboneSize);
			}

		inline const VALUE &operator [] (int iIndex) const
			{
			ASSERT(iIndex < m_iTotalSize);
			TArray<VALUE> &SubUnit = m_Array[iIndex / m_iSubUnitSize];
			if (SubUnit.GetCount() == 0)
				return m_rInitialValue;
			else
				return SubUnit[iIndex % m_iSubUnitSize];
			}

		inline int GetCount (void) const { return m_iTotalSize; }

		inline VALUE &SetAt (int iIndex)
			{
			TArray<VALUE> &SubUnit = SetSubUnit(iIndex);
			return SubUnit[iIndex % m_iSubUnitSize];
			}

		inline void SetAt (int iIndex, VALUE rValue)
			{
			TArray<VALUE> &SubUnit = SetSubUnit(iIndex);
			SubUnit[iIndex % m_iSubUnitSize] = rValue;
			}

	private:
		TArray<VALUE> &SetSubUnit (int iIndex)
			{
			int i;
			ASSERT(iIndex < m_iTotalSize);

			TArray<VALUE> &SubUnit = m_Array[iIndex / m_iSubUnitSize];
			if (SubUnit.GetCount() == 0)
				{
				SubUnit.InsertEmpty(m_iSubUnitSize);
				for (i = 0; i < m_iSubUnitSize; i++)
					SubUnit[i] = m_rInitialValue;
				}

			return SubUnit;
			}

		TArray<TArray<VALUE>> m_Array;
		int m_iTotalSize;
		int m_iSubUnitSize;
		VALUE m_rInitialValue;
	};

//	Functions ------------------------------------------------------------------

const DWORD SSP_FLAG_WHITESPACE_SEPARATOR =		0x00000001;	//	Separate by whitespace
const DWORD SSP_FLAG_NO_EMPTY_ITEMS =			0x00000002;	//	No empty/blank items
const DWORD SSP_FLAG_LINE_SEPARATOR =			0x00000004;	//	Separate by line breaks
const DWORD SSP_FLAG_FORCE_LOWERCASE =			0x00000008;	//	Convert all to lowercase
const DWORD SSP_FLAG_ALPHANUMERIC_ONLY =		0x00000010;	//	Anything except alphanumeric is a separator
void strSplit (const CString &sString, const CString &sSeparators, TArray<CString> *retResult, int iMaxCount = -1, DWORD dwFlags = 0);
TArray<CString> strToLower (TArray<CString> &&List);
