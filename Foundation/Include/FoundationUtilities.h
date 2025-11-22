//	FoundationUtilities.h
//
//	Foundation header file
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IMemoryBlock;

//	Useful macros

#define DEF_STRING(str)					(sizeof(str)-1), str
#define HIDWORD(l)						((DWORD)(((DWORDLONG)(l)>>32)&0xFFFFFFFF))
#define LODWORD(l)						((DWORD)((DWORDLONG)(l)))
#define MAKEDWORDLONG(a,b)				((DWORDLONG)(((DWORD)(a))|(((DWORDLONG)((DWORD)(b)))<<32)))
#define SIZEOF_STATIC_ARRAY(theArray)	(sizeof(theArray) / sizeof(theArray[0]))

//	Attributes

class CAttributeList
	{
	public:
		void Delete (const CString &sAttrib);
		void DeleteAll (void) { m_sAttributes = NULL_STR; }
		void GetAll (TArray<CString> *retAttribs) const;
		bool HasAttribute (const CString &sAttrib) const { return FindAttribute(strToLower(sAttrib)); }
		void Insert (CStringView sAttrib);
		bool IsEmpty (void) const { return m_sAttributes.IsEmpty(); }
		static bool ValidateAttribute (const CString &sAttrib);

	private:
		bool FindAttribute (const CString &sAttrib, int *retiPos = NULL) const;

		CString m_sAttributes;
	};

//	Comparison functions

template<class KEY> int KeyCompare (const KEY &Key1, const KEY &Key2)
	{
	if (Key1 > Key2)
		return 1;
	else if (Key1 < Key2)
		return -1;
	else
		return 0;
	}

int KeyCompare (const LPCSTR &pKey1, const LPCSTR &pKey2);
int KeyCompare (const LPCSTR &pKey1, int iKey1Len, const LPCSTR &pKey2, int iKey2Len);
inline int KeyCompare (const LPSTR &pKey1, const LPSTR &pKey2) { return KeyCompare((LPCSTR)pKey1, (LPCSTR)pKey2); }
inline int KeyCompare (const CString &sKey1, const CString &sKey2) { return KeyCompare((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline int KeyCompare (CStringView sKey1, CStringView sKey2) { return KeyCompare((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }

int KeyCompareNoCase (const LPCSTR &pKey1, int iKey1Len, const LPCSTR &pKey2, int iKey2Len);
inline int KeyCompareNoCase (const CString &sKey1, const CString &sKey2) { return KeyCompareNoCase((LPSTR)sKey1, sKey1.GetLength(), (LPSTR)sKey2, sKey2.GetLength()); }
inline int KeyCompareNoCase (CStringView sKey1, CStringView sKey2) { return KeyCompareNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline int KeyCompareNoCase (const CString& sKey1, CStringView sKey2) { return KeyCompareNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }
inline int KeyCompareNoCase (CStringView sKey1, const CString& sKey2) { return KeyCompareNoCase((LPCSTR)sKey1, sKey1.GetLength(), (LPCSTR)sKey2, sKey2.GetLength()); }

template <> class CKeyCompareEquivalent<CString>
	{
	public:
		static int Compare(const CString& key1, const CString& key2) { return ::KeyCompareNoCase((LPSTR)key1, key1.GetLength(), (LPSTR)key2, key2.GetLength()); }
	};

//	Ref-counted classes

class IRefCounted
	{
	public:
		IRefCounted (void) : m_iRefCount(1) { }
		virtual ~IRefCounted (void) { }

		void AddRef (void) noexcept { m_iRefCount++; }
		void Release (void) noexcept { if (--m_iRefCount == 0) delete this; }

	private:
		int m_iRefCount;
	};

template <class VALUE> class TSmartRefCountPtr
	{
	public:
		TSmartRefCountPtr (void) : m_pObj(NULL) { }
		TSmartRefCountPtr (VALUE *pObj) : m_pObj(pObj) { if (m_pObj) m_pObj->AddRef(); }
		TSmartRefCountPtr (const TSmartRefCountPtr<VALUE> &SmartObj) : m_pObj(SmartObj.m_pObj) { if (m_pObj) m_pObj->AddRef(); }
		~TSmartRefCountPtr (void)
			{ Release(); }

		operator VALUE * (void) const { return m_pObj; }

		TSmartRefCountPtr<VALUE> &operator = (const TSmartRefCountPtr<VALUE> &Src)
			{
			if (this != &Src)
				{
				//	Release old pointer

				Release();

				//	Copy the new pointer and add ref

				m_pObj = Src.m_pObj;
				if (m_pObj)
					m_pObj->AddRef();
				}

			return *this;
			}

		TSmartRefCountPtr<VALUE> &operator = (VALUE *pSrc)
			{
			Release();

			m_pObj = pSrc;
			if (m_pObj)
				m_pObj->AddRef();

			return *this;
			}

		VALUE &operator * (void)
			{ return *m_pObj; }

		VALUE *operator -> (void)
			{ return m_pObj; }

		bool IsEmpty (void) const { return (m_pObj == NULL); }

		void Release (void)
			{
			if (m_pObj)
				{
				m_pObj->Release();
				m_pObj = NULL;
				}
			}

	private:
		VALUE *m_pObj;
	};

class CRecursionState
	{
	public:

		bool IsLocked () const { return m_bInUse; }
		void Lock () const { ASSERT(!m_bInUse); m_bInUse = true; }
		void Unlock () const { ASSERT(m_bInUse); m_bInUse = false; }

	private:

		mutable bool m_bInUse = false;
	};

class CRecursionSmartLock
	{
	public:

		CRecursionSmartLock (const CRecursionState& Lock) : 
				m_Lock(Lock),
				m_bInRecursion(Lock.IsLocked())
			{
			if (!m_bInRecursion)
				m_Lock.Lock();
			}

		~CRecursionSmartLock (void)
			{
			if (!m_bInRecursion)
				m_Lock.Unlock();
			}

		bool InRecursion () const { return m_bInRecursion; }

	private:

		const CRecursionState& m_Lock;
		bool m_bInRecursion;
	};

//	Memory utilities

void utlMemCopy (const void *pSource, void *pDest, DWORD dwCount);
inline void utlMemCopy (const void *pSource, void *pDest, int iCount) { if (iCount > 0) utlMemCopy(pSource, pDest, (DWORD)iCount); }
void utlMemCopy (const void *pSource, void *pDest, DWORDLONG dwCount);
bool utlMemCompare (void *pSource, void *pDest, int iCount);
void utlMemReverse (void *pSource, void *pDest, int iCount);
void utlMemSet (void *pDest, int Count, char Value = 0);

DWORD utlAdler32 (IMemoryBlock &Data);
int utlBitsSet (DWORD dwValue);

//	Win32 utilities

struct SSystemMemoryInfo
	{
	DWORDLONG dwlTotalPhysicalMemory = 0;	//	Bytes of physical memory in the system.
	DWORDLONG dwlAvailPhysicalMemory = 0;	//	Bytes of physical memory available.
	};

struct SProcessMemoryInfo
	{
	DWORDLONG dwlWorkingSetSize = 0;		//	Bytes of physical memory used by the process.
	DWORDLONG dwlCommittedSize = 0;			//	Bytes of virtual memory committed by the process.
	};

CString sysGetDNSName (void);
bool sysGetMemoryInfo (SSystemMemoryInfo *retInfo);
bool sysGetProcessMemoryInfo (SProcessMemoryInfo& retInfo);
CString sysGetOSErrorText (DWORD dwError);
inline DWORD sysGetTickCount (void) { return ::GetTickCount(); }
inline DWORDLONG sysGetTickCount64 (void) { return ::GetTickCount64(); }
DWORD sysGetTicksElapsed (DWORD dwTick, DWORD *retdwNow = NULL);
DWORDLONG sysGetTicksElapsed (DWORDLONG dwTick, DWORDLONG *retdwNow = NULL);
DWORD sysGetSecondsElapsed (DWORDLONG dwTick, DWORDLONG *retdwNow = NULL);
bool sysIsBigEndian (void);
