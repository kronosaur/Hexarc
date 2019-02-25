//	MsgResizeImage.cpp
//
//	Hyperion.resizeImage
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc");

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD_DESC,		"Aeon.fileDownloadDesc");
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command");

DECLARE_CONST_STRING(ERR_BAD_PARAMS,					"Invalid parameters.");
DECLARE_CONST_STRING(ERR_UNKNOWN_IMAGE_FORMAT,			"Unable to determine image format from extension: %s.");

class CResizeImageSession : public CAeonFileDownloadSession
	{
	public:
		CResizeImageSession (CHyperionEngine &Engine, const CString &sReplyAddr, const CString &sFilePath, DWORD dwNewSize, const CString &sCacheID, const CHexeSecurityCtx *pSecurityCtx) : 
				CAeonFileDownloadSession(sReplyAddr, sFilePath, pSecurityCtx),
				m_Engine(Engine),
				m_dwNewSize(dwNewSize),
				m_sCacheID(sCacheID)
			{ }

	protected:
		//	CAeonFileDownloadSession virtuals
		virtual void OnFileDownloaded (CDatum dFileDesc, CDatum dData) override;
		virtual void OnFileUnmodified (void) override;

	private:
		CHyperionEngine &m_Engine;
		DWORD m_dwNewSize;
		CString m_sCacheID;
	};

void CHyperionEngine::MsgResizeImage (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgResizeImage
//
//	Hyperion.resizeImage filePath newSize [options]

	{
	//	Get parameters

	CString sFilePath = Msg.dPayload.GetElement(0);
	if (sFilePath.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_BAD_PARAMS, Msg);
		return;
		}
		
	DWORD dwSize = (DWORD)Msg.dPayload.GetElement(1);
	if (dwSize == 0)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_BAD_PARAMS, Msg);
		return;
		}

	//	Generate an ID for the generated file.

	CString sID = strPattern("%s#size=%d", sFilePath, dwSize);

	//	If we have it in the cache, then we're done.

	CDatum dData;
	if (m_Cache.FindEntry(sID, &dData))
		{
		Log(MSG_LOG_DEBUG, strPattern("Found entry in cache: %s", sID));
		SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dData, Msg);
		return;
		}

	//	Otherwise, we need to resize the file. Start a session to get the 
	//	original file bits.

	StartSession(Msg, new CResizeImageSession(*this, GenerateAddress(PORT_HYPERION_COMMAND), sFilePath, dwSize, sID, pSecurityCtx));
	}

//	CResizeImageSession --------------------------------------------------------

void CResizeImageSession::OnFileDownloaded (CDatum dFileDesc, CDatum dData)

//	OnFileDownloaded
//
//	We have the original file. Now resize it, store it in our cache, and return
//	it to the caller.

	{
	//	Resize the image.

	CImageLoader::EFormats iFormat = CImageLoader::GetFormatFromExtension(GetFilePath());
	if (iFormat == CImageLoader::formatUnknown)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_IMAGE_FORMAT, fileGetFilename(GetFilePath())));
		return;
		}

	//	Compose a fileDownloadDesc for the newly resized image.

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_FILE_DESC, dFileDesc);
	dResult.SetElement(FIELD_DATA, dData);

	//	Store it in our cache

	m_Engine.GetCache().SetEntry(m_sCacheID, dResult);
	m_Engine.GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Added entry to cache: %s (cache size = %d)", m_sCacheID, (DWORD)m_Engine.GetCache().GetTotalSize()));

	//	Reply to original caller

	SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dResult);
	}

void CResizeImageSession::OnFileUnmodified (void)

//	OnFileUnmodified
//
//	File has not changed since cache version.

	{
	ASSERT(false);
	}
