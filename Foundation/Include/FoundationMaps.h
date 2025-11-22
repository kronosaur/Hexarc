//	FoundationMaps.h
//
//	Foundation header file
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include <functional>
#include <initializer_list>
#include <type_traits>
#include <utility>

//	Maps

const DWORD NULL_ATOM = 0xffffffff;

template <typename KEY>
class CKeyCompare
	{
	public:
		static int Compare (const KEY& key1, const KEY& key2) { return ::KeyCompare(key1, key2); }
	};

template <typename KEY>
class CKeyCompareEquivalent
	{
	public:
		static int Compare (const KEY& key1, const KEY& key2) { return ::KeyCompare(key1, key2); }
	};

template <typename KEY, typename VALUE, typename COMPARE> class TSortMap
	{
	public:
		TSortMap (ESortOptions iOrder = AscendingSort) : m_iOrder(iOrder) { }
		TSortMap (const std::initializer_list<std::pair<KEY, VALUE>> &Src, ESortOptions iOrder = AscendingSort) : m_iOrder(iOrder)
			{
			GrowToFit(Src.size());
			for (auto Entry : Src)
				{
				SetAt(Entry.first, Entry.second);
				}
			}

		TSortMap (const TSortMap& Src) = default;
		TSortMap (TSortMap&& Src) noexcept = default;

		VALUE &operator [] (int iIndex) const { return GetValue(iIndex); }

		TSortMap& operator= (const TSortMap& Obj) = default;
		TSortMap& operator= (TSortMap&& Src) noexcept = default;

		void AppendSortedIfUnique (const KEY &newKey, const VALUE &newValue)
			{
			if (m_Index.GetCount() == 0 || CKeyCompare<KEY>::Compare(newKey, GetKey(m_Index.GetCount() - 1)) != 0)
				InsertSorted(newKey, newValue);
			}

		void AppendOrReplaceSorted (const KEY &newKey, const VALUE &newValue)
			{
			if (m_Index.GetCount() == 0 || CKeyCompare<KEY>::Compare(newKey, GetKey(m_Index.GetCount() - 1)) != 0)
				InsertSorted(newKey, newValue);
			else
				m_Array[m_Index[m_Index.GetCount() - 1]].theValue = newValue;
			}

		void Delete (int iIndex)
			{
			ASSERT(iIndex >= 0 && iIndex < m_Index.GetCount());
			int iPos = m_Index[iIndex];
			m_Index.Delete(iIndex);

			//	Add the array slot to free list (we can't delete it
			//	because it would move the indices of other entries)

			m_Array[iPos].theValue = VALUE();
			m_Free.Insert(iPos);
			}

		void DeleteAll (void)
			{
			m_Index.DeleteAll();
			m_Array.DeleteAll();
			m_Free.DeleteAll();
			}

		bool DeleteAt (const KEY &key)
			{
			int iPos;
			if (FindPos(key, &iPos))
				{
				Delete(iPos);
				return true;
				}
			else
				return false;
			}

		void Exclude (const TSortMap<KEY, VALUE, COMPARE> &Src)
			{
			//	Walk through both lists in order and remove.

			int iSrcPos = 0;
			int iDestPos = 0;

			while (iSrcPos < Src.GetCount() && iDestPos < GetCount())
				{
				int iCompare = COMPARE::Compare(Src.GetKey(iSrcPos), GetKey(iDestPos));
				if (iCompare == 0)
					{
					Delete(iDestPos);
					iSrcPos++;
					}
				else if (iCompare < 0)
					iSrcPos++;
				else
					iDestPos++;
				}
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

				int iCompare = m_iOrder * COMPARE::Compare(key, GetKey(iTry));
				if (iCompare == 0)
					{
					if (retiPos)
						*retiPos = iTry;
					return true;
					}
				else if (iCompare < 0)
					{
					iMin = iTry + 1;
					iTry = iMin + (iMax - iMin) / 2;
					}
				else if (iCompare > 0)
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
			m_Array.GrowToFit(iCount);
			m_Index.GrowToFit(iCount);
			}

		void GrowToFit (size_t iCount)
			{
			if (iCount > INT32_MAX)
				throw CException(errFail);

			GrowToFit((int)iCount);
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

		void Insert (const KEY &newKey, VALUE &&newValue)
			{
			VALUE *pNewValue = atom_Insert(newKey);
			*pNewValue = std::move(newValue);
			}

		void Insert (const TSortMap& Src)
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

		void Merge (const TSortMap& Src, std::function<void(VALUE &, const VALUE &)> fnReplace = DefaultReplace)
			{
			//	For now this only works if we have the same sort order

			if (m_iOrder != Src.m_iOrder)
				throw CException(errFail);

			//	Set up

			int iSrcPos = 0;
			int iDestPos = 0;

			//	Merge

			while (iSrcPos < Src.m_Index.GetCount())
				{
				//	If we're at the end of the destination then just insert

				if (iDestPos == m_Index.GetCount())
					{
					InsertSorted(Src.GetKey(iSrcPos), Src.GetValue(iSrcPos));

					//	Advance

					iSrcPos++;
					}

				//	Otherwise, see if we need to insert or replace

				else
					{
					int iCompare = m_iOrder * COMPARE::Compare(Src.GetKey(iSrcPos), GetKey(iDestPos));

					//	If the same key then we replace

					if (iCompare == 0)
						{
						fnReplace(GetValue(iDestPos), Src.GetValue(iSrcPos));

						//	Advance

						iDestPos++;
						iSrcPos++;
						}

					//	If the source is less than dest then we insert at this
					//	position.

					else if (iCompare > 0)
						{
						InsertSorted(Src.GetKey(iSrcPos), Src.GetValue(iSrcPos), iDestPos);

						//	Advance

						iSrcPos++;
						}

					//	Otherwise, go to the next destination slot

					else
						iDestPos++;
					}
				}
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

		void SetAt (const KEY &key, VALUE &&value, bool *retbInserted = NULL)
			{
			VALUE *pValue = SetAt(key, retbInserted);
			*pValue = std::move(value);
			}

		void SetAtHandoff (const KEY &key, VALUE &value, bool *retbInserted = NULL)
			{
			VALUE *pValue = SetAt(key, retbInserted);
			pValue->TakeHandoff(value);
			}

		void TakeHandoff (TSortMap& Src)
			{
			m_iOrder = Src.m_iOrder;
			m_Index.TakeHandoff(Src.m_Index);
			m_Array.TakeHandoff(Src.m_Array);
			m_Free.TakeHandoff(Src.m_Free);
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
					m_Free.Insert(dwAtom);
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

		//	CString/CStringSlice specialization

		template <typename T = KEY>
		typename std::enable_if<std::is_same<T, CString>::value, bool>::type
		FindPos (CStringSlice key, int *retiPos = NULL) const
			{
			int iCount = m_Index.GetCount();
			int iMin = 0;
			int iMax = iCount;
			int iTry = iMax / 2;

			LPCSTR pKey = key.GetPointer();
			int iKeyLen = key.GetLength();

			while (true)
				{
				if (iMax <= iMin)
					{
					if (retiPos)
						*retiPos = iMin;
					return false;
					}

				const CString &DestKey = GetKey(iTry);
				CStringView DestView = DestKey.AsView();
				LPCSTR pDest = DestView;
				int iCompare = m_iOrder * ::KeyCompare(pKey, iKeyLen, pDest, DestView.GetLength());
				if (iCompare == 0)
					{
					if (retiPos)
						*retiPos = iTry;
					return true;
					}
				else if (iCompare < 0)
					{
					iMin = iTry + 1;
					iTry = iMin + (iMax - iMin) / 2;
					}
				else if (iCompare > 0)
					{
					iMax = iTry;
					iTry = iMin + (iMax - iMin) / 2;
					}
				}

			return false;
			}

		template <typename T = KEY>
		typename std::enable_if<std::is_same<T, CString>::value, VALUE *>::type
		GetAt (CStringSlice key) const
			{
			int iPos;
			if (!FindPos(key, &iPos))
				return NULL;

			return &m_Array[m_Index[iPos]].theValue;
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

		static void DefaultReplace (VALUE &Dest, const VALUE &Src)
			{
			Dest = Src;
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
				int iFreePos = m_Free[m_Free.GetCount() - 1];
				m_Free.Delete(m_Free.GetCount() - 1);

				*retiPos = iFreePos;
				pEntry = &m_Array[iFreePos];
				}
			return pEntry;
			}

		ESortOptions m_iOrder;
		TArray<int> m_Index;
		TArray<SEntry> m_Array;
		TArray<int> m_Free;
	};

template <class VALUE> class TAtomTable
	{
	public:
		TAtomTable (ESortOptions iOrder = AscendingSort) : m_Map(iOrder) { }

		VALUE &operator [] (DWORD dwAtom) const { return GetAt(dwAtom); }

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
		TIDTable (int iGranularity) : m_Array(iGranularity) { }
		TIDTable (const TIDTable<VALUE> &Obj) : m_Array(Obj.m_Array), m_Free(Obj.m_Free) { }

		~TIDTable (void) { DeleteAll(); }

		TIDTable<VALUE> &operator= (const TIDTable<VALUE> &Obj)
			{
			m_Array = Obj.m_Array;
			m_Free = Obj.m_Free;
			return *this;
			}

		VALUE &operator [] (DWORD dwID) const { return GetAt(dwID); }

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

		VALUE &GetAt (DWORD dwID) const
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

		VALUE &GetNext (SIDTableEnumerator &i, DWORD *retdwID) const
			{
			ASSERT(i.iPos != -1);
			VALUE &theValue = m_Array[i.iPos].Value;
			*retdwID = m_Array[i.iPos].dwID;

			Next(i);

			return theValue;
			}

		bool HasMore (SIDTableEnumerator &i) const
			{
			return (i.iPos != -1);
			}

		DWORD Insert (const VALUE &Value)
			{
			DWORD dwID;
			*Insert(&dwID) = Value;
			return dwID;
			}

		DWORD Insert (VALUE &&Value)
			{
			DWORD dwID;
			*Insert(&dwID) = std::move(Value);
			return dwID;
			}

		void Insert (const VALUE &Value, DWORD *retdwID)
			{
			*Insert(retdwID) = Value;
			}

		VALUE *Insert (DWORD *retdwID = NULL)
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

		bool IsValid (DWORD dwID) const
			{
			int iIndex = GetIndex(dwID);
			return (iIndex >= 0 && iIndex < m_Array.GetCount() && m_Array[iIndex].dwID == dwID && dwID != 0);
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
			int iCount = m_Array.GetCount();
			int iIndex = GetIndex(dwID);

			if (iIndex >= iCount)
				{
				int iExtra = iIndex - iCount;
				m_Array.InsertEmpty(iExtra + 1);

				for (int i = 0; i < iExtra; i++)
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

		static bool IsCustomID (DWORD dwID) { return (dwID && GetCounter(dwID) == 0); }

	private:
		struct SEntry
			{
			DWORD dwID;
			VALUE Value;
			};

		static int GetCounter (DWORD dwID) { return (dwID >> 24); }
		static int GetIndex (DWORD dwID) { return (dwID & 0x00ffffff); }
		static DWORD MakeID (int iIndex, int iCounter) { return ((((DWORD)iCounter) << 24) | (iIndex & 0x00ffffff)); }

		TArray<SEntry> m_Array;
		TArray<DWORD> m_Free;
		int m_iCustomID = 1;
	};

template <class VALUE> class TObjArray
	{
	public:
		~TObjArray (void) { DeleteAll(); }

		VALUE *operator [] (DWORD dwID) const { return GetAt(dwID); }

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
		bool IsValidID (DWORD dwID) const { return (dwID < (DWORD)m_Array.GetCount()); }

		TArray<VALUE *> m_Array;
		TArray<int> m_Free;
	};

class CSymbolTable
	{
	public:

		DWORD Atomize (CStringView sSymbol);
		DWORD Atomize (CStringSlice sSymbol);
		DWORD FindSymbol (const CString& sSymbol) const;
		const CString& Symbol (DWORD dwAtom) const { if (dwAtom == 0) return NULL_STR; else return m_Array[Atom2Index(dwAtom)]; }

	private:

		bool FindPos (LPCSTR pSymbol, int iSymbolLen, int *retiPos = NULL) const;
		bool FindPos (const CString& sSymbol, int *retiPos = NULL) const { return FindPos(sSymbol.GetPointer(), sSymbol.GetLength(), retiPos); }
		const CString& GetKey (int iIndex) const { return m_Array[m_Index[iIndex]]; }

		static int Atom2Index (DWORD dwAtom) { return ((int)dwAtom) - 1; }
		static DWORD Index2Atom (int iIndex) { return (DWORD)(iIndex + 1); }

		TArray<int> m_Index;
		TArray<CString> m_Array;
	};

template <class VALUE>
inline void TArray<VALUE>::Delete (const TSortMap<int, bool>& SortedIndices)
	{
	//	We delete in reverse order.

	for (int i = SortedIndices.GetCount() - 1; i >= 0; i--)
		{
		int iToDelete = SortedIndices.GetKey(i);
		if (iToDelete >= 0 && iToDelete < GetCount())
			Delete(iToDelete);
		}
	}
