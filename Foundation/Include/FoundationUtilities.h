//	FoundationUtilities.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
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
		inline void DeleteAll (void) { m_sAttributes = NULL_STR; }
		void GetAll (TArray<CString> *retAttribs) const;
		inline bool HasAttribute (const CString &sAttrib) const { return FindAttribute(strToLower(sAttrib)); }
		void Insert (const CString &sAttrib);
		inline bool IsEmpty (void) const { return m_sAttributes.IsEmpty(); }
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

int KeyCompare (const LPSTR &pKey1, const LPSTR &pKey2);
inline int KeyCompare (const CString &sKey1, const CString &sKey2) { return KeyCompare((LPSTR)sKey1, (LPSTR)sKey2); }

int KeyCompareNoCase (const LPSTR &pKey1, int iKey1Len, const LPSTR &pKey2, int iKey2Len);
inline int KeyCompareNoCase (const CString &sKey1, const CString &sKey2) { return KeyCompareNoCase((LPSTR)sKey1, sKey1.GetLength(), (LPSTR)sKey2, sKey2.GetLength()); }

//	Ref-counted classes

class IRefCounted
	{
	public:
		IRefCounted (void) : m_iRefCount(1) { }
		virtual ~IRefCounted (void) { }

		inline void AddRef (void) { m_iRefCount++; }
		inline void Release (void) { if (--m_iRefCount == 0) delete this; }

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

		inline bool IsEmpty (void) const { return (m_pObj == NULL); }

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
	DWORDLONG dwlTotalPhysicalMemory;		//	Bytes of physical memory in the system.
	DWORDLONG dwlAvailPhysicalMemory;		//	Bytes of physical memory available.
	};

CString sysGetDNSName (void);
bool sysGetMemoryInfo (SSystemMemoryInfo *retInfo);
CString sysGetOSErrorText (DWORD dwError);
inline DWORD sysGetTickCount (void) { return ::GetTickCount(); }
inline DWORDLONG sysGetTickCount64 (void) { return ::GetTickCount64(); }
DWORD sysGetTicksElapsed (DWORD dwTick, DWORD *retdwNow = NULL);
DWORDLONG sysGetTicksElapsed (DWORDLONG dwTick, DWORDLONG *retdwNow = NULL);
bool sysIsBigEndian (void);
