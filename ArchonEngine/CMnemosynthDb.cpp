//	CMnemosynth.cpp
//
//	CMnemosynth class
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ID,							"id")
DECLARE_CONST_STRING(FIELD_PROCESS_ID,					"processID")
DECLARE_CONST_STRING(FIELD_SEQ_RECV,					"seqRecv")
DECLARE_CONST_STRING(FIELD_SEQ_SENT,					"seqSent")

DECLARE_CONST_STRING(STR_CENTRAL_MODULE,				"CentralModule")

DECLARE_CONST_STRING(STR_COLLECTIONS,					"collections")
DECLARE_CONST_STRING(STR_ENDPOINT,						"endpoint")
DECLARE_CONST_STRING(STR_ENTRIES,						"entries")

const DWORD NULL_ENDPOINT_ID =							0xffffffff;

SWatermark NULL_WATERMARK = {	0,	0 };

DWORD CMnemosynthDb::AddEndpoint (const CString &sName, DWORD dwProcessID, bool bSyncNow)

//	AddEndpoint
//
//	Adds an endpoint of the given name and returns its ID.

	{
	CSmartLock Lock(m_cs);

	SEndpoint *pEndpoint = GetOrAddEndpoint(sName, dwProcessID);

	//	If necessary, notify engine

	if (bSyncNow)
		m_ModifiedEvent.Set();

	//	Done

	return pEndpoint->dwID;
	}

void CMnemosynthDb::Boot (IArchonProcessCtx *pProcess)

//	Boot
//
//	Initialize

	{
	ASSERT(m_Endpoints.GetCount() == 0);

	m_pProcess = pProcess;
	m_ModifiedEvent.Create();
	m_dwNextID = 0;
	m_Seq = 1;

	//	Initialize our endpoint

	AddEndpoint(strPattern("%s/%s", m_pProcess->GetMachineName(), m_pProcess->GetModuleName()), ::GetCurrentProcessId());
	}

void CMnemosynthDb::DebugDump (IByteStream &Stream)

//	DebugDump
//
//	Dumps the entire database

	{
	CSmartLock Lock(m_cs);

	int i, j;

	for (i = 0; i < m_Collections.GetCount(); i++)
		{
		SCollection *pCollection = &m_Collections[i];
		if (pCollection->Entries.GetCount() == 0)
			continue;

		//	Write out the collection name

		const CString &sCollectionName = m_Collections.GetKey(i);
		Stream.Write(sCollectionName);
		Stream.Write("\n", 1);

		//	Write out each entry in the collection

		for (j = 0; j < pCollection->Entries.GetCount(); j++)
			{
			//	Write the key

			const CString &sKey = pCollection->Entries.GetKey(j);
			Stream.Write("\t", 1);
			Stream.Write(sKey);
			Stream.Write("\n", 1);

			SEntry *pEntry = &pCollection->Entries[j];
			Stream.Write("\t\t", 2);
			pEntry->dValue.Serialize(CDatum::EFormat::AEONScript, Stream);
			Stream.Write("\n", 1);
			}
		}
	}

void CMnemosynthDb::Delete (const CString &sCollection, const CString &sKey)

//	Delete
//
//	Delete an entry in a collection. This is just a write of Nil.

	{
	Write(sCollection, sKey, CDatum());
	}

void CMnemosynthDb::DeleteEntry (const CString &sCollection, const CString &sKey)

//	DeleteEntry
//
//	Delete an entry from the database

	{
	//	Look for the collection. If we can't find it, we're done

	SCollection *pCol = m_Collections.GetAt(sCollection);
	if (pCol == NULL)
		return;

	//	Look for the key in the collection. If we can't find it, we're done.

	int iIndex;
	if (!pCol->Entries.FindPos(sKey, &iIndex))
		return;

	//	Delete the entry

	pCol->Entries.Delete(iIndex);
	}

int CMnemosynthDb::FindEndpointIndex (DWORD dwID)

//	FindEndpointIndex
//
//	Returns the index of the given endpoint (or -1 if not found)
//	Assumes that we already have a lock.

	{
	int iPos;
	if (m_Endpoints.FindPos(dwID, &iPos))
		return iPos;

	return -1;
	}

int CMnemosynthDb::FindEndpointIndex (const CString &sName)

//	FindEndpointIndex
//
//	Returns the index of the given endpoint (or -1 if not found)
//	Assumes that we already have a lock.

	{
	for (int i = 0; i < m_Endpoints.GetCount(); i++)
		if (strEquals(m_Endpoints[i].sName, sName))
			return i;

	return -1;
	}

CMnemosynthDb::SEndpoint *CMnemosynthDb::FindEndpoint (DWORD dwID)

//	FindEndpoint
//
//	Returns the entry. NOTE: This must be called inside a lock and the result
//	must not be used outside of a lock (because it might change).

	{
	return m_Endpoints.GetAt(dwID);
	}

CMnemosynthDb::SEndpoint *CMnemosynthDb::FindEndpoint (const CString &sName)

//	FindEndpoint
//
//	Returns the entry. NOTE: This must be called inside a lock and the result
//	must not be used outside of a lock (because it might change).

	{
	for (int i = 0; i < m_Endpoints.GetCount(); i++)
		if (strEquals(m_Endpoints[i].sName, sName))
			return &m_Endpoints[i];

	return NULL;
	}

CDatum CMnemosynthDb::GenerateCollectionsArray (void)

//	GenerateCollectionsArray
//
//	Creates an array of collection names.
//
//	NOTE: Our callers rely on the fact that this includes all colelctions,
//	even ones with no entries.

	{
	int i;

	CComplexArray *pCollections = new CComplexArray;
	for (i = 0; i < m_Collections.GetCount(); i++)
		pCollections->Insert(m_Collections.GetKey(i));

	return CDatum(pCollections);
	}

void CMnemosynthDb::GenerateDelta (TArray<SMnemosynthUpdate> *retUpdates, CDatum *retdLocalUpdates)

//	GenerateDelta
//
//	Generates a list of changes since the last time we called this

	{
	CSmartLock Lock(m_cs);

	int i, j;

	//	Edge conditions

	if (m_Endpoints.GetCount() == 0)
		{
		*retdLocalUpdates = CDatum();
		return;
		}

	//	If we are not the central module, then we only need to
	//	generate a list of updates made by our own endpoint
	//	(and send it to our central module).

	if (!m_pProcess->IsCentralModule())
		{
		if (GetLocalEndpoint().dwSeqRecv > GetLocalEndpoint().dwSeqSent)
			{
			SMnemosynthUpdate *pUpdate = retUpdates->Insert();
			pUpdate->sDestEndpoint = strPattern("%s/CentralModule", m_pProcess->GetMachineName());

			//	Generate a payload for our updates

			CComplexArray *pEntries = new CComplexArray;
			for (i = 0; i < m_Collections.GetCount(); i++)
				{
				const CString &sCollection = m_Collections.GetKey(i);
				SCollection *pCollection = &m_Collections[i];

				for (j = 0; j < pCollection->Entries.GetCount(); j++)
					{
					SEntry *pEntry = &pCollection->Entries[j];
					if (pEntry->dwOwnerID == 0 && pEntry->dwSequence > GetLocalEndpoint().dwSeqSent)
						{
						CDatum dEntry = GenerateEntry(i, pCollection->Entries.GetKey(j), pEntry);
						pEntries->Insert(dEntry);

						//	If this entry is Nil then we can deleted. (We don't
						//	need it as a deletion stub since we just composed
						//	the update. If this message gets lost we need to
						//	resend everything).

						if (pCollection->Entries.GetValue(j).dValue.IsNil())
							{
							pCollection->Entries.Delete(j);
							j--;
							}
						}
					}
				}

			//	Create the payload

			CComplexStruct *pPayload = new CComplexStruct;
			pPayload->SetElement(STR_COLLECTIONS, GenerateCollectionsArray());
			pPayload->SetElement(STR_ENDPOINT, GetLocalEndpoint().sName);
			pPayload->SetElement(STR_ENTRIES, CDatum(pEntries));
			pPayload->SetElement(FIELD_PROCESS_ID, CDatum(GetLocalEndpoint().dwProcessID));

			//	Add it

			CDatum dLocalUpdates = CDatum(pPayload);
			pUpdate->Payloads.Insert(dLocalUpdates);

			//	Return it

			*retdLocalUpdates = dLocalUpdates;
			}
		else
			*retdLocalUpdates = CDatum();
		}

	//	Otherwise, loop over all endpoints and generate a different
	//	update entry for each one that we need to handle

	else
		{
		bool bFullUpdateNeeded = false;
		for (i = 1; i < m_Endpoints.GetCount(); i++)
			if (m_Endpoints[i].bFullUpdate)
				{
				bFullUpdateNeeded = true;
				break;
				}

		//	Collections

		CDatum dCollections = GenerateCollectionsArray();

		//	We end up creating one or two arrays of deltas. The first
		//	array has all the changes since we last generated a delta
		//	(this is used for endpoints that we updated last time).
		//
		//	The second array has a full set of data (this is for new
		//	endpoints).

		TArray<CComplexArray *> UpdateEntries;
		TArray<CComplexArray *> FullEntries;
		TArray<CDatum> UpdatePayloads;
		TArray<CDatum> FullPayloads;

		UpdateEntries.InsertEmpty(m_Endpoints.GetCount());
		UpdatePayloads.InsertEmpty(m_Endpoints.GetCount());
		if (bFullUpdateNeeded)
			{
			FullEntries.InsertEmpty(m_Endpoints.GetCount());
			FullPayloads.InsertEmpty(m_Endpoints.GetCount());
			}

		for (i = 0; i < m_Endpoints.GetCount(); i++)
			{
			UpdateEntries[i] = new CComplexArray;

			CComplexStruct *pStruct = new CComplexStruct;
			pStruct->SetElement(STR_COLLECTIONS, dCollections);
			pStruct->SetElement(STR_ENDPOINT, m_Endpoints[i].sName);
			pStruct->SetElement(STR_ENTRIES, CDatum(UpdateEntries[i]));
			pStruct->SetElement(FIELD_PROCESS_ID, CDatum(m_Endpoints[i].dwProcessID));
			UpdatePayloads[i] = CDatum(pStruct);

			if (bFullUpdateNeeded)
				{
				FullEntries[i] = new CComplexArray;

				pStruct = new CComplexStruct;
				pStruct->SetElement(STR_COLLECTIONS, dCollections);
				pStruct->SetElement(STR_ENDPOINT, m_Endpoints[i].sName);
				pStruct->SetElement(STR_ENTRIES, CDatum(FullEntries[i]));
				pStruct->SetElement(FIELD_PROCESS_ID, CDatum(m_Endpoints[i].dwProcessID));
				FullPayloads[i] = CDatum(pStruct);
				}
			}

		//	Loop over all entries in the database and add them to the
		//	appropriate payload arrays

		for (i = 0; i < m_Collections.GetCount(); i++)
			{
			const CString &sCollection = m_Collections.GetKey(i);
			SCollection *pCollection = &m_Collections[i];

			for (j = 0; j < pCollection->Entries.GetCount(); j++)
				{
				SEntry *pEntry = &pCollection->Entries[j];

				//	Get the endpoint for the owner of this collection

				int iOwner = FindEndpointIndex(pEntry->dwOwnerID);
				if (iOwner == -1)
					continue;

				//	Add to the update array

				if (pEntry->dwSequence > m_Endpoints[iOwner].dwSeqSent)
					{
					CDatum dEntry = GenerateEntry(i, pCollection->Entries.GetKey(j), pEntry);
					UpdateEntries[iOwner]->Insert(dEntry);

#ifdef DEBUG_MNEMOSYNTH
					printf("[CMnemosynthDb::GenerateDelta]: Endpoint %s %x\n", (LPSTR)m_Endpoints[iOwner].sName, m_Endpoints[iOwner].dwSeqSent);
#endif
					}

				//	Add to full array, if necessary

				if (bFullUpdateNeeded)
					{
					//	Don't bother inserting Nil entries (since this is a full
					//	update).

					if (!pEntry->dValue.IsNil())
						{
						CDatum dEntry = GenerateEntry(i, pCollection->Entries.GetKey(j), pEntry);
						FullEntries[iOwner]->Insert(dEntry);
						}
					}

				//	If this entry is Nil then we can deleted. (We don't
				//	need it as a deletion stub since we just composed
				//	the update. If this message gets lost we need to
				//	resend everything).

				if (pEntry->dValue.IsNil())
					{
					pCollection->Entries.Delete(j);
					j--;
					}
				}
			}

		//	Now iterate over all destination endpoints

		for (i = 1; i < m_Endpoints.GetCount(); i++)
			{
			SEndpoint *pDestEndpoint = &m_Endpoints[i];

#ifdef DEBUG_MNEMOSYNTH
			printf("[CMnemosynthDb::GenerateDelta]: Composing for endpoint %s %s%s.\n", (LPSTR)pDestEndpoint->sName, (pDestEndpoint->bCentralModule ? "CentralModule " : ""), (pDestEndpoint->bLocalMachine ? "local" : ""));
#endif

			//	If this is a local module, then send it any updates
			//	for everything except itself

			if (pDestEndpoint->bLocalMachine && !pDestEndpoint->bCentralModule)
				{
				SMnemosynthUpdate *pUpdate = NULL;

				for (j = 0; j < m_Endpoints.GetCount(); j++)
					if (i != j)
						{
						//	If we have no update entries, then skip.

						if (!pDestEndpoint->bFullUpdate
								&& UpdatePayloads[j].GetElement(STR_ENTRIES).GetCount() == 0)
							continue;

						//	Add an update entry

						if (pUpdate == NULL)
							{
							pUpdate = retUpdates->Insert();
							pUpdate->sDestEndpoint = pDestEndpoint->sName;
							}

						if (pDestEndpoint->bFullUpdate)
							pUpdate->Payloads.Insert(FullPayloads[j]);
						else
							pUpdate->Payloads.Insert(UpdatePayloads[j]);
						}
				}

			//	Otherwise, if this is a foreign central module, then
			//	send it any updates for all local endpoints

			else if (pDestEndpoint->bCentralModule && !pDestEndpoint->bLocalMachine)
				{
				SMnemosynthUpdate *pUpdate = NULL;

#ifdef DEBUG_MNEMOSYNTH
				if (pDestEndpoint->bFullUpdate)
					printf("[CMnemosynthDb::GenerateDelta]: Composing FULL update for %s\n", (LPSTR)pDestEndpoint->sName);
				else
					printf("[CMnemosynthDb::GenerateDelta]: Composing DIFF update for %s\n", (LPSTR)pDestEndpoint->sName);
#endif

				for (j = 0; j < m_Endpoints.GetCount(); j++)
					if (m_Endpoints[j].bLocalMachine)
						{
						//	If we have no update entries, then skip.

						if (!pDestEndpoint->bFullUpdate
								&& UpdatePayloads[j].GetElement(STR_ENTRIES).GetCount() == 0)
							continue;

#ifdef DEBUG_MNEMOSYNTH
						printf("[CMnemosynthDb::GenerateDelta]: Updates from %s\n", (LPSTR)m_Endpoints[j].sName);
#endif

						//	Add an update entry

						if (pUpdate == NULL)
							{
							pUpdate = retUpdates->Insert();
							pUpdate->sDestEndpoint = pDestEndpoint->sName;
							}

						if (pDestEndpoint->bFullUpdate)
							pUpdate->Payloads.Insert(FullPayloads[j]);
						else
							pUpdate->Payloads.Insert(UpdatePayloads[j]);
						}
				}
			}

		//	Local updates

		*retdLocalUpdates = UpdatePayloads[0];
		}

	//	Reset

	for (i = 0; i < m_Endpoints.GetCount(); i++)
		{
		m_Endpoints[i].dwSeqSent = m_Endpoints[i].dwSeqRecv;
		m_Endpoints[i].bFullUpdate = false;
		}

	m_ModifiedEvent.Reset();
	}

void CMnemosynthDb::GenerateEndpointList (CDatum *retdList)

//	GenerateEndpointList
//
//	Returns data about all endpoints

	{
	CSmartLock Lock(m_cs);
	int i;

	CComplexArray *pList = new CComplexArray;
	for (i = 0; i < m_Endpoints.GetCount(); i++)
		{
		CComplexStruct *pData = new CComplexStruct;
		pData->SetElement(FIELD_ID, m_Endpoints[i].sName);
		pData->SetElement(FIELD_SEQ_RECV, m_Endpoints[i].dwSeqRecv);
		pData->SetElement(FIELD_SEQ_SENT, m_Endpoints[i].dwSeqSent);

		pList->Append(CDatum(pData));
		}

	*retdList = CDatum(pList);
	}

CString CMnemosynthDb::GenerateEndpointName (const CString &sMachine, const CString &sModule)

//	GenerateEndpointName
//
//	Generates an endpoint name from machine and module

	{
	return strPattern("%s/%s", sMachine, sModule);
	}

CDatum CMnemosynthDb::GenerateEntry (int iCollectionIndex, const CString &sKey, SEntry *pEntry)

//	GenerateEntry
//
//	Generates an entry of the form:
//		0: collection index
//		1: key
//		2: value
//		3: sequence

	{
	CComplexArray *pArray = new CComplexArray;
	pArray->Insert(iCollectionIndex);
	pArray->Insert(sKey);
	pArray->Insert(pEntry->dValue);
	pArray->Insert(pEntry->dwSequence);

	return CDatum(pArray);
	}

DWORD CMnemosynthDb::GetEndpointID (const CString &sEndpoint)

//	GetEndpointID
//
//	Returns the ID for the given endpoint

	{
	CSmartLock Lock(m_cs);
	int iIndex = FindEndpointIndex(sEndpoint);
	if (iIndex == -1)
		return NULL_ENDPOINT_ID;

	return m_Endpoints[iIndex].dwID;
	}

const CString &CMnemosynthDb::GetEndpointName (DWORD dwEndpointID)

//	GetEndpointName
//
//	Returns the name of the endpoint by ID
	
	{
	CSmartLock Lock(m_cs);
	
	SEndpoint *pEndpoint = FindEndpoint(dwEndpointID);
	if (pEndpoint == NULL)
		return NULL_STR;

	return pEndpoint->sName;
	}

MnemosynthSequence CMnemosynthDb::GetNextSequence (void)

//	GetNextSequence
//
//	Returns a new sequence number

	{
	SEndpoint &Endpoint = GetLocalEndpoint();
	return ++Endpoint.dwSeqRecv;
	}

CMnemosynthDb::SEndpoint *CMnemosynthDb::GetOrAddEndpoint (const CString &sName, DWORD dwProcessID)

//	GetOrAddEndpoint
//
//	Returns an endpoint for the given name

	{
	CSmartLock Lock(m_cs);

	//	If we already have it, skip

	SEndpoint *pEndpoint = FindEndpoint(sName);
	if (pEndpoint != NULL)
		{
		//	If this is a new process ID, then it means that the module
		//	has been restarted, so we need to restart the sequence number.s

		if (dwProcessID != 0 && dwProcessID != pEndpoint->dwProcessID)
			{
#ifdef DEBUG_MODULE_RESTART
			printf("[%s] Resetting endpoint %s to process ID %x\n", (LPSTR)m_pProcess->GetModuleName(), (LPSTR)sName, dwProcessID);
#endif
			pEndpoint->dwProcessID = dwProcessID;
			pEndpoint->dwSeqRecv = 0;
			pEndpoint->dwSeqSent = 0;
			}

		return pEndpoint;
		}

	//	Add

	DWORD dwID = m_dwNextID++;
	pEndpoint = m_Endpoints.Insert(dwID);
	pEndpoint->dwID = dwID;
	pEndpoint->sName = sName;
	pEndpoint->pPort = NULL;
	pEndpoint->dwSeqRecv = 0;
	pEndpoint->dwSeqSent = 0;

	//	If the module name ends with CentralModule then this is
	//	a module with Exarch (responsible for cross-machine sync)

	pEndpoint->bCentralModule = strEndsWith(sName, STR_CENTRAL_MODULE);

	//	If the module starts with our machine name, then this is
	//	a local endpoint.

	pEndpoint->bLocalMachine = strStartsWith(sName, m_pProcess->GetMachineName());
	pEndpoint->dwProcessID = (pEndpoint->bLocalMachine ? dwProcessID : 0);

	//	This endpoint needs a full update of everything we've got

	pEndpoint->bFullUpdate = true;

	//	Done

	return pEndpoint;
	}

MnemosynthSequence CMnemosynthDb::GetSequence (DWORD dwEndpointID)

//	GetSequence
//
//	Returns the current sequence for the given endpoint

	{
	CSmartLock Lock(m_cs);
	SEndpoint *pEndpoint = FindEndpoint(dwEndpointID);
	if (pEndpoint == NULL)
		return NULL_MNEMO_SEQ;

	return pEndpoint->dwSeqRecv;
	}

MnemosynthSequence CMnemosynthDb::GetSequence (const CString &sEndpoint)

//	GetSequence
//
//	Returns the current sequence for the given endpoint

	{
	CSmartLock Lock(m_cs);
	SEndpoint *pEndpoint = FindEndpoint(sEndpoint);
	if (pEndpoint == NULL)
		return NULL_MNEMO_SEQ;

	return pEndpoint->dwSeqRecv;
	}

void CMnemosynthDb::GetWatermark (CMnemosynthWatermark *retWatermark)

//	GetWatermark
//
//	Gets the sequence numbers for all endpoints

	{
	CSmartLock Lock(m_cs);
	int i;

	retWatermark->DeleteAll();
	retWatermark->InsertEmpty(m_Endpoints.GetCount());

	for (i = 0; i < m_Endpoints.GetCount(); i++)
		{
		retWatermark->GetAt(i).dwEndpointID = m_Endpoints[i].dwID;
		retWatermark->GetAt(i).sEndpoint = m_Endpoints[i].sName;
		retWatermark->GetAt(i).dwSeq = m_Endpoints[i].dwSeqRecv;
		}
	}

CMnemosynthDb::SEntry *CMnemosynthDb::GetWriteEntry (const CString &sCollection, const CString &sKey)

//	GetWriteEntry
//
//	Returns an entry of the given key in the given collection. If it does not exist
//	it creates a new one.
//	
//	Assumes that we have a lock.

	{
	//	Look for the collection. If we can't find it, create it

	SCollection *pCol = m_Collections.GetAt(sCollection);
	if (pCol == NULL)
		pCol = m_Collections.Insert(sCollection);

	//	Look for the key in the collection. If we can't find it,
	//	create a new key.

	SEntry *pEntry = pCol->Entries.GetAt(sKey);
	if (pEntry == NULL)
		pEntry = pCol->Entries.Insert(sKey);

	return pEntry;
	}

void CMnemosynthDb::IncorporateDelta (CDatum dPayload)

//	IncorporateDelta
//
//	Incorporates the delta data

	{
	CSmartLock Lock(m_cs);

	int i;

	//	Get various elements

	CDatum dCollections = dPayload.GetElement(STR_COLLECTIONS);
	CDatum dEndpoint = dPayload.GetElement(STR_ENDPOINT);
	CDatum dEntries = dPayload.GetElement(STR_ENTRIES);
	DWORD dwProcessID = dPayload.GetElement(FIELD_PROCESS_ID);

	//	Make sure the endpoint exists

	SEndpoint *pEndpoint = GetOrAddEndpoint(dEndpoint.AsStringView(), dwProcessID);

	MnemosynthSequence dwOriginalSeq = pEndpoint->dwSeqRecv;
	MnemosynthSequence dwMaxSeq = dwOriginalSeq;

	//	Loop over all entries

	for (i = 0; i < dEntries.GetCount(); i++)
		{
		CDatum dEntry = dEntries.GetElement(i);
		MnemosynthSequence dwSeq = (MnemosynthSequence)dEntry.GetElement(3);
		if (dwSeq > dwOriginalSeq)
			{
			//	LATER: Detect and resolve conflicts

			if (dwSeq > dwMaxSeq)
				dwMaxSeq = dwSeq;

			CStringView sCollection = dCollections.GetElement((int)dEntry.GetElement(0));
			CStringView sKey = dEntry.GetElement(1);
			CDatum dValue = dEntry.GetElement(2);

			//	If we're not CentralModule and we get a deletion, then delete
			//	right away. If we're CentralModule then we incorporate because
			//	we will send it out on the next update.

			if (dValue.IsNil() && !m_pProcess->IsCentralModule())
				{
				DeleteEntry(sCollection, sKey);
#ifdef DEBUG_MNEMOSYNTH
				printf("Delete entry: %s/%s\n", (LPSTR)sCollection, (LPSTR)sKey);
#endif
				}

			//	Incorporate

			else
				{
				SEntry *pEntry = GetWriteEntry(sCollection, sKey);
				pEntry->dValue = dValue;
				pEntry->dwOwnerID = pEndpoint->dwID;
				pEntry->dwSequence = dwSeq;

#ifdef DEBUG_MNEMOSYNTH
				printf("Modify entry: %s/%s [owner = %s seq = %d]\n", (LPSTR)sCollection, (LPSTR)sKey, (LPSTR)pEndpoint->sName, dwSeq);
#endif
				}
			}
#ifdef DEBUG_MNEMOSYNTH
		else
			{
			printf("%s: Skipping %s because %d <= %d\n", (LPSTR)m_pProcess->GetModuleName(), (LPSTR)dCollections.GetElement((int)dEntry.GetElement(0)).AsString(), dwSeq, dwOriginalSeq);
			}
#endif
		}

	//	Done

	pEndpoint->dwSeqRecv = dwMaxSeq;
	m_Seq++;
	}

void CMnemosynthDb::Mark (void)

//	Mark
//
//	Mark all items in use

	{
	int i, j;

	//	Technically there is no need to lock since we only call this
	//	from a single thread while all other threads are stopped.

	for (i = 0; i < m_Collections.GetCount(); i++)
		{
		SCollection *pCol = &m_Collections.GetValue(i);

		for (j = 0; j < pCol->Entries.GetCount(); j++)
			pCol->Entries.GetValue(j).dValue.Mark();
		}
	}

void CMnemosynthDb::ParseEndpointName (const CString &sEndpoint, CString *retsMachine, CString *retsModule)

//	ParseEndpointName
//
//	Parses out an endpoint name into machine and module

	{
	char *pPos = sEndpoint.GetParsePointer();

	char *pStart = pPos;
	while (*pPos != '/' && *pPos != '\0')
		pPos++;

	if (*pPos == '\0')
		return;

	if (retsMachine)
		*retsMachine = CString(pStart, pPos - pStart);

	pPos++;

	if (retsModule)
		*retsModule = CString(pPos);
	}

CDatum CMnemosynthDb::Read (const CString &sCollection, const CString &sKey) const

//	Read
//
//	Read an entry in a collection

	{
	CSmartLock Lock(m_cs);

	//	Look for the collection. If we can't find it, then we return Nil

	SCollection *pCol = m_Collections.GetAt(sCollection);
	if (pCol == NULL)
		return CDatum();

	//	Look for the key. If we can't find it, then we return Nil

	SEntry *pEntry = pCol->Entries.GetAt(sKey);
	if (pEntry == NULL)
		return CDatum();

	//	Return the value

	return pEntry->dValue;
	}

void CMnemosynthDb::ReadCollection (const CString &sCollection, TArray<CString> *retKeys) const

//	ReadCollection
//
//	Returns a list of all the keys in the given collection.

	{
	CSmartLock Lock(m_cs);

	//	Look for the collection. If we can't find it, then we return Nil

	SCollection *pCol = m_Collections.GetAt(sCollection);
	if (pCol == NULL)
		return;

	//	Iterate over all keys

	for (int i = 0; i < pCol->Entries.GetCount(); i++)
		{
		//	A nil entry means that the entry was deleted.

		if (!pCol->Entries[i].dValue.IsNil())
			retKeys->Insert(pCol->Entries.GetKey(i));
		}
	}

void CMnemosynthDb::ReadCollection (const CString &sCollection, TSortMap<CString, CDatum> &retCollection) const

//	ReadCollection
//
//	Returns the entire collection.

	{
	CSmartLock Lock(m_cs);

	retCollection.DeleteAll();

	//	Look for the collection. If we can't find it, then we return Nil

	SCollection *pCol = m_Collections.GetAt(sCollection);
	if (pCol == NULL)
		return;

	//	Iterate over all keys

	retCollection.GrowToFit(pCol->Entries.GetCount());
	for (int i = 0; i < pCol->Entries.GetCount(); i++)
		{
		//	A nil entry means that the entry was deleted.

		if (pCol->Entries[i].dValue.IsNil())
			continue;

		//	Add the entry (it's already sorted, so we don't need to do an
		//	insertion sort).

		retCollection.InsertSorted(pCol->Entries.GetKey(i), pCol->Entries[i].dValue);
		}
	}

void CMnemosynthDb::ReadCollectionList (TArray<CString> *retCollections) const

//	ReadCollectionList
//
//	Returns a list of all collections

	{
	CSmartLock Lock(m_cs);
	int i;

	retCollections->DeleteAll();

	for (i = 0; i < m_Collections.GetCount(); i++)
		{
		SCollection *pCollection = &m_Collections[i];
		if (pCollection->Entries.GetCount() == 0)
			continue;

		//	Return the collection name

		retCollections->Insert(m_Collections.GetKey(i));
		}
	}

void CMnemosynthDb::RemoveEndpoint (const CString &sName)

//	RemoveEndpoint
//
//	Removes the given endpoint

	{
	CSmartLock Lock(m_cs);

	//	Find it.

	int iEndpoint;
	if ((iEndpoint = FindEndpointIndex(sName)) == -1)
		return;

#ifdef DEBUG_MNEMOSYNTH
	printf("[CMnemosynthDb::RemoveEndpoints]: Remove %s\n", (LPSTR)m_Endpoints[iEndpoint].sName);
#endif

	//	The first endpoint is always reserved for us--it cannot be deleted.

	if (iEndpoint == 0)
		return;

	//	Remove

	m_Endpoints.Delete(iEndpoint);
	}

void CMnemosynthDb::RemoveMachineEndpoints (const CString &sName)

//	RemoveMachineEndpoints
//
//	Removes all endpoints for the given machine.

	{
	CSmartLock Lock(m_cs);
	int i;

	//	Look for endpoints that start with the machine name

	CString sPattern = strPattern("%s/", sName);

	//	Delete them. We always start at 1 because the first endpoint is us
	//	and can never be deleted.

	for (i = 1; i < m_Endpoints.GetCount(); i++)
		if (strStartsWith(m_Endpoints[i].sName, sPattern))
			{
#ifdef DEBUG_MNEMOSYNTH
			printf("[CMnemosynthDb::RemoveMachineEndpoints]: Remove %s\n", (LPSTR)m_Endpoints[i].sName);
#endif
			m_Endpoints.Delete(i);
			i--;
			}
	}

void CMnemosynthDb::Write (const CString &sCollection, const CString &sKey, CDatum dValue)

//	Write
//
//	Write an entry in a collection

	{
	CSmartLock Lock(m_cs);

	//	Get the entry (creating it if necessary)

	SEntry *pEntry = GetWriteEntry(sCollection, sKey);

	//	Set the sequence number and owner. Since this is a local write, the
	//	owner is always us (which is always the first entry).

	pEntry->dwOwnerID = 0;
	pEntry->dwSequence = GetNextSequence();

	//	Set the data

	pEntry->dValue = dValue;

#ifdef DEBUG_MNEMOSYNTH
	printf("[CMnemosynthDb::Write]: Wrote %s/%s Seq = %x\n", (LPSTR)sCollection, (LPSTR)sKey, pEntry->dwSequence);
#endif

	//	Modified

	m_Seq++;
	m_ModifiedEvent.Set();
	}
