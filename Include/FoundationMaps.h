//	FoundationMaps.h
//
//	Foundation header file
//	Copyright (c) 2017 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

//	Maps

const DWORD NULL_ATOM = 0xffffffff;

template <class KEY, class VALUE> class TSortMap
	{
	public:
		TSortMap (ESortOptions iOrder = AscendingSort) : m_iOrder(iOrder), m_Free(0) { }

		inline VALUE &operator [] (int iIndex) const { return GetValue(iIndex); }

		TSortMap<KEY, VALUE> &operator= (const TSortMap<KEY, VALUE> &Obj)
			{
			m_iOrder = Obj.m_iOrder;
			m_Index = Obj.m_Index;
			m_Array = Obj.m_Array;
			m_Free = Obj.m_Free;
			return *this;
			}

		void Delete (int iIndex)
			{
			ASSERT(iIndex >= 0 && iIndex < m_Index.GetCount());
			int iPos = m_Index[iIndex];
			m_Index.Delete(iIndex);

			//	Add the array slot to free list (we can't delete it
			//	because it would move the indices of other entries)

			m_Array[iPos].theValue = VALUE();
			m_Free.EnqueueAndGrow(iPos);
			}

		void DeleteAll (void)
			{
			m_Index.DeleteAll();
			m_Array.DeleteAll();
			m_Free.DeleteAll();
			}

		void DeleteAt (const KEY &key)
			{
			int iPos;
			if (FindPos(key, &iPos))
				Delete(iPos);
			}

		bool Find (const KEY &key, VALUE *retpValue = NULL) const
			{
			int iPos;
			if (!FindPos(key, &iPos))
				return false;

			if (retpValue)
				*retpValue = GetValue(iPos);

			return true;
			}

		bool FindPos (const KEY &key, int *retiPos = NULL) const
			{
			int iCount = m_Index.GetCount();
			int iMin = 0;
			int iMax = iCount;
			int iTry = iMax / 2;

			while (true)
				{
				if (iMax <= iMin)
					{
					if (retiPos)
						*retiPos = iMin;
					return false;
					}

				int iCompare = m_iOrder * KeyCompare(key, GetKey(iTry));
				if (iCompare == 0)
					{
					if (retiPos)
						*retiPos = iTry;
					return true;
					}
				else if (iCompare == -1)
					{
					iMin = iTry + 1;
					iTry = iMin + (iMax - iMin) / 2;
					}
				else if (iCompare == 1)
					{
					iMax = iTry;
					iTry = iMin + (iMax - iMin) / 2;
					}
				}

			return false;
			}

		VALUE *GetAt (const KEY &key) const
			{
			int iPos;
			if (!FindPos(key, &iPos))
				return NULL;

			return &m_Array[m_Index[iPos]].theValue;
			}

		int GetCount (void) const
			{
			return m_Index.GetCount();
			}

		const KEY &GetKey (int iIndex) const
			{
			return m_Array[m_Index[iIndex]].theKey;
			}

		VALUE &GetValue (int iIndex) const
			{
			return m_Array[m_Index[iIndex]].theValue;
			}

		void GrowToFit (int iCount)
			{
			int i;

			int iPos = m_Array.GetCount();
			m_Array.InsertEmpty(iCount);
			m_Free.GrowToFit(iCount);
			for (i = 0; i < iCount; i++)
				m_Free.Enqueue(iPos + i);

			m_Index.GrowToFit(iCount);
			}

		VALUE *Insert (const KEY &newKey)
			{
			return atom_Insert(newKey);
			}

		void Insert (const KEY &newKey, const VALUE &newValue)
			{
			VALUE *pNewValue = atom_Insert(newKey);
			*pNewValue = newValue;
			}

		void Insert (const TSortMap<KEY, VALUE> &Src)
			{
			int i;

			for (i = 0; i < Src.GetCount(); i++)
				Insert(Src.GetKey(i), Src.GetValue(i));
			}

		void InsertSorted (const KEY &newKey, const VALUE &newValue, int iPos = -1)
			{
			//	Find where to insert it in the array

			int iInsertPos;
			SEntry *pEntry = InsertEntry(&iInsertPos);

			//	Insert in the index

			m_Index.Insert(iInsertPos, iPos);
			pEntry->theKey = newKey;
			pEntry->theValue = newValue;
			}

		VALUE *SetAt (const KEY &key, bool *retbInserted = NULL, DWORD *retdwAtom = NULL)
			{
			int iIndex;

			if (FindPos(key, &iIndex))
				{
				int iPos = m_Index[iIndex];

				if (retbInserted)
					*retbInserted = false;

				if (retdwAtom)
					*retdwAtom = (DWORD)iPos;

				return &m_Array[iPos].theValue;
				}
			else
				{
				int iPos;
				SEntry *pEntry = InsertEntry(&iPos);

				//	Do it

				m_Index.Insert(iPos, iIndex);
				pEntry->theKey = key;

				if (retbInserted)
					*retbInserted = true;

				if (retdwAtom)
					*retdwAtom = (DWORD)iPos;

				return &pEntry->theValue;
				}
			}

		void SetAt (const KEY &key, const VALUE &value, bool *retbInserted = NULL)
			{
			VALUE *pValue = SetAt(key, retbInserted);
			*pValue = value;
			}

		//	Atom helper functions

		void atom_Delete (DWORD dwAtom)
			{
			int i;

			//	Look for the array position in the index and delete it.

			for (i = 0; i < m_Index.GetCount(); i++)
				if (m_Index[i] == (int)dwAtom)
					{
					m_Index.Delete(i);

					//	Add the array slot to the free list

					m_Array[dwAtom].theValue = VALUE();
					m_Free.EnqueueAndGrow(dwAtom);
					break;
					}
			}

		DWORD atom_Find (const KEY &key)
			{
			int iPos;
			if (!FindPos(key, &iPos))
				return NULL_ATOM;

			return (DWORD)m_Index[iPos];
			}

		DWORD atom_GetFromPos (int iPos) const
			{
			return m_Index[iPos];
			}

		const KEY &atom_GetKey (DWORD dwAtom) const
			{
			return m_Array[dwAtom].theKey;
			}

		VALUE &atom_GetValue (DWORD dwAtom) const
			{
			return m_Array[dwAtom].theValue;
			}

		VALUE *atom_InsertExisting (const KEY &key, DWORD *retdwAtom = NULL)
			{
			int iIndex;

			if (FindPos(key, &iIndex))
				return NULL;

			int iPos;
			SEntry *pEntry = InsertEntry(&iPos);

			//	Do it

			m_Index.Insert(iPos, iIndex);
			pEntry->theKey = key;

			if (retdwAtom)
				*retdwAtom = (DWORD)iPos;

			return &pEntry->theValue;
			}

		bool atom_IsValid (DWORD dwAtom) const
			{
			int i;

			if (dwAtom >= (DWORD)m_Array.GetCount())
				return false;

			//	Make sure the entry is not deleted.

			for (i = 0; i < m_Free.GetCount(); i++)
				if (m_Free[i] == (int)dwAtom)
					return false;

			//	Valid

			return true;
			}

	private:
		struct SEntry
			{
			KEY theKey;
			VALUE theValue;
			};

		VALUE *atom_Insert (const KEY &newKey, DWORD *retdwAtom = NULL)
			{
			//	Find index insertion position

			int iIndex;
			FindPos(newKey, &iIndex);

			//	Find where to insert it in the array

			int iPos;
			SEntry *pEntry = InsertEntry(&iPos);

			//	Do it

			if (retdwAtom)
				*retdwAtom = (DWORD)iPos;

			m_Index.Insert(iPos, iIndex);
			pEntry->theKey = newKey;
			return &pEntry->theValue;
			}

		SEntry *InsertEntry (int *retiPos)
			{
			SEntry *pEntry;
			if (m_Free.GetCount() == 0)
				{
				*retiPos = m_Array.GetCount();
				pEntry = m_Array.Insert();
				}
			else
				{
				int iFreePos = m_Free.Head();
				m_Free.Dequeue();

				*retiPos = iFreePos;
				pEntry = &m_Array[iFreePos];
				}
			return pEntry;
			}

		ESortOptions m_iOrder;
		TArray<int> m_Index;
		TArray<SEntry> m_Array;
		TQueue<int> m_Free;
	};

template <class VALUE> class TAtomTable
	{
	public:
		TAtomTable (ESortOptions iOrder = AscendingSort) : m_Map(iOrder) { }

		inline VALUE &operator [] (DWORD dwAtom) const { return GetAt(dwAtom); }

		DWORD Atomize (const CString &sKey) const
			{
			int iPos;
			if (!m_Map.FindPos(sKey, &iPos))
				return NULL_ATOM;

			return m_Map.atom_GetFromPos(iPos);
			}

		const CString &Deatomize (DWORD dwAtom) const
			{
			return (dwAtom == NULL_ATOM ? NULL_STR : m_Map.atom_GetKey(dwAtom));
			}

		void Delete (DWORD dwAtom) { if (dwAtom != NULL_ATOM) m_Map.atom_Delete(dwAtom); }
		void DeleteAll (void) { m_Map.DeleteAll(); }
		int GetCount (void) const { return m_Map.GetCount(); }
		VALUE &GetAt (DWORD dwAtom) const { return (dwAtom != NULL_ATOM ? m_Map.atom_GetValue(dwAtom) : m_Null); }
		DWORD GetAtom (int iIndex) const { return m_Map.atom_GetFromPos(iIndex); }
		const CString &GetKey (int iIndex) const { return m_Map.GetKey(iIndex); }
		VALUE &GetValue (int iIndex) const { return m_Map.GetValue(iIndex); }
		VALUE *Insert (const CString &newKey, DWORD *retdwAtom = NULL) { return m_Map.SetAt(newKey, (bool *)NULL, retdwAtom); }

		DWORD Insert (const CString &newKey, const VALUE &newValue)
			{
			DWORD dwAtom;
			VALUE *pNewValue = m_Map.SetAt(newKey, (bool *)NULL, &dwAtom);
			*pNewValue = newValue;
			return dwAtom;
			}

		DWORD InsertExisting (const CString &newKey, const VALUE &newValue)
			{
			DWORD dwAtom;
			VALUE *pNewValue = m_Map.atom_InsertExisting(newKey, &dwAtom);
			if (pNewValue == NULL)
				return NULL_ATOM;

			*pNewValue = newValue;
			return dwAtom;
			}

		bool IsValid (DWORD dwAtom) const { return m_Map.atom_IsValid(dwAtom); }
		void SetAt (DWORD dwAtom, const VALUE &value) { GetAt(dwAtom) = value; }

	private:
		TSortMap<CString, VALUE> m_Map;

		static VALUE m_Null;
	};

template <class VALUE> VALUE TAtomTable<VALUE>::m_Null = VALUE();

struct SIDTableEnumerator
	{
	int iPos;
	};

template <class VALUE> class TIDTable
	{
	public:
		TIDTable (void) { }
		TIDTable (int iGranularity) : m_Array(iGranularity), m_iCustomID(1) { }
		TIDTable (const TIDTable<VALUE> &Obj) : m_Array(Obj.m_Array), m_Free(Obj.m_Free) { }

		~TIDTable (void) { DeleteAll(); }

		TIDTable<VALUE> &operator= (const TIDTable<VALUE> &Obj)
			{
			m_Array = Obj.m_Array;
			m_Free = Obj.m_Free;
			return *this;
			}

		inline VALUE &operator [] (DWORD dwID) const { return GetAt(dwID); }

		void Delete (DWORD dwID, const VALUE &DeleteValue = VALUE())
			{
			SEntry *pEntry = &m_Array[GetIndex(dwID)];
			if (pEntry->dwID == dwID)
				{
				pEntry->dwID = 0;
				pEntry->Value = DeleteValue;

				m_Free.Insert(dwID);
				}
			}

		void DeleteAll (void)
			{
			m_Array.DeleteAll();
			m_Free.DeleteAll();
			}

		void DeleteAllAndFreeValues (void)
			{
			SIDTableEnumerator i;
			Reset(i);
			while (HasMore(i))
				delete GetNext(i);

			DeleteAll();
			}

		inline VALUE &GetAt (DWORD dwID) const
			{
			return m_Array[GetIndex(dwID)].Value;
			}

		int GetCount (void) const
			{
			int i;
			int iCount = 0;

			for (i = 0; i < m_Array.GetCount(); i++)
				if (m_Array[i].dwID != 0)
					iCount++;

			return iCount;
			}

		VALUE &GetNext (SIDTableEnumerator &i) const
			{
			ASSERT(i.iPos != -1);
			VALUE &theValue = m_Array[i.iPos].Value;

			Next(i);

			return theValue;
			}

		inline bool HasMore (SIDTableEnumerator &i) const
			{
			return (i.iPos != -1);
			}

		void Insert (const VALUE &Value, DWORD *retdwID)
			{
			*Insert(retdwID) = Value;
			}

		VALUE *Insert (DWORD *retdwID)
			{
			DWORD dwID;
			int iIndex;

			int iFreeCount = m_Free.GetCount();
			if (iFreeCount > 0)
				{
				DWORD dwFree = m_Free[iFreeCount - 1];
				m_Free.Delete(iFreeCount - 1);
				iIndex = GetIndex(dwFree);
				dwID = MakeID(iIndex, ((GetCounter(dwFree) + 1) % 255) + 1);
				}
			else
				{
				iIndex = m_Array.GetCount();
				dwID = MakeID(m_Array.GetCount(), 1);

				if (iIndex > 0x00ffffff)
					throw CException(errOutOfMemory);

				m_Array.Insert();
				}

			m_Array[iIndex].dwID = dwID;

			if (retdwID)
				*retdwID = dwID;

			return &m_Array[iIndex].Value;
			}

		inline bool IsValid (DWORD dwID) const
			{
			int iIndex = GetIndex(dwID);
			return (iIndex >= 0 && iIndex < m_Array.GetCount() && m_Array[iIndex].dwID == dwID);
			}

		DWORD MakeCustomID (void)
			{
			//	IDs have the following format:
			//
			//	0x CC IIIIII
			//
			//	CC = 00, IIIIII = 0 : NULL ID
			//	CC = 00, IIIIII > 0 : custom ID
			//	CC = 01-FF			: index ID

			DWORD dwID = MakeID(m_iCustomID, 0);

			if (m_iCustomID == 0xffffff)
				m_iCustomID = 1;
			else
				m_iCustomID += 1;

			return dwID;
			}

		void Next (SIDTableEnumerator &i) const
			{
			int j;

			i.iPos++;
			for (j = i.iPos; j < m_Array.GetCount(); j++)
				if (m_Array[j].dwID != 0)
					{
					i.iPos = j;
					return;
					}

			i.iPos = -1;
			}

		void Reset (SIDTableEnumerator &i) const
			{
			i.iPos = -1;
			Next(i);
			}

		VALUE &SetAt (DWORD dwID)
			{
			int i;
			int iCount = m_Array.GetCount();
			int iIndex = GetIndex(dwID);

			if (iIndex >= iCount)
				{
				int iExtra = dwID - iCount;
				m_Array.InsertEmpty(iExtra + 1);

				for (i = 0; i < iExtra; i++)
					{
					m_Array[iCount + i].dwID = 0;
					m_Free.Insert(MakeID(iCount + i, 1));
					}
				}

			m_Array[iIndex].dwID = dwID;
			return m_Array[iIndex].Value;
			}

		void TakeHandoff (TIDTable<VALUE> &Obj)
			{
			m_Array.TakeHandoff(Obj.m_Array);
			m_Free.TakeHandoff(Obj.m_Free);
			}

	private:
		struct SEntry
			{
			DWORD dwID;
			VALUE Value;
			};

		inline int GetCounter (DWORD dwID) const { return (dwID >> 24); }
		inline int GetIndex (DWORD dwID) const { return (dwID & 0x00ffffff); }
		inline DWORD MakeID (int iIndex, int iCounter) const { return ((((DWORD)iCounter) << 24) | (iIndex & 0x00ffffff)); }

		TArray<SEntry> m_Array;
		TArray<DWORD> m_Free;
		int m_iCustomID;
	};

#if 0
template <class VALUE> class TIDArray
	{
	public:
		~TIDArray (void) { }

		inline VALUE &operator [] (int iIndex) const { return GetAt((DWORD)iIndex); }

		void Delete (DWORD dwID)
			{
			m_Array[dwID] = VALUE();
			m_Free.Insert(dwID);
			}

		void DeleteAll (void)
			{
			m_Array.DeleteAll();
			m_Free.DeleteAll();
			}

		VALUE &GetAt (DWORD dwID) const { return m_Array[dwID]; }

		int GetCount (void) const { return m_Array.GetCount(); }

		void Insert (const VALUE &Value, DWORD *retdwID = NULL)
			{
			int iIndex;

			//	If there is a free entry, use that

			int iLastFree = m_Free.GetCount() - 1;
			if (iLastFree >= 0)
				{
				iIndex = m_Free[iLastFree];
				m_Array[iIndex] = Value;
				m_Free.Delete(iLastFree);
				}

			//	Otherwise, allocate a new entry

			else
				{
				iIndex = m_Array.GetCount();
				m_Array.Insert(Value);
				}

			if (retdwID)
				*retdwID = (DWORD)iIndex;
			}

		void InsertEmpty (int iCount = 1)
			{
			int i;

			int iIndex = m_Array.GetCount();
			m_Array.InsertEmpty(iCount);

			for (i = 0; i < iCount; i++)
				m_Free.Insert(iIndex++);
			}

		bool IsValid (DWORD dwID)
			{
			int i;

			if ((int)dwID >= m_Array.GetCount())
				return false;

			for (i = 0; i < m_Free.GetCount(); i++)
				if ((DWORD)m_Free[i] == dwID)
					return false;

			return true;
			}

	private:
		TArray<VALUE> m_Array;
		TArray<int> m_Free;
	};
#endif

template <class VALUE> class TObjArray
	{
	public:
		~TObjArray (void) { DeleteAll(); }

		inline VALUE *operator [] (DWORD dwID) const { return GetAt(dwID); }

		void Delete (DWORD dwID)
			{
			VALUE *pObj = Remove(dwID);
			if (pObj)
				delete pObj;
			}

		void DeleteAll (void)
			{
			int i;

			for (i = 0; i < m_Array.GetCount(); i++)
				{
				VALUE *pObj = m_Array[i];
				if (pObj)
					delete pObj;
				}

			m_Array.DeleteAll();
			m_Free.DeleteAll();
			}

		VALUE *GetAt (DWORD dwID) const { return (IsValidID(dwID) ? m_Array[dwID] : NULL); }

		int GetCount (void) const { return m_Array.GetCount(); }

		void Insert (VALUE *pObj, DWORD *retdwID = NULL)
			{
			int iIndex;

			//	If there is a free entry, use that

			int iLastFree = m_Free.GetCount() - 1;
			if (iLastFree >= 0)
				{
				iIndex = m_Free[iLastFree];
				m_Array[iIndex] = pObj;
				m_Free.Delete(iLastFree);
				}

			//	Otherwise, allocate a new entry

			else
				{
				iIndex = m_Array.GetCount();
				m_Array.Insert(pObj);
				}

			if (retdwID)
				*retdwID = (DWORD)iIndex;
			}

		VALUE *Remove (DWORD dwID)
			{
			VALUE *pObj = GetAt(dwID);
			if (pObj == NULL)
				return NULL;

			m_Array[dwID] = NULL;
			m_Free.Insert(dwID);
			return pObj;
			}

	private:
		inline bool IsValidID (DWORD dwID) const { return (dwID < (DWORD)m_Array.GetCount()); }

		TArray<VALUE *> m_Array;
		TArray<int> m_Free;
	};

