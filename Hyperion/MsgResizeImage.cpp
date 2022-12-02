//	MsgResizeImage.cpp
//
//	Hyperion.resizeImage
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_CROP_BOTTOM,					"cropBottom");
DECLARE_CONST_STRING(FIELD_CROP_LEFT,					"cropLeft");
DECLARE_CONST_STRING(FIELD_CROP_RIGHT,					"cropRight");
DECLARE_CONST_STRING(FIELD_CROP_TOP,					"cropTop");
DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc");
DECLARE_CONST_STRING(FIELD_MODIFIED_ON,					"modifiedOn");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD_DESC,		"Aeon.fileDownloadDesc");
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply");
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command");

DECLARE_CONST_STRING(ERR_BAD_PARAMS,					"Invalid parameters.");
DECLARE_CONST_STRING(ERR_UNKNOWN_IMAGE_FORMAT,			"%s: Unable to determine image format from extension.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_IMAGE_FORMAT,		"%s: Unsupported image format.");
DECLARE_CONST_STRING(ERR_CANT_LOAD_JPEG,				"%s: Unable to load JPEG: %s");
DECLARE_CONST_STRING(ERR_CANT_RESIZE,					"%s: Unable to resize image.");
DECLARE_CONST_STRING(ERR_CANT_SAVE_JPEG,				"%s: Unable to save JPEG image.");

static constexpr DWORD MAX_RESIZE_SIZE = 16384;

class CResizeImageSession : public CAeonFileDownloadSession
	{
	public:
		CResizeImageSession (CHyperionEngine &Engine, const CString &sReplyAddr, const CString &sFilePath, DWORD dwNewSize, CDatum dOptions, const CHexeSecurityCtx *pSecurityCtx) : 
				CAeonFileDownloadSession(sReplyAddr, sFilePath, pSecurityCtx),
				m_Engine(Engine),
				m_dwNewSize(dwNewSize)
			{
			LoadOptions(dOptions);
			}

	protected:
		//	CAeonFileDownloadSession virtuals
		virtual void OnFileDownloaded (CDatum dFileDesc, CDatum dData) override;
		virtual void OnFileUnmodified (void) override;
		virtual bool OnPrepareRequest (CString &sFilePath, SOptions &Options) override;

	private:
		static CImageLoader::EFormats CalcFormat (const CString& sFilePath, CDatum dFileDesc);
		bool IsCropped (void) const { return (m_rcCrop.left != 0 || m_rcCrop.right != 0 || m_rcCrop.bottom != 0 || m_rcCrop.top != 0); }
		void LoadOptions (CDatum dOptions);
		bool ResizeImage (const CRGBA32Image &Input, DWORD dwNewSize, const RECT &rcCrop, CRGBA32Image &retOutput);

		CHyperionEngine &m_Engine;
		DWORD m_dwNewSize;
		RECT m_rcCrop;
		CString m_sCacheID;
		CDatum m_dResult;

		bool m_bDebugLog = false;
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
	if (dwSize == 0 || dwSize > MAX_RESIZE_SIZE)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_BAD_PARAMS, Msg);
		return;
		}

	CDatum dOptions = Msg.dPayload.GetElement(2);

	//	Otherwise, we need to resize the file. Start a session to get the 
	//	original file bits.

	StartSession(Msg, new CResizeImageSession(*this, GenerateAddress(PORT_HYPERION_COMMAND), sFilePath, dwSize, dOptions, pSecurityCtx));
	}

//	CResizeImageSession --------------------------------------------------------

CImageLoader::EFormats CResizeImageSession::CalcFormat (const CString& sFilePath, CDatum dFileDesc)

//	CalcFormat
//
//	Calculates the image format based on the fileDesc and/or extension.

	{
	CString sFormat = dFileDesc.GetElement(FIELD_TYPE);
	if (sFormat.IsEmpty())
		sFormat = sFilePath;

	return CImageLoader::GetFormatFromString(sFormat);
	}

void CResizeImageSession::LoadOptions (CDatum dOptions)

//	LoadOptions
//
//	Loads options.

	{
	m_rcCrop.left = (int)dOptions.GetElement(FIELD_CROP_LEFT);
	m_rcCrop.right = (int)dOptions.GetElement(FIELD_CROP_RIGHT);
	m_rcCrop.top = (int)dOptions.GetElement(FIELD_CROP_TOP);
	m_rcCrop.bottom = (int)dOptions.GetElement(FIELD_CROP_BOTTOM);
	}

void CResizeImageSession::OnFileDownloaded (CDatum dFileDesc, CDatum dData)

//	OnFileDownloaded
//
//	We have the original file. Now resize it, store it in our cache, and return
//	it to the caller.

	{
	//	If we don't support the format, then we just send it out unchanged.

	CImageLoader::EFormats iFormat = CalcFormat(GetFilePath(), dFileDesc);
	if (iFormat != CImageLoader::formatJPEG)
		{
		CDatum dResult(CDatum::typeStruct);
		dResult.SetElement(FIELD_FILE_DESC, dFileDesc);
		dResult.SetElement(FIELD_DATA, dData);
		SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dResult);
		return;
		}

	//	Get the modified time

	CDateTime ModifiedOn = dFileDesc.GetElement(FIELD_MODIFIED_ON);

	//	Load the image

	CRGBA32Image FullSizeImage;
	CString sError;
	CBuffer Buffer((const CString &)dData);
	if (!CJPEG::Load(Buffer, FullSizeImage, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_LOAD_JPEG, fileGetFilename(GetFilePath()), sError));
		return;
		}

	//	Resize the image

	CRGBA32Image ResizedImage;
	if (!ResizeImage(FullSizeImage, m_dwNewSize, m_rcCrop, ResizedImage))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_RESIZE, fileGetFilename(GetFilePath())));
		return;
		}

	//	Save as JPEG

	CStringBuffer JPEGBuffer;
	if (!CJPEG::Save(ResizedImage, JPEGBuffer, 80, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_CANT_SAVE_JPEG, fileGetFilename(GetFilePath()), sError));
		return;
		}

	//	Now store as a datum

	CDatum::CreateBinaryFromHandoff(JPEGBuffer, &dData);

	//	Compose a fileDownloadDesc for the newly resized image.

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_FILE_DESC, dFileDesc);
	dResult.SetElement(FIELD_DATA, dData);

	//	Store it in our cache

	m_Engine.GetCache().SetEntry(m_sCacheID, dResult, ModifiedOn);
	if (m_bDebugLog)
		m_Engine.GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Added entry to cache: %s (cache size = %d)", m_sCacheID, (DWORD)m_Engine.GetCache().GetTotalSize()));

	//	Reply to original caller

	SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dResult);
	}

void CResizeImageSession::OnFileUnmodified (void)

//	OnFileUnmodified
//
//	File has not changed since cache version.

	{
	//	If the file has not been modified since our cached version, we return 
	//	our cached version back.

	if (m_bDebugLog)
		m_Engine.GetProcessCtx()->Log(MSG_LOG_DEBUG, strPattern("Found entry in cache: %s", m_sCacheID));
	SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, m_dResult);
	}

bool CResizeImageSession::OnPrepareRequest (CString &sFilePath, SOptions &Options)

//	OnPrepareRequest
//
//	We're about to request a download from Aeon. This give us a chance to 
//	modify options, if nessary.

	{
	CHyperionCache &Cache = m_Engine.GetCache();

	//	Generate an ID for the generated file.

	if (IsCropped())
		m_sCacheID = strPattern("%s#size=%d;cropB=%d;cropL=%d;cropR=%d;cropT=%d", sFilePath, m_dwNewSize, m_rcCrop.bottom, m_rcCrop.left, m_rcCrop.right, m_rcCrop.top);
	else
		m_sCacheID = strPattern("%s#size=%d", sFilePath, m_dwNewSize);

	//	If we have it in the cache, then we need to see if the file has been 
	//	changed since we cached it.

	CDateTime ModifiedOn;
	if (Cache.FindEntry(m_sCacheID, &m_dResult, &ModifiedOn))
		{
		Options.IfModifiedAfter = ModifiedOn;
		}

	//	Otherwise we wait for the data to come back.

	return true;
	}

bool CResizeImageSession::ResizeImage (const CRGBA32Image &Input, DWORD dwNewSize, const RECT &rcCrop, CRGBA32Image &retOutput)

//	ResizeImage
//
//	Resize the image.

	{
	try
		{
		int xSrc = rcCrop.left;
		int ySrc = rcCrop.top;
		int cxOriginal = Input.GetWidth();
		int cyOriginal = Input.GetHeight();
		int cxSrc = cxOriginal - (rcCrop.left + rcCrop.right);
		int cySrc = cyOriginal - (rcCrop.top + rcCrop.bottom);

		if (cxSrc <= 0 || cySrc <= 0)
			return false;

		int cxMaxWidth = dwNewSize;
		int cyMaxHeight = dwNewSize;

		double rScaleX = (double)cxMaxWidth / (double)cxSrc;
		double rScaleY = (double)cyMaxHeight / (double)cySrc;
		double rScale = Min(rScaleX, rScaleY);

		int cxNewWidth = Min(cxMaxWidth, (int)mathRound(rScale * cxSrc));
		int cyNewHeight = Min(cyMaxHeight, (int)mathRound(rScale * cySrc));

		retOutput.Create(cxNewWidth, cyNewHeight, CRGBA32Image::alphaNone);
		CImageDraw::BltScaled(retOutput, 0, 0, cxNewWidth, cyNewHeight, Input, xSrc, ySrc, cxSrc, cySrc);

		return true;
		}
	catch (...)
		{
		return false;
		}
	}
