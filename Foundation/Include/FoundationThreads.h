//	FoundationThreads.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

#include <process.h>

class COSObject
	{
	public:
		COSObject (void) : m_hHandle(INVALID_HANDLE_VALUE) { }
		COSObject (const COSObject &Src) { Copy(Src); }
		~COSObject (void) { if (m_hHandle != INVALID_HANDLE_VALUE) ::CloseHandle(m_hHandle); }

		COSObject &operator= (const COSObject &Src) { Close(); Copy(Src); return *this; }

		inline void Close (void) { if (m_hHandle != INVALID_HANDLE_VALUE) { ::CloseHandle(m_hHandle); m_hHandle = INVALID_HANDLE_VALUE; } }
		inline HANDLE GetHandoff (void) { HANDLE hHandle = m_hHandle; m_hHandle = INVALID_HANDLE_VALUE; return hHandle; }
		inline HANDLE GetWaitObject (void) const { return m_hHandle; }
		inline void TakeHandoff (COSObject &Obj) { Close(); m_hHandle = Obj.m_hHandle; Obj.m_hHandle = INVALID_HANDLE_VALUE; }
		inline bool Wait (DWORD dwTimeout = INFINITE) const { return (::WaitForSingleObject(m_hHandle, dwTimeout) != WAIT_TIMEOUT); }

	protected:
		inline void Copy (const COSObject &Src)
			{
			if (Src.m_hHandle != INVALID_HANDLE_VALUE) 
				::DuplicateHandle(GetCurrentProcess(), Src.m_hHandle, GetCurrentProcess(), &m_hHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);
			else 
				m_hHandle = INVALID_HANDLE_VALUE;
			}

		HANDLE m_hHandle;
	};

const int OS_WAIT_TIMEOUT = -1;

class CWaitArray
	{
	public:
		~CWaitArray (void) { CleanUp(); }

		inline void CleanUp (void) 
			{
			int i;
			for (i = 0; i < m_Extra.GetCount(); i++)
				if (m_Extra[i].bCloseHandle)
					::CloseHandle(m_Array[i]);

			m_Array.DeleteAll();
			m_Extra.DeleteAll();
			}
		inline void DeleteAll (void) { CleanUp(); }
		inline int GetCount (void) const { return m_Array.GetCount(); }
		inline const CString &GetData (int iIndex) const { return m_Extra[iIndex].sData; }
		inline int Insert (HANDLE hObj, const CString &sData = NULL_STR, int iIndex = -1) 
			{
			m_Array.Insert(hObj, iIndex);
			SExtra *pExtra = m_Extra.InsertAt(iIndex);
			pExtra->sData = sData;
			return (iIndex == -1 ? m_Array.GetCount() - 1 : iIndex); 
			}
		inline int Insert (COSObject &Obj, const CString &sData = NULL_STR, int iIndex = -1) 
			{
			m_Array.Insert(Obj.GetWaitObject(), iIndex);
			SExtra *pExtra = m_Extra.InsertAt(iIndex);
			pExtra->sData = sData;
			return (iIndex == -1 ? m_Array.GetCount() - 1 : iIndex); 
			}
		inline int InsertCopy (COSObject &Obj, const CString &sData = NULL_STR, int iIndex = -1) 
			{
			COSObject Copy(Obj);
			m_Array.Insert(Copy.GetHandoff(), iIndex);
			SExtra *pExtra = m_Extra.InsertAt(iIndex);
			pExtra->sData = sData;
			pExtra->bCloseHandle = true;
			return (iIndex == -1 ? m_Array.GetCount() - 1 : iIndex); 
			}
		inline bool WaitForAll (DWORD dwTimeout = INFINITE) { return (ConvertWaitResult(WaitForMultipleObjects(m_Array.GetCount(), &m_Array[0], TRUE, dwTimeout)) != OS_WAIT_TIMEOUT); }
		inline int WaitForAny (DWORD dwTimeout = INFINITE) { return ConvertWaitResult(WaitForMultipleObjects(m_Array.GetCount(), &m_Array[0], FALSE, dwTimeout));	}

	private:
		struct SExtra
			{
			SExtra (void) :
					bCloseHandle(false)
				{ }

			CString sData;
			bool bCloseHandle;
			};

		inline int ConvertWaitResult (DWORD dwWait) { return (dwWait == WAIT_TIMEOUT ? OS_WAIT_TIMEOUT : (dwWait & ~WAIT_ABANDONED_0) - WAIT_OBJECT_0); }

		TArray<HANDLE> m_Array;
		TArray<SExtra> m_Extra;
	};

template <class CLASS> class TThread : public COSObject
	{
	public:
		inline bool IsStarted (void) const { return (m_hHandle != INVALID_HANDLE_VALUE); }

		void Start (void)
			{
			ASSERT(m_hHandle == INVALID_HANDLE_VALUE);

			m_hHandle = (HANDLE)_beginthreadex(NULL,
					0,
					(unsigned int (__stdcall *)(void *))Thunk,
					this,
					0,
					(unsigned int *)&m_dwThreadID);
			if (m_hHandle == (HANDLE)-1)
				{
				m_hHandle = INVALID_HANDLE_VALUE;
				throw CException(errFail);
				}
			}

	private:
		static DWORD WINAPI Thunk (LPVOID pData)
			{
			CLASS *pObj = (CLASS *)pData;
			::srand(::GetTickCount() * ::GetCurrentThreadId());
			pObj->Run();
			return 0;
			}

		DWORD m_dwThreadID;
	};

//	Synchronization classes

class CCriticalSection
	{
	public:
		CCriticalSection (void) { ::InitializeCriticalSection(&m_cs); }
		~CCriticalSection (void) { ::DeleteCriticalSection(&m_cs); }

		inline void Lock (void) const { ::EnterCriticalSection(&m_cs); }
		inline void Unlock (void) const { ::LeaveCriticalSection(&m_cs); }

	private:
		mutable CRITICAL_SECTION m_cs;
	};

class CSmartLock
	{
	public:
		CSmartLock(const CCriticalSection &cs) : m_cs(cs), m_bLocked(false) { Lock(); }
		~CSmartLock (void) { Unlock(); }

		inline void Lock (void) const { if (!m_bLocked) { m_cs.Lock(); m_bLocked = true; } }
		inline void Unlock (void) const { if (m_bLocked) { m_cs.Unlock(); m_bLocked = false; } }

	private:
		const CCriticalSection &m_cs;
		mutable bool m_bLocked;
	};

class CManualEvent : public COSObject
	{
	public:
		inline void Create (void) { Create(NULL_STR); }
		void Create (const CString &sName, bool *retbExists = NULL);
		inline bool IsSet (void) { return (::WaitForSingleObject(m_hHandle, 0) == WAIT_OBJECT_0); }
		inline void Reset (void) { ::ResetEvent(m_hHandle); }
		inline void Set (void) { ::SetEvent(m_hHandle); }
	};

class CMutex : public COSObject
	{
	public:
		inline void Create (void) { Create(NULL_STR, false); }
		void Create (const CString &sName, bool bInitialOwner, bool *retbExists = NULL);
		inline void Lock (void) { ::WaitForSingleObject(m_hHandle, INFINITE); }
		inline void Unlock (void) { ::ReleaseMutex(m_hHandle); }
	};

class CProcess : public COSObject
	{
	public:
		struct SMemoryInfo
			{
			DWORDLONG dwCurrentAlloc;		//	Bytes currently committed to the process (private working set)
			DWORDLONG dwCurrentReserved;	//	Bytes reserved for the process (working set)
			DWORDLONG dwPeakReserved;		//	Peak reserved size for the process (peak working set)
			};

		void Create (const CString sCmdLine);
		inline void CreateCurrentProcess (void) { Close(); m_hHandle = ::GetCurrentProcess(); }
		CString GetExecutableFilespec (void) const;
		inline DWORD GetID (void) const { return (m_hHandle == INVALID_HANDLE_VALUE ? 0 : ::GetProcessId(m_hHandle)); }
		bool GetMemoryInfo (SMemoryInfo *retInfo) const;
	};

class CSemaphore : public COSObject
	{
	public:
		inline void Create (int iMaxCount) { Create(NULL_STR, iMaxCount); }
		void Create (const CString &sName, int iMaxCount, bool *retbExists = NULL);
		void Decrement (int iCount = 1);
		inline int GetMaxCount (void) const { return m_iMaxCount; }
		void Increment (int iCount = 1);
		bool TryIncrement (int iCount = 1, DWORD dwTimeout = 0);

	private:
		int m_iMaxCount;
	};

class CReaderWriterSemaphore : private CSemaphore
	{
	public:
		inline void Create (int iMaxCount) { CSemaphore::Create(NULL, iMaxCount); }
		inline void Create (LPSTR sName, int iMaxCount) { CSemaphore::Create(sName, iMaxCount); }

		inline void LockReader (void) { CSemaphore::Increment(1); }
		inline void LockWriter (void) { CSemaphore::Increment(GetMaxCount()); }
		inline void UnlockReader (void) { CSemaphore::Decrement(1); }
		inline void UnlockWriter (void) { CSemaphore::Decrement(GetMaxCount()); }
	};

//	Functions

inline DWORD sysGetCurrentProcessID (void) { return ::GetCurrentProcessId(); }
