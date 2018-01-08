//	FoundationIOCompletionPort.h
//
//	Foundation header file
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IIOCPEntry
	{
	public:
		enum EOperations
			{
			opNone,
			opConnect,
			opRead,
			opWrite,
			};

		IIOCPEntry (const CString &sType) : 
				m_dwID(0),
				m_sType(sType),
				m_iCurrentOp(opNone),
				m_dwOpStartTime(0),
				m_bDeleteOnCompletion(false)
			{
			}

		bool BeginConnection (const CString &sAddress, DWORD dwPort, CString *retsError);
		bool BeginRead (CString *retsError);
		bool BeginWrite (const CString &sData, CString *retsError);
		inline EOperations GetCurrentOp (void) const { return m_iCurrentOp; }
		inline DWORD GetCurrentOpStartTime (void) const { return m_dwOpStartTime; }
		inline DWORD GetID (void) const { return m_dwID; }
		inline const CString &GetType (void) const { return m_sType; }
		inline bool IsDeleted (void) const { return m_bDeleteOnCompletion; }
		void OperationComplete (DWORD dwBytesTransferred);
		void OperationFailed (void);
		void Process (void);
		inline void SetDeleteOnCompletion (void) { m_bDeleteOnCompletion = true; }
		inline void SetID (DWORD dwID) { m_dwID = dwID; }
		bool TimeoutCheck (DWORD dwNow, DWORD dwTimeout);

		//	IIOCPEntry interface

		virtual ~IIOCPEntry (void) { }
		virtual IMemoryBlock *GetBuffer (void) { return NULL; }
		virtual HANDLE GetCompletionHandle (void) const { return INVALID_HANDLE_VALUE; }

	protected:

		//	IIOCPEntry overrides

		virtual void OnBeginRead (void) { }
		virtual void OnBeginWrite (const CString &sData) { }
		virtual void OnOperationComplete (EOperations iOp, DWORD dwBytesTransferred) { }
		virtual void OnOperationFailed (EOperations iOp) { }
		virtual void OnProcess (void) { }

	private:
		DWORD m_dwID;
		CString m_sType;
		OVERLAPPED m_Overlapped;
		DWORD m_dwOpStartTime;				//	Tick when we started current operation

		EOperations m_iCurrentOp;
		bool m_bDeleteOnCompletion;			//	Delete when we complete operation
	};

class CIOCPSocket : public IIOCPEntry
	{
	public:
		CIOCPSocket (SOCKET hSocket);
		virtual ~CIOCPSocket (void);

		bool ResetSocket (void);

		//	IIOCPEntry interface

		virtual IMemoryBlock *GetBuffer (void) override { return &m_Buffer; }
		virtual HANDLE GetCompletionHandle (void) const override { return (HANDLE)m_hSocket; }

	protected:

		//	IIOCPEntry overrides

		virtual void OnBeginRead (void) override { SetReadBufferLen(); }
		virtual void OnBeginWrite (const CString &sData) override { SetWriteBuffer(sData); }
		virtual void OnOperationComplete (EOperations iOp, DWORD dwBytesTransferred) override { OnSocketOperationComplete(iOp, dwBytesTransferred); }
		virtual void OnOperationFailed (EOperations iOp) override { OnSocketOperationFailed(iOp); }

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred) { }
		virtual void OnSocketOperationFailed (EOperations iOp) { }

		//	Helpers

		CBuffer &GetBufferRaw (void) { return m_Buffer; }

	private:
		void SetReadBufferLen ();
		void SetWriteBuffer (const CString &sData);

		SOCKET m_hSocket;
		CBuffer m_Buffer;
	};

class CIOCompletionPort
	{
	public:
		CIOCompletionPort (void);
		~CIOCompletionPort (void);

		void AddObject (IIOCPEntry *pObject);
		bool ProcessCompletion (IIOCPEntry **retpObject, DWORD *retdwBytesTransferred);
		void SignalEvent (IIOCPEntry *pObject);

	private:
		HANDLE m_hIOCP;
		int m_iThreads;
		CCriticalSection m_cs;
	};
