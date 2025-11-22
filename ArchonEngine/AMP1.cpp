//	AMP1.cpp
//
//	CExarchEngine class
//	Copyright (c) 2015 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_ESPER_COMMAND,				"Esper.command@~/~");
DECLARE_CONST_STRING(ADDRESS_EXARCH_COMMAND,			"Exarch.command@~/CentralModule");

DECLARE_CONST_STRING(AMP1_AUTH,							"AUTH");
DECLARE_CONST_STRING(AMP1_JOIN,							"JOIN");
DECLARE_CONST_STRING(AMP1_LEAVE,						"LEAVE");
DECLARE_CONST_STRING(AMP1_PING,							"PING");
DECLARE_CONST_STRING(AMP1_REJOIN,						"REJOIN");
DECLARE_CONST_STRING(AMP1_SEND,							"SEND");
DECLARE_CONST_STRING(AMP1_WELCOME,						"WELCOME");

DECLARE_CONST_STRING(FIELD_ARCOLOGY_PRIME_KEY,			"arcologyPrimeKey");
DECLARE_CONST_STRING(FIELD_AUTH_KEY,					"authKey");
DECLARE_CONST_STRING(FIELD_AUTH_NAME,					"authName");
DECLARE_CONST_STRING(FIELD_HOST_ADDRESS,				"hostAddress");
DECLARE_CONST_STRING(FIELD_NODE_ID,						"nodeID");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_ESPER_AMP1,					"Esper.amp1");
DECLARE_CONST_STRING(MSG_ESPER_AMP1_DISCONNECT,			"Esper.amp1Disconnect");
DECLARE_CONST_STRING(MSG_ESPER_SET_CONNECTION_PROPERTY,	"Esper.setConnectionProperty");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info");
DECLARE_CONST_STRING(MSG_OK,							"OK");

DECLARE_CONST_STRING(MNEMO_ARC_MACHINES,				"Arc.machines");

DECLARE_CONST_STRING(MNEMO_STATUS_RUNNING,				"running");
DECLARE_CONST_STRING(MNEMO_STATUS_STOPPED,				"stopped");

DECLARE_CONST_STRING(STR_ANONYMOUS_PING,				"Received unauthenticated ping.");
DECLARE_CONST_STRING(STR_PING,							"Ping from arcology machine: %s.");
DECLARE_CONST_STRING(STR_AMP1_SEND_DISPATCH,			"Dispatching %s to %s from %s.");
DECLARE_CONST_STRING(STR_CONNECTED_TO_SERVER,			"Connected to Arcology Prime.");
DECLARE_CONST_STRING(STR_CLIENT_CONNECTED,				"[%s]: Connected successfully.");
DECLARE_CONST_STRING(STR_CLIENT_DISCONNECTED,			"[%s]: Disconnected.");

DECLARE_CONST_STRING(ERR_INVALID_JOIN,					"Arcology Prime cannot join another arcology.");
DECLARE_CONST_STRING(ERR_INVALID_LEAVE,					"Arcology Prime cannot leave the arcology.");
DECLARE_CONST_STRING(ERR_INVALID_AUTH,					"Invalid machine authentication.");
DECLARE_CONST_STRING(ERR_CANT_JOIN,						"Unable to join arcology: %s");
DECLARE_CONST_STRING(ERR_CANT_LEAVE,					"Unable to leave arcology: %s");
DECLARE_CONST_STRING(ERR_AMP1_NO_AUTH,					"[%x] Unable to obey AMP1 command without authentication: %s.");
DECLARE_CONST_STRING(ERR_CANT_BIND,						"Unable to bind to address: %s.");
DECLARE_CONST_STRING(ERR_CANT_SEND,						"Unable to send to address: %s.");
DECLARE_CONST_STRING(ERR_CANT_SEND_AMP1_COMMAND,		"Unable to send AMP1 command to %s.");
DECLARE_CONST_STRING(ERR_CANT_WRITE_CONFIG_FILE,		"Unable to write configuration file.");
DECLARE_CONST_STRING(ERR_UNKNOWN_AMP1_COMMAND,			"Unknown AMP1 command: %s.");
DECLARE_CONST_STRING(ERR_AMP1_ERROR,					"AMP1 ERROR: %s.");

void CExarchEngine::OnAMP1ClientConnected (CStringView sNodeID)
	{
	CSmartLock Lock(m_cs);
	if (m_bInGC)
		{
		m_AMP1Queue.OnAMP1ClientConnected(sNodeID);
		return;
		}
	Lock.Unlock();

	Log(MSG_LOG_INFO, strPattern(STR_CLIENT_CONNECTED, sNodeID));
	}

void CExarchEngine::OnAMP1ClientDisconnected (CStringView sNodeID)
	{
	CSmartLock Lock(m_cs);
	if (m_bInGC)
		{
		m_AMP1Queue.OnAMP1ClientDisconnected(sNodeID);
		return;
		}

	Log(MSG_LOG_INFO, strPattern(STR_CLIENT_DISCONNECTED, sNodeID));

	//	When a machine disconnects, we need to update our mnemosynth database

	SMachineDesc Desc;
	if (!m_MecharcologyDb.FindMachineByNodeID(sNodeID, &Desc))
		return;

	CDatum dMachineInfo(CDatum::typeStruct);
	dMachineInfo.SetElement(FIELD_NODE_ID, sNodeID);
	dMachineInfo.SetElement(FIELD_HOST_ADDRESS, NULL_STR);
	dMachineInfo.SetElement(FIELD_STATUS, MNEMO_STATUS_STOPPED);

	MnemosynthWrite(MNEMO_ARC_MACHINES, 
			Desc.sName, 
			dMachineInfo);
	}

void CExarchEngine::OnAMP1ConnectedToServer ()
	{
	CSmartLock Lock(m_cs);
	if (m_bInGC)
		{
		m_AMP1Queue.OnAMP1ConnectedToServer();
		return;
		}
	Lock.Unlock();

	LogBlackBox(STR_CONNECTED_TO_SERVER);
	}

void CExarchEngine::OnAMP1FatalError (CStringView sError)
	{
	CSmartLock Lock(m_cs);
	if (m_bInGC)
		{
		m_AMP1Queue.OnAMP1FatalError(sError);
		return;
		}
	Lock.Unlock();

	if (IsArcologyPrime())
		Log(MSG_LOG_ERROR, strPattern(ERR_AMP1_ERROR, sError));

	//	If we're a secondary machine, we just log to the local log file since
	//	we can't reach Arcology Prime.

	else
		LogBlackBox(strPattern(ERR_AMP1_ERROR, sError));
	}

void CExarchEngine::OnAMP1Message (CStringView sNodeID, CStringView sMsg, CBuffer&& retData)
	{
	CSmartLock Lock(m_cs);
	if (m_bInGC)
		{
		m_AMP1Queue.OnAMP1Message(sNodeID, sMsg, std::move(retData));
		return;
		}
	}

void CExarchEngine::MsgEsperOnAMP1 (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgEsperOnAMP1
//
//	Esper.onAMP1 command data connectionID authName
//
//	NOTE: We don't need to reply to the caller. The Esper engine will reply when
//	it delivers the message. If there is a need to reply, we will send our own
//	message.

	{
	CStringView sCommand = Msg.dPayload.GetElement(0);
	CDatum dData = Msg.dPayload.GetElement(1);
	CDatum dConnection = Msg.dPayload.GetElement(2);
	CStringView sAuthName = Msg.dPayload.GetElement(3);

#ifdef DEBUG_AMP1
	printf("[AMP1] Received %s from %s.\n", (LPSTR)sCommand, (LPSTR)sAuthName);
#endif

	//	Only a few commands are valid without authentication

	if (strEquals(sCommand, AMP1_JOIN))
		AMP1Join(dData);

	else if (strEquals(sCommand, AMP1_PING))
		AMP1Ping(sAuthName);

	//	The remainder require authenticate

	else if (sAuthName.IsEmpty())
		Log(MSG_LOG_ERROR, strPattern(ERR_AMP1_NO_AUTH, CEsperInterface::ConnectionToFriendlyID(dConnection), sCommand));

	else if (strEquals(sCommand, AMP1_AUTH))
		AMP1Auth(dData, dConnection);

	else if (strEquals(sCommand, AMP1_LEAVE))
		AMP1Leave();

	else if (strEquals(sCommand, AMP1_REJOIN))
		AMP1Rejoin();

	else if (strEquals(sCommand, AMP1_SEND))
		AMP1Send(dData);

	else if (strEquals(sCommand, AMP1_WELCOME))
		AMP1Welcome();

	else
		Log(MSG_LOG_ERROR, strPattern(ERR_UNKNOWN_AMP1_COMMAND, sCommand));
	}

bool CExarchEngine::SendAMP1Command (const CString &sMachineName, const CString &sCommand, CDatum dData)

//	SendAMP1Command
//
//	Sends an AMP1 command to the given machine

	{
	SMachineDesc Desc;
	if (!m_MecharcologyDb.FindMachineByName(sMachineName, &Desc))
		return false;

	return SendAMP1Command(Desc, sCommand, dData);
	}

bool CExarchEngine::SendAMP1Command (const SMachineDesc &Desc, const CString &sCommand, CDatum dData)

//	SendAMP1Command
//
//	Sends an AMP1 command to the given machine.

	{
	CComplexArray *pPayload = new CComplexArray;
	pPayload->Append(Desc.sAddress);
	pPayload->Append(sCommand);
	pPayload->Append(dData);
	if (!Desc.Key.IsEmpty())
		{
		//	We always give Esper our machine name, and the key that we need to
		//	talk to the destination machine. The destination machine will then
		//	validate the key and associate our machine name with that key.

		pPayload->Append(GetMachineName());
		pPayload->Append(Desc.Key);
		}

	return SendMessageCommand(ADDRESS_ESPER_COMMAND, MSG_ESPER_AMP1, ADDRESS_EXARCH_COMMAND, 0, CDatum(pPayload));
	}

void CExarchEngine::MsgSendToMachine (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSendToMachine
//
//	Exarch.sendToMachine {machineName} {address} {msg} {ticket} {replyAddr} {payload} {sendingMachine} {noLog}

	{
	CStringView sMachineName = Msg.dPayload.GetElement(0);
	CDatum dNoLog = Msg.dPayload.GetElement(7);

	if (!SendAMP1Command(sMachineName, AMP1_SEND, Msg.dPayload))
		{
		//	dNoLog is TRUE if we're sending a log message. This prevents us from
		//	infinitely recursing.

		if (dNoLog.IsNil())
			Log(MSG_LOG_ERROR, strPattern(ERR_CANT_SEND_AMP1_COMMAND, sMachineName));

		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_SEND_AMP1_COMMAND, sMachineName), Msg);
		return;
		}

	SendMessageReply(MSG_OK, CDatum(), Msg);
	}

void CExarchEngine::AMP1Auth (CDatum dData, CDatum dConnection)

//	AMP1Auth
//
//	A machine is giving us their secret key to authenticate.

	{
	CStringView sName = dData.GetElement(FIELD_AUTH_NAME);

	//	By the time we get here, Esper has already validated the key, so we just
	//	need to complete the authentication process. We remember the machine 
	//	name that was used to authenticate. We get back a nodeID so that we can
	//	update our Mnemosynth entry.

	CString sNodeID;
	if (m_MecharcologyDb.OnCompleteAuth(sName, sNodeID))
		OnMachineConnection(sNodeID, sName);
	}

void CExarchEngine::AMP1Join (CDatum dData)

//	AMP1Join
//
//	Arcology Prime is asking us to join an arcology.

	{
	//	If we're Arcology Prime, then this is invalid and we don't need to 
	//	respond.

	if (IsArcologyPrime())
		{
		Log(MSG_LOG_ERROR, ERR_INVALID_JOIN);
		return;
		}

	//	NOTE: It is OK to not authenticate the sender, since this is how we get
	//	the secret key.

	//	Add the machine (with its secret key) to the arcology.

	CStringView sPrimeName = dData.GetElement(FIELD_AUTH_NAME);
	CIPInteger PrimeKey = dData.GetElement(FIELD_AUTH_KEY);

	CString sError;
	if (!m_MecharcologyDb.JoinArcology(sPrimeName, PrimeKey, &sError))
		{
		Log(MSG_LOG_ERROR, strPattern(ERR_CANT_JOIN, sError));
		return;
		}

	//	Save the secret key

	m_dMachineConfig.SetElement(FIELD_ARCOLOGY_PRIME_KEY, PrimeKey);
	if (!WriteConfig())
		{
		Log(MSG_LOG_ERROR, ERR_CANT_WRITE_CONFIG_FILE);
		return;
		}

	//	Ping Arcology Prime

	SendAMP1Command(sPrimeName, AMP1_PING, CDatum());
	}

void CExarchEngine::AMP1Leave (void)

//	AMP1Leave
//
//	Arcology Prime is asking us to leave.

	{
	//	If we're Arcology Prime, then this is invalid and we don't need to 
	//	respond.

	if (IsArcologyPrime())
		{
		Log(MSG_LOG_ERROR, ERR_INVALID_LEAVE);
		return;
		}

	//	Tell Esper to shut down our outbound connection to Arcology Prime.

	SMachineDesc APDesc;
	if (m_MecharcologyDb.FindArcologyPrime(&APDesc)
			&& !APDesc.sAddress.IsEmpty())
		{
		CDatum dPayload(CDatum::typeArray);
		dPayload.Append(APDesc.sAddress);

		SendMessageCommand(ADDRESS_ESPER_COMMAND, MSG_ESPER_AMP1_DISCONNECT, NULL_STR, 0, dPayload);
		}

	//	Leave the arcology

	CString sError;
	if (!m_MecharcologyDb.LeaveArcology(&sError))
		Log(MSG_LOG_ERROR, strPattern(ERR_CANT_LEAVE, sError));

	//	Save the secret key

	m_dMachineConfig.SetElement(FIELD_ARCOLOGY_PRIME_KEY, CDatum());
	if (!WriteConfig())
		Log(MSG_LOG_ERROR, ERR_CANT_WRITE_CONFIG_FILE);
	}

void CExarchEngine::AMP1Ping (const CString &sSender)

//	AMP1Ping
//
//	A secondary machine wants to connect to us.

	{
	//	We respond with a WELCOME message, so the secondary machine knows that
	//	its connection succeeded (but only if we're authenticated).

	if (!sSender.IsEmpty())
		{
		SendAMP1Command(sSender, AMP1_WELCOME, CDatum());

		//	Log it

		Log(MSG_LOG_INFO, strPattern(STR_PING, sSender));
		}

	//	Otherwise, we just log it for debugging purposes

	else
		{
		Log(MSG_LOG_INFO, STR_ANONYMOUS_PING);
		}
	}

void CExarchEngine::AMP1Rejoin (void)

//	AMP1Rejoin
//
//	Arcology Prime is asking us to rejoin the arcology after it restarted.

	{
	//	If we're Arcology Prime, then this is invalid and we don't need to 
	//	respond.

	if (IsArcologyPrime())
		{
		Log(MSG_LOG_ERROR, ERR_INVALID_JOIN);
		return;
		}

	//	NOTE: This is an authenticated command, which means we've already
	//	verified that the caller sent us the correct key.
	//
	//	We should already have arcology prime.

	SMachineDesc PrimeDesc;
	if (!m_MecharcologyDb.FindArcologyPrime(&PrimeDesc))
		{
		Log(MSG_LOG_ERROR, ERR_INVALID_JOIN);
		return;
		}

	//	Ping Arcology Prime

	SendAMP1Command(PrimeDesc, AMP1_PING, CDatum());
	}

void CExarchEngine::AMP1Send (CDatum dData)

//	AMP1Send
//
//	We've been sent a message from another machine. Dispatch it appropriately.

	{
	//	See CIntermachinePort.cpp for parameters

	CStringView sAddr = dData.GetElement(1);
	SArchonMessage Msg;
	Msg.sMsg = dData.GetElement(2).AsStringView();
	Msg.dwTicket = dData.GetElement(3);
	Msg.sReplyAddr = dData.GetElement(4).AsStringView();
	Msg.dPayload = dData.GetElement(5);
	CDatum dNoLog = dData.GetElement(7);

#ifdef DEBUG_AMP1
	if (dNoLog.IsNil())
		Log(MSG_LOG_DEBUG, strPattern(STR_AMP1_SEND_DISPATCH, Msg.sMsg, sAddr, dData.GetElement(6).AsString()));
#endif

	//	Send the message

	SendMessage(sAddr, Msg);
	}

void CExarchEngine::AMP1Welcome (void)

//	AMP1Welcome
//
//	Arcology Prime has sent us a WELCOME message.

	{
	}
