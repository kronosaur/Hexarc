//	CAEONURL.cpp
//
//	CAEONURL class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(FIELD_FRAGMENT,				"fragment");
DECLARE_CONST_STRING(FIELD_HOST,					"host");
DECLARE_CONST_STRING(FIELD_ID,						"id");
DECLARE_CONST_STRING(FIELD_NAME,					"name");
DECLARE_CONST_STRING(FIELD_PATH,					"path");
DECLARE_CONST_STRING(FIELD_PORT,					"port");
DECLARE_CONST_STRING(FIELD_PROTOCOL,				"protocol");
DECLARE_CONST_STRING(FIELD_QUERY,					"query");

DECLARE_CONST_STRING(TYPENAME_URL,					"url");

TDatumPropertyHandler<CAEONURL> CAEONURL::m_Properties = {
	{
		"fragment",
		"s",
		"Returns the fragment, if specified.",
		[](const CAEONURL &Obj, const CString &sProperty)
			{
			CString sFragment;
			if (!ParseURL(Obj.m_sURL, NULL, NULL, NULL, NULL, NULL, &sFragment))
				return CDatum();

			return CDatum(sFragment);
			},
		NULL,
		},
	{
		"host",
		"s",
		"Returns the host, if this is a valid URL.",
		[](const CAEONURL &Obj, const CString &sProperty)
			{
			CString sHost;
			if (!ParseURL(Obj.m_sURL, NULL, &sHost, NULL, NULL, NULL, NULL))
				return CDatum();

			return CDatum(sHost);
			},
		NULL,
		},
	{
		"path",
		"s",
		"Returns the path, if specified.",
		[](const CAEONURL &Obj, const CString &sProperty)
			{
			CString sPath;
			if (!ParseURL(Obj.m_sURL, NULL, NULL, NULL, &sPath, NULL, NULL))
				return CDatum();

			return CDatum(sPath);
			},
		NULL,
		},
	{
		"port",
		"I",
		"Returns the port, if specified.",
		[](const CAEONURL &Obj, const CString &sProperty)
			{
			DWORD dwPort;
			if (!ParseURL(Obj.m_sURL, NULL, NULL, &dwPort, NULL, NULL, NULL))
				return CDatum();

			return CDatum(dwPort);
			},
		NULL,
		},
	{
		"protocol",
		"s",
		"Returns the protocol, if specified.",
		[](const CAEONURL &Obj, const CString &sProperty)
			{
			CString sProtocol;
			if (!ParseURL(Obj.m_sURL, &sProtocol, NULL, NULL, NULL, NULL, NULL))
				return CDatum();

			return CDatum(sProtocol);
			},
		NULL,
		},
	{
		"query",
		"x",
		"Returns the query, if specified.",
		[](const CAEONURL &Obj, const CString &sProperty)
			{
			CDatum dQuery;
			if (!ParseURL(Obj.m_sURL, NULL, NULL, NULL, NULL, &dQuery, NULL))
				return CDatum();

			return dQuery;
			},
		NULL,
		},
	};

TDatumMethodHandler<CAEONURL> CAEONURL::m_Methods = {
	};

CDatum CAEONURL::Create (const CString& sURL, CDatum dComponents)

//	Create
//
//	Creates a URL.

	{
	CString sProtocol;
	CString sHost;
	DWORD dwPort;
	CString sPath;
	CDatum dQuery;
	CString sFragment;

	//	If we have an URL, then parse it first.

	if (!ParseURL(sURL, &sProtocol, &sHost, &dwPort, &sPath, &dQuery, &sFragment))
		return CDatum();

	//	Next we take values from the components

	CDatum dProtocol = dComponents.GetElement(FIELD_PROTOCOL);
	if (!dProtocol.IsNil())
		sProtocol = dProtocol.AsStringView();

	CDatum dHost = dComponents.GetElement(FIELD_HOST);
	if (!dHost.IsNil())
		sHost = dHost.AsStringView();

	CDatum dPort = dComponents.GetElement(FIELD_PORT);
	if (!dPort.IsNil())
		dwPort = dPort;

	CDatum dPath = dComponents.GetElement(FIELD_PATH);
	if (!dPath.IsNil())
		sPath = dPath.AsStringView();

	CDatum dQueryInput = dComponents.GetElement(FIELD_QUERY);
	if (!dQueryInput.IsNil())
		dQuery = dQueryInput;

	CDatum dFragment = dComponents.GetElement(FIELD_FRAGMENT);
	if (!dFragment.IsNil())
		sFragment = dFragment.AsStringView();

	//	Host and protocol are required

	if (sHost.IsEmpty() || sProtocol.IsEmpty())
		return CDatum();

	//	Now compose an URL, encoding as appropriate.

	return CDatum(new CAEONURL(ComposeURL(sProtocol, sHost, dwPort, sPath, dQuery, sFragment)));
	}

size_t CAEONURL::CalcMemorySize () const

//	CalcMemorySize
//
//	Returns the amount of memory we are using.

	{
	return m_sURL.GetLength();
	}

CDatum CAEONURL::GetDatatype () const

//	GetDatatype
//
//	Returns our datatype.

	{
	if (CHTTPUtil::URL_TYPE == 0)
		throw CException(errFail);

	return CAEONTypeSystem::GetCoreType(CHTTPUtil::URL_TYPE);
	}

bool CAEONURL::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize.

	{
	m_sURL = CString::Deserialize(Stream);
	return true;
	}

void CAEONURL::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized)
	{
	m_sURL = CString::Deserialize(Stream);
	}

void CAEONURL::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize.

	{
	m_sURL.Serialize(Stream);
	}

void CAEONURL::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const
	{
	m_sURL.Serialize(Stream);
	}

int CAEONURL::OpCompare (CDatum::Types iValueType, CDatum dValue) const

//	OpCompare
//
//	-1:		If dKey1 < dKey2
//	0:		If dKey1 == dKey2
//	1:		If dKey1 > dKey2

	{
	//	Compare as a string

	return KeyCompare(m_sURL, dValue.AsString());
	}

bool CAEONURL::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const

//	OpIsEqual
//
//	Returns TRUE if we are equivalent.

	{
	return strEquals(m_sURL, dValue.AsString());
	}

bool CAEONURL::OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const

//	OpIsIdentical
//
//	Returns TRUE if we are identical.

	{
	//	If we're both names, then we compare 

	if (strEquals(dValue.GetTypename(), TYPENAME_URL))
		{
		return strEquals(m_sURL, dValue.AsString());
		}

	//	Otherwise, not equal

	else
		return false;
	}

const CString &CAEONURL::StaticGetTypename (void) { return TYPENAME_URL; }
