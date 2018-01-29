//	FoundationHashMaps.h
//
//	Foundation header file
//	Copyright (c) 2017 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

struct HashBase
	{
	protected:
		static DWORD Get16Bits (BYTE *pData) { return ((DWORD)pData[1] << 8) + (DWORD)pData[0]; }
		static DWORD SuperFastHash (BYTE *pData, int iLen);
	};

template <class VALUE> struct THash : public HashBase
	{
	public:
		DWORD operator ()(const VALUE &Value) const { return SuperFastHash((BYTE *)&Value, sizeof(VALUE)); }
	};

template <> struct THash<CString> : public HashBase
	{
	public:
		DWORD operator ()(const CString &sValue) const { return SuperFastHash((BYTE *)(LPSTR)sValue, sValue.GetLength()); }
	};

template <class KEY, class VALUE, class HASH = THash<KEY>> class THashMap
	{
	public:
		THashMap (int iTableSize = 256)
			{
			ASSERT(iTableSize > 0);
			m_Map.InsertEmpty(iTableSize);
			}

		void DeleteAll (void)
			{
			int iTableSize = m_Map.GetCount();
			m_Map.DeleteAll();
			m_Map.InsertEmpty(iTableSize);
			}

		void DeleteAt (const KEY &key)
			{
			SEntry &Entry = GetHashEntry(key);
			Entry.DeleteAt(key);
			}

		bool Find (const KEY &key, VALUE *retpValue = NULL) const
			{
			SEntry &Entry = GetHashEntry(key);
			VALUE *pValue = Entry.GetAt(key);
			if (pValue == NULL)
				return false;

			if (retpValue)
				*retpValue = *pValue;

			return true;
			}

		VALUE *GetAt (const KEY &key) const
			{
			return GetHashEntry(key).GetAt(key);
			}

		void Insert (const KEY &key, const VALUE &value)
			{
			*SetAt(key) = value;
			}

		VALUE *SetAt (const KEY &key, bool *retbInserted = NULL)
			{
			return GetHashEntry(key).SetAt(key);
			}

	private:

#define ONE_ENTRY	((TSortMap<KEY, VALUE> *)1)

		struct SEntry
			{
			SEntry (void) :
					pExtra(NULL)
				{ }
			SEntry (const SEntry &Src)
				{
				Key = Src.Key;
				Value = Src.Value;
				if (Src.pExtra && pExtra != ONE_ENTRY)
					pExtra = new TSortMap<KEY, VALUE>(*Src.pExtra);
				else
					pExtra = Src.pExtra;
				}

			~SEntry (void)
				{
				if (pExtra && pExtra != ONE_ENTRY)
					delete pExtra;
				}

			SEntry &operator= (const SEntry &Src)
				{
				if (pExtra && pExtra != ONE_ENTRY)
					delete pExtra;

				Key = Src.Key;
				Value = Src.Value;
				if (Src.pExtra && pExtra != ONE_ENTRY)
					pExtra = new TSortMap<KEY, VALUE>(*Src.pExtra);
				else
					pExtra = Src.pExtra;
				}

			inline void AddEntry (const KEY &KeyArg, const VALUE &ValueArg)
				{
				if (pExtra == NULL)
					{
					Key = KeyArg;
					Value = ValueArg;
					pExtra = ONE_ENTRY;
					}
				else if (pExtra == ONE_ENTRY)
					{
					pExtra = new TSortMap<KEY, VALUE>;
					pExtra->Insert(Key, Value);
					pExtra->Insert(KeyArg, ValueArg);
					Key = KEY();
					Value = VALUE();
					}
				else
					{
					pExtra->Insert(KeyArg, ValueArg);
					}
				}

			inline VALUE *GetAt (const KEY &KeyArg)
				{
				if (pExtra == NULL)
					return NULL;
				else if (pExtra == ONE_ENTRY)
					{
					if (KeyCompare(Key, KeyArg) == 0)
						return &Value;
					else
						return NULL;
					}
				else
					return pExtra->GetAt(KeyArg);
				}

			inline void RemoveEntry (const KEY &KeyArg)
				{
				if (pExtra == NULL)
					;
				else if (pExtra == ONE_ENTRY)
					{
					if (KeyCompare(Key, KeyArg) == 0)
						{
						Key = KEY();
						Value = VALUE();
						pExtra = NULL;
						}
					}
				else
					pExtra->DeleteAt(KeyArg);
				}

			inline VALUE *SetAt (const KEY &KeyArg, bool *retbInserted = NULL)
				{
				if (pExtra == NULL)
					{
					Key = KeyArg;
					pExtra = ONE_ENTRY;
					if (retbInserted)
						*retbInserted = true;
					return &Value;
					}
				else if (pExtra == ONE_ENTRY)
					{
					if (KeyCompare(Key, KeyArg) == 0)
						{
						if (retbInserted)
							*retbInserted = false;
						return &Value;
						}
					else
						{
						pExtra = new TSortMap<KEY, VALUE>;
						pExtra->Insert(Key, Value);
						Key = KEY();
						Value = VALUE();
						if (retbInserted)
							*retbInserted = true;
						return pExtra->SetAt(KeyArg);
						}
					}
				else
					return pExtra->SetAt(KeyArg, retbInserted);
				}

			inline int GetCount (void) const { if (pExtra == NULL) return 0; else if (pExtra == ONE_ENTRY) return 1; else return pExtra->GetCount(); }
			inline bool IsEmpty (void) const { return (pExtra == NULL); }

			KEY Key;
			VALUE Value;
			TSortMap<KEY, VALUE> *pExtra;
			};

#undef ONE_ENTRY

		SEntry &GetHashEntry (const KEY &key) const
			{
			return m_Map[(HASH{}(key) % m_Map.GetCount())];
			}

		TArray<SEntry> m_Map;
	};

