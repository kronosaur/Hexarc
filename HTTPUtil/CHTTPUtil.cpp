//	CHTTPUtil.cpp
//
//	CHTTPUtil Class
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(DATUM_TYPENAME_XML_ELEMENT,		"xmlElement");

DECLARE_CONST_STRING(FIELD_ADDITIONAL_PROPERTIES,		"additionalProperties");
DECLARE_CONST_STRING(FIELD_ANY_OF,						"anyOf");
DECLARE_CONST_STRING(FIELD_DATA,						"data");
DECLARE_CONST_STRING(FIELD_ENUM,						"enum");
DECLARE_CONST_STRING(FIELD_FORMAT,						"format");
DECLARE_CONST_STRING(FIELD_HEADERS,						"headers");
DECLARE_CONST_STRING(FIELD_ITEMS,						"items");
DECLARE_CONST_STRING(FIELD_PROPERTIES,					"properties");
DECLARE_CONST_STRING(FIELD_STATUS,						"status");
DECLARE_CONST_STRING(FIELD_STATUS_CODE,					"statusCode");
DECLARE_CONST_STRING(FIELD_TYPE,						"type");

DECLARE_CONST_STRING(HEADER_CONTENT_TYPE,				"content-type");
DECLARE_CONST_STRING(HEADER_HOST,						"host");

DECLARE_CONST_STRING(MEDIA_TYPE_FORM_URL_ENCODED,		"application/x-www-form-urlencoded");
DECLARE_CONST_STRING(MEDIA_TYPE_JSON,					"application/json");
DECLARE_CONST_STRING(MEDIA_TYPE_JSON_REQUEST,			"application/jsonrequest");
DECLARE_CONST_STRING(MEDIA_TYPE_MULTIPART_FORM,			"multipart/form-data");
DECLARE_CONST_STRING(MEDIA_TYPE_HTML,					"text/html");
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT,					"text/plain");
DECLARE_CONST_STRING(MEDIA_TYPE_TEXT_PREFIX,			"text/");

DECLARE_CONST_STRING(METHOD_GET,						"GET");

DECLARE_CONST_STRING(PROTOCOL_HTTPS,					"https");

DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_BOOLEAN,			"boolean");
DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_ARRAY,			"array");
DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_INTEGER,			"integer");
DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_NULL,				"null");
DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_NUMBER,			"number");
DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_OBJECT,			"object");
DECLARE_CONST_STRING(JSON_SCHEMA_TYPE_STRING,			"string");

DECLARE_CONST_STRING(JSON_SCHEMA_FORMAT_DATE_TIME,		"date-time");

DECLARE_CONST_STRING(TYPENAME_URL,						"URLType");
DECLARE_CONST_STRING(TYPENAME_XML_ELEMENT,				"XMLElementType");

DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_MULTIPART,		"Error parsing MIME multipart/form-data.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_JSON,			"Error parsing JSON.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED,	"Error parsing form URL encoded.");
DECLARE_CONST_STRING(ERR_UNSUPPORTED_MEDIA_TYPE,		"Unsupported media type: %s.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_RECEIVE,				"Unable to receive data from %s; connection lost.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_SEND,				"Unable to send data to %s; connection lost.");
DECLARE_CONST_STRING(ERR_UNABLE_TO_CONNECT,				"Unable to connect to server at %s.");
DECLARE_CONST_STRING(ERR_INVALID_PORT,					"Unable to determine port from URL: %s.");
DECLARE_CONST_STRING(ERR_INVALID_URL,					"Invalid URL: %s.");

static CDatum JSONSchemaFromType (CDatum dType, TArray<const IDatatype*> TypeStack);

bool CHTTPUtil::m_bAEONRegistered = false;
DWORD CHTTPUtil::URL_TYPE = 0;
DWORD CHTTPUtil::XML_ELEMENT_TYPE = 0;

CDatum CJSONSchema::FromType (CDatum dType)

//	FromType
//
//	Converts an AEON/GridLang datatype to a JSON Schema.

	{
	TArray<const IDatatype*> TypeStack;
	return JSONSchemaFromType(dType, TypeStack);
	}

static CDatum JSONSchemaFromType (CDatum dType, TArray<const IDatatype*> TypeStack)

//	JSONSchemaFromType
//
//	Converts a datatype to a JSON Schema, returning an empty schema on recursion.

	{
	CDatum dResult(CDatum::typeStruct);

	if (dType.GetBasicType() != CDatum::typeDatatype)
		return dResult;

	const IDatatype& Type = dType;
	for (int i = 0; i < TypeStack.GetCount(); i++)
		if (TypeStack[i] == &Type)
			return dResult;

	TypeStack.Insert(&Type);

	if (Type.GetClass() == IDatatype::ECategory::Enum)
		{
		CDatum dEnum(CDatum::typeArray);
		for (int i = 0; i < Type.GetMemberCount(); i++)
			{
			IDatatype::SMemberDesc Member = Type.GetMember(i);
			if (Member.iType == IDatatype::EMemberType::EnumValue)
				dEnum.Append(Member.sID);
			}

		dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_STRING);
		dResult.SetElement(FIELD_ENUM, dEnum);
		return dResult;
		}
	else if (Type.GetClass() == IDatatype::ECategory::Nullable)
		{
		CDatum dAnyOf(CDatum::typeArray);
		dAnyOf.Append(JSONSchemaFromType(Type.GetVariantType(), TypeStack));

		CDatum dNullSchema(CDatum::typeStruct);
		dNullSchema.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_NULL);
		dAnyOf.Append(dNullSchema);

		dResult.SetElement(FIELD_ANY_OF, dAnyOf);
		return dResult;
		}
	else if (Type.GetClass() == IDatatype::ECategory::Array
			|| Type.GetClass() == IDatatype::ECategory::Table
			|| Type.GetClass() == IDatatype::ECategory::Tensor)
		{
		dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_ARRAY);
		dResult.SetElement(FIELD_ITEMS, JSONSchemaFromType(Type.GetElementType(), TypeStack));
		return dResult;
		}
	else if (Type.GetClass() == IDatatype::ECategory::Dictionary)
		{
		dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_OBJECT);
		dResult.SetElement(FIELD_ADDITIONAL_PROPERTIES, JSONSchemaFromType(Type.GetElementType(), TypeStack));
		return dResult;
		}
	else if (Type.GetClass() == IDatatype::ECategory::Schema)
		{
		CDatum dProperties(CDatum::typeStruct);
		for (int i = 0; i < Type.GetMemberCount(); i++)
			{
			IDatatype::SMemberDesc Member = Type.GetMember(i);
			switch (Member.iType)
				{
				case IDatatype::EMemberType::InstanceKeyVar:
				case IDatatype::EMemberType::InstanceVar:
					dProperties.SetElement(Member.sID, JSONSchemaFromType(Member.dType, TypeStack));
					break;
				}
			}

		dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_OBJECT);
		dResult.SetElement(FIELD_PROPERTIES, dProperties);
		return dResult;
		}

	switch (Type.GetCoreType())
		{
		case IDatatype::NULL_T:
			dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_NULL);
			break;

		case IDatatype::BOOL:
			dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_BOOLEAN);
			break;

		case IDatatype::STRING:
			dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_STRING);
			break;

		case IDatatype::DATE_TIME:
			dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_STRING);
			dResult.SetElement(FIELD_FORMAT, JSON_SCHEMA_FORMAT_DATE_TIME);
			break;

		case IDatatype::STRUCT:
			dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_OBJECT);
			break;

		default:
			if (Type.IsA(IDatatype::INTEGER))
				dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_INTEGER);
			else if (Type.IsA(IDatatype::NUMBER))
				dResult.SetElement(FIELD_TYPE, JSON_SCHEMA_TYPE_NUMBER);
			break;
		}

	return dResult;
	}

bool CHTTPUtil::Boot ()

//	Boot
//
//	Register AEON types.

	{
	if (!m_bAEONRegistered)
		{
		CAEONURL::RegisterFactory();
		CAEONXMLElement::RegisterFactory();

		URL_TYPE = CAEONTypes::RegisterSimple(TYPENAME_URL, CDatatypeList(), false);
		XML_ELEMENT_TYPE = CAEONTypes::RegisterAEON(TYPENAME_XML_ELEMENT, CDatatypeList(), DATUM_TYPENAME_XML_ELEMENT, CAEONXMLElement::GetMembers());

		m_bAEONRegistered = true;
		}

	return true;
	}

bool CHTTPUtil::ConvertBodyToDatum (const CHTTPMessage &Message, CDatum &retdBody)

//	ConvertBodyToDatum
//
//	Converts the body of a message to a datum, based on the content-type.

	{
	//	Edge condition. This is valid (e.g.) if we have a pure GET request.

	IMediaTypePtr pBody = Message.GetBody();
	if (!pBody)
		{
		retdBody = CDatum();
		return true;
		}

	//	Get the media and media type

	CString sMediaType;
	if (!CHTTPMessage::ParseHeaderValue(pBody->GetMediaType(), &sMediaType, NULL))
		{
		retdBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, pBody->GetMediaType());
		return false;
		}

	const CString &sBuffer = pBody->GetMediaBuffer();

	//	Parse based on media type

	if (strEquals(sMediaType, MEDIA_TYPE_FORM_URL_ENCODED))
		{
		if (!ConvertFormURLEncodedToDatum(sBuffer, retdBody))
			{
			retdBody = ERR_UNABLE_TO_PARSE_FORM_URL_ENCODED;
			return false;
			}
		}
	else if (strEquals(sMediaType, MEDIA_TYPE_JSON))
		{
		CStringBuffer Buffer(sBuffer);
		if (!CDatum::Deserialize(CDatum::EFormat::AEONJSON, Buffer, &retdBody))
			{
			retdBody = ERR_UNABLE_TO_PARSE_JSON;
			return false;
			}
		}
	else if (strEquals(sMediaType, MEDIA_TYPE_MULTIPART_FORM))
		{
		CBuffer Buffer(pBody->GetMediaBuffer());
		CHTTPMultipartParser Parser(pBody->GetMediaType(), Buffer);
		if (!Parser.ParseAsDatum(retdBody))
			{
			retdBody = ERR_UNABLE_TO_PARSE_MULTIPART;
			return false;
			}
		}
	else if (strStartsWith(sMediaType, MEDIA_TYPE_TEXT_PREFIX))
		{
		retdBody = sBuffer;
		}
	else
		{
		retdBody = strPattern(ERR_UNSUPPORTED_MEDIA_TYPE, sMediaType);
		return false;
		}

	//	Done

	return true;
	}

bool CHTTPUtil::ConvertFormURLEncodedToDatum (const CString &sText, CDatum &retdValue)

//	ConvertFormURLEncodedToDatum
//
//	Converts from x-www-form-urlencoded to a struct

	{
	enum class EState
		{
		Start,
		Skip,
		FieldName,
		FieldValue,
		Escape,
		Done,
		};

	CComplexStruct *pResult = new CComplexStruct;

	char *pPos = sText.GetParsePointer();
	char *pPosEnd = pPos + sText.GetLength();
	EState iState = EState::Start;
	EState iOldState;

	CStringBuffer Token;
	CString sFieldName;

	while (iState != EState::Done)
		{
		switch (iState)
			{
			case EState::Start:
				if (pPos == pPosEnd)
					iState = EState::Done;
				else if (*pPos == '&')
					break;
				else if (*pPos == '=')
					iState = EState::Skip;
				else
					{
					Token.SetLength(0);
					Token.Write(pPos, 1);
					iState = EState::FieldName;
					}

				break;

			case EState::Skip:
				if (pPos == pPosEnd)
					iState = EState::Done;
				else if (*pPos == '&')
					iState = EState::Start;
				break;

			case EState::FieldName:
				if (pPos == pPosEnd)
					iState = EState::Done;
				else if (*pPos == '=')
					{
					sFieldName = CString(Token.GetPointer(), Token.GetLength());
					Token.SetLength(0);
					iState = EState::FieldValue;
					}
				else if (*pPos == '+')
					Token.Write(" ", 1);
				else if (*pPos == '%')
					{
					iOldState = iState;
					iState = EState::Escape;
					}
				else
					Token.Write(pPos, 1);
				break;

			case EState::FieldValue:
				if (pPos == pPosEnd
						|| *pPos == '&')
					{
					pResult->SetElement(sFieldName, CString(Token.GetPointer(), Token.GetLength()));
					Token.SetLength(0);
					iState = ((pPos == pPosEnd) ? EState::Done : EState::Start);
					}
				else if (*pPos == '+')
					Token.Write(" ", 1);
				else if (*pPos == '%')
					{
					iOldState = iState;
					iState = EState::Escape;
					}
				else
					Token.Write(pPos, 1);
				break;

			case EState::Escape:
				if (pPos+1 >= pPosEnd)
					iState = EState::Done;
				else
					{
					DWORD dwValue = (strParseHexChar(*pPos++) << 4);
					dwValue += strParseHexChar(*pPos);

					char chChar = (char)(BYTE)dwValue;
					Token.Write(&chChar, 1);
					iState = iOldState;
					}
				break;
			}

		if (iState != EState::Done)
			pPos++;
		else
			break;
		}

	//	Return result

	if (pResult->GetCount() == 0)
		{
		delete pResult;
		retdValue = CDatum();
		return true;
		}

	retdValue = CDatum(pResult);
	return true;
	}

CDatum CHTTPUtil::ConvertHeadersToDatum (const CHTTPMessage &Message)

//	ConvertHeadersToDatum
//
//	Converts the headers in an HTTP message to a struct.

	{
	CDatum dResult(CDatum::typeStruct);
	for (int i = 0; i < Message.GetHeaderCount(); i++)
		{
		CString sHeader;
		CString sValue;

		Message.GetHeader(i, &sHeader, &sValue);

		//	If this header is already in the structure, then we need to append it.
		//
		//	NOTE: CHTTPMessage always converts headers to lowercase for ease of
		//	compare, since the HTTP spec says header names are case-insensitive.

		CStringView sOriginalData = dResult.GetElement(sHeader);
		if (!sOriginalData.IsEmpty())
			dResult.SetElement(sHeader, strPattern("%s, %s", sOriginalData, sValue));
		
		//	Otherwise we just set it

		else
			dResult.SetElement(sHeader, sValue);
		}

	//	Done

	if (dResult.GetCount() > 0)
		return dResult;
	else
		return CDatum();
	}

CDatum CHTTPUtil::CreateURL (const CString& sURL, CDatum dComponents)

//	CreateURL
//
//	Creates an URL object.

	{
	return CAEONURL::Create(sURL, dComponents);
	}

CDatum CHTTPUtil::CreateXMLElement (const IMemoryBlock& FileData)

//	CreateXMLElement
//
//	Creates an XML element from a file.

	{
	return CAEONXMLElement::Create(FileData);
	}

CDatum CHTTPUtil::DecodeResponse (const CHTTPMessage &Response)
	{
	CDatum dBody;
	if (false)
		dBody = CDatum(Response.GetBodyBuffer());
	else
		{
		ConvertBodyToDatum(Response, dBody);
		}

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_STATUS_CODE, Response.GetStatusCode());
	dResult.SetElement(FIELD_STATUS, Response.GetStatusMsg());
	dResult.SetElement(FIELD_HEADERS, ConvertHeadersToDatum(Response));
	dResult.SetElement(FIELD_DATA, dBody);

	//	Done

	return dResult;
	}

CDatum CHTTPUtil::DecodeResponse (CDatum dResponse)

//	DecodeResponse
//
//	Decodes a response from Esper.

	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_STATUS_CODE, dResponse.GetElement(0));
	dResult.SetElement(FIELD_STATUS, dResponse.GetElement(1));
	dResult.SetElement(FIELD_HEADERS, dResponse.GetElement(2));
	dResult.SetElement(FIELD_DATA, dResponse.GetElement(3));

	return dResult;
	}

CHTTPMessage CHTTPUtil::EncodeRequest (const CString &sMethod, const CString &sHost, const CString &sPath, CDatum dHeaders, CDatum dBody)
	{
	CHTTPMessage Request;
	Request.InitRequest(sMethod, sPath);

	//	Add headers
	//
	//	NOTE: HTTP headers are case-insensitive, but dHeaders has case-sensitive 
	//	keys, which means we could end up adding multiple headers with the same
	//	name. For now, it is up to callers to not be stupid, but in the future
	//	we might catch that here.

	bool bFoundHost = false;
	for (int i = 0; i < dHeaders.GetCount(); i++)
		{
		Request.AddHeader(dHeaders.GetKey(i), dHeaders.GetElement(i).AsStringView());
		if (strEquals(strToLower(dHeaders.GetKey(i)), HEADER_HOST))
			bFoundHost = true;
		}

	//	Add body

	if (!dBody.IsNil())
		{
		CString sMediaType;
		if (!Request.FindHeader(HEADER_CONTENT_TYPE, &sMediaType))
			sMediaType = MEDIA_TYPE_TEXT;

		IMediaTypePtr pBody = IMediaTypePtr(new CRawMediaType);
		pBody->DecodeFromBuffer(sMediaType, CStringBuffer(dBody.AsString()));

		Request.SetBody(pBody);
		}

	//	If there is no Host header and we have a host, then add it

	if (!bFoundHost && !sHost.IsEmpty())
		Request.AddHeader(HEADER_HOST, sHost);

	//	Done

	return Request;
	}

bool CHTTPUtil::RPC (const CString &sProtocol, const CString &sHostname, DWORD dwPort, const CHTTPMessage &Request, CHTTPMessage &retResponse, CDatum &retdResult)
	{
	bool bUseSSL = strEquals(sProtocol, PROTOCOL_HTTPS);

	//	Serialize request

	CStringBuffer RequestBuffer;
	Request.WriteToBuffer(RequestBuffer);

	//	Connect to the host

	CSSLSocketStream SocketStream;
	if (!SocketStream.Connect(sHostname, dwPort, bUseSSL, NULL))
		{
		retdResult = strPattern(ERR_UNABLE_TO_CONNECT, Request.GetRequestedURL());
		return false;
		}

	//	Write out (synchronously, for now)

	int iSent = SocketStream.Write(RequestBuffer.GetPointer(), RequestBuffer.GetLength());
	if (iSent != RequestBuffer.GetLength())
		{
		retdResult = strPattern(ERR_UNABLE_TO_SEND, Request.GetRequestedURL());
		return false;
		}

	//	Keep reading until we have a full HTTP message (or until we get an error)

	if (!retResponse.InitFromStream(SocketStream))
		{
		retdResult = strPattern(ERR_UNABLE_TO_RECEIVE, Request.GetRequestedURL());
		return false;
		}

	return true;
	}
