//	CMnemosynthEngine.cpp
//
//	CMnemosynthEngine class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(PORT_MNEMOSYNTH_COMMAND,			"Mnemosynth.command");

DECLARE_CONST_STRING(ADDRESS_MNEMOSYNTH_COMMAND,		"Mnemosynth.command@~/~");
DECLARE_CONST_STRING(ADDRESS_MNEMOSYNTH_NOTIFY,			"Mnemosynth.notify");

DECLARE_CONST_STRING(ENGINE_NAME_MNEMOSYNTH,			"Mnemosynth");

DECLARE_CONST_STRING(FIELD_COLLECTIONS,					"collections");
DECLARE_CONST_STRING(FIELD_ENTRIES,						"entries");
DECLARE_CONST_STRING(FIELD_REPLY_MSG,					"replyMsg");
DECLARE_CONST_STRING(FIELD_WATERMARK,					"watermark");

DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");

DECLARE_CONST_STRING(MSG_EXARCH_SHUTDOWN,				"Exarch.shutdown");

DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug");
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error");

DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ENDPOINT_LIST,		"Mnemosynth.endpointList");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_NOTIFY_ON_ARCOLOGY_UPDATE,	"Mnemosynth.notifyOnArcologyUpdate");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_NOTIFY_ON_UPDATE,	"Mnemosynth.notifyOnEndpointUpdate");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ON_ARCOLOGY_UPDATED,"Mnemosynth.onArcologyUpdated");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ON_DB_MODIFIED,		"Mnemosynth.onDbModified");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ON_ENDPOINT_UPDATED,"Mnemosynth.onEndpointUpdated");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_ON_MODIFIED,		"Mnemosynth.onModified");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_READ,				"Mnemosynth.read");
DECLARE_CONST_STRING(MSG_MNEMOSYNTH_UPDATE,				"Mnemosynth.update");

DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

CMnemosynthEngine::CMnemosynthEngine (CMnemosynthDb *pDb) : CSimpleEngine(ENGINE_NAME_MNEMOSYNTH, 1),
		m_pDb(pDb),
		m_bDebug(false) 

//	CMnemosynthEngine constructor

	{ 
	}

bool CMnemosynthEngine::IsNotifyRequestUpToDate (SLocalCheckpoint *pEntry)

//	IsNotifyRequestUpToDate
//
//	Returns true if the given notify request is up to date (meaning
//	that all endpoints are up to the request sequences

	{
	int i;

	for (i = 0; i < pEntry->Endpoints.GetCount(); i++)
		{
		SEndpointSequence *pEndSeq = &pEntry->Endpoints[i];

		if (pEndSeq->dwSeqActual < pEndSeq->dwSeqToCheck)
			{
			//	If we don't yet have an endpoint ID, look it up

			if (pEndSeq->dwEndpointID == 0xffffffff)
				{
				pEndSeq->dwEndpointID = m_pDb->GetEndpointID(pEndSeq->sEndpoint);

				//	If we don't find the endpoint, that's OK. It means that we haven't
				//	yet gotten that replication. Clearly we are not up to date.

				if (pEndSeq->dwEndpointID == 0xffffffff)
					return false;
				}

			//	Get the new sequence and see if we're up to date

			pEndSeq->dwSeqActual = m_pDb->GetSequence(pEndSeq->dwEndpointID);
			if (pEndSeq->dwSeqActual < pEndSeq->dwSeqToCheck)
				return false;
			}
		}

	//	If we get this far, all endpoints in the entry are up to date
	//	(This also implies that all dwSeqActual values are initialized).

	return true;
	}

void CMnemosynthEngine::MsgEndpointList (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgEndpointList
//
//	Mnemosynth.endpoingList [{module}]

	{
	CDatum dResult;
	m_pDb->GenerateEndpointList(&dResult);
	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}

void CMnemosynthEngine::MsgRead (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgRead
//
//	Mnemosynth.read [{collection} [{key} [{module}]]]

	{
	const CString &sCollection = Msg.dPayload.GetElement(0);
	const CString &sKey = Msg.dPayload.GetElement(1);
	const CString &sModule = Msg.dPayload.GetElement(2);

	//	If we have a module then redirect to the appropriate module.



	//	If we have no collection then we return a list of all collections

	if (sCollection.IsEmpty())
		{
		TArray<CString> List;
		m_pDb->ReadCollectionList(&List);

		//	Short circuit.

		if (List.GetCount() == 0)
			{
			SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);
			return;
			}

		//	Return list of collection names

		CComplexArray *pData = new CComplexArray(List);
		SendMessageReply(MSG_REPLY_DATA, CDatum(pData), Msg);
		}

	//	Otherwise, if we have no key, then return all the keys in the 
	//	collection.

	else if (sKey.IsEmpty())
		{
		TArray<CString> List;
		m_pDb->ReadCollection(sCollection, &List);

		//	Short circuit.

		if (List.GetCount() == 0)
			{
			SendMessageReply(MSG_REPLY_DATA, CDatum(), Msg);
			return;
			}

		//	Return list of collection names

		CComplexArray *pData = new CComplexArray(List);
		SendMessageReply(MSG_REPLY_DATA, CDatum(pData), Msg);
		}

	//	Else, return the value

	else
		{
		CDatum dResult = m_pDb->Read(sCollection, sKey);
		SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
		}
	}

void CMnemosynthEngine::NotifyOnArcologyUpdate (const CString &sAddress, DWORD dwTicket, CDatum dPayload)

//	NotifyOnArcologyUpdate
//
//	Mnemosynth sends out Mnemosynth.notifyOnEndpointUpdate message to all other endpoints.
//	When all have replied with a confirmation, we send out a message in return.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Decode the payload

	const CString &sReplyMsg = dPayload.GetElement(FIELD_REPLY_MSG);
	CDatum dWatermark = dPayload.GetElement(FIELD_WATERMARK);

	//	Create a new notify entry

	SArcologyCheckpoint *pEntry = m_ArcologyCheckpoints.Insert();
	pEntry->dwID = m_dwNextID++;
	pEntry->sAddress = sAddress;
	pEntry->sMsg = (sReplyMsg.IsEmpty() ? MSG_MNEMOSYNTH_ON_ARCOLOGY_UPDATED : sReplyMsg);
	pEntry->dwTicket = dwTicket;

	//	Get the list of endpoints to send to.
	//	LATER: For now we send to all modules on the machine; later we will want options

	CMnemosynthWatermark DestList;
	m_pDb->GetWatermark(&DestList);

	//	Add all the endpoints to check

	for (i = 0; i < DestList.GetCount(); i++)
		{
		//	Add to the entry

		SEndpointSequence *pEndSeq = pEntry->Endpoints.Insert();
		pEndSeq->sEndpoint = DestList[i].sEndpoint;
		pEndSeq->dwEndpointID = DestList[i].dwEndpointID;

		//	This is unused

		pEndSeq->dwSeqToCheck = 0;

		//	We use this to determine when we get notifications. When non-zero, it means
		//	that we have an ack from this endpoint.

		pEndSeq->dwSeqActual = 0;

		//	Send out a message to the endpoint's Mnemosynth, requesting an update

		CString sAddress = strPattern("%s@%s", PORT_MNEMOSYNTH_COMMAND, (LPSTR)pEndSeq->sEndpoint);

		//	Always reply to our module

		CString sReplyAddr = GenerateAddress(PORT_MNEMOSYNTH_COMMAND);

		//	Send the message

		CComplexStruct *pPayload = new CComplexStruct;
		pPayload->SetElement(FIELD_WATERMARK, dWatermark);

		SendMessageCommand(sAddress, MSG_MNEMOSYNTH_NOTIFY_ON_UPDATE, sReplyAddr, pEntry->dwID, CDatum(pPayload));
		}
	}

void CMnemosynthEngine::NotifyOnEndpointUpdate (const CString &sAddress, DWORD dwTicket, CDatum dPayload)

//	NotifyOnEndpointUpdate
//
//	This asks Mnemosynth to send a notification when this endpoint reaches the
//	given watermark.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Decode the payload

	const CString &sReplyMsg = dPayload.GetElement(FIELD_REPLY_MSG);
	CDatum dWatermark = dPayload.GetElement(FIELD_WATERMARK);

	//	Create a new notify entry

	SLocalCheckpoint *pEntry = m_LocalCheckpoints.Insert();
	pEntry->sAddress = sAddress;
	pEntry->sMsg = (sReplyMsg.IsEmpty() ? MSG_MNEMOSYNTH_ON_ENDPOINT_UPDATED : sReplyMsg);
	pEntry->dwTicket = dwTicket;

	//	Add all the endpoints to check

	for (i = 0; i < dWatermark.GetCount(); i++)
		{
		CDatum dEntry = dWatermark.GetElement(i);

		//	Get the endpoint for the entry.
		//	NOTE: It is OK if we can't find the endpoint yet. It just means
		//	that we haven't gotten a replication.

		const CString &sEndpoint = dEntry.GetElement(0);
		DWORD dwEndpointID = m_pDb->GetEndpointID(sEndpoint);
		MnemosynthSequence dwSeq = (MnemosynthSequence)dEntry.GetElement(1);

		//	Add to the entry

		SEndpointSequence *pEndSeq = pEntry->Endpoints.Insert();
		pEndSeq->sEndpoint = sEndpoint;
		pEndSeq->dwEndpointID = dwEndpointID;
		pEndSeq->dwSeqToCheck = dwSeq;
		pEndSeq->dwSeqActual = 0;
		}

	//	If this notify request is already up to date, then we send out
	//	a message and delete this entry

	if (IsNotifyRequestUpToDate(pEntry))
		{
		SendOnEndpointUpdated(pEntry);
		m_LocalCheckpoints.Delete(m_LocalCheckpoints.GetCount() - 1);
		pEntry = NULL;
		}
	}

void CMnemosynthEngine::NotifyOnModified (CDatum dLocalUpdates, bool bLocalUpdate)

//	NotifyOnModified
//
//	This method sends out a Mnemosynth.onModified message to the
//	Mnemosynth.notify port. This is called whenever the local database changes.

	{
	int i;

	//	Debug log

	if (m_bDebug)
		{
		CStringBuffer DebugLog;

		if (bLocalUpdate)
			DebugLog.Write(strPattern("Mnemosynth modified on %s/%s:\n", GetMachineName(), GetModuleName()));
		else
			DebugLog.Write(strPattern("Mnemosynth synchronized:\n"));

		CDatum dCollections = dLocalUpdates.GetElement(FIELD_COLLECTIONS);
		CDatum dEntries = dLocalUpdates.GetElement(FIELD_ENTRIES);

		for (i = 0; i < dEntries.GetCount(); i++)
			{
			if (i != 0)
				DebugLog.Write("\n", 1);

			CDatum dEntry = dEntries.GetElement(i);
			DebugLog.Write(strPattern("%s %s.",
					dCollections.GetElement((int)dEntry.GetElement(0)),
					dEntry.GetElement(1).AsString()));
			}

		//	Debug

		Log(MSG_LOG_DEBUG, DebugLog);
		}

	//	Notify the process. [Since the process does not have its own engine, we 
	//	need to call it here, in case it cares.]

	OnMnemosynthDbModified(dLocalUpdates);

	//	Notify any engines who care

	SendMessageNotify(ADDRESS_MNEMOSYNTH_NOTIFY, MSG_MNEMOSYNTH_ON_MODIFIED, dLocalUpdates);
	}

void CMnemosynthEngine::OnBoot (void)

//	OnBoot
//
//	Boot up

	{
	//	Init some stuff

	m_dwNextID = 1;

	//	Register our port

	AddPort(PORT_MNEMOSYNTH_COMMAND);

	//	Request a message when Mnemosynth changes

	AddEventRequest(MSG_MNEMOSYNTH_ON_DB_MODIFIED, m_pDb->GetModifiedEvent(), this, MSG_MNEMOSYNTH_ON_DB_MODIFIED, 0);
	}

void CMnemosynthEngine::OnDbModified (void)

//	OnDbModified
//
//	Process message that tells us that mnemosynth has been modified on this
//	endpoint. We broadcast out to all other endpoints.

	{
	CSmartLock Lock(m_cs);
	int i, j;

#ifdef DEBUG_MNEMOSYNTH
	printf("[CMnemosynthEngine::OnDbModified]\n");
#endif

	//	Get list of changes (this will also reset the event
	//	and update the sequence watermark).

	TArray<SMnemosynthUpdate> Updates;
	CDatum dLocalUpdates;
	m_pDb->GenerateDelta(&Updates, &dLocalUpdates);

	//	Notify local engines that Mnemosynth has changed

	NotifyOnModified(dLocalUpdates, true);

	//	Send messages

	for (i = 0; i < Updates.GetCount(); i++)
		{
		SMnemosynthUpdate *pUpdate = &Updates[i];

		//	Send one message for each endpoint update
		//	Bind to the appropriate port

		CString sAddress = strPattern("%s@%s", PORT_MNEMOSYNTH_COMMAND, pUpdate->sDestEndpoint);

#ifdef DEBUG_MNEMOSYNTH
		Log(MSG_LOG_DEBUG, strPattern("Send %d mnemosynth update(s) to %s.", pUpdate->Payloads.GetCount(), sAddress));
#endif

		for (j = 0; j < pUpdate->Payloads.GetCount(); j++)
			{
			SendMessageCommand(sAddress, MSG_MNEMOSYNTH_UPDATE, NULL_STR, 0, pUpdate->Payloads[j]);
			}
		}

	//	Add a new request to be notified when the database is modified.

	AddEventRequest(MSG_MNEMOSYNTH_ON_DB_MODIFIED, m_pDb->GetModifiedEvent(), this, MSG_MNEMOSYNTH_ON_DB_MODIFIED, 0);
	}

void CMnemosynthEngine::OnEndpointUpdated (const CString &sReplyAddr, DWORD dwTicket, CDatum dWatermark)

//	OnEndpointUpdated
//
//	Handle a notification from some endpoint that it has reached its watermark

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Find the arcology checkpoint by ticket

	SArcologyCheckpoint *pCheckpoint = NULL;
	int iIndex;
	for (i = 0; i < m_ArcologyCheckpoints.GetCount(); i++)
		if (m_ArcologyCheckpoints[i].dwID == dwTicket)
			{
			pCheckpoint = &m_ArcologyCheckpoints[i];
			iIndex = i;
			break;
			}

	if (pCheckpoint == NULL)
		{
		Log(MSG_LOG_ERROR, CString("Unable to find arcology checkpoint ticket."));
		return;
		}

	//	Parse the reply address into an endpoint

	CString sModuleName, sMachineName;
	CMessageTransporter::ParseAddress(sReplyAddr, NULL, &sModuleName, &sMachineName);
	CString sEndpoint = CMnemosynthDb::GenerateEndpointName(sMachineName, sModuleName);

	//	Find the endpoint

	for (i = 0; i < pCheckpoint->Endpoints.GetCount(); i++)
		{
		SEndpointSequence *pEndSeq = &pCheckpoint->Endpoints[i];
		if (strEquals(pEndSeq->sEndpoint, sEndpoint))
			{
			pEndSeq->dwSeqActual = 1;
			break;
			}
		}

	//	See if all endpoints are up to date.

	bool bUpToDate = true;
	for (i = 0; i < pCheckpoint->Endpoints.GetCount(); i++)
		{
		SEndpointSequence *pEndSeq = &pCheckpoint->Endpoints[i];
		if (pEndSeq->dwSeqActual == 0)
			{
			bUpToDate = false;
			break;
			}
		}

	//	If all are up to date, then we can fire a message and remove
	//	this checkpoint from our list

	if (bUpToDate)
		{
		//	Send the message

		SendMessageCommand(pCheckpoint->sAddress, 
				pCheckpoint->sMsg, 
				GenerateAddress(PORT_MNEMOSYNTH_COMMAND),
				pCheckpoint->dwTicket, 
				dWatermark);

		//	Remove checkpoint

		m_ArcologyCheckpoints.Delete(iIndex);
		}
	}

void CMnemosynthEngine::OnProcessMessages (CArchonMessageList &List)

//	OnProcessMessages
//
//	Process messages

	{
	int i;

	for (i = 0; i < List.GetCount(); i++)
		{
		const SArchonMessage &Msg = List[i];

		try
			{
			ProcessMessage(Msg);
			}
		catch (CException e)
			{
			LogCrashProcessingMessage(Msg, e);
			}
		catch (...)
			{
			LogCrashProcessingMessage(Msg, CException(errUnknownError));
			}
		}
	}

void CMnemosynthEngine::OnStartRunning (void)

//	OnStartRunning
//
//	Module is running

	{
	}

void CMnemosynthEngine::ProcessMessage (const SArchonMessage &Msg)

//	ProcessMessage
//
//	Process the given message.

	{
	//	Exarch.shutdown

	if (strEquals(Msg.sMsg, MSG_EXARCH_SHUTDOWN))
		InitiateShutdown();

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_ENDPOINT_LIST))
		MsgEndpointList(Msg, NULL);

	//	Mnemosynth.notifyOnArcologUpdate {replyMsg:} {watermark: (({endpoint} {sequence}) (...))}

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_NOTIFY_ON_ARCOLOGY_UPDATE))
		NotifyOnArcologyUpdate(Msg.sReplyAddr, Msg.dwTicket, Msg.dPayload);

	//	Mnemosynth.notifyOnEndpointUpdate {replyMsg:} {watermark: (({endpoint} {sequence}) (...))}

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_NOTIFY_ON_UPDATE))
		NotifyOnEndpointUpdate(Msg.sReplyAddr, Msg.dwTicket, Msg.dPayload);

	//	Mnemosynth.onEndpointUpdated (({endpoint} {sequence}) (...))

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_ON_ENDPOINT_UPDATED))
		OnEndpointUpdated(Msg.sReplyAddr, Msg.dwTicket, Msg.dPayload);

	//	Mnemosynth.onModified

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_ON_DB_MODIFIED))
		OnDbModified();

	//	Mnemosynth.read {collection} {key} [{module}]

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_READ))
		MsgRead(Msg, NULL);

	//	Mnemosynth.update {delta}

	else if (strEquals(Msg.sMsg, MSG_MNEMOSYNTH_UPDATE))
		{
		CSmartLock Lock(m_cs);
		int i, j;

		m_pDb->IncorporateDelta(Msg.dPayload);

		//	Notify local engines that Mnemosynth has changed

		NotifyOnModified(Msg.dPayload, false);

		//	If we're central module then send out update to other modules

		if (GetProcessCtx()->IsCentralModule())
			{
			//	Get list of changes (this will also reset the event
			//	and update the sequence watermark).

			TArray<SMnemosynthUpdate> Updates;
			CDatum dLocalUpdates;
			m_pDb->GenerateDelta(&Updates, &dLocalUpdates);

			//	Send messages

			for (i = 0; i < Updates.GetCount(); i++)
				{
				SMnemosynthUpdate *pUpdate = &Updates[i];

				//	Send one message for each endpoint update
				//	Bind to the appropriate port

				CString sAddress = strPattern("%s@%s", PORT_MNEMOSYNTH_COMMAND, pUpdate->sDestEndpoint);

#ifdef DEBUG_MNEMOSYNTH
				printf("Broadcast Mnemosynth.update to %s\n", (LPSTR)sAddress);
#endif

				for (j = 0; j < pUpdate->Payloads.GetCount(); j++)
					SendMessageCommand(sAddress, MSG_MNEMOSYNTH_UPDATE, NULL_STR, 0, pUpdate->Payloads[j]);
				}
			}

		ProcessNotifyRequests();
		}
	}

void CMnemosynthEngine::ProcessNotifyRequests (void)

//	ProcessNotifyRequests
//
//	See if we need to fire events

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_LocalCheckpoints.GetCount(); i++)
		{
		SLocalCheckpoint *pEntry = &m_LocalCheckpoints[i];

		if (IsNotifyRequestUpToDate(pEntry))
			{
			SendOnEndpointUpdated(pEntry);

			//	Remove the notification entry

			m_LocalCheckpoints.Delete(i);
			i--;
			}
		}
	}

void CMnemosynthEngine::SendOnEndpointUpdated (SLocalCheckpoint *pEntry)

//	SendOnEndpointUpdated
//
//	Sends an endpoint updated event

	{
	int i;

	//	Create the payload and add each of the endpoints that are up to date

	CComplexArray *pPayload = new CComplexArray;
	for (i = 0; i < pEntry->Endpoints.GetCount(); i++)
		{
		CComplexArray *pEndSeq = new CComplexArray;
		pEndSeq->Insert(pEntry->Endpoints[i].sEndpoint);
		pEndSeq->Insert((int)pEntry->Endpoints[i].dwSeqActual);

		pPayload->Insert(CDatum(pEndSeq));
		}

	//	Send the message

	SendMessageCommand(pEntry->sAddress, 
			pEntry->sMsg, 
			GenerateAddress(PORT_MNEMOSYNTH_COMMAND),
			pEntry->dwTicket, 
			CDatum(pPayload));
	}
