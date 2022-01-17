//	EsperImpl.h
//
//	Esper Implementation Classes and Functions
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "OpenSSLUtil.h"

class CEsperAMP1ConnectionIn : public CEsperConnection
	{
	public:
		CEsperAMP1ConnectionIn (CEsperConnectionManager &Manager, const CString &sClientAddr, SOCKET hSocket);

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual void ClearBusy (void) override;
		virtual void OnConnect (void) override;
		virtual CDatum GetProperty (const CString &sProperty) const override;
		virtual bool SetBusy (void) override;
		virtual bool SetProperty (const CString &sProperty, CDatum dValue) override;

	protected:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperations iOp) override;

	private:
		enum EStates
			{
			stateNone,						//	Not busy
			stateBusy,						//	BeginXXX is about to be called

			stateReadingHeader,
			stateReadingHeaderContinues,
			stateReadingData,
			stateWriting,
			};

		bool GetHeader (const IMemoryBlock &Data, CString *retsCommand, DWORD *retdwDataLen, char **retpPartialData, DWORD *retdwPartialDataLen);
		void OpReadRequest (EStates iState);
		void OpSendAMPMessage (const CString &sCommand, IMemoryBlock &Data);

		static bool ParseHeaderWord (char *pPos, char *pPosEnd, CString *retsWord, char **retpPos);

		CEsperConnectionManager &m_Manager;
		CString m_sClientAddr;

		CString m_sMachineName;

		EStates m_iState;
		CString m_sCommand;
		DWORD m_dwDataLen;
		CBuffer m_Data;
	};

class CEsperAMP1ConnectionOut : public CEsperConnection
	{
	public:
		CEsperAMP1ConnectionOut (CEsperConnectionManager &Manager, const CString &sHostConnection, const CString &sAddress, DWORD dwPort);
		~CEsperAMP1ConnectionOut (void);

		//	CEsperConnection virtuals

		virtual void AccumulateResult (TArray<CString> &Result) override;
		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual bool BeginAMP1Request (const SArchonMessage &Msg, const SAMP1Request &Request, CString *retsError) override;
		virtual void ClearBusy (void) override;
		virtual const CString &GetHostConnection (void) override { return m_sHostConnection; }
		virtual CDatum GetProperty (const CString &sProperty) const override;
		virtual bool SetBusy (void) override;

	protected:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperations iOp) override;

	private:
		enum EStates
			{
			stateConnected,					//	Not busy, connected
			stateConnectedBusy,				//	Connected, waiting for BeginXXX
			stateDisconnected,				//	Not busy, not connected
			stateDisconnectedBusy,			//	Disconnected, waiting for BeginXXX

			stateWaitForConnect,
			stateWaitForAuthAck,
			stateWaitForAuthResponse,
			stateWaitForRequestAck,
			stateWaitForResponse,

			stateSendRequest,
			};

		virtual void OnMark () override { m_Msg.dPayload.Mark(); }
		bool OpConnect (bool bReconnect = false);
		bool OpRead (EStates iNewState = stateWaitForResponse);
		bool OpSendAuth (void);
		bool OpSendRequest (void);
		bool OpTransmissionFailed (const CString &sError);
		bool OpWrite (const CString &sData, EStates iNewState);
		bool SerializeAMP1Request (const CString &sCommand, CDatum dData, CStringBuffer &Stream, CString *retsError);

		//	Set at creation time

		CEsperConnectionManager &m_Manager;
		CString m_sHostConnection;
		CString m_sAddress;
		DWORD m_dwPort = 0;
		CString m_sAuthName;
		CIPInteger m_AuthKey;

		EStates m_iState = stateDisconnected;

		//	Set per request

		SArchonMessage m_Msg;
		CStringBuffer m_RequestBuffer;
		CString m_sLastResult;
		bool m_bReconnect = false;
		bool m_bResetBuffer = false;
		bool m_bDeleteWhenDone = false;
	};

class CEsperHTTPOutConnection : public CEsperConnection
	{
	public:
		CEsperHTTPOutConnection (CEsperConnectionManager &Manager, const CString &sHostConnection, const CString &sAddress, DWORD dwPort);
		~CEsperHTTPOutConnection (void);

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual bool BeginHTTPRequest (const SArchonMessage &Msg, const SHTTPRequest &Request, CString *retsError) override;
		virtual void ClearBusy (void) override;
		virtual const CString &GetHostConnection (void) override { return m_sHostConnection; }
		virtual bool SetBusy (void) override;

	protected:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperations iOp) override;

	private:
		enum EStates
			{
			stateNone,

			stateConnected,					//	Not busy, connected
			stateConnectedBusy,				//	Connected, waiting for BeginXXX
			stateDisconnected,				//	Not busy, not connected
			stateDisconnectedBusy,			//	Disconnected, waiting for BeginXXX

			stateWaitForConnect,
			stateWaitForRequestAck,
			stateWaitForResponse,

			stateSendRequest,
			stateWaitToReceiveSSLData,
			stateWaitToSendSSLData,
			stateWaitToSendSSLDataThenReceive,
			stateWaitToSendSSLDataThenReady,
			};

		virtual void OnMark () override { m_Msg.dPayload.Mark(); }
		void OnSSLOperationComplete (void);

		bool OpConnect (bool bReconnect = false);
		bool OpMessageComplete (void);
		bool OpProcessReceivedData (const IMemoryBlock &Buffer);
		bool OpProcessSSL (void);
		bool OpRead (EStates iNewState = stateWaitForResponse);
		bool OpReceiveResponse (void);
		bool OpSendRequest (void);
		bool OpTransmissionFailed (void);
		bool OpWrite (const CString &sData, EStates iNewState);

		//	Set at creation time

		CEsperConnectionManager &m_Manager;
		CString m_sHostConnection;

		//	Set at first connect time

		CString m_sAddress;
		DWORD m_dwPort;
		bool m_bSSL;
		CSSLAsyncEngine *m_pSSL;

		//	Set per request

		EStates m_iState;
		EStates m_iSSLSavedState;
		SArchonMessage m_Msg;
		CHTTPMessage m_HTTPMessage;
		bool m_bProxy;
		bool m_bRaw;
		bool m_bReconnect;
		bool m_bResetBuffer;
	};

class CEsperSimpleConnection : public CEsperConnection
	{
	public:
		CEsperSimpleConnection (CEsperConnectionManager &Manager, SOCKET hSocket);

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual bool BeginRead (const SArchonMessage &Msg, CString *retsError) override;
		virtual bool BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError) override;
		virtual void ClearBusy (void) override;
		virtual bool SetBusy (void) override;

	protected:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperations iOp) override;

	private:
		enum EStates
			{
			stateNone,						//	Not busy
			stateBusy,						//	BeginXXX is about to be called

			stateReplyOnRead,
			stateReplyOnWrite,
			};

		virtual void OnMark () override { m_Msg.dPayload.Mark(); }

		CEsperConnectionManager &m_Manager;

		EStates m_iState;
		SArchonMessage m_Msg;
	};

class CEsperTLSConnectionIn : public CEsperConnection
	{
	public:
		CEsperTLSConnectionIn (CEsperConnectionManager &Manager, const CString &sListener, const CString &sNetworkAddress, CSSLCtx &SSLCtx, const SArchonMessage &Msg, SOCKET hSocket);
		~CEsperTLSConnectionIn (void);

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual bool BeginRead (const SArchonMessage &Msg, CString *retsError) override;
		virtual bool BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError) override;
		virtual void ClearBusy (void) override;
		virtual void OnConnect (void) override;
		virtual bool SetBusy (void) override;
		virtual bool SetProperty (const CString &sProperty, CDatum dValue) override;

	protected:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperations iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperations iOp) override;

	private:
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
			stateWaitToSendSSLDataThenReady,
			};

		virtual void OnMark () override { m_Msg.dPayload.Mark(); }
		void OnSSLOperationComplete (void);
		bool OpProcessSSL (CString *retsError = NULL);
		bool OpRead (EStates iNewState);
		bool OpWrite (const CString &sData, EStates iNewState);

		CEsperConnectionManager &m_Manager;
		CString m_sListener;
		CString m_sNetworkAddress;

		EStates m_iState;
		EStates m_iSSLSavedState;
		DWORD m_dwBytesToSend;
		SArchonMessage m_Msg;

		CSSLAsyncEngine *m_pSSL;
	};

