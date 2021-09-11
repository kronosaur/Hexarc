//	CInterprocessMessageQueue.cpp
//
//	CInterprocessMessageQueue class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_BLOCKS
#endif

DECLARE_CONST_STRING(MSG_ARC_FILE_MSG,					"Arc.fileMsg")
DECLARE_CONST_STRING(MSG_LOG_INFO,						"Log.info")

DECLARE_CONST_STRING(STR_FWD_COMMAND,					"FWD")
DECLARE_CONST_STRING(STR_PROTOCOL_AMP1_00,				"AMP/1.00")

DECLARE_CONST_STRING(PATH_HEXARC_IPQ,					"HexarcIPQ")

DECLARE_CONST_STRING(ERR_ENQUEUE_LARGE,					"IPMQ: Enqueing large payload (%s bytes).")
DECLARE_CONST_STRING(ERR_SERIALIZE_TIME_WARNING,		"IPMQ: Serialization complete.")
DECLARE_CONST_STRING(ERR_FILE_TIME_WARNING,				"IPMQ: Wrote payload to temp file.")

static constexpr size_t SIZE_WARNING_THRESHOLD =		200 * 1024;
static constexpr size_t DISK_QUEUE_THRESHOLD =			200 * 1024;
const int DEFAULT_ENTRY_SIZE =						4096;
const int MAX_ENQUEUE_TRIES =						10;

DWORD CInterprocessMessageQueue::AllocEntry (int iSize)

//	AllocEntry
//
//	Allocates a new entry of at least the given size

	{
	iSize = AlignUp(iSize, (int)sizeof(DWORD));

	//	Find a free block big enough to fit the message

	SFreeEntry *pPrev;
	DWORD dwOffset = FindFreeBlock(iSize, &pPrev);
	if (dwOffset == 0)
		{
#ifdef DEBUG_BLOCKS
		DebugDumpBlocks();
#endif
		return 0;
		}

	SFreeEntry *pFree = GetFreeEntry(dwOffset);

	//	Split up the free slot if it is bigger than
	//	the requested size (and if there is enough room
	//	for a second block)

	if (Abs(pFree->iSize) - iSize >= sizeof(SFreeEntry))
		{
		//	Allocate at the end of the block (so that when we free
		//	it there is a greater chance that there will be free
		//	space in front of us).

		DWORD dwNewBlock = dwOffset + Abs(pFree->iSize) - iSize;
		SEntry *pNewEntry = GetEntry(dwNewBlock);
		pNewEntry->iSize = iSize;

		//	Resize the previous block

		SetFree(pFree,
				Abs(pFree->iSize) - iSize - sizeof(SEntry),
				pFree->dwNext);

		//	Return the new block

		dwOffset = dwNewBlock;
		}

	//	Otherwise, we devote the whole block

	else
		{
		SEntry *pNewEntry = GetEntry(dwOffset);

		if (pPrev)
			pPrev->dwNext = pFree->dwNext;
		else
			m_pHeader->dwFirstFree = pFree->dwNext;

		pNewEntry->iSize = Abs(pFree->iSize);
		}

	return dwOffset;
	}

void CInterprocessMessageQueue::CombineFreeBlocks (DWORD dwFirst, DWORD dwSecond)

//	CombineFreeBlocks
//
//	Combines two free blocks that are adjacent to each other

	{
	ASSERT(dwFirst != 0 && dwSecond != 0);

	SFreeEntry *pFirst = GetFreeEntry(dwFirst);
	SFreeEntry *pSecond = GetFreeEntry(dwSecond);

	//	First find the block that points to the second block

	SFreeEntry *pPrev = NULL;
	DWORD dwOffset = m_pHeader->dwFirstFree;
	while (dwOffset != 0)
		{
		if (dwOffset == dwSecond)
			break;

		pPrev = GetFreeEntry(dwOffset);
		dwOffset = pPrev->dwNext;
		}

	//	If we can't find it, then we can't coallesce
	//	(This should never happen)

	if (dwOffset == 0)
		{
		ASSERT(false);
		return;
		}

	//	The previous block now points to wherever the
	//	second block points to.

	if (pPrev == NULL)
		m_pHeader->dwFirstFree = pSecond->dwNext;
	else
		pPrev->dwNext = pSecond->dwNext;

	//	Increase the size of the first block

	SetFree(pFirst, 
			Abs(pFirst->iSize) + Abs(pSecond->iSize) + sizeof(SEntry), 
			pFirst->dwNext);
	}

bool CInterprocessMessageQueue::Create (IArchonProcessCtx *pCtx, const CString &sMachine, const CString &sProcess, int iMaxSize)

//	Create
//
//	Creates the queue. If the queue already exists (or if we have another error)
//	then we return FALSE.
//
//	In memory, the queue looks like this:
//
//	DWORD			total size of block
//	DWORD			size of queue (including extra slot)
//	DWORD			head
//	DWORD			tail
//	DWORD			offset to first free entry
//	char[256]		local temp path (UTF8, null-terminated)
//	---------------------
//	DWORD			offset to first entry
//	DWORD			offset to second entry
//	...
//	DWORD			offset to last entry
//	---------------------
//	DWORD			size of entry (if negative, this is free)
//	DWORD			(if free, this is the offset to next free entry)
//	...
//	DWORD			0 (we always end with a 0-size entry)

	{
	ASSERT(iMaxSize > 0);

	m_pProcess = pCtx;
	CString sName = strPattern("%s-%s", sMachine, sProcess);

	//	Create the semaphore (if it already exists, then we fail)

	bool bExists;
	m_Lock.Create(GetSemaphoreName(sName), 1, &bExists);
	if (bExists)
		{
		//	Close explicitly so we don't hold on to the handle
		m_Lock.Close();
		return false;
		}

	//	Lock the semaphore

	m_Lock.Increment();

	//	Figure out the temp path

	CString sTempPath = fileGetTempPath();
	if (sTempPath.IsEmpty())
		{
		m_Lock.Decrement();
		m_Lock.Close();
		return false;
		}

	m_sTempPath = fileAppend(sTempPath, PATH_HEXARC_IPQ);
	if (m_sTempPath.GetLength() > sizeof(m_pHeader->szTempPath) - 1)
		{
		m_Lock.Decrement();
		m_Lock.Close();
		return false;
		}

	if (!filePathCreate(m_sTempPath))
		{
		m_Lock.Decrement();
		m_Lock.Close();
		return false;
		}

	//	Create the event

	m_HasMessages.Create(GetEventName(sName), &bExists);
	if (bExists)
		{
		m_HasMessages.Close();
		m_Lock.Decrement();
		m_Lock.Close();
		return false;
		}

	//	Figure out the size of the block that we need to reserve

	int iBlockSize = AlignUp((int)sizeof(SHeader)
			+ ((iMaxSize + 1) * (int)sizeof(DWORD))
			+ (iMaxSize * DEFAULT_ENTRY_SIZE),
			4096);

	//	Create the shared block of memory

	m_Queue.Create(GetMemoryName(sName), iBlockSize, &bExists);
	if (bExists)
		{
		m_Queue.Close();
		m_HasMessages.Close();
		m_Lock.Decrement();
		m_Lock.Close();
		return false;
		}

	//	Initialize the structure. We start by defining the header

	m_pHeader = (SHeader *)m_Queue.GetPointer();
	m_pHeader->dwTotalSize = iBlockSize;
	m_pHeader->dwSize = iMaxSize + 1;
	m_pHeader->dwHead = 0;
	m_pHeader->dwTail = 0;
	m_pHeader->dwFirstFree = sizeof(SHeader) 
			+ (m_pHeader->dwSize * sizeof(DWORD));

	utlMemCopy(m_sTempPath.GetParsePointer(), m_pHeader->szTempPath, m_sTempPath.GetLength() + 1);

	//	Initialize the first free entry to include all of the 
	//	remaining space.

	SFreeEntry *pFree = GetFreeEntry(m_pHeader->dwFirstFree);
	SetFree(pFree,
			m_pHeader->dwTotalSize 
				- sizeof(SHeader)
				- (m_pHeader->dwSize * sizeof(DWORD))
				- sizeof(SEntry)	//	Header for first free entry
				- sizeof(SEntry),	//	Last entry is 0 size
			0);

	//	The last entry has 0 size

	SFreeEntry *pLast = GetFreeEntry(m_pHeader->dwTotalSize - sizeof(SEntry));
	pLast->iSize = 0;

	//	We remember the name

	m_sName = sName;

#ifdef DEBUG_BLOCKS
	DebugDumpBlocks();
#endif

	//	Done

	m_Lock.Decrement();
	return true;
	}

bool CInterprocessMessageQueue::CreateTempFile (CString *retsFilespec, CFile *retFile, CString *retsError)

//	CreateTempFile
//
//	Creates a temp file.

	{
	CString sFilespec;

	bool bRecreatePath = true;
	bool bSuccess = false;
	int iTries = 0;
	do
		{
		iTries++;

		//	Pick a random ID for the file

		CDateTime Now(CDateTime::Now);
		DWORD dwID1 = (DWORD)(Now.DaysSince1AD() - (2000 * 365));
		DWORD dwID2 = mathRandom(1, 65535);
		DWORD dwID3 = (DWORD)Now.MillisecondsSinceMidnight();

		//	Attempt to create a file of that name

		sFilespec = fileAppend(m_sTempPath, strPattern("%04x%04x_%08x.aeon", dwID1, dwID2, dwID3));
		bSuccess = retFile->Create(sFilespec, CFile::FLAG_CREATE_NEW, retsError);

		//	If we fail, try recreating the temp file path. Sometimes a different 
		//	process (Windows Update?) deletes our directory.

		if (!bSuccess && bRecreatePath)
			{
			::filePathCreate(m_sTempPath);
			bRecreatePath = false;
			}
		}
	while (!bSuccess && iTries < 20);

	//	Done

	if (!bSuccess)
		return false;

	if (retsFilespec)
		*retsFilespec = sFilespec;

	return true;
	}

void CInterprocessMessageQueue::DebugDumpBlocks (void)

//	DebugDumpBlocks
//
//	Outputs the current state of the buffer

	{
	//	Output the queue

	DWORD dwEntry = m_pHeader->dwHead;
	while (dwEntry != m_pHeader->dwTail)
		{
		SQueueSlot *pSlot = GetQueueSlot(dwEntry);
		SEntry *pEntry = GetEntry(pSlot->dwOffset);
		printf("%08x: Allocated: %x bytes.\n", pSlot->dwOffset, pEntry->iSize);

		dwEntry = (dwEntry + 1) % m_pHeader->dwSize;
		}

	//	Output the free list

	SFreeEntry *pPrev = NULL;
	DWORD dwOffset = m_pHeader->dwFirstFree;
	while (dwOffset != 0)
		{
		SFreeEntry *pFree = GetFreeEntry(dwOffset);
		printf("%08x: Free: %x bytes.\n", dwOffset, -pFree->iSize);

		pPrev = pFree;
		dwOffset = pFree->dwNext;
		}
	}

bool CInterprocessMessageQueue::DecodeFileMsg (const SArchonMessage &Msg, SArchonMessage *retMsg)

//	DecodeFileMsg
//
//	Converts an Arc.fileMsg message into the full message contained in the 
//	temporary file.

	{
	CFile TempFile;

	if (!TempFile.Create(Msg.dPayload, CFile::FLAG_OPEN_READ_ONLY))
		return false;

	bool bSuccess;
	try
		{
        CBufferedIO FileBuffer(TempFile);

		SArchonEnvelope Env;
		bSuccess = DeserializeMessage(FileBuffer, &Env);
		*retMsg = Env.Msg;
		}
	catch (...)
		{
		bSuccess = false;
		}

	//	Delete the temp file since we don't need it.

	TempFile.Close();
	fileDelete(TempFile.GetFilespec());

	//	Done

	return bSuccess;
	}

bool CInterprocessMessageQueue::Dequeue (int iMaxCount, TArray<CString> *retList)

//	Dequeue
//
//	Adds up to iMaxCount messages to the list (or returns false if there
//	are no messages in the queue).

	{
	int i;

	ASSERT(IsOpen());

	//	Lock the semaphore

	m_Lock.Increment();

	//	Pull entries out of the queue

	int iCount = 0;
	for (i = 0; i < iMaxCount && !IsEmpty(); i++)
		{
		//	Get the head of the queue

		SQueueSlot *pHead = GetQueueSlot(m_pHeader->dwHead);

		//	Get the entry and add it to the return list

		SEntry *pEntry = GetEntry(pHead->dwOffset);
		retList->Insert(CString((char *)&pEntry[1], pEntry->iSize));
		iCount++;

		//	Free the entry

		FreeEntry(pHead->dwOffset);

		//	Remove from the queue

		m_pHeader->dwHead = (m_pHeader->dwHead + 1) % m_pHeader->dwSize;
		}

	//	Done

	if (IsEmpty())
		m_HasMessages.Reset();

	m_Lock.Decrement();

	return (iCount > 0);
	}

bool CInterprocessMessageQueue::DeserializeMessage (const CString &sEnv, SArchonEnvelope *retEnv)

//	DeserializeMessage
//
//	Deserializes

	{
	CBuffer Stream(sEnv);

	if (!DeserializeMessage(Stream, retEnv))
		return false;

	//	Done

	return true;
	}

bool CInterprocessMessageQueue::DeserializeMessage (IByteStream &Stream, SArchonEnvelope *retEnv)

//	DeserializeMessage
//
//	Deserialize from a stream

	{
	//	We expect to read:
	//
	//	AMP/1.00 FWD {destAddr} {replyAddr} {ticket} {msg} {payload}

	CDatum Item;
	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &Item))
		return false;

	if (!strEquals(Item, STR_PROTOCOL_AMP1_00))
		return false;

	//	FWD

	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &Item))
		return false;

	if (!strEquals(Item, STR_FWD_COMMAND))
		return false;

	//	{destAddr}

	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &Item))
		return false;

	retEnv->sAddr = Item;

	//	{replyAddr}

	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &Item))
		return false;

	retEnv->Msg.sReplyAddr = Item;

	//	{ticket}

	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &Item))
		return false;

	retEnv->Msg.dwTicket = (int)Item;

	//	{msg}

	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &Item))
		return false;

	retEnv->Msg.sMsg = Item;

	//	Payload

	if (!CDatum::Deserialize(CDatum::EFormat::AEONLocal, Stream, &retEnv->Msg.dPayload))
		return false;

	//	Done

#ifdef DEBUG_BLOB_PERF
    DWORD dwTime = ::sysGetTicksElapsed(dwStart);
    if (dwTime > 100)
        printf("Deserialize message %s took %d ms.\n", (LPSTR)retEnv->Msg.sMsg, dwTime);
#endif

	return true;
	}

bool CInterprocessMessageQueue::Enqueue (const SArchonEnvelope &Env, CString *retsError)

//	Enqueue
//
//	Enqueues a message to the queue. Returns TRUE if the enqueue succeeded.

	{
	//	If we're not yet open, we try to open

	if (!IsOpen())
		{
		if (m_sName.IsEmpty())
			{
			*retsError = strPattern("IPMQ: invalid name.");
			return false;
			}

		if (!Open())
			{
			*retsError = strPattern("IPMQ %s: Unable to open queue.", m_sName);
			return false;
			}
		}

	//	Compute the approximate size of the payload. If it's particularly large,
	//	we log it.

	size_t PayloadSize = Env.Msg.dPayload.CalcSerializeSize(CDatum::EFormat::AEONLocal);
	if (PayloadSize > SIZE_WARNING_THRESHOLD)
		m_pProcess->Log(MSG_LOG_INFO, strPattern(ERR_ENQUEUE_LARGE, strFormatInteger((int)PayloadSize, -1, FORMAT_THOUSAND_SEPARATOR)));

	//	Serialize the payload into a buffer

	CArchonTimer Timer;

	CMemoryBuffer Buffer(4096 + (int)PayloadSize);
	SerializeMessage(Env, Buffer);

	Timer.LogTime(m_pProcess, ERR_SERIALIZE_TIME_WARNING);

	//	If the buffer is very large then we store it as a temp file instead
	//	(to save our limited memory).

	if (Buffer.GetLength() > DISK_QUEUE_THRESHOLD)
		{
		CString sFilespec;
		CFile TempFile;

		Timer.Start();

#ifdef DEBUG_BLOB_PERF
        printf("Serializing to file.\n");
#endif

		CString sError;
		if (!CreateTempFile(&sFilespec, &TempFile, &sError))
			{
			*retsError = strPattern("IPMQ %s: Unable to create temp file: %s.", m_sName, sError);
			return false;
			}

		try
			{
			TempFile.Write(Buffer.GetPointer(), Buffer.GetLength());
			TempFile.Close();
			}
		catch (...)
			{
			*retsError = strPattern("IPMQ %s: Unable to write temp file %s.", m_sName, sFilespec);
			return false;
			}

		//	Generate a new buffer containing a pointer to the file.

		SArchonEnvelope FileEnv;
		FileEnv.sAddr = Env.sAddr;
		FileEnv.Msg.sMsg = MSG_ARC_FILE_MSG;
		FileEnv.Msg.dPayload = CDatum(sFilespec);

		//	Rewrite the buffer

		Buffer.SetLength(0);
		SerializeMessage(FileEnv, Buffer);

		Timer.LogTime(m_pProcess, ERR_FILE_TIME_WARNING);
		}

	//	Try enqueueing

	int iTries = 0;
	while (!Enqueue(Buffer, retsError))
		{
		if (++iTries >= MAX_ENQUEUE_TRIES)
			return false;

		//	Wait a bit to see if the queue clears

		::Sleep(30);
		}

	return true;
	}

bool CInterprocessMessageQueue::Enqueue (CMemoryBuffer &Buffer, CString *retsError)

//	Enqueue
//
//	Enqueues a buffer.

	{
	m_Lock.Increment();

	//	If we're full, then we're done

	if (IsFull())
		{
		*retsError = strPattern("IPMQ %s: Queue is full.", m_sName);
		m_Lock.Decrement();
		return false;
		}

	//	Find a free block big enough to fit the message

	DWORD dwOffset = AllocEntry(Buffer.GetLength() + 1);
	if (dwOffset == 0)
		{
		*retsError = strPattern("IPMQ %s: Queue is out of memory.", m_sName);
		m_Lock.Decrement();
		return false;
		}

	SEntry *pNewEntry = GetEntry(dwOffset);

	//	Copy the buffer to the free block

	utlMemCopy(Buffer.GetPointer(), &pNewEntry[1], Buffer.GetLength());

	//	Copy a terminating NULL (because sometimes the size of the block
	//	is larger than the buffer).

	utlMemSet(((char *)&pNewEntry[1]) + Buffer.GetLength(), 1);

	//	Remember if we're empty (so that we can signal the event)

	bool bWasEmpty = IsEmpty();

	//	Add to the queue

	SQueueSlot *pPos = GetQueueSlot(m_pHeader->dwTail);
	pPos->dwOffset = dwOffset;
	m_pHeader->dwTail = (m_pHeader->dwTail + 1) % m_pHeader->dwSize;

	//	Set the event

	if (bWasEmpty)
		m_HasMessages.Set();

	//	Done

	m_Lock.Decrement();

	return true;
	}

DWORD CInterprocessMessageQueue::FindFreeBlock (int iMinSize, SFreeEntry **retpPrev)

//	FindFreeBlock
//
//	Finds a free block of at least the given size (or 0 if none are found).

	{
	while (true)
		{
		SFreeEntry *pPrev = NULL;
		DWORD dwOffset = m_pHeader->dwFirstFree;
		while (dwOffset != 0)
			{
			SFreeEntry *pFree = GetFreeEntry(dwOffset);
			if (Abs(pFree->iSize) >= iMinSize)
				{
				if (retpPrev)
					*retpPrev = pPrev;
				return dwOffset;
				}

			pPrev = pFree;
			dwOffset = pFree->dwNext;
			}

		//	If we can't find a block that is big enough then try to combine
		//	two adjacent free blocks together.

		bool bCombined = false;
		pPrev = NULL;
		dwOffset = m_pHeader->dwFirstFree;
		while (dwOffset != 0)
			{
			SFreeEntry *pFree = GetFreeEntry(dwOffset);
			DWORD dwNext = GetNextEntry(dwOffset);
			SEntry *pNext = GetEntry(dwNext);

			if (pNext->iSize != 0 && IsFree(dwNext))
				{
				CombineFreeBlocks(dwOffset, dwNext);
				bCombined = true;

				//	Try again.

#ifdef DEBUG_BLOCKS
				printf("COMBINED\n");
				DebugDumpBlocks();
#endif

				break;
				}

			pPrev = pFree;
			dwOffset = pFree->dwNext;
			}

		//	If we couldn't combine anything then we're done.
		//	We failed to allocate. Otherwise we loop and try again.

		if (!bCombined)
			return 0;
		}

	return 0;
	}

void CInterprocessMessageQueue::FreeEntry (DWORD dwOffset)

//	FreeEntry
//
//	Frees the entry at the given offset

	{
	//	This entry now points to the first free

	SFreeEntry *pFree = GetFreeEntry(dwOffset);
	SetFree(pFree, Abs(pFree->iSize), m_pHeader->dwFirstFree);

	//	First free points to this entry

	m_pHeader->dwFirstFree = dwOffset;

	//	If the entry following this one is also free, then 
	//	coallesce the two

	DWORD dwNext = GetNextEntry(dwOffset);
	SEntry *pNext = GetEntry(dwNext);
	if (pNext->iSize == 0)
		return;
	else if (IsFree(dwNext))
		CombineFreeBlocks(dwOffset, dwNext);
	}

bool CInterprocessMessageQueue::Open (void)

//	Open
//
//	Opens an existing queue.

	{
	//	Open the semaphore (if it doesn't exists, then we fail)

	bool bExists;
	m_Lock.Create(GetSemaphoreName(m_sName), 1, &bExists);
	if (!bExists)
		{
		//	Close explicitly so we don't hold on to the handle
		m_Lock.Close();
		return false;
		}

	//	Create the event

	m_HasMessages.Create(GetEventName(m_sName), &bExists);
	if (!bExists)
		{
		m_HasMessages.Close();
		m_Lock.Close();
		return false;
		}

	//	Create the shared block of memory

	m_Queue.Create(GetMemoryName(m_sName), sizeof(SHeader), &bExists);
	if (!bExists)
		{
		m_Queue.Close();
		m_HasMessages.Close();
		m_Lock.Close();
		return false;
		}

	//	Initialize

	m_pHeader = (SHeader *)m_Queue.GetPointer();

	//	Now that we have the header we can map the actual size
	//	of the entire block

	m_Queue.SetMaxSize(m_pHeader->dwTotalSize);

	//	Reset the pointer (because SetMaxSize changes it)

	m_pHeader = (SHeader *)m_Queue.GetPointer();
	m_sTempPath = CString(m_pHeader->szTempPath);

	return true;
	}

void CInterprocessMessageQueue::SerializeMessage (const SArchonEnvelope &Env, IByteStream &Stream)

//	SerializeMessage
//
//	Serialize the message to the byte stream. We use the AMP/1 message format:
//
//	AMP/1.00 FWD {destAddr} {replyAddr} {ticket} {msg} {payload}
//
//	NOTE: Since this queue is only valid on the given machine, we use the
//	AEONLocal format, which can optimize certain operations by minimizing disk
//	copies and streaming.

	{
#ifdef DEBUG_BLOB_PERF
    DWORD dwStart = ::sysGetTickCount();
#endif

	Stream.Write("AMP/1.00 FWD ", 13);

	CDatum Item(Env.sAddr);
	Item.Serialize(CDatum::EFormat::AEONLocal, Stream);
	Stream.Write(" ", 1);

	Item = CDatum(Env.Msg.sReplyAddr);
	Item.Serialize(CDatum::EFormat::AEONLocal, Stream);
	Stream.Write(" ", 1);

	Item = CDatum((int)Env.Msg.dwTicket);
	Item.Serialize(CDatum::EFormat::AEONLocal, Stream);
	Stream.Write(" ", 1);

	Item = CDatum(Env.Msg.sMsg);
	Item.Serialize(CDatum::EFormat::AEONLocal, Stream);
	Stream.Write(" ", 1);

	Env.Msg.dPayload.Serialize(CDatum::EFormat::AEONLocal, Stream);

#ifdef DEBUG_BLOB_PERF
    DWORD dwTime = ::sysGetTicksElapsed(dwStart);
    if (dwTime > 100)
        printf("Serialize message %s took %d ms.\n", (LPSTR)Env.Msg.sMsg, dwTime);
#endif
	}
