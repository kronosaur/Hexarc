//	CAeonFileDownloadSession.cpp
//
//	CAeonFileDownloadSession class
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDRESS_AEON_COMMAND,				"Aeon.command")

DECLARE_CONST_STRING(FIELD_DATA,						"data")
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc")
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_SIZE,						"size")
DECLARE_CONST_STRING(FIELD_UNMODIFIED,					"unmodified")

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD_DESC,		"Aeon.fileDownloadDesc")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")

DECLARE_CONST_STRING(ERR_BAD_PARAMS,					"Invalid parameters for CAeonInterface::ParseFilePath.")
DECLARE_CONST_STRING(ERR_UNEXPECTED_RESPONSE,			"Unexpected response from AeonDB: %s.")

const DWORD MESSAGE_TIMEOUT =							30 * 1000;

CAeonFileDownloadSession::CAeonFileDownloadSession (const CString &sReplyAddr, const CString &sFilePath, const CHexeSecurityCtx *pSecurityCtx, const SOptions &Options) :
		m_sReplyAddr(sReplyAddr),
		m_sFilePath(sFilePath),
		m_SecurityCtx(pSecurityCtx ? *pSecurityCtx : CHexeSecurityCtx()),
		m_Options(Options)

//	CAeonFileDownloadSession constructor

	{
	}

bool CAeonFileDownloadSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a response

	{
	//	If we got an error back, then pass it back to the caller.

	if (IsError(Msg))
		{
		SendMessageReplyError(Msg.sMsg, Msg.dPayload.GetElement(0));
		return false;
		}

	//	If this is not the proper reply, then error.

	else if (!strEquals(Msg.sMsg, MSG_AEON_FILE_DOWNLOAD_DESC))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNEXPECTED_RESPONSE, Msg.sMsg));
		return false;
		}

	//	If the file has not been modified then we tell our subclass and we're
	//	done.

	else if (!Msg.dPayload.GetElement(FIELD_UNMODIFIED).IsNil())
		{
		OnFileUnmodified();
		return false;
		}

	//	Otherwise, handle it.

	else
		{
		//	Parse result

		CDatum dFileDesc = Msg.dPayload.GetElement(FIELD_FILE_DESC);
		CDatum dData = Msg.dPayload.GetElement(FIELD_DATA);

		//	If we have previous data then combine it

		if (!m_dFileData.IsNil())
			m_dFileData.Append(dData);
		else
			m_dFileData = dData;

		DWORD dwTotalRead = ((const CString &)m_dFileData).GetLength();

		//	Figure out how big the entire file is

		DWORD dwTotalSize = (int)dFileDesc.GetElement(FIELD_SIZE);

		//	If we don't have the entire file yet, we save what we have and send
		//	a command to get more.

		if (dwTotalRead < dwTotalSize)
			return SendFileDownloadRequest(((const CString &)m_dFileData).GetLength());

		//	Success!

		OnFileDownloaded(dFileDesc, m_dFileData);
		return false;
		}
	}

bool CAeonFileDownloadSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Initiate

	{
	if (!OnPrepareRequest(m_sFilePath, m_Options))
		return false;

	return SendFileDownloadRequest(0);
	}

bool CAeonFileDownloadSession::SendFileDownloadRequest (int iOffset)

//	SendFileDownloadRequest
//
//	Asks to download the file starting at the given offset. We return FALSE if
//	we get an error (and we reply to the session).

	{
	//	Compose the appropriate message based on parameters

	CString sAddr;
	CString sMsg;
	CDatum dPayload;
	if (!CAeonInterface::ParseFilePath(m_sFilePath, m_Options.sRoot, iOffset, m_Options.IfModifiedAfter, &sAddr, &sMsg, &dPayload))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_BAD_PARAMS);
		return false;
		}

	//	If necessary, encode into a sandbox message

	if (!m_SecurityCtx.IsEmpty())
		CHexeProcess::ComposeHexarcMessage(m_SecurityCtx, sMsg, dPayload, &sMsg, &dPayload);

	//	Send the message out

	ISessionHandler::SendMessageCommand(ADDRESS_AEON_COMMAND,
			sMsg,
			m_sReplyAddr,
			dPayload,
			MESSAGE_TIMEOUT);

	//	Expect reply

	return true;
	}