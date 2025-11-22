//	CBlackBoxProcessor.cpp
//
//	CBlackBoxProcessor class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_INVALID_SEARCH,				"Invalid log search.")
DECLARE_CONST_STRING(ERR_UNKNOWN_SESSION,				"Unknown search session ID.")

const DWORDLONG EXPIRE_TIME =				60 * 1000;	//	Search stays around for 1 minute since last query

CBlackBoxProcessor::CBlackBoxProcessor (void) :
		m_dwNextSessionID(1)

//	BlackBoxReader constructor

	{
	}

CBlackBoxProcessor::~CBlackBoxProcessor (void)

//	CBlackBoxProcessor destructor

	{
	int i;

	for (i = 0; i < m_Sessions.GetCount(); i++)
		delete m_Sessions[i];
	}

bool CBlackBoxProcessor::CanDelete (SSession &Session, DWORDLONG dwNow) const

//	CanDelete
//
//	Returns TRUE if we can (and should) delete the session.

	{
	return (Session.iStatus == statusDeleted)
			|| (Session.iStatus == statusComplete && Session.Expires <= dwNow);
	}

bool CBlackBoxProcessor::CreateSession (const CString &sSearch, const SOptions &Options, DWORD *retdwID, CString *retsError)

//	CreateSession
//
//	Creates a new session with the given parameters and returns a session ID
//	(or an error).

	{
	CSmartLock Lock(m_cs);

	if (Options.iMaxLines <= 0 
			|| (Options.After.IsValid() && Options.Before.IsValid() && Options.After >= Options.Before))
		{
		if (retsError) *retsError = ERR_INVALID_SEARCH;
		return false;
		}

	SSession *pNewSession = new SSession;
	pNewSession->dwID = m_dwNextSessionID++;

	pNewSession->bNoCase = Options.bNoCase;
	pNewSession->sSearch = (pNewSession->bNoCase ? ::strToLower(sSearch) : sSearch);

	pNewSession->iMaxLines = Options.iMaxLines;
	pNewSession->After = Options.After;
	pNewSession->Before = Options.Before;

	//	Add to list

	m_Sessions.SetAt(pNewSession->dwID, pNewSession);

	//	Done

	if (retdwID)
		*retdwID = pNewSession->dwID;

	return true;
	}

CBlackBoxProcessor::SSession *CBlackBoxProcessor::GetNextSession (void)

//	GetNextSession
//
//	Looks for a session that needs processing and returns it, marking it as
//	being processed. Callers must call DoneProcessing when done with the
//	session (even if more work is needed).
//
//	If there are no sessions that need processing, we return NULL.

	{
	CSmartLock Lock(m_cs);
	int i;

	for (i = 0; i < m_Sessions.GetCount(); i++)
		if (m_Sessions[i]->iStatus == statusNone)
			{
			m_Sessions[i]->iStatus = statusRunning;
			return m_Sessions[i];
			}

	return NULL;
	}

bool CBlackBoxProcessor::GetResults (DWORD dwID, DWORD *retdwLinesSearched, CDatum *retdResult, CString *retsError)

//	GetResults
//
//	Returns the results for this session. If we haven't yet finished searching,
//	then retdResult is Nil. If we've finished searching and have no results then
//	the result value is True.

	{
	CSmartLock Lock(m_cs);

	SSession *pSession;
	if (!m_Sessions.Find(dwID, &pSession) || pSession->iStatus == statusDeleted)
		{
		if (retsError) *retsError = ERR_UNKNOWN_SESSION;
		return false;
		}

	//	If the session result is a string, then we got an error searching, so
	//	we return it here.

	if (pSession->dResult.GetBasicType() == CDatum::typeString)
		{
		if (retsError) *retsError = pSession->dResult.AsString();
		return false;
		}

	//	Otherwise, we return what the session has

	else
		{
		*retdwLinesSearched = pSession->dwLinesChecked;
		*retdResult = pSession->dResult;

		//	If we have valid results, then we can delete the session, since no one
		//	will ask for it again.

		if (!pSession->dResult.IsNil())
			pSession->iStatus = statusDeleted;

		return true;
		}
	}

bool CBlackBoxProcessor::Init (const CString &sPath, CString *retsError)

//	Init
//
//	Initialize

	{
	if (!m_LogFiles.Init(sPath, retsError))
		return false;

	return true;
	}

void CBlackBoxProcessor::Mark (void)

//	Mark
//
//	Mark data in use

	{
	CSmartLock Lock(m_cs);
	int i;

	DWORDLONG Now = ::sysGetTickCount64();

	for (i = 0; i < m_Sessions.GetCount(); i++)
		{
		//	If this session is complete, then see if it has expired. If so, we
		//	take this opportunity to delete it.

		if (CanDelete(*m_Sessions[i], Now))
			{
			delete m_Sessions[i];
			m_Sessions.Delete(i);
			i--;
			}

		//	Otherwise, mark the data

		else
			m_Sessions[i]->dResult.Mark();
		}

	//	Take this opportunity (when threads are paused) to close the underlying
	//	files, if necessary.

	m_LogFiles.CloseIfUnused();
	}

CBlackBoxProcessor::EResults CBlackBoxProcessor::Process (void)

//	Process
//
//	Process a session. We return resultContinue if we still need to process
//	more. Otherwise we return resultDone.

	{
	int i;

	//	We only handle one session at a time, so pull one that needs to be 
	//	worked on.

	SSession *pSession = GetNextSession();
	if (pSession == NULL)
		return resultDone;

	//	Set the access time so that the files stay open for a while.

	m_LogFiles.SetAccessTime();

	//	We store results in temporary variables so we don't have to 
	//	constantly update the session (which requires a lock).

	TArray<CDatum> Results;
	int iResultsLeft = Max(10, pSession->iMaxLines);
	DWORD dwLinesChecked = 0;

	//	LATER: If we're starting before a certain time, then we need to move
	//	the cursor to the proper spot.

	CBBRLogFiles::SCursor Pos;

	//	Keep looping until we're done.

	CString sError;
	while (m_LogFiles.MoveBackwards(Pos, &sError))
		{
		//	If we've exceeded the time window, then we're done.

		CDateTime LineDate;
		if (pSession->After.IsValid() 
				&& (LineDate = m_LogFiles.GetLineDate(Pos)).IsValid()
				&& LineDate < pSession->After)
			break;

		//	Otherwise, check the line to see if it has our search

		dwLinesChecked++;
		if (pSession->sSearch.IsEmpty() 
				|| m_LogFiles.SearchLine(Pos, pSession->sSearch, pSession->bNoCase))
			{
			Results.Insert(m_LogFiles.GetLine(Pos));

			//	If we've got all the results we want, then we're done.

			if (--iResultsLeft == 0)
				break;
			}

		//	Every 1,000 lines, update the session with our status.

		if ((dwLinesChecked % 1000) == 0)
			SetSessionResult(pSession, dwLinesChecked);
		}

	CDatum dResults;

	//	If we got an error, then that's the result.

	if (!sError.IsEmpty())
		dResults = sError;

	//	If we got no results, then we set results to TRUE to indicate that we
	//	finished the search.

	else if (Results.GetCount() == 0)
		dResults = CDatum(true);

	//	We need to reverse the array to show the results in chronological order.

	else
		{
		dResults = CDatum(CDatum::typeArray);
		dResults.GrowToFit(Results.GetCount());
		for (i = Results.GetCount() - 1; i >= 0; i--)
			dResults.Append(Results[i]);
		}

	//	Update the session. This will release the session.

	SetSessionResult(pSession, dwLinesChecked, dResults);

	//	There may be more work

	return resultContinue;
	}

void CBlackBoxProcessor::SetSessionResult (SSession *pSession, DWORD dwLinesChecked, CDatum dResult)

//	SetSessionResult
//
//	Sets the result.

	{
	CSmartLock Lock(m_cs);

	//	If we don't have a result yet, then just update lines checked.

	if (dResult.IsNil())
		pSession->dwLinesChecked = dwLinesChecked;

	//	Otherwise, update the result and switch the session to done.

	else
		{
		pSession->dwLinesChecked = dwLinesChecked;
		pSession->dResult = dResult;
		pSession->iStatus = statusComplete;
		}

	//	Set the expiration time so we don't get deleted

	pSession->Expires = sysGetTickCount64() + EXPIRE_TIME;
	}
