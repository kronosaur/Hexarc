//	MsgAMP1Test.cpp
//
//	AMP1 diagnostics
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(STR_MSG_AMP1_TEST_DR_HOUSE_AMP1_SOCKET_CLIENT,	"DrHouse AMP1 socket client");
DECLARE_CONST_STRING(STR_MSG_AMP1_TEST_DR_HOUSE_AMP1_SOCKET_SERVER,	"DrHouse AMP1 socket server");

DECLARE_CONST_STRING(ADDR_ESPER_COMMAND,				"Esper.command@~/~")
DECLARE_CONST_STRING(AMP1_PING,							"PING")
DECLARE_CONST_STRING(AMP1_REPLY_OK,						"AMP/1.00 OK\r\n")
DECLARE_CONST_STRING(FIELD_ACK,							"ack")
DECLARE_CONST_STRING(FIELD_COMMAND,						"command")
DECLARE_CONST_STRING(FIELD_LISTENER,					"listener")
DECLARE_CONST_STRING(FIELD_MESSAGE_COUNT,				"messageCount")
DECLARE_CONST_STRING(FIELD_PAYLOAD,						"payload")
DECLARE_CONST_STRING(FIELD_PORT,						"port")
DECLARE_CONST_STRING(FIELD_REQUEST,						"request")
DECLARE_CONST_STRING(FIELD_STATUS,						"status")
DECLARE_CONST_STRING(FIELD_TEST,						"test")
DECLARE_CONST_STRING(FIELD_TIMEOUT,						"timeoutMs")
DECLARE_CONST_STRING(MSG_DRHOUSE_AMP1_TEST,			"DrHouse.amp1Test")
DECLARE_CONST_STRING(MSG_ERROR_TIMEOUT,					"Error.timeout")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_ESPER_AMP1,					"Esper.amp1")
DECLARE_CONST_STRING(MSG_ESPER_ON_AMP1,					"Esper.onAMP1")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STARTED,		"Esper.onListenerStarted")
DECLARE_CONST_STRING(MSG_ESPER_ON_LISTENER_STOPPED,		"Esper.onListenerStopped")
DECLARE_CONST_STRING(MSG_ESPER_START_LISTENER,			"Esper.startListener")
DECLARE_CONST_STRING(MSG_ESPER_STOP_LISTENER,			"Esper.stopListener")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")
DECLARE_CONST_STRING(PROTOCOL_AMP1,						"amp1")
DECLARE_CONST_STRING(PROTOCOL_AMP1_00,					"AMP/1.00")
DECLARE_CONST_STRING(STR_LOCALHOST,						"127.0.0.1")
DECLARE_CONST_STRING(STR_STATUS_PASS,					"pass")
DECLARE_CONST_STRING(TEST_ALL,							"all")
DECLARE_CONST_STRING(TEST_INBOUND_PING,					"inboundPing")
DECLARE_CONST_STRING(TEST_INBOUND_SPLIT_HEADER_AND_PAYLOAD,	"inboundSplitHeaderAndPayload")
DECLARE_CONST_STRING(TEST_INBOUND_SPLIT_HEADER,			"inboundSplitHeader")
DECLARE_CONST_STRING(TEST_INBOUND_SPLIT_HEADER_BYTEWISE,	"inboundSplitHeaderBytewise")
DECLARE_CONST_STRING(TEST_INBOUND_SPLIT_PAYLOAD,		"inboundSplitPayload")
DECLARE_CONST_STRING(TEST_INBOUND_SPLIT_PAYLOAD_BYTEWISE,	"inboundSplitPayloadBytewise")
DECLARE_CONST_STRING(TEST_INBOUND_PIPELINED_MESSAGES,	"inboundPipelinedMessages")
DECLARE_CONST_STRING(TEST_INBOUND_TWO_MESSAGES_SAME_SOCKET,	"inboundTwoMessagesSameSocket")
DECLARE_CONST_STRING(TEST_OUTBOUND_PING,				"outboundPing")
DECLARE_CONST_STRING(TEST_OUTBOUND_SILENT_PEER,			"outboundSilentPeer")
DECLARE_CONST_STRING(TEST_OUTBOUND_TWO_MESSAGES_SAME_SOCKET,	"outboundTwoMessagesSameSocket")

DECLARE_CONST_STRING(ERR_ACK_TIMEOUT,					"Timed out waiting for AMP1 socket ACK.")
DECLARE_CONST_STRING(ERR_BAD_ACK,						"Unexpected AMP1 ACK: %s")
DECLARE_CONST_STRING(ERR_BAD_COMMAND,					"Unexpected AMP1 command: %s")
DECLARE_CONST_STRING(ERR_BAD_OUTBOUND_PAYLOAD,			"Unexpected outbound AMP1 payload.")
DECLARE_CONST_STRING(ERR_BAD_OUTBOUND_REQUEST,			"Unexpected outbound AMP1 request: %s")
DECLARE_CONST_STRING(ERR_BAD_PAYLOAD,					"Unexpected AMP1 payload.")
DECLARE_CONST_STRING(ERR_CONNECT_FAILED,				"Unable to connect to AMP1 listener on port %d.")
DECLARE_CONST_STRING(ERR_INVALID_TEST,					"Unknown AMP1 test: %s")
DECLARE_CONST_STRING(ERR_LISTENER_FAILED,				"Unable to start AMP1 listener.")
DECLARE_CONST_STRING(ERR_LISTEN_FAILED,					"Unable to listen for outbound AMP1 connection on port %d.")
DECLARE_CONST_STRING(ERR_NO_TEST,						"No AMP1 test specified.")
DECLARE_CONST_STRING(ERR_READ_FAILED,					"Unable to read AMP1 ACK.")
DECLARE_CONST_STRING(ERR_SERVER_ACCEPT_TIMEOUT,			"Timed out waiting for outbound AMP1 connection.")
DECLARE_CONST_STRING(ERR_SERVER_READ_FAILED,			"Unable to read outbound AMP1 request.")
DECLARE_CONST_STRING(ERR_SERVER_WRITE_FAILED,			"Unable to write outbound AMP1 ACK.")
DECLARE_CONST_STRING(ERR_WRITE_FAILED,					"Unable to write AMP1 test chunk.")
DECLARE_CONST_STRING(ERR_SUITE_FAILED,					"AMP1 test suite failed at %s: %s")
DECLARE_CONST_STRING(ERR_UNEXPECTED_REPLY,				"Unexpected reply while running AMP1 test suite: %s")

DECLARE_CONST_STRING(PORT_DRHOUSE_COMMAND,				"DrHouse.command")

static constexpr DWORD DEFAULT_TIMEOUT =					10000;
static constexpr DWORD SOCKET_TIMEOUT =					5000;
static constexpr DWORD CHUNK_DELAY =						150;

class CAMP1SocketClientThread : public TThread<CAMP1SocketClientThread>
	{
	public:
		CAMP1SocketClientThread () : TThread(STR_MSG_AMP1_TEST_DR_HOUSE_AMP1_SOCKET_CLIENT) { m_Done.Create(); }

		const CString &GetAck () const { return m_sAck; }
		const CString &GetError () const { return m_sError; }
		bool IsSuccess () const { return m_sError.IsEmpty(); }

		void Init (DWORD dwPort, TArray<TArray<CString>> &&MessageChunks, bool bPipeline)
			{
			m_dwPort = dwPort;
			m_MessageChunks = std::move(MessageChunks);
			m_bPipeline = bPipeline;
			}

		void Run ()
			{
			CSocket Socket;

			if (!Socket.Connect(STR_LOCALHOST, m_dwPort))
				{
				m_sError = strPattern(ERR_CONNECT_FAILED, m_dwPort);
				m_Done.Set();
				return;
				}

			DWORD dwTimeout = SOCKET_TIMEOUT;
			::setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&dwTimeout, sizeof(dwTimeout));
			::setsockopt(Socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&dwTimeout, sizeof(dwTimeout));

			if (m_bPipeline)
				{
				for (int iMessage = 0; iMessage < m_MessageChunks.GetCount(); iMessage++)
					if (!WriteChunks(Socket, m_MessageChunks[iMessage]))
						{
						m_Done.Set();
						return;
						}

				for (int iMessage = 0; iMessage < m_MessageChunks.GetCount(); iMessage++)
					if (!ReadAck(Socket))
						{
						m_Done.Set();
						return;
						}
				}
			else
				{
				for (int iMessage = 0; iMessage < m_MessageChunks.GetCount(); iMessage++)
					{
					if (!WriteChunks(Socket, m_MessageChunks[iMessage]))
						{
						m_Done.Set();
						return;
						}

					if (!ReadAck(Socket))
						{
						m_Done.Set();
						return;
						}
					}
				}

			m_Done.Set();
			}

		bool Wait (DWORD dwTimeout = INFINITE) const { return m_Done.Wait(dwTimeout); }

	private:
		bool WriteChunks (CSocket &Socket, const TArray<CString> &Chunks)
			{
			for (int i = 0; i < Chunks.GetCount(); i++)
				{
				const char *pPos = Chunks[i].GetParsePointer();
				int iLeft = Chunks[i].GetLength();
				while (iLeft > 0)
					{
					int iWritten = Socket.Write(pPos, iLeft);
					if (iWritten <= 0)
						{
						m_sError = ERR_WRITE_FAILED;
						return false;
						}

					pPos += iWritten;
					iLeft -= iWritten;
					}

				if (i + 1 < Chunks.GetCount())
					::Sleep(CHUNK_DELAY);
				}

			return true;
			}

		bool ReadAck (CSocket &Socket)
			{
			CStringBuffer Ack;
			while (Ack.GetLength() < AMP1_REPLY_OK.GetLength())
				{
				char szBuffer[1024];
				int iToRead = Min((int)sizeof(szBuffer), AMP1_REPLY_OK.GetLength() - Ack.GetLength());
				int iRead = Socket.Read(szBuffer, iToRead);
				if (iRead <= 0)
					{
					m_sError = ERR_READ_FAILED;
					return false;
					}

				Ack.Write(szBuffer, iRead);
				}

			CString sAck(Ack.GetPointer(), Ack.GetLength());
			if (!strEquals(sAck, AMP1_REPLY_OK))
				{
				m_sError = strPattern(ERR_BAD_ACK, sAck);
				return false;
				}

			m_sAck += sAck;
			return true;
			}

		DWORD m_dwPort = 0;
		TArray<TArray<CString>> m_MessageChunks;
		CString m_sAck;
		CString m_sError;
		CManualEvent m_Done;
		bool m_bPipeline = false;
	};

class CAMP1SocketServerThread : public TThread<CAMP1SocketServerThread>
	{
	public:
		CAMP1SocketServerThread () : TThread(STR_MSG_AMP1_TEST_DR_HOUSE_AMP1_SOCKET_SERVER) { m_Started.Create(); m_RequestRead.Create(); m_StopRequested.Create(); m_Done.Create(); }

		const CString &GetError () const { return m_sError; }
		const CString &GetRequest () const { return m_sRequest; }
		bool IsSuccess () const { return m_sError.IsEmpty(); }

		void Init (DWORD dwPort, const CString &sCommand, CDatum dExpectedPayload, int iExpectedMessageCount, bool bSilentPeer = false, DWORD dwSilentHoldMs = SOCKET_TIMEOUT)
			{
			m_dwPort = dwPort;
			m_sExpectedCommand = sCommand;
			m_dExpectedPayload = dExpectedPayload;
			m_iExpectedMessageCount = iExpectedMessageCount;
			m_bSilentPeer = bSilentPeer;
			m_dwSilentHoldMs = dwSilentHoldMs;
			}

		void Run ()
			{
			SOCKET hListen = INVALID_SOCKET;
			SOCKET hClient = INVALID_SOCKET;

			hListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (hListen == INVALID_SOCKET)
				{
				m_sError = strPattern(ERR_LISTEN_FAILED, m_dwPort);
				m_Started.Set();
				m_Done.Set();
				return;
				}

			sockaddr_in Addr = {};
			Addr.sin_family = AF_INET;
			Addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
			Addr.sin_port = htons((u_short)m_dwPort);

			if (bind(hListen, (sockaddr *)&Addr, sizeof(Addr)) == SOCKET_ERROR
					|| listen(hListen, SOMAXCONN) == SOCKET_ERROR)
				{
				m_sError = strPattern(ERR_LISTEN_FAILED, m_dwPort);
				closesocket(hListen);
				m_Started.Set();
				m_Done.Set();
				return;
				}

			m_Started.Set();

			fd_set ReadSet;
			FD_ZERO(&ReadSet);
			FD_SET(hListen, &ReadSet);

			timeval Timeout = {};
			Timeout.tv_sec = SOCKET_TIMEOUT / 1000;
			Timeout.tv_usec = (SOCKET_TIMEOUT % 1000) * 1000;

			int iSelect = select(0, &ReadSet, NULL, NULL, &Timeout);
			if (iSelect <= 0)
				{
				m_sError = ERR_SERVER_ACCEPT_TIMEOUT;
				closesocket(hListen);
				m_Done.Set();
				return;
				}

			hClient = accept(hListen, NULL, NULL);
			closesocket(hListen);
			if (hClient == INVALID_SOCKET)
				{
				m_sError = ERR_SERVER_ACCEPT_TIMEOUT;
				m_Done.Set();
				return;
				}

			DWORD dwTimeout = SOCKET_TIMEOUT;
			::setsockopt(hClient, SOL_SOCKET, SO_RCVTIMEO, (const char *)&dwTimeout, sizeof(dwTimeout));
			::setsockopt(hClient, SOL_SOCKET, SO_SNDTIMEO, (const char *)&dwTimeout, sizeof(dwTimeout));

			for (int i = 0; i < m_iExpectedMessageCount; i++)
				{
				if (!ReadRequest(hClient))
					break;

				m_RequestRead.Set();

				if (m_bSilentPeer)
					{
					m_StopRequested.Wait(m_dwSilentHoldMs);
					break;
					}

				if (!WriteAll(hClient, AMP1_REPLY_OK))
					{
					m_sError = ERR_SERVER_WRITE_FAILED;
					break;
					}
				}

			closesocket(hClient);
			m_Done.Set();
			}

		bool Wait (DWORD dwTimeout = INFINITE) const { return m_Done.Wait(dwTimeout); }
		bool WaitRequestRead (DWORD dwTimeout = INFINITE) const { return m_RequestRead.Wait(dwTimeout); }
		bool WaitStarted (DWORD dwTimeout = INFINITE) const { return m_Started.Wait(dwTimeout); }
		void SignalStop () { m_StopRequested.Set(); }

	private:
		bool ReadRequest (SOCKET hClient)
			{
			CBuffer Data;

			while (true)
				{
				CString sCommand;
				DWORD dwDataLen;
				const char *pPartialData;
				DWORD dwPartialDataLen;
				if (CAMP1Protocol::GetHeader(Data, &sCommand, &dwDataLen, &pPartialData, &dwPartialDataLen))
					{
					if (dwPartialDataLen < dwDataLen)
						{
						if (!ReadMore(hClient, Data))
							return false;

						continue;
						}

					if (!strEquals(sCommand, m_sExpectedCommand))
						{
						m_sError = strPattern(ERR_BAD_OUTBOUND_REQUEST, sCommand);
						return false;
						}

					CBuffer Payload(pPartialData, dwDataLen);
					CDatum dPayload;
					if (!CDatum::Deserialize(CDatum::EFormat::AEONScript, Payload, &dPayload)
							|| !strEquals(dPayload.AsStringView(), m_dExpectedPayload.AsStringView()))
						{
						m_sError = ERR_BAD_OUTBOUND_PAYLOAD;
						return false;
						}

					CString sRequest(Data.GetPointer(), Data.GetLength());
					m_sRequest += sRequest;
					return true;
					}
				else if (!sCommand.IsEmpty())
					{
					m_sError = strPattern(ERR_BAD_OUTBOUND_REQUEST, sCommand);
					return false;
					}

				if (!ReadMore(hClient, Data))
					return false;
				}
			}

		bool ReadMore (SOCKET hClient, CBuffer &Data)
			{
			char szBuffer[1024];
			int iRead = recv(hClient, szBuffer, sizeof(szBuffer), 0);
			if (iRead <= 0)
				{
				m_sError = ERR_SERVER_READ_FAILED;
				return false;
				}

			Data.Write(szBuffer, iRead);
			return true;
			}

		bool WriteAll (SOCKET hClient, const CString &sData)
			{
			const char *pPos = sData.GetParsePointer();
			int iLeft = sData.GetLength();
			while (iLeft > 0)
				{
				int iWritten = send(hClient, pPos, iLeft, 0);
				if (iWritten <= 0)
					return false;

				pPos += iWritten;
				iLeft -= iWritten;
				}

			return true;
			}

		DWORD m_dwPort = 0;
		CString m_sExpectedCommand;
		CDatum m_dExpectedPayload;
		int m_iExpectedMessageCount = 1;
		CString m_sRequest;
		CString m_sError;
		CManualEvent m_Started;
		CManualEvent m_RequestRead;
		CManualEvent m_StopRequested;
		CManualEvent m_Done;
		bool m_bSilentPeer = false;
		DWORD m_dwSilentHoldMs = SOCKET_TIMEOUT;
	};

class CAMP1TestSession : public ISessionHandler
	{
	public:
		CAMP1TestSession () { }
		virtual ~CAMP1TestSession () 
			{ 
			if (m_ClientThread.IsStarted()) 
				m_ClientThread.Wait(SOCKET_TIMEOUT); 
			if (m_ServerThread.IsStarted())
				{
				m_ServerThread.SignalStop();
				m_ServerThread.Wait(SOCKET_TIMEOUT);
				}
			}

	protected:
		virtual void OnMark (void) override { m_dExpectedPayload.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;
		virtual bool OnTimeout (const SArchonMessage &Msg) override;

	private:
		enum class EState
			{
			Unknown,
			WaitingForListener,
			WaitingForAMP1,
			WaitingForClient,
			WaitingForOutboundReply,
			WaitingForOutboundServer,
			WaitingForSilentPeerError,
			WaitingForStop,
			Done,
			};

		static DWORD GetDefaultPort () { return 17000 + (sysGetTickCount() % 10000); }
		static CString MakeAMP1Message (CStringView sCommand, CDatum dPayload);

		bool Fail (const CString &sError);
		bool CompleteClientThread ();
		bool CompleteOutboundServerThread ();
		bool FinishIfClientComplete ();
		bool FinishIfOutboundServerComplete ();
		bool IsInboundTest () const;
		bool IsOutboundSilentPeerTest () const;
		bool IsOutboundTest () const;
		TArray<CString> MakeChunks (const CString &sMessage) const;
		TArray<TArray<CString>> MakeMessageChunks () const;
		bool SendPassReply ();
		void StartSocketClient ();
		bool StartOutboundRequests ();
		bool StartOutboundServer ();
		void StopListener ();

		EState m_iState = EState::Unknown;
		CString m_sTest;
		CString m_sListenerName;
		DWORD m_dwPort = 0;
		DWORD m_dwTimeout = DEFAULT_TIMEOUT;
		CDatum m_dExpectedPayload;
		CString m_sAck;
		CString m_sRequest;
		CString m_sPendingError;
		CAMP1SocketClientThread m_ClientThread;
		CAMP1SocketServerThread m_ServerThread;
		int m_iExpectedMessageCount = 1;
		int m_iMessagesReceived = 0;
		int m_iOutboundRepliesReceived = 0;
		bool m_bListenerStarted = false;
		bool m_bStopRequested = false;
	};

bool CAMP1TestSession::Fail (const CString &sError)
	{
	if (m_bListenerStarted)
		{
		m_sPendingError = sError;
		StopListener();
		m_iState = EState::WaitingForStop;
		ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), m_dwTimeout);
		return true;
		}

	m_iState = EState::Done;
	SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
	return false;
	}

bool CAMP1TestSession::CompleteClientThread ()
	{
	if (!m_ClientThread.IsSuccess())
		{
		return Fail(m_ClientThread.GetError());
		}

	m_sAck = m_ClientThread.GetAck();
	StopListener();
	m_iState = EState::WaitingForStop;
	ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), m_dwTimeout);
	return true;
	}

bool CAMP1TestSession::CompleteOutboundServerThread ()
	{
	if (!m_ServerThread.IsSuccess())
		return Fail(m_ServerThread.GetError());

	m_sRequest = m_ServerThread.GetRequest();
	m_iMessagesReceived = m_iExpectedMessageCount;
	m_iState = EState::Done;
	SendPassReply();
	return false;
	}

bool CAMP1TestSession::FinishIfClientComplete ()
	{
	if (!m_ClientThread.Wait(0))
		{
		m_iState = EState::WaitingForClient;
		ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), SOCKET_TIMEOUT);
		return true;
		}

	return CompleteClientThread();
	}

bool CAMP1TestSession::FinishIfOutboundServerComplete ()
	{
	if (!m_ServerThread.Wait(0))
		{
		m_iState = EState::WaitingForOutboundServer;
		ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), SOCKET_TIMEOUT);
		return true;
		}

	return CompleteOutboundServerThread();
	}

bool CAMP1TestSession::IsInboundTest () const
	{
	return (strEquals(m_sTest, TEST_INBOUND_SPLIT_HEADER)
			|| strEquals(m_sTest, TEST_INBOUND_SPLIT_HEADER_BYTEWISE)
			|| strEquals(m_sTest, TEST_INBOUND_SPLIT_PAYLOAD_BYTEWISE)
			|| strEquals(m_sTest, TEST_INBOUND_SPLIT_HEADER_AND_PAYLOAD)
			|| strEquals(m_sTest, TEST_INBOUND_TWO_MESSAGES_SAME_SOCKET)
			|| strEquals(m_sTest, TEST_INBOUND_PIPELINED_MESSAGES)
			|| strEquals(m_sTest, TEST_INBOUND_PING)
			|| strEquals(m_sTest, TEST_INBOUND_SPLIT_PAYLOAD));
	}

bool CAMP1TestSession::IsOutboundSilentPeerTest () const
	{
	return strEquals(m_sTest, TEST_OUTBOUND_SILENT_PEER);
	}

bool CAMP1TestSession::IsOutboundTest () const
	{
	return (strEquals(m_sTest, TEST_OUTBOUND_PING)
			|| strEquals(m_sTest, TEST_OUTBOUND_SILENT_PEER)
			|| strEquals(m_sTest, TEST_OUTBOUND_TWO_MESSAGES_SAME_SOCKET));
	}

CString CAMP1TestSession::MakeAMP1Message (CStringView sCommand, CDatum dPayload)
	{
	CStringBuffer Payload;
	dPayload.Serialize(CDatum::EFormat::AEONScript, Payload);

	CStringBuffer Message;
	Message.Write(PROTOCOL_AMP1_00);
	Message.Write(" ", 1);
	Message.Write(sCommand);
	Message.Write(" ", 1);
	Message.Write(strFromInt(Payload.GetLength()));
	Message.Write("\r\n", 2);
	Message.Write(Payload);
	Message.Write("\r\n", 2);

	return CString::CreateFromHandoff(Message);
	}

TArray<CString> CAMP1TestSession::MakeChunks (const CString &sMessage) const
	{
	TArray<CString> Result;

	const char *pStart = sMessage.GetParsePointer();
	const char *pEnd = pStart + sMessage.GetLength();
	const char *pHeaderEnd = pStart;
	while (pHeaderEnd < pEnd - 1 && !(pHeaderEnd[0] == '\r' && pHeaderEnd[1] == '\n'))
		pHeaderEnd++;

	const char *pPayloadStart = Min(pHeaderEnd + 2, pEnd);

	if (strEquals(m_sTest, TEST_INBOUND_PING)
			|| strEquals(m_sTest, TEST_INBOUND_PIPELINED_MESSAGES))
		{
		Result.Insert(sMessage);
		}
	else if (strEquals(m_sTest, TEST_INBOUND_SPLIT_HEADER_BYTEWISE))
		{
		const char *pPos = pStart;
		const char *pHeaderLimit = Min(pHeaderEnd + 2, pEnd);
		while (pPos < pHeaderLimit)
			{
			Result.Insert(CString(pPos, 1));
			pPos++;
			}

		if (pPos < pEnd)
			Result.Insert(CString(pPos, (int)(pEnd - pPos)));
		}
	else if (strEquals(m_sTest, TEST_INBOUND_SPLIT_PAYLOAD))
		{
		int iFirstLen = (int)(pHeaderEnd + 3 - pStart);
		Result.Insert(CString(pStart, iFirstLen));
		Result.Insert(CString(pStart + iFirstLen, sMessage.GetLength() - iFirstLen));
		}
	else if (strEquals(m_sTest, TEST_INBOUND_SPLIT_PAYLOAD_BYTEWISE))
		{
		Result.Insert(CString(pStart, (int)(pPayloadStart - pStart)));

		const char *pPos = pPayloadStart;
		while (pPos < pEnd)
			{
			Result.Insert(CString(pPos, 1));
			pPos++;
			}
		}
	else if (strEquals(m_sTest, TEST_INBOUND_SPLIT_HEADER_AND_PAYLOAD))
		{
		int iFirstLen = 10;
		if (iFirstLen >= (int)(pPayloadStart - pStart))
			iFirstLen = Max(1, (int)(pPayloadStart - pStart) / 2);

		Result.Insert(CString(pStart, iFirstLen));
		Result.Insert(CString(pStart + iFirstLen, (int)(pPayloadStart - pStart) - iFirstLen));

		int iPayloadLen = (int)(pEnd - pPayloadStart);
		int iPayloadFirstLen = Max(1, iPayloadLen / 2);
		Result.Insert(CString(pPayloadStart, iPayloadFirstLen));
		Result.Insert(CString(pPayloadStart + iPayloadFirstLen, iPayloadLen - iPayloadFirstLen));
		}
	else
		{
		int iFirstLen = 10;
		if (iFirstLen >= sMessage.GetLength())
			iFirstLen = Max(1, sMessage.GetLength() / 2);

		Result.Insert(CString(pStart, iFirstLen));
		Result.Insert(CString(pStart + iFirstLen, sMessage.GetLength() - iFirstLen));
		}

	return Result;
	}

TArray<TArray<CString>> CAMP1TestSession::MakeMessageChunks () const
	{
	CString sMessage = MakeAMP1Message(AMP1_PING, m_dExpectedPayload);

	TArray<TArray<CString>> Result;
	Result.Insert(MakeChunks(sMessage));

	if (strEquals(m_sTest, TEST_INBOUND_TWO_MESSAGES_SAME_SOCKET)
			|| strEquals(m_sTest, TEST_INBOUND_PIPELINED_MESSAGES))
		Result.Insert(MakeChunks(sMessage));

	return Result;
	}

bool CAMP1TestSession::SendPassReply ()
	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_STATUS, STR_STATUS_PASS);
	dResult.SetElement(FIELD_TEST, m_sTest);
	dResult.SetElement(FIELD_COMMAND, AMP1_PING);
	dResult.SetElement(FIELD_PORT, (int)m_dwPort);
	dResult.SetElement(FIELD_LISTENER, m_sListenerName);
	dResult.SetElement(FIELD_ACK, m_sAck);
	dResult.SetElement(FIELD_REQUEST, m_sRequest);
	dResult.SetElement(FIELD_MESSAGE_COUNT, m_iMessagesReceived);

	SendMessageReply(MSG_REPLY_DATA, dResult);
	return false;
	}

bool CAMP1TestSession::OnProcessMessage (const SArchonMessage &Msg)
	{
	if (IsError(Msg))
		{
		CString sError = Msg.dPayload.AsString();
		if (m_iState == EState::WaitingForSilentPeerError && IsOutboundSilentPeerTest())
			{
			m_iState = EState::Done;
			return SendPassReply();
			}

		if (m_iState == EState::WaitingForStop)
			{
			m_iState = EState::Done;
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, (!m_sPendingError.IsEmpty() ? m_sPendingError : sError));
			return false;
			}

		return Fail(sError);
		}

	switch (m_iState)
		{
		case EState::WaitingForListener:
			if (strEquals(Msg.sMsg, MSG_ESPER_ON_LISTENER_STARTED))
				{
				m_bListenerStarted = true;
				m_iState = EState::WaitingForAMP1;
				ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), m_dwTimeout);
				StartSocketClient();
				return true;
				}
			break;

		case EState::WaitingForAMP1:
			if (strEquals(Msg.sMsg, MSG_ESPER_ON_AMP1))
				{
				CStringView sCommand = Msg.dPayload.GetElement(0);
				CDatum dPayload = Msg.dPayload.GetElement(1);

				if (!strEquals(sCommand, AMP1_PING))
					{
					return Fail(strPattern(ERR_BAD_COMMAND, sCommand));
					}

				if (!strEquals(dPayload.AsStringView(), m_dExpectedPayload.AsStringView()))
					{
					return Fail(ERR_BAD_PAYLOAD);
					}

				m_iMessagesReceived++;
				if (m_iMessagesReceived < m_iExpectedMessageCount)
					return true;

				return FinishIfClientComplete();
				}
			break;

		case EState::WaitingForClient:
			break;

		case EState::WaitingForOutboundReply:
			if (strEquals(Msg.sMsg, MSG_OK))
				{
				m_iOutboundRepliesReceived++;
				if (m_iOutboundRepliesReceived < m_iExpectedMessageCount)
					return true;

				return FinishIfOutboundServerComplete();
				}
			break;

		case EState::WaitingForOutboundServer:
			break;

		case EState::WaitingForSilentPeerError:
			break;

		case EState::WaitingForStop:
			if (strEquals(Msg.sMsg, MSG_ESPER_ON_LISTENER_STOPPED))
				{
				if (!m_sPendingError.IsEmpty())
					{
					m_iState = EState::Done;
					SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, m_sPendingError);
					return false;
					}

				m_iState = EState::Done;
				return SendPassReply();
				}
			break;
		}

	return Fail(strPattern("Unexpected message while running AMP1 test: %s", Msg.sMsg));
	}

bool CAMP1TestSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)
	{
	m_sTest = Msg.dPayload.GetElement(0).AsString();
	if (m_sTest.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_NO_TEST);
		return false;
		}

	if (!IsInboundTest() && !IsOutboundTest())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_INVALID_TEST, m_sTest));
		return false;
		}

	CDatum dOptions = Msg.dPayload.GetElement(1);
	m_dwPort = (DWORD)(int)dOptions.GetElement(FIELD_PORT);
	if (m_dwPort == 0)
		m_dwPort = GetDefaultPort();

	m_dwTimeout = (DWORD)(int)dOptions.GetElement(FIELD_TIMEOUT);
	if (m_dwTimeout == 0)
		m_dwTimeout = DEFAULT_TIMEOUT;

	m_dExpectedPayload = dOptions.GetElement(FIELD_PAYLOAD);
	if (m_dExpectedPayload.IsNil())
		m_dExpectedPayload = strPattern("DrHouse.amp1Test/%s", m_sTest);

	m_iExpectedMessageCount = (strEquals(m_sTest, TEST_INBOUND_TWO_MESSAGES_SAME_SOCKET)
			|| strEquals(m_sTest, TEST_INBOUND_PIPELINED_MESSAGES)
			|| strEquals(m_sTest, TEST_OUTBOUND_TWO_MESSAGES_SAME_SOCKET) ? 2 : 1);

	m_sListenerName = strPattern("DrHouse.amp1Test.%x.%x", sysGetTickCount(), m_dwPort);

	if (IsOutboundTest())
		return StartOutboundServer();

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(m_sListenerName);
	dPayload.Append((int)m_dwPort);
	dPayload.Append(PROTOCOL_AMP1);

	m_iState = EState::WaitingForListener;
	if (!SendMessageCommand(ADDR_ESPER_COMMAND, MSG_ESPER_START_LISTENER, GenerateAddress(PORT_DRHOUSE_COMMAND), dPayload, m_dwTimeout))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_LISTENER_FAILED);
		return false;
		}

	return true;
	}

bool CAMP1TestSession::OnTimeout (const SArchonMessage &Msg)
	{
	if (m_iState == EState::WaitingForOutboundReply && IsOutboundSilentPeerTest())
		{
		if (!m_ServerThread.WaitRequestRead(0))
			{
			if (m_ServerThread.Wait(0) && !m_ServerThread.IsSuccess())
				return Fail(m_ServerThread.GetError());
			else
				return Fail(MSG_ERROR_TIMEOUT);
			}

		m_ServerThread.SignalStop();
		if (!m_ServerThread.Wait(SOCKET_TIMEOUT))
			return Fail(MSG_ERROR_TIMEOUT);
		else if (!m_ServerThread.IsSuccess())
			return Fail(m_ServerThread.GetError());

		m_sRequest = m_ServerThread.GetRequest();
		m_iMessagesReceived = 1;
		m_iState = EState::WaitingForSilentPeerError;
		ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), SOCKET_TIMEOUT);
		return true;
		}

	if (m_iState == EState::WaitingForSilentPeerError && IsOutboundSilentPeerTest())
		return Fail(MSG_ERROR_TIMEOUT);

	if (m_iState == EState::WaitingForClient)
		{
		if (!m_ClientThread.Wait(0))
			return Fail(ERR_ACK_TIMEOUT);

		return CompleteClientThread();
		}

	if (m_iState == EState::WaitingForOutboundServer)
		{
		if (!m_ServerThread.Wait(0))
			return Fail(MSG_ERROR_TIMEOUT);

		return CompleteOutboundServerThread();
		}

	if (m_ClientThread.IsStarted() && m_ClientThread.Wait(0) && !m_ClientThread.IsSuccess())
		return Fail(m_ClientThread.GetError());

	if (m_ServerThread.IsStarted() && m_ServerThread.Wait(0) && !m_ServerThread.IsSuccess())
		return Fail(m_ServerThread.GetError());

	if (m_iState == EState::WaitingForStop)
		{
		m_iState = EState::Done;
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, (!m_sPendingError.IsEmpty() ? m_sPendingError : MSG_ERROR_TIMEOUT));
		return false;
		}

	return Fail(MSG_ERROR_TIMEOUT);
	}

void CAMP1TestSession::StartSocketClient ()
	{
	m_ClientThread.Init(m_dwPort, MakeMessageChunks(), strEquals(m_sTest, TEST_INBOUND_PIPELINED_MESSAGES));
	m_ClientThread.Start();
	}

bool CAMP1TestSession::StartOutboundRequests ()
	{
	CString sAddress = strPattern("%s:%d", STR_LOCALHOST, m_dwPort);

	for (int i = 0; i < m_iExpectedMessageCount; i++)
		{
		CDatum dPayload(CDatum::typeArray);
		dPayload.Append(sAddress);
		dPayload.Append(AMP1_PING);
		dPayload.Append(m_dExpectedPayload);

		if (!SendMessageCommand(ADDR_ESPER_COMMAND, MSG_ESPER_AMP1, GenerateAddress(PORT_DRHOUSE_COMMAND), dPayload))
			return Fail(ERR_LISTENER_FAILED);
		}

	m_iState = EState::WaitingForOutboundReply;
	ResetTimeout(GenerateAddress(PORT_DRHOUSE_COMMAND), m_dwTimeout);
	return true;
	}

bool CAMP1TestSession::StartOutboundServer ()
	{
	m_ServerThread.Init(m_dwPort, AMP1_PING, m_dExpectedPayload, m_iExpectedMessageCount, IsOutboundSilentPeerTest(), m_dwTimeout + SOCKET_TIMEOUT);
	m_ServerThread.Start();

	if (!m_ServerThread.WaitStarted(SOCKET_TIMEOUT))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_LISTEN_FAILED, m_dwPort));
		return false;
		}

	if (!m_ServerThread.IsSuccess())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, m_ServerThread.GetError());
		return false;
		}

	return StartOutboundRequests();
	}

void CAMP1TestSession::StopListener ()
	{
	if (m_bStopRequested)
		return;

	m_bStopRequested = true;

	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(m_sListenerName);

	SendMessageCommand(ADDR_ESPER_COMMAND, MSG_ESPER_STOP_LISTENER, GenerateAddress(PORT_DRHOUSE_COMMAND), dPayload, m_dwTimeout);
	}

class CAMP1AllTestSession : public ISessionHandler
	{
	public:
		CAMP1AllTestSession () { }

	protected:
		virtual void OnMark (void) override { m_dOptions.Mark(); }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;
		virtual bool OnTimeout (const SArchonMessage &Msg) override;

	private:
		bool Fail (const CString &sError);
		void InitTests ();
		bool SendNextTest ();
		bool SendPassReply (const CString &sFinalLine);

		TArray<CString> m_Tests;
		CDatum m_dOptions;
		DWORD m_dwTimeout = DEFAULT_TIMEOUT;
		int m_iPos = 0;
		int m_iPass = 0;
	};

bool CAMP1AllTestSession::Fail (const CString &sError)
	{
	CString sTest = (m_iPos >= 0 && m_iPos < m_Tests.GetCount() ? m_Tests[m_iPos] : TEST_ALL);
	SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_SUITE_FAILED, sTest, sError));
	return false;
	}

void CAMP1AllTestSession::InitTests ()
	{
	m_Tests.Insert(TEST_INBOUND_PING);
	m_Tests.Insert(TEST_INBOUND_SPLIT_HEADER);
	m_Tests.Insert(TEST_INBOUND_SPLIT_PAYLOAD);
	m_Tests.Insert(TEST_INBOUND_SPLIT_HEADER_AND_PAYLOAD);
	m_Tests.Insert(TEST_INBOUND_SPLIT_HEADER_BYTEWISE);
	m_Tests.Insert(TEST_INBOUND_SPLIT_PAYLOAD_BYTEWISE);
	m_Tests.Insert(TEST_INBOUND_TWO_MESSAGES_SAME_SOCKET);
	m_Tests.Insert(TEST_INBOUND_PIPELINED_MESSAGES);
	m_Tests.Insert(TEST_OUTBOUND_PING);
	m_Tests.Insert(TEST_OUTBOUND_TWO_MESSAGES_SAME_SOCKET);
	m_Tests.Insert(TEST_OUTBOUND_SILENT_PEER);
	}

bool CAMP1AllTestSession::OnProcessMessage (const SArchonMessage &Msg)
	{
	if (IsError(Msg))
		return Fail(Msg.dPayload.AsString());

	if (!strEquals(Msg.sMsg, MSG_REPLY_DATA))
		return Fail(strPattern(ERR_UNEXPECTED_REPLY, Msg.sMsg));

	CString sPassLine = strPattern("%s...pass", m_Tests[m_iPos]);
	m_iPass++;

	if (++m_iPos < m_Tests.GetCount())
		{
		SendMessageReplyProgress(sPassLine);
		return SendNextTest();
		}

	return SendPassReply(sPassLine);
	}

bool CAMP1AllTestSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)
	{
	m_dOptions = Msg.dPayload.GetElement(1);

	m_dwTimeout = (DWORD)(int)m_dOptions.GetElement(FIELD_TIMEOUT);
	if (m_dwTimeout == 0)
		m_dwTimeout = DEFAULT_TIMEOUT;

	InitTests();
	return SendNextTest();
	}

bool CAMP1AllTestSession::OnTimeout (const SArchonMessage &Msg)
	{
	return Fail(MSG_ERROR_TIMEOUT);
	}

bool CAMP1AllTestSession::SendNextTest ()
	{
	CDatum dPayload(CDatum::typeArray);
	dPayload.Append(m_Tests[m_iPos]);

	if (!m_dOptions.IsNil())
		dPayload.Append(m_dOptions);

	if (!SendMessageCommand(GenerateAddress(PORT_DRHOUSE_COMMAND), MSG_DRHOUSE_AMP1_TEST, GenerateAddress(PORT_DRHOUSE_COMMAND), dPayload, m_dwTimeout + SOCKET_TIMEOUT))
		return Fail(strPattern(ERR_INVALID_TEST, m_Tests[m_iPos]));

	return true;
	}

bool CAMP1AllTestSession::SendPassReply (const CString &sFinalLine)
	{
	SendMessageReply(MSG_REPLY_DATA, sFinalLine);
	return false;
	}

void CDrHouseEngine::MsgAMP1Test (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgAMP1Test
//
//	DrHouse.amp1Test {testCode} {options}

	{
	if (strEquals(Msg.dPayload.GetElement(0).AsStringView(), TEST_ALL))
		StartSession(Msg, new CAMP1AllTestSession);
	else
		StartSession(Msg, new CAMP1TestSession);
	}
