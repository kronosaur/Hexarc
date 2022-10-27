//	IMediaType.cpp
//
//	IMediaType class
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ENCODING_GZIP,						"gzip")
DECLARE_CONST_STRING(ENCODING_IDENTITY,					"identity")

DECLARE_CONST_STRING(EXTENSION_BIN,						".bin")
DECLARE_CONST_STRING(EXTENSION_CSS,						".css")
DECLARE_CONST_STRING(EXTENSION_CSV,						".csv")
DECLARE_CONST_STRING(EXTENSION_DLL,						".dll")
DECLARE_CONST_STRING(EXTENSION_DOC,						".doc")
DECLARE_CONST_STRING(EXTENSION_DOCX,					".docx")
DECLARE_CONST_STRING(EXTENSION_EXE,						".exe")
DECLARE_CONST_STRING(EXTENSION_HTM,						".htm")
DECLARE_CONST_STRING(EXTENSION_HTML,					".html")
DECLARE_CONST_STRING(EXTENSION_JPE,						".jpe")
DECLARE_CONST_STRING(EXTENSION_JPEG,					".jpeg")
DECLARE_CONST_STRING(EXTENSION_JPG,						".jpg")
DECLARE_CONST_STRING(EXTENSION_JS,						".js")
DECLARE_CONST_STRING(EXTENSION_LHA,						".lha")
DECLARE_CONST_STRING(EXTENSION_LZH,						".lzh")
DECLARE_CONST_STRING(EXTENSION_OTF,						".otf")
DECLARE_CONST_STRING(EXTENSION_PNG,						".png")
DECLARE_CONST_STRING(EXTENSION_ZIP,						".zip")

DECLARE_CONST_STRING(MEDIA_TYPE_BINARY,					"application/octet-stream")
DECLARE_CONST_STRING(MEDIA_TYPE_CSS,					"text/css")
DECLARE_CONST_STRING(MEDIA_TYPE_CSV,					"text/csv")
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html")
DECLARE_CONST_STRING(MEDIA_TYPE_JAVASCRIPT,				"application/javascript")
DECLARE_CONST_STRING(MEDIA_TYPE_JPG,					"image/jpeg")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json")
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest")
DECLARE_CONST_STRING(MEDIA_TYPE_MSWORD,					"application/msword")
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data")
DECLARE_CONST_STRING(MEDIA_TYPE_OTF,					"application/x-font-otf")
DECLARE_CONST_STRING(MEDIA_TYPE_PNG,					"image/png")
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain")
DECLARE_CONST_STRING(MEDIA_TYPE_ZIP,					"application/zip")

struct SMediaTypeMapEntry
	{
	const CString &sExtension;
	const CString &sMediaType;
	};

SMediaTypeMapEntry g_MediaTypeMap[] =
	{
		{	EXTENSION_BIN,			MEDIA_TYPE_BINARY,		},
		{	EXTENSION_CSS,			MEDIA_TYPE_CSS,			},
		{	EXTENSION_CSV,			MEDIA_TYPE_CSV,			},
		{	EXTENSION_DLL,			MEDIA_TYPE_BINARY,		},
		{	EXTENSION_DOC,			MEDIA_TYPE_MSWORD,		},
		{	EXTENSION_DOCX,			MEDIA_TYPE_MSWORD,		},
		{	EXTENSION_EXE,			MEDIA_TYPE_BINARY,		},
		{	EXTENSION_HTM,			MEDIA_TYPE_HTML,		},
		{	EXTENSION_HTML,			MEDIA_TYPE_HTML,		},
		{	EXTENSION_JPE,			MEDIA_TYPE_JPG,			},
		{	EXTENSION_JPEG,			MEDIA_TYPE_JPG,			},
		{	EXTENSION_JPG,			MEDIA_TYPE_JPG,			},
		{	EXTENSION_JS,			MEDIA_TYPE_JAVASCRIPT,	},
		{	EXTENSION_LHA,			MEDIA_TYPE_BINARY,		},
		{	EXTENSION_LZH,			MEDIA_TYPE_BINARY,		},
		{	EXTENSION_OTF,			MEDIA_TYPE_OTF,			},
		{	EXTENSION_PNG,			MEDIA_TYPE_PNG,			},
		{	EXTENSION_ZIP,			MEDIA_TYPE_ZIP,			},
	};

int g_MediaTypeMapCount = SIZEOF_STATIC_ARRAY(g_MediaTypeMap);

struct SMediaTypeData
	{
	const CString &sMediaType;
	EContentEncodingTypes iDefaultEncoding;
	};

SMediaTypeData g_MediaTypeData[] =
	{
		{	MEDIA_TYPE_BINARY,			http_encodingIdentity },
		{	MEDIA_TYPE_CSS,				http_encodingGzip },
		{	MEDIA_TYPE_HTML,			http_encodingGzip },
		{	MEDIA_TYPE_JAVASCRIPT,		http_encodingGzip },
		{	MEDIA_TYPE_JPG,				http_encodingIdentity },
		{	MEDIA_TYPE_JSON,			http_encodingGzip },
		{	MEDIA_TYPE_JSON_REQUEST,	http_encodingGzip },
		{	MEDIA_TYPE_MSWORD,			http_encodingGzip },
		{	MEDIA_TYPE_OTF,				http_encodingIdentity },
		{	MEDIA_TYPE_PNG,				http_encodingIdentity },
		{	MEDIA_TYPE_TEXT,			http_encodingGzip },
		{	MEDIA_TYPE_ZIP,				http_encodingIdentity },
	};

int g_MediaTypeDataCount = SIZEOF_STATIC_ARRAY(g_MediaTypeData);

EContentEncodingTypes IMediaType::GetDefaultEncodingType (const CString &sMediaType)

//	GetDefaultEncodingType
//
//	Returns the default encoding type for the given media type

	{
	int i;
	CString sSearch = strToLower(sMediaType);

	for (i = 0; i < g_MediaTypeDataCount; i++)
		if (strEquals(sSearch, g_MediaTypeData[i].sMediaType))
			return g_MediaTypeData[i].iDefaultEncoding;

	return http_encodingIdentity;
	}

const CString &IMediaType::GetMediaEncodingHeader (void) const

//	GetMediaEncodingHeader
//
//	Returns the encoding header

	{
	switch (GetMediaEncoding())
		{
		case http_encodingIdentity:
			return ENCODING_IDENTITY;

		case http_encodingGzip:
			return ENCODING_GZIP;

		default:
			ASSERT(false);
			return NULL_STR;
		}
	}

CString IMediaType::MediaTypeFromExtension (const CString &sExtension)

//	MediaTypeFromExtension
//
//	Returns the media type for the given file extension. If not found, returns
//	NULL_STR.
//
//	The caller must pass in the extension in lowercase.

	{
	int i;

	for (i = 0; i < g_MediaTypeMapCount; i++)
		if (strEquals(sExtension, g_MediaTypeMap[i].sExtension))
			return g_MediaTypeMap[i].sMediaType;

	return NULL_STR;
	}

//	CRawMediaType --------------------------------------------------------------

bool CRawMediaType::DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer)

//	DecodeFromBuffer

	{
	m_sMediaType = sMediaType;
	m_sBody = CString(Buffer.GetPointer(), Buffer.GetLength());
	return true;
	}

void CRawMediaType::EncodeContent (EContentEncodingTypes iEncoding)

//	EncodeContent
//
//	Encode content

	{
	//	If we're already encoded, we're done

	if (iEncoding == m_iEncoding)
		return;

	//	We don't support re-encoding to some other format (for now)

	if (m_iEncoding != http_encodingIdentity)
		{
		ASSERT(false);
		return;
		}

	//	Encode the content

	CStringBuffer Encoded;
	switch (iEncoding)
		{
		case http_encodingGzip:
			{
			CBuffer Buffer(m_sBody);
			compCompress(Buffer, compressionGzip, &Encoded);
			break;
			}

		default:
			ASSERT(false);
			return;
		}

	//	Done

	m_iEncoding = iEncoding;
	m_sBody = CString::CreateFromHandoff(Encoded);
	}

bool CRawMediaType::EncodeToBuffer (IByteStream &Stream, DWORD dwOffset, DWORD dwSize) const

//	EncodeToBuffer

	{
	if (dwSize == 0xffffffff)
		Stream.Write(m_sBody);
	else
		{
		DWORD dwBodyLen = m_sBody.GetLength();
		if (dwOffset < dwBodyLen)
			{
			DWORD dwActualSize = Min(dwSize, dwBodyLen - dwOffset);
			Stream.Write(m_sBody.GetParsePointer() + dwOffset, dwSize);
			}
		}

	return true;
	}

DWORD CRawMediaType::GetMediaLength (void) const

//	GetMediaLength

	{
	return m_sBody.GetLength();
	}

const CString &CRawMediaType::GetMediaType (void) const

//	GetMediaType

	{
	return m_sMediaType;
	}
