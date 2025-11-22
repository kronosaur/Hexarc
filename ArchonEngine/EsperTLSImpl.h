//	EsperTLSImpl.h
//
//	Esper Implementation Classes and Functions
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

class CEsperTLSConnectionImpl : public CEsperConnection
	{
	public:

		CEsperTLSConnectionImpl (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLCtx& SSLCtx, SOCKET hSocket);
		CEsperTLSConnectionImpl (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLAsyncEngine* pSSL, SOCKET hSocket);
		virtual ~CEsperTLSConnectionImpl ();

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus* ioStatus) override;
		virtual bool BeginRead (const SArchonMessage& Msg, CString* retsError) override;
		virtual bool BeginWrite (const SArchonMessage& Msg, const CString& sData, CString* retsError) override;
		virtual bool IsBusy () const override { CSmartLock Lock(m_cs); return !IsDeleted() && (m_bReadRequested || m_bWriteRequested || m_iReadState != stateNone || m_iWriteState != stateNone); }
		virtual void OnConnect () override;
		virtual bool SetBusy (EOperation iOperation) override;
		virtual bool SetProperty (const CString& sProperty, CDatum dValue) override;
		virtual CEsperConnection* UpgradeWebSocket () override;

	protected:

		void DeleteConnection ();

		CEsperConnectionManager& m_Manager;
		CString m_sListener;
		CString m_sNetworkAddress;

	private:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperation iOp, CStringView sError) override;

		//	CEsperTLSConnectionImpl

		virtual void OnTLSBeginRead (const SArchonMessage& Msg) { }
		virtual void OnTLSBeginWrite (const SArchonMessage& Msg) { }
		virtual void OnTLSConnect () { }
		virtual void OnTLSDisconnect () { }
		virtual void OnTLSRead (CString&& sData) { }
		virtual void OnTLSWriteComplete (DWORD dwBytesWritten) { }
		virtual void OnTLSMark () { }

		enum EStates
			{
			stateNone,
			stateBusy,						//	BeginXXX is about to be called

			stateReplyOnConnect,			//	Send OnConnect to caller when done
			stateReplyOnRead,				//	Send OnRead to caller when done
			stateReplyOnWrite,				//	Send OnWrite to caller when done

			stateWaitToReceiveSSLData,
			stateWaitToSendSSLData,
			stateWaitToSendSSLDataThenReceive,
			};

		virtual void OnMark () override { OnTLSMark(); }

		void OnSSLOperationComplete (EOperation iOperation);
		bool OpProcessSSL (CString *retsError = NULL);
		bool OpRead (EStates iNewState);
		bool OpWrite (const CString& sData, EStates iNewState);

		EStates m_iReadState = stateNone;
		EStates m_iWriteState = stateNone;
		DWORD m_dwBytesToSend = 0;
		bool m_bConnectedRequested = false;
		bool m_bReadRequested = false;		//	Client requested read
		bool m_bWriteRequested = false;		//	Client requested write
		bool m_bReadSubmitted = false;		//	We have an outstanding read on the socket
		bool m_bWriteSubmitted = false;		//	We have an outstanding write on the socket

		CSSLAsyncEngine* m_pSSL = NULL;

#ifdef DEBUG_PERF
		void DebugPerfReset ()
			{
			//	Set to 0 so that we know to initialize as soon as we get the first byte
			//	of input (we're trying to measure read speed, not wait time).

			m_dwStartOp = 0;
			}

		void DebugPerfInit ()
			{
			if (m_dwStartOp == 0)
				{
				m_dwStartOp = ::sysGetTickCount64();
				m_dwBytesTransmitted = 0;
				}
			}

		void DebugPerfData (DWORD dwBytes) { m_dwBytesTransmitted += dwBytes; }

		void DebugPerfReport ()
			{
			if (m_dwStartOp != 0)
				{
				DWORD dwTime = (DWORD)::sysGetTicksElapsed(m_dwStartOp);
				if (dwTime >= 100)
					printf("DebugPerf: Transmitted %d bytes in %d.%ds\n", m_dwBytesTransmitted, (dwTime / 1000), (dwTime % 1000) / 100);

				m_dwStartOp = 0;
				}
			}

		DWORDLONG m_dwStartOp = 0;			//	Tick on which we started to read or write
		DWORD m_dwBytesTransmitted = 0;
#else
		void DebugPerfReset () { }
		void DebugPerfInit () { }
		void DebugPerfData (DWORD dwBytes) { }
		void DebugPerfReport () { }
#endif
	};
