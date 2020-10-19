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

		void Close (void) { if (m_hHandle != INVALID_HANDLE_VALUE) { ::CloseHandle(m_hHandle); m_hHandle = INVALID_HANDLE_VALUE; } }
		HANDLE GetHandoff (void) { HANDLE hHandle = m_hHandle; m_hHandle = INVALID_HANDLE_VALUE; return hHandle; }
		HANDLE GetWaitObject (void) const { return m_hHandle; }
		void TakeHandoff (COSObject &Obj) { Close(); m_hHandle = Obj.m_hHandle; Obj.m_hHandle = INVALID_HANDLE_VALUE; }
		bool Wait (DWORD dwTimeout = INFINITE) const { return (::WaitForSingleObject(m_hHandle, dwTimeout) != WAIT_TIMEOUT); }

	protected:
		void Copy (const COSObject &Src)
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

		void CleanUp (void) 
			{
			int i;
			for (i = 0; i < m_Extra.GetCount(); i++)
				if (m_Extra[i].bCloseHandle)
					::CloseHandle(m_Array[i]);

			m_Array.DeleteAll();
			m_Extra.DeleteAll();
			}
		void DeleteAll (void) { CleanUp(); }
		int GetCount (void) const { return m_Array.GetCount(); }
		const CString &GetData (int iIndex) const { return m_Extra[iIndex].sData; }
		int Insert (HANDLE hObj, const CString &sData = NULL_STR, int iIndex = -1) 
			{
			m_Array.Insert(hObj, iIndex);
			SExtra *pExtra = m_Extra.InsertAt(iIndex);
			pExtra->sData = sData;
			return (iIndex == -1 ? m_Array.GetCount() - 1 : iIndex); 
			}
		int Insert (COSObject &Obj, const CString &sData = NULL_STR, int iIndex = -1) 
			{
			m_Array.Insert(Obj.GetWaitObject(), iIndex);
			SExtra *pExtra = m_Extra.InsertAt(iIndex);
			pExtra->sData = sData;
			return (iIndex == -1 ? m_Array.GetCount() - 1 : iIndex); 
			}
		int InsertCopy (COSObject &Obj, const CString &sData = NULL_STR, int iIndex = -1) 
			{
			COSObject Copy(Obj);
			m_Array.Insert(Copy.GetHandoff(), iIndex);
			SExtra *pExtra = m_Extra.InsertAt(iIndex);
			pExtra->sData = sData;
			pExtra->bCloseHandle = true;
			return (iIndex == -1 ? m_Array.GetCount() - 1 : iIndex); 
			}
		bool WaitForAll (DWORD dwTimeout = INFINITE) { return (ConvertWaitResult(WaitForMultipleObjects(m_Array.GetCount(), &m_Array[0], TRUE, dwTimeout)) != OS_WAIT_TIMEOUT); }
		int WaitForAny (DWORD dwTimeout = INFINITE) { return ConvertWaitResult(WaitForMultipleObjects(m_Array.GetCount(), &m_Array[0], FALSE, dwTimeout));	}

	private:
		struct SExtra
			{
			SExtra (void) :
					bCloseHandle(false)
				{ }

			CString sData;
			bool bCloseHandle;
			};

		int ConvertWaitResult (DWORD dwWait) { return (dwWait == WAIT_TIMEOUT ? OS_WAIT_TIMEOUT : (dwWait & ~WAIT_ABANDONED_0) - WAIT_OBJECT_0); }

		TArray<HANDLE> m_Array;
		TArray<SExtra> m_Extra;
	};

template <class CLASS> class TThread : public COSObject
	{
	public:
		DWORD GetID (void) const { return (IsStarted() ? ::GetThreadId(m_hHandle) : 0); }
		bool IsStarted (void) const { return (m_hHandle != INVALID_HANDLE_VALUE); }

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
			_endthreadex(0);
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

		void Lock (void) const { ::EnterCriticalSection(&m_cs); }
		void Unlock (void) const { ::LeaveCriticalSection(&m_cs); }

	private:
		mutable CRITICAL_SECTION m_cs;
	};

template <typename T> class TSmartLock
	{
	public:
		TSmartLock(const T &obj) : m_obj(obj) { Lock(); }
		~TSmartLock (void) { Unlock(); }

		void Lock (void) const { if (!m_bLocked) { m_obj.Lock(); m_bLocked = true; } }
		void Unlock (void) const { if (m_bLocked) { m_obj.Unlock(); m_bLocked = false; } }

	private:
		const T &m_obj;
		mutable bool m_bLocked = false;
	};

class CSmartLock : public TSmartLock<CCriticalSection>
	{
	public:
		CSmartLock (const CCriticalSection &cs) : TSmartLock<CCriticalSection>(cs) { }
	};

class CManualEvent : public COSObject
	{
	public:
		void Create (void) { Create(NULL_STR); }
		void Create (const CString &sName, bool *retbExists = NULL);
		bool IsSet (void) { return (::WaitForSingleObject(m_hHandle, 0) == WAIT_OBJECT_0); }
		void Reset (void) { ::ResetEvent(m_hHandle); }
		bool Set (void) { return (::SetEvent(m_hHandle) != 0); }
	};

class CMutex : public COSObject
	{
	public:
		void Create (void) { Create(NULL_STR, false); }
		void Create (const CString &sName, bool bInitialOwner, bool *retbExists = NULL);
		void Lock (void) { ::WaitForSingleObject(m_hHandle, INFINITE); }
		void Unlock (void) { ::ReleaseMutex(m_hHandle); }
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
		void CreateCurrentProcess (void) { Close(); m_hHandle = ::GetCurrentProcess(); }
		CString GetExecutableFilespec (void) const;
		DWORD GetID (void) const { return (m_hHandle == INVALID_HANDLE_VALUE ? 0 : ::GetProcessId(m_hHandle)); }
		bool GetMemoryInfo (SMemoryInfo *retInfo) const;
	};

class CSemaphore : public COSObject
	{
	public:
		void Create (int iMaxCount) { Create(NULL_STR, iMaxCount); }
		void Create (const CString &sName, int iMaxCount, bool *retbExists = NULL);
		void Decrement (int iCount = 1) const;
		int GetMaxCount (void) const { return m_iMaxCount; }
		void Increment (int iCount = 1) const;
		void Lock () const { Increment(); }
		bool TryIncrement (int iCount = 1, DWORD dwTimeout = 0);
		void Unlock () const { Decrement(); }

	private:
		int m_iMaxCount;
	};

class CReaderWriterSemaphore : private CSemaphore
	{
	public:
		void Create (int iMaxCount) { CSemaphore::Create(NULL, iMaxCount); }
		void Create (LPSTR sName, int iMaxCount) { CSemaphore::Create(sName, iMaxCount); }

		void LockReader (void) { CSemaphore::Increment(1); }
		void LockWriter (void) { CSemaphore::Increment(GetMaxCount()); }
		void UnlockReader (void) { CSemaphore::Decrement(1); }
		void UnlockWriter (void) { CSemaphore::Decrement(GetMaxCount()); }
	};

//	Functions

inline DWORD sysGetCurrentProcessID (void) { return ::GetCurrentProcessId(); }
inline DWORD sysGetCurrentThreadID (void) { return ::GetCurrentThreadId(); }
