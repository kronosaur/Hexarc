//	MsgSetCertificate.cpp
//
//	MsgSetCertificate
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ADDR_CRYPTOSAUR_NOTIFY,			"Cryptosaur.notify")

DECLARE_CONST_STRING(FIELD_CERTIFICATES,				"certificates")
DECLARE_CONST_STRING(FIELD_DATA,						"data")
DECLARE_CONST_STRING(FIELD_ISSUER,						"issuer")
DECLARE_CONST_STRING(FIELD_NAME,						"name")
DECLARE_CONST_STRING(FIELD_PRIVATE_KEY,					"privateKey")
DECLARE_CONST_STRING(FIELD_SUBJECT,						"subject")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")

DECLARE_CONST_STRING(MSG_CRYPTOSAUR_ON_NEW_CERTIFICATE,	"Cryptosaur.onNewCertificate")
DECLARE_CONST_STRING(MSG_ERROR_DOES_NOT_EXIST,			"Error.doesNotExist")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

DECLARE_CONST_STRING(PORT_CRYPTOSAUR_COMMAND,			"Cryptosaur.command")

DECLARE_CONST_STRING(TABLE_ARC_CERTIFICATES,			"Arc.certificates")

DECLARE_CONST_STRING(TYPE_KEY_ENVELOPE,					"keyEnvelope")
DECLARE_CONST_STRING(TYPE_SSL_CERTIFICATE,				"sslCertificate")
DECLARE_CONST_STRING(TYPE_X509,							"X509")

DECLARE_CONST_STRING(ERR_CANT_PARSE_PEM,				"Unable to parse PEM section.")
DECLARE_CONST_STRING(ERR_CANT_LOAD_CERTIFICATE,			"Unable to load PEM certificate: %s.")
DECLARE_CONST_STRING(ERR_CANT_LOAD_KEY,					"Unable to load PEM private key: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_CERT,					"Unable to find certificate: %s.")
DECLARE_CONST_STRING(ERR_UNKNOWN_CERT_TYPE,				"Unknown certificate type: %s.")
DECLARE_CONST_STRING(ERR_NO_PRIVATE_KEY,				"PEM file must have a private key.")
DECLARE_CONST_STRING(ERR_NO_CERTIFICATES,				"PEM file must have at least one certificate.")

class CSaveCertSession : public CAeonMutateSession
	{
	public:
		CSaveCertSession (CCryptosaurEngine &Engine, CDatum dKeyPath, CDatum dData);

	protected:
		virtual void OnSuccess (CDatum dData) override;

	private:
		CCryptosaurEngine &m_Engine;
	};

void CCryptosaurEngine::MsgGetCertificate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgGetCertificate
//
//	Cryptosaur.getCertificate {type} {name}

	{
	CDatum dType = Msg.dPayload.GetElement(0);
	CDatum dName = Msg.dPayload.GetElement(1);

	//	For now we only support sslCertificates

	if (!strEqualsNoCase(dType, TYPE_SSL_CERTIFICATE))
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_CERT_TYPE, dType.AsString()), Msg);
		return;
		}

	//	Lock and get the element from cache

	CSmartLock Lock(m_cs);
	CDatum dRecord;

	if (!m_Certificates.Find(dName, &dRecord))
		{
		SendMessageReplyError(MSG_ERROR_DOES_NOT_EXIST, strPattern(ERR_UNKNOWN_CERT, dName.AsString()), Msg);
		return;
		}

	Lock.Unlock();

	//	Return it.

	SendMessageReply(MSG_REPLY_DATA, dRecord, Msg);
	}

void CCryptosaurEngine::MsgSetCertificate (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgSetCertificate
//
//	Cryptosaur.setCertificate {type} {data}

	{
	CString sType = Msg.dPayload.GetElement(0);
	CDatum dData = Msg.dPayload.GetElement(1);

	//	Handle each type

	if (strEqualsNoCase(sType, TYPE_SSL_CERTIFICATE))
		{
		//	Generate a record for Arc.certificates.

		CDatum dKey;
		CDatum dRecord;
		CString sError;
		if (!LoadPEMCertAndKey(dData, &dKey, &dRecord, &sError))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, sError, Msg);
			return;
			}

		//	Start a session to write the record to Arc.certificates. We will
		//	return MSG_OK on success.

		StartSession(Msg, new CSaveCertSession(*this, dKey, dRecord));
		}

	//	Otherwise, unknown type

	else
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern(ERR_UNKNOWN_CERT_TYPE, sType), Msg);
		}
	}

bool CCryptosaurEngine::LoadPEMCertAndKey (CDatum dData, CDatum *retKey, CDatum *retRecord, CString *retsError)

//	LoadPEMCertAndKey
//
//	Loads a PEM file containing a certificate (and optional chain) plus a 
//	private key. Returns a record structure for Arc.certificates

	{
	int i;
	CString sError;
	CStringBuffer Data(dData);

	TArray<CSSLCert> Certificates;
	CSSLEnvelopeKey PrivateKey;

	//	We expect a PEM file with multiple segments

	int iPos = 0;
	while (CSSLCert::HasPEMSection(Data, iPos))
		{
		CString sData;
		CString sType;

		if (!CSSLCert::ParsePEMSection(Data, iPos, &sData, &sType, &iPos))
			{
			if (retsError) *retsError = ERR_CANT_PARSE_PEM;
			return false;
			}

		//	If this is a certificate, then add it to our certificate list.

		if (CSSLCert::IsCertificateSection(sType))
			{
			//	Load the certificate so we know what it is.

			CSSLCert *pCert = Certificates.Insert();
			CStringBuffer Buffer(sData);
			if (!pCert->InitFromPEM(Buffer, true, &sError))
				{
				if (retsError) *retsError = strPattern(ERR_CANT_LOAD_CERTIFICATE, sError);
				return false;
				}
			}

		//	Otherwise, if this is a key, then add it to our key

		else if (CSSLEnvelopeKey::IsKeyPEMSection(sType))
			{
			if (PrivateKey.IsEmpty())
				{
				CStringBuffer Buffer(sData);
				if (!PrivateKey.InitFromPEM(Buffer, NULL_STR, true, &sError))
					{
					if (retsError) *retsError = strPattern(ERR_CANT_LOAD_KEY, sError);
					return false;
					}
				}
			}
		}

	//	We must have a private key and at least one certificate

	if (PrivateKey.IsEmpty())
		{
		if (retsError) *retsError = ERR_NO_PRIVATE_KEY;
		return false;
		}

	if (Certificates.GetCount() == 0)
		{
		if (retsError) *retsError = ERR_NO_CERTIFICATES;
		return false;
		}

	//	If we have more than one certificate, then we order them from bottom up to root.

	if (Certificates.GetCount() > 1)
		CSSLCert::SortChain(Certificates);

	//	The domain that we're certifying is the common name of the first certificate.

	CString sName = Certificates[0].GetSubject().GetPart(CDistinguishedName::CN);
	if (sName.IsEmpty())
		{
		if (retsError) *retsError = ERR_NO_CERTIFICATES;
		return false;
		}

	//	Create a key

	CDatum dKey(CDatum::typeArray);
	dKey.Append(TYPE_SSL_CERTIFICATE);
	dKey.Append(sName);

	//	Now we're ready to create the record.

	CDatum dRecord(CDatum::typeStruct);
	dRecord.SetElement(FIELD_TYPE, TYPE_SSL_CERTIFICATE);
	dRecord.SetElement(FIELD_NAME, sName);

	//	Certificates

	CDatum dCertChain(CDatum::typeArray);
	for (i = 0; i < Certificates.GetCount(); i++)
		{
		CDatum dCert(CDatum::typeStruct);
		dCert.SetElement(FIELD_TYPE, TYPE_X509);
		dCert.SetElement(FIELD_NAME, Certificates[i].GetSubject().GetPart(CDistinguishedName::CN));
		dCert.SetElement(FIELD_SUBJECT, Certificates[i].GetSubject().GetDN());
		dCert.SetElement(FIELD_ISSUER, Certificates[i].GetIssuer().GetDN());
		dCert.SetElement(FIELD_DATA, Certificates[i].GetPEMData());

		dCertChain.Append(dCert);
		}

	dRecord.SetElement(FIELD_CERTIFICATES, dCertChain);
	
	//	Private key

	CDatum dPrivateKey(CDatum::typeStruct);
	dPrivateKey.SetElement(FIELD_TYPE, TYPE_KEY_ENVELOPE);
	dPrivateKey.SetElement(FIELD_DATA, PrivateKey.GetPEMData());

	dRecord.SetElement(FIELD_PRIVATE_KEY, dPrivateKey);

	//	Done

	if (retKey)
		*retKey = dKey;
		
	if (retRecord)
		*retRecord = dRecord;

	return true;
	}

//	CSaveCertSession -----------------------------------------------------------

CSaveCertSession::CSaveCertSession (CCryptosaurEngine &Engine, CDatum dKeyPath, CDatum dData) :
		CAeonMutateSession(Engine.GetProcessCtx()->GenerateAddress(PORT_CRYPTOSAUR_COMMAND), TABLE_ARC_CERTIFICATES, dKeyPath, dData, CDatum()),
		m_Engine(Engine)

//	CSaveCertSession constructor

	{
	}

void CSaveCertSession::OnSuccess (CDatum dData)

//	OnSuccess
//
//	Successfully wrote the certificate

	{
	CDatum dDomain = dData.GetElement(FIELD_NAME);

	//	Add to our cache

	m_Engine.InsertCertificate(dDomain.AsString(), dData);

	//	Notify our subscribers that a certificate has been added/changed

	SendMessageNotify(ADDR_CRYPTOSAUR_NOTIFY, MSG_CRYPTOSAUR_ON_NEW_CERTIFICATE, dDomain);

	//	Reply success

	SendMessageReply(MSG_OK, CDatum());
	}
