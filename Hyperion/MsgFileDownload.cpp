//	MsgFileDownload.cpp
//
//	Hyperion.fileDownload
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_DATA,						"data")
DECLARE_CONST_STRING(FIELD_FILE_DESC,					"fileDesc")
DECLARE_CONST_STRING(FIELD_FILE_PATH,					"filePath")
DECLARE_CONST_STRING(FIELD_HEIGHT,						"height")
DECLARE_CONST_STRING(FIELD_TRANSFORMS,					"transforms")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")
DECLARE_CONST_STRING(FIELD_WIDTH,						"width")

DECLARE_CONST_STRING(MSG_AEON_FILE_DOWNLOAD_DESC,		"Aeon.fileDownloadDesc")
DECLARE_CONST_STRING(MSG_ERROR_NOT_ALLOWED,				"Error.notAllowed")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")

DECLARE_CONST_STRING(PORT_HYPERION_COMMAND,				"Hyperion.command")

DECLARE_CONST_STRING(TYPE_RESIZE,						"resize")

DECLARE_CONST_STRING(ERR_BAD_PARAMS,					"Invalid parameters.")
DECLARE_CONST_STRING(ERR_INVALID_TRANS_TYPE,			"Invalid transform description")
DECLARE_CONST_STRING(ERR_NOT_IN_SANDBOX,				"Table %s cannot be accessed by service: %s.")
DECLARE_CONST_STRING(ERR_PARSING_FILE_PATH,				"Unable to parse filePath: %s")
DECLARE_CONST_STRING(ERR_INVALID_TYPE,					"Unknown transform: %s")
DECLARE_CONST_STRING(ERR_INVALID_SIZE,					"Invalid image size.")
DECLARE_CONST_STRING(ERR_INVALID_IMAGE,					"Invalid image.")
DECLARE_CONST_STRING(ERR_UNKNOWN_IMAGE_FORMAT,			"Unknown image format.")

class CFileDownloadSession : public CAeonFileDownloadSession
	{
	public:
		enum ETransformTypes
			{
			transNone,

			transResize,					//	Resize an image
			};

		struct STransformDesc
			{
			ETransformTypes iType;
			CDatum dParams;
			};

		CFileDownloadSession (CHyperionEngine &Engine, const CString &sFilePath, CDatum dFileDownloadDesc) : 
				CAeonFileDownloadSession(GenerateAddress(PORT_HYPERION_COMMAND), sFilePath),
				m_Engine(Engine),
				m_dFileDownloadDesc(dFileDownloadDesc)
			{ }

	protected:
		//	CAeonFileDownloadSession virtuals
		virtual void OnFileDownloaded (CDatum dFileDesc, CDatum dData) override;
		virtual void OnFileUnmodified (void) override;
		virtual void OnMark (void) override;

	private:
		bool ParseSingleTransform (CDatum dTrans, TArray<STransformDesc> &Trans, CString *retsError = NULL) const;
		bool ParseTransforms (CDatum dDesc, TArray<STransformDesc> &Trans, CString *retsError = NULL) const;
		bool ProcessResize (CDatum dFileDesc, CDatum dData, int cxWidth, int cyHeight, CDatum *retdFileDesc, CDatum *retdData, CString *retsError = NULL) const;
		bool ProcessTransforms (CDatum dFileDesc, CDatum dData, const TArray<STransformDesc> &Trans, CDatum *retdFileDesc, CDatum *retdData, CString *retsError = NULL) const;

		CHyperionEngine m_Engine;
		CDatum m_dFileDownloadDesc;
	};

void CHyperionEngine::MsgFileDownload (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgFileDownload
//
//	Hyperion.fileDownload filePath [{fileDownloadDesc}]
//
//	fileDownloadDesc can have a transforms entry, which is a list of transforms.
//	Each transform is a struct with fields based on the type of transform:
//
//	type: 'resize
//	width: New maximum width in pixels
//	height: New maximum height in pixels

	{
	CString sError;

	//	Get parameters

	CString sFilePath = Msg.dPayload.GetElement(0);
	CDatum dFileDownloadDesc = Msg.dPayload.GetElement(1);
	if (sFilePath.IsEmpty())
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, ERR_BAD_PARAMS, Msg);
		return;
		}

	//	Get the filePath

	CString sTable;
	CString sFilespec;
	if (!CAeonInterface::ParseTableFilePath(Msg.dPayload.GetElement(0), &sTable, &sFilespec, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_PARSING_FILE_PATH, sError), Msg);
		return;
		}

	//	Make sure we have access

	if (pSecurityCtx && !pSecurityCtx->IsNamespaceAccessible(sTable))
		{
		SendMessageReplyError(MSG_ERROR_NOT_ALLOWED, strPattern(ERR_NOT_IN_SANDBOX, sTable, pSecurityCtx->GetSandboxName()), Msg);
		return;
		}

	//	Start a session to get the file from Aeon

	StartSession(Msg, new CFileDownloadSession(*this, sFilePath, dFileDownloadDesc));
	}

//	CResizeImageSession --------------------------------------------------------

void CFileDownloadSession::OnFileDownloaded (CDatum dFileDesc, CDatum dData)

//	OnFileDownloaded
//
//	We have the original file. Now resize it, store it in our cache, and return
//	it to the caller.

	{
	CString sError;

	//	Parse the transforms

	TArray<STransformDesc> Transforms;
	if (!ParseTransforms(m_dFileDownloadDesc, Transforms, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
		return;
		}

	//	Process the image 

	if (!ProcessTransforms(dFileDesc, dData, Transforms, &dFileDesc, &dData, &sError))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError);
		return;
		}

	//	Compose a fileDownloadDesc for the newly resized image.

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_FILE_DESC, dFileDesc);
	dResult.SetElement(FIELD_DATA, dData);

	//	Reply to original caller

	SendMessageReply(MSG_AEON_FILE_DOWNLOAD_DESC, dResult);
	}

void CFileDownloadSession::OnFileUnmodified (void)

//	OnFileUnmodified
//
//	File has not changed since cache version.

	{
	ASSERT(false);
	}

void CFileDownloadSession::OnMark (void)

//	OnMark
//
//	Mark data in use

	{
	m_dFileDownloadDesc.Mark();
	CAeonFileDownloadSession::Mark();
	}

bool CFileDownloadSession::ParseSingleTransform (CDatum dTrans, TArray<STransformDesc> &Trans, CString *retsError) const

//	ParseSingleTransform
//
//	Parses a single transform

	{
	STransformDesc NewTrans;

	//	Parse the type

	const CString &sType = dTrans.GetElement(FIELD_TYPE);
	if (strEquals(sType, TYPE_RESIZE))
		NewTrans.iType = transResize;
	else
		{
		if (retsError) *retsError = strPattern(ERR_INVALID_TYPE, sType);
		return false;
		}

	//	Params are in dTrans

	NewTrans.dParams = dTrans;

	//	Store

	Trans.Insert(NewTrans);
	return true;
	}

bool CFileDownloadSession::ParseTransforms (CDatum dDesc, TArray<STransformDesc> &Trans, CString *retsError) const

//	ParseTransforms
//
//	Parse the list of transforms.

	{
	int i;

	Trans.DeleteAll();

	CDatum dTrans = dDesc.GetElement(FIELD_TRANSFORMS);
	if (dTrans.IsNil())
		return true;

	//	If this is a struct, then it is a single transform.

	if (dTrans.GetBasicType() == CDatum::typeStruct)
		{
		if (!ParseSingleTransform(dTrans, Trans, retsError))
			return false;
		}

	//	Otherwise, if this is a list, then we each is a transform

	else if (dTrans.GetBasicType() == CDatum::typeArray)
		{
		for (i = 0; i < dTrans.GetCount(); i++)
			{
			if (!ParseSingleTransform(dTrans.GetElement(i), Trans, retsError))
				return false;
			}
		}

	//	Otherwise, invalid

	else
		{
		if (retsError) *retsError = ERR_INVALID_TRANS_TYPE;
		return false;
		}

	//	Success!

	return true;
	}

bool CFileDownloadSession::ProcessResize (CDatum dFileDesc, CDatum dData, int cxWidth, int cyHeight, CDatum *retdFileDesc, CDatum *retdData, CString *retsError) const

//	ProcessResize
//
//	Resize an image file.

	{
	const CString &sFilePath = dFileDesc.GetElement(FIELD_FILE_PATH);

	//	Check inputs

	if (cxWidth <= 0 || cyHeight <= 0)
		{
		if (retsError) *retsError = ERR_INVALID_SIZE;
		return false;
		}

	//	Figure out the file format

	CImageLoader::EFormats iFormat = CImageLoader::GetFormatFromExtension(sFilePath);
	if (iFormat == CImageLoader::formatUnknown)
		{
		if (retsError) *retsError = ERR_UNKNOWN_IMAGE_FORMAT;
		return false;
		}

	//	Load the image

	CRGBA32Image Image;
	CBuffer ImageData((const CString &)dData);
	if (!CImageLoader::Load(ImageData, iFormat, Image, retsError))
		return false;

	if (Image.GetWidth() == 0 || Image.GetHeight() == 0)
		{
		if (retsError) *retsError = ERR_INVALID_IMAGE;
		return false;
		}

	//	Resize the image

	int cxNewWidth;
	int cyNewHeight;
	Metric rScaleX = (Metric)(cxWidth / Image.GetWidth());
	Metric rScaleY = (Metric)(cyHeight / Image.GetHeight());
	if (rScaleX < rScaleY)
		{
		cxNewWidth = cxWidth;
		cyNewHeight = (int)mathRound(Image.GetHeight() * rScaleX);
		}
	else
		{
		cyNewHeight = cyHeight;
		cxNewWidth = (int)mathRound(Image.GetWidth() * rScaleY);
		}

	CRGBA32Image ScaledImage;
	ScaledImage.Create(cxNewWidth, cyNewHeight, Image.GetAlphaType());
	CImageDraw::BltScaled(ScaledImage, 0, 0, cxNewWidth, cyNewHeight, Image);

	//	Save back to the proper format


	//	Done

	return true;
	}

bool CFileDownloadSession::ProcessTransforms (CDatum dFileDesc, CDatum dData, const TArray<STransformDesc> &Trans, CDatum *retdFileDesc, CDatum *retdData, CString *retsError) const

//	ProcessTransforms
//
//	Transform

	{
	int i;

	for (i = 0; i < Trans.GetCount(); i++)
		{
		switch (Trans[i].iType)
			{
			case transResize:
				{
				int cxWidth = Trans[i].dParams.GetElement(FIELD_WIDTH);
				int cyHeight = Trans[i].dParams.GetElement(FIELD_HEIGHT);

				if (!ProcessResize(dFileDesc, dData, cxWidth, cyHeight, &dFileDesc, &dData, retsError))
					return false;

				break;
				}
			}
		}

	//	Done

	*retdFileDesc = dFileDesc;
	*retdData = dData;
	return true;
	}
