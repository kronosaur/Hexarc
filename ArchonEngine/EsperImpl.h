//	EsperImpl.h
//
//	Esper Implementation Classes and Functions
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#pragma once

#include "OpenSSLUtil.h"
#include "EsperTLSImpl.h"

class CEsperAMP1ConnectionIn : public CEsperConnection
	{
	public:
		CEsperAMP1ConnectionIn (CEsperConnectionManager &Manager, const CString &sClientAddr, SOCKET hSocket);

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual void OnConnect (void) override;
		virtual CDatum GetProperty (const CString &sProperty) const override;
		virtual bool IsBusy () const override { return false; }
		virtual bool SetBusy (EOperation iOperation) override;
		virtual bool SetProperty (const CString &sProperty, CDatum dValue) override;

	private:

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperation iOp, CStringView sError) override;

		enum EStates
			{
			stateNone,						//	Not busy
			stateBusy,						//	BeginXXX is about to be called

			stateReadingHeader,
			stateReadingHeaderContinues,
			stateReadingData,
			stateWriting,
			};

		void OpReadRequest (EStates iState);
		void OpSendAMPMessage (const CString &sCommand, IMemoryBlock &Data);

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
		virtual const CString &GetHostConnection (void) override { return m_sHostConnection; }
		virtual CDatum GetProperty (const CString &sProperty) const override;
		virtual bool IsBusy () const override { return !IsDeleted() && m_iState != stateConnected && m_iState != stateDisconnected; }
		virtual bool SetBusy (EOperation iOperation) override;

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

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperation iOp, CStringView sError) override;

		//	CEsperConnection overrides

		virtual void OnMark () override 
			{
			DEBUG_TRY
			m_Msg.dPayload.Mark();
			DEBUG_CATCH
			}

		void DeleteConnection ();
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
		virtual const CString &GetHostConnection (void) override { return m_sHostConnection; }
		virtual bool IsBusy () const override { return !IsDeleted() && m_iState != stateConnected && m_iState != stateDisconnected; }
		virtual bool SetBusy (EOperation iOperation) override;

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

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperation iOp, CStringView sError) override;

		//	CEsperConnection overrides

		virtual void OnMark () override 
			{
			DEBUG_TRY
			m_Msg.dPayload.Mark();
			DEBUG_CATCH
			}

		void DeleteConnection ();
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
		DWORD m_dwPort = 0;
		bool m_bSSL = false;
		CSSLAsyncEngine *m_pSSL = NULL;

		//	Set per request

		EStates m_iState = stateDisconnected;
		EStates m_iSSLSavedState = stateNone;
		SArchonMessage m_Msg;
		CHTTPMessage m_HTTPMessage;
		bool m_bProxy = false;
		bool m_bRaw = false;
		bool m_bReconnect = false;
		bool m_bResetBuffer = false;
	};

class CEsperSimpleConnection : public CEsperConnection
	{
	public:

		CEsperSimpleConnection (CEsperConnectionManager &Manager, SOCKET hSocket);

		//	CEsperConnection virtuals

		virtual void AccumulateStatus (SStatus *ioStatus) override;
		virtual bool BeginRead (const SArchonMessage &Msg, CString *retsError) override;
		virtual bool BeginWrite (const SArchonMessage &Msg, const CString &sData, CString *retsError) override;
		virtual bool IsBusy () const override { return !IsDeleted() && m_iState != stateNone; }
		virtual bool SetBusy (EOperation iOperation) override;

	private:

		enum EStates
			{
			stateNone,						//	Not busy
			stateBusy,						//	BeginXXX is about to be called

			stateReplyOnRead,
			stateReplyOnWrite,
			};

		//	CIOCPSocket overrides

		virtual void OnSocketOperationComplete (EOperation iOp, DWORD dwBytesTransferred) override;
		virtual void OnSocketOperationFailed (EOperation iOp, CStringView sError) override;

		//	CEsperConnection overrides

		virtual void OnMark () override
			{
			DEBUG_TRY
			m_Msg.dPayload.Mark();
			DEBUG_CATCH
			}

		CEsperConnectionManager &m_Manager;

		EStates m_iState;
		SArchonMessage m_Msg;
	};

class CEsperTLSConnectionIn : public CEsperTLSConnectionImpl
	{
	public:

		CEsperTLSConnectionIn (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLCtx& SSLCtx, const SArchonMessage& Msg, SOCKET hSocket);
#ifdef DEBUG_MARK_CRASH
		virtual ~CEsperTLSConnectionIn ();
#endif

	private:

		//	CEsperTLSConnectionImpl

		virtual void OnTLSBeginRead (const SArchonMessage& Msg) override { m_Msg = Msg; }
		virtual void OnTLSBeginWrite (const SArchonMessage& Msg) override { m_Msg = Msg; }
		virtual void OnTLSConnect () override;
		virtual void OnTLSDisconnect () override;
		virtual void OnTLSRead (CString&& sData) override;
		virtual void OnTLSWriteComplete (DWORD dwBytesWritten) override;
		virtual void OnTLSMark () override { m_Msg.dPayload.Mark(); }

		SArchonMessage m_Msg;
	};

class CEsperWSSConnectionIn : public CEsperTLSConnectionImpl
	{
	public:

		CEsperWSSConnectionIn (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLCtx& SSLCtx, SOCKET hSocket);
		CEsperWSSConnectionIn (CEsperConnectionManager& Manager, CStringView sListener, CStringView sNetworkAddress, CSSLAsyncEngine* pSSL, SOCKET hSocket);

		//	CEsperConnection

		virtual bool SendWSMessage (CDatum dMessage, CString* retsError) override;

	private:

		//	CEsperTLSConnectionImpl

		virtual void OnTLSConnect () override;
		virtual void OnTLSDisconnect () override;
		virtual void OnTLSRead (CString&& sData) override;
		virtual void OnTLSWriteComplete (DWORD dwBytesWritten) override;

		//	CEsperConnection

		virtual void OnUpgradedToWebSocket (CDatum dConnectInfo, CStringView sKey) override;

		void DisconnectWithError (CStringView sError);
		void ReadData ();
		void SendHTTPBadRequest ();
		void SendHTTPResponse (CHTTPMessage& Response);
		void SendWSOnMessage (CDatum dMessage);
		void SendWSOnDisconnect ();
		void SendWSFrame (CWebSocketProtocol::EOpCode iOpCode, CStringView sData);
		void WriteData (CStringView sData);

		CString m_sClientAddress;			//	Address to send to
		CString m_sClientMsg;				//	Message to send to client
		CString m_sAPI;						//	API string for client

		CWebSocketProtocol m_Protocol;
		TArray<CString> m_Output;
		bool m_bDisconnected = false;
	};

