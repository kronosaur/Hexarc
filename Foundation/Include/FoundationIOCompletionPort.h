//	FoundationIOCompletionPort.h
//
//	Foundation header file
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CIODiagnostics
	{
	public:

		CString AsString () const;
		bool IsEnabled () const { return m_bEnabled; }
		void Log (CStringView sLine);
		template <typename FUNC> void LogFn (FUNC&& f) { if (m_bEnabled) Log(f()); }
		void SetEnabled (bool bEnabled = true) { m_bEnabled = bEnabled; }

	private:
		bool m_bEnabled = false;
		TArray<CString> m_Log;
	};

class IIOCPEntry
	{
	public:

		enum class EOperation
			{
			none,
			connect,
			read,
			write,
			};

		IIOCPEntry (CStringView sType, EOperation iOp = EOperation::none) : 
				m_sType(sType)
			{
			//	If we pass in iOp == connect, then it means we already have a connection

			if (iOp == EOperation::connect)
				{
				m_bConnected = true;
				m_dwLastActivity = ::sysGetTickCount64();
				}
			}

		bool BeginConnection (CStringView sAddress, DWORD dwPort, CString* retsError);
		bool BeginRead (CString* retsError);
		bool BeginWrite (CStringView sData, CString* retsError);
		CIODiagnostics& GetDiagnostics () { return m_Diagnostics; }
		DWORD GetID () const { return m_dwID; }
		DWORDLONG GetLastActivityTime (void) const { return m_dwLastActivity; }
		CStringView GetType () const { return m_sType; }
		bool IsDeleted () const { return m_bDeleted; }
		bool IsMarkedForDelete () const { return m_bMarkedForDelete; }
		bool IsOperationInProgress () const { return m_bConnecting || m_dwReadStartTime || m_dwWriteStartTime; }
		bool OperationComplete (DWORD dwBytesTransferred, OVERLAPPED* pOverlapped = NULL);
		void OperationFailed (OVERLAPPED* pOverlapped = NULL);
		void Process ();
		void SetDeleted () { m_bDeleted = true; }
		void SetMarkedForDelete () { m_bMarkedForDelete = true; }
		void SetEnableDiagnostics (bool bEnable) { m_Diagnostics.SetEnabled(bEnable); }
		void SetID (DWORD dwID) { m_dwID = dwID; }
		bool TimeoutCheck (DWORDLONG dwNow, DWORDLONG dwTimeout);

		//	IIOCPEntry interface

		virtual ~IIOCPEntry () { }
		virtual IMemoryBlock* GetReadBuffer () { return NULL; }
		virtual IMemoryBlock* GetWriteBuffer () { return NULL; }
		virtual HANDLE GetCompletionHandle () const { return INVALID_HANDLE_VALUE; }

	protected:

		void OnResetConnection () { m_bConnecting = false; m_bConnected = false; m_dwReadStartTime = 0; m_dwWriteStartTime = 0; }

		CCriticalSection m_cs;

	private:

		//	IIOCPEntry overrides

		virtual bool CreateConnection (CStringView sAddress, DWORD dwPort, OVERLAPPED& Overlapped, CString* retsError = NULL) { return false; }
		virtual void OnBeginRead () { }
		virtual void OnBeginWrite (CStringView sData) { }
		virtual void OnOperationComplete (EOperation iOp, DWORD dwBytesTransferred) { }
		virtual void OnOperationFailed (EOperation iOp, CStringView sError) { }
		virtual void OnProcess () { }

		bool WriteBuffer (CString *retsError = NULL);

		DWORD m_dwID = 0;
		CString m_sType;

		bool m_bConnecting = false;				//	TRUE if we are in the middle of connecting
		bool m_bConnected = false;				//	TRUE if we are connected
		bool m_bDeleted = false;				//	Deleted
		bool m_bMarkedForDelete = false;		//	Delete next time we process
		DWORDLONG m_dwLastActivity = 0;

		OVERLAPPED m_OverlappedRead = { 0 };
		DWORDLONG m_dwReadStartTime = 0;		//	Tick when we started current operation (0 = no read outstanding)

		OVERLAPPED m_OverlappedWrite = { 0 };
		DWORDLONG m_dwWriteStartTime = 0;		//	Tick when we started current operation (0 = no write outstanding)
		DWORD m_dwWriteOffset = 0;				//	Offset in buffer for write

		CIODiagnostics m_Diagnostics;
	};

class CIOCPSocket : public IIOCPEntry
	{
	public:

		CIOCPSocket (SOCKET hSocket);
		CIOCPSocket (CStringView sAddress, DWORD dwPort);
		CIOCPSocket (const CIOCPSocket& Src) = delete;
		CIOCPSocket (CIOCPSocket&& Src) = delete;

		virtual ~CIOCPSocket ();

		CIOCPSocket& operator= (const CIOCPSocket& Src) = delete;
		CIOCPSocket& operator= (CIOCPSocket&& Src) = delete;

		SOCKET GetSocket () const { return m_hSocket; }
		SOCKET GetSocketHandoff () { SOCKET hSocket = m_hSocket; m_hSocket = INVALID_SOCKET; return hSocket; }
		bool ResetSocket ();

		//	IIOCPEntry interface

		virtual IMemoryBlock* GetReadBuffer () override { return &m_ReadBuffer; }
		virtual IMemoryBlock* GetWriteBuffer () override { return &m_WriteBuffer; }
		virtual HANDLE GetCompletionHandle () const override { return (HANDLE)m_hSocket; }

	private:

		//	IIOCPEntry overrides

		virtual bool CreateConnection (CStringView sAddress, DWORD dwPort, OVERLAPPED& Overlapped, CString* retsError = NULL) override;
		virtual void OnBeginRead () override { SetReadBufferLen(); }
		virtual void OnBeginWrite (CStringView sData) override { SetWriteBuffer(sData); }
		virtual void OnOperationComplete (EOperation iOp, DWORD dwBytesTransferred) override { OnSocketOperationComplete(iOp, dwBytesTransferred); }
		virtual void OnOperationFailed (EOperation iOp, CStringView sError) override { OnSocketOperationFailed(iOp, sError); }

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred) { }
		virtual void OnSocketOperationFailed (EOperation iOp, CStringView sError) { }


		void SetReadBufferLen ();
		void SetWriteBuffer (CStringView sData);

		SOCKET m_hSocket = INVALID_SOCKET;
		CWSAddrInfo m_AI;
		CBuffer m_ReadBuffer;
		CBuffer m_WriteBuffer;
	};

class CIOCompletionPort
	{
	public:

		struct SResult
			{
			DWORD_PTR Ctx = 0;
			DWORD dwBytesTransferred = 0;
			OVERLAPPED* pOverlapped = NULL;
			};

		CIOCompletionPort ();
		CIOCompletionPort (const CIOCompletionPort& Src) = delete;
		CIOCompletionPort (CIOCompletionPort&& Src) = delete;

		~CIOCompletionPort ();

		CIOCompletionPort& operator= (const CIOCompletionPort& Src) = delete;
		CIOCompletionPort& operator= (CIOCompletionPort&& Src) = delete;

		void AddObject (HANDLE hHandle, DWORD_PTR Ctx);
		bool ProcessCompletion (SResult& retResult);
		void SignalEvent (DWORD_PTR Ctx);

	private:

		HANDLE m_hIOCP;
		int m_iThreads = 0;
		CCriticalSection m_cs;
	};
