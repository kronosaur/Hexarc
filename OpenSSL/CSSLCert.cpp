//	CSSLCert.cpp
//
//	CSSLCert class
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPE_CERTIFICATE,					"CERTIFICATE")

DECLARE_CONST_STRING(STR_BEGIN_PREFIX,					"BEGIN ")
DECLARE_CONST_STRING(STR_END_PREFIX,					"END ")

DECLARE_CONST_STRING(ERR_CANT_PARSE_X509,				"Unable to parse X509 certificate.")
DECLARE_CONST_STRING(ERR_NO_CERTIFICATE_FOUND,			"Unable to find X509 certificate.")
DECLARE_CONST_STRING(ERR_OUT_OF_MEMORY,					"Out of memory creating certificate.")
DECLARE_CONST_STRING(ERR_CANT_SIGN,						"Unable to sign certificate.")

CCriticalSection CSSLCert::m_cs;

CSSLCert::CSSLCert (void) :
		m_pData(NULL)

//	CSSLCert constructor

	{
	}

CSSLCert::CSSLCert (OpenSSL_X509Ptr pX509)

//	CSSLCert constructor

	{
	m_pData = new SCert;
	m_pData->pX509 = pX509;

	//	Get some info about the certificate

	m_pData->Subject = CDistinguishedName(X509_get_subject_name(COpenSSL::AsX509(pX509)));
	m_pData->Issuer = CDistinguishedName(X509_get_issuer_name(COpenSSL::AsX509(pX509)));
	}

void CSSLCert::CleanUp (void)

//	CleanUp
//
//	Clean up the certificate

	{
	if (m_pData)
		{
		CSmartLock Lock(m_cs);

		if (--m_pData->iRefCount == 0)
			{
			X509_free(COpenSSL::AsX509(m_pData->pX509));
			delete m_pData;
			}

		m_pData = NULL;
		}
	}

void CSSLCert::Copy (const CSSLCert &Src)

//	Copy
//
//	Copy from the source

	{
	if (Src.m_pData)
		{
		CSmartLock Lock(m_cs);
		m_pData = Src.m_pData;
		m_pData->iRefCount++;
		}
	else
		m_pData = NULL;
	}

const CDistinguishedName &CSSLCert::GetIssuer (void) const

//	GetIssuer
//
//	Returns the subject of the certificate

	{
	if (m_pData == NULL)
		return CDistinguishedName::Null();

	return m_pData->Issuer;
	}

const CDistinguishedName &CSSLCert::GetSubject (void) const

//	GetSubject
//
//	Returns the subject of the certificate

	{
	if (m_pData == NULL)
		return CDistinguishedName::Null();

	return m_pData->Subject;
	}

bool CSSLCert::HasPEMSection (IMemoryBlock &Data, int iOffset)

//	HasPEMSection
//
//	Returns TRUE if we have a PEM section starting with "-----BEGIN..."

	{
	//	Edge-conditions

	if (iOffset >= Data.GetLength() || Data.GetPointer() == NULL)
		return false;

	//	Start

	char *pPos = Data.GetPointer() + iOffset;
	char *pPosEnd = Data.GetPointer() + Data.GetLength();

	//	Skip any whitespace

	while (pPos < pPosEnd && strIsWhitespace(pPos))
		pPos++;

	//	Dashes

	int iCount = 0;
	while (pPos < pPosEnd && *pPos == '-')
		{
		iCount++;
		pPos++;
		}

	if (iCount < 5)
		return false;

	//	If we have a B, then we count it as a section

	if (pPos < pPosEnd && *pPos == 'B')
		return true;

	return false;
	}

bool CSSLCert::InitFromPEM (IMemoryBlock &Data, bool bKeepRaw, CString *retsError)

//	InitFromPEM
//
//	Initialize from a PEM

	{
	CleanUp();

	//	Make sure OpenSSL is initialized.

	if (!g_SSLGlobal.Init())
		return false;

	//	Keep parsing until we find a certificate

	int iPos = 0;
	while (CSSLCert::HasPEMSection(Data, iPos))
		{
		CString sData;
		CString sType;

		if (!CSSLCert::ParsePEMSection(Data, iPos, &sData, &sType, &iPos))
			{
			if (retsError) *retsError = ERR_CANT_PARSE_X509;
			return false;
			}

		//	We skip any types that we don't understand. This is not an error, 
		//	since we often store private keys and other things in a PEM file.

		if (!IsCertificateSection(sType))
			continue;

		//	Create BIO stream for the section

		CMemoryBIO CertData(sData);

		//	Create the X509 cert

		X509 *pX509 = PEM_read_bio_X509(CertData, NULL, NULL, NULL);
		if (!pX509)
			{
			if (retsError) *retsError = ERR_CANT_PARSE_X509;
			return false;
			}

		//	Replace

		*this = CSSLCert(pX509);

		//	If we need to keep the raw data, do it.

		if (bKeepRaw)
			m_pData->sPEMData = sData;

		//	We load the first certificate we find, so we're done.

		break;
		}

	//	If we didn't find a key, then we return an error

	if (m_pData == NULL)
		{
		if (retsError) *retsError = ERR_NO_CERTIFICATE_FOUND;
		return false;
		}

	//	Success

	return true;
	}

bool CSSLCert::InitSelfSigned (const SDesc &Desc, CString *retsError)

//	InitSelfSigned
//
//	Generates a self-signed certificate with the given parameters.

	{
	if (Desc.Key.IsEmpty() || Desc.Subject.IsEmpty() || Desc.Issuer.IsEmpty())
		return false;

	//	Make sure OpenSSL is initialized.

	if (!g_SSLGlobal.Init())
		return false;

	//	Allocate a new X509 structure

	X509 *pX509 = X509_new();
	if (pX509 == NULL)
		{
		if (retsError) *retsError = ERR_OUT_OF_MEMORY;
		return false;
		}

	//	Set the serial number to 1. The default of 0 is sometimes rejected by
	//	some software.

    ASN1_INTEGER_set(X509_get_serialNumber(pX509), 1);

    //	Set the valid range of the certificate (in seconds)

    X509_gmtime_adj(X509_get_notBefore(pX509), 0);
    X509_gmtime_adj(X509_get_notAfter(pX509), Desc.ValidTime.Seconds());

	//	Set the public key

    X509_set_pubkey(pX509, COpenSSL::AsEVPPKey(Desc.Key));

	//	Get the subject name object from the cert

	X509_NAME *pName = X509_get_subject_name(pX509);
	Desc.Subject.CopyTo(pName);

	//	Since it's self-signed, we copy to the issuer

    X509_set_issuer_name(pX509, pName);

	//	Now sign it.

	if (!X509_sign(pX509, COpenSSL::AsEVPPKey(Desc.Key), EVP_sha256()))
		{
		if (retsError) *retsError = ERR_CANT_SIGN;
		return false;
		}

	//	Take ownership

	CleanUp();
	m_pData = new SCert;
	m_pData->pX509 = pX509;
	m_pData->Subject = Desc.Subject;
	m_pData->Issuer = Desc.Issuer;

	//	Done

	return true;
	}

bool CSSLCert::IsCertificateSection (const CString &sType)

//	IsCertificateSection
//
//	Returns TRUE if the PEM type is for a certificate that we can load.

	{
	return (strEqualsNoCase(sType, TYPE_CERTIFICATE));
	}

bool CSSLCert::ParsePEMSection (IMemoryBlock &Data, int iOffset, CString *retsData, CString *retsType, int *retiEnd)

//	ParsePEMSection
//
//	Parses a section of the form "-----BEGIN XXX-----...-----END XXX-----"
//	and returns the entire section (including BEGIN/END wrappers).
//	Optionally returns the type (XXX) and the offset into the block where we
//	stopped parsing (generally the end of the dashes).
//
//	If we get an error, we return FALSE.

	{
	enum EStates
		{
		stateStart,
		stateBeginLeadingDash,
		stateBeginText,
		stateBeginTrailingDash,
		stateContent,
		stateEndLeadingDash,
		stateEndText,
		stateEndTrailingDash,
		stateDone,
		};

	//	Edge-conditions

	if (iOffset >= Data.GetLength() || Data.GetPointer() == NULL)
		return false;

	//	Start

	char *pPos = Data.GetPointer() + iOffset;
	char *pPosEnd = Data.GetPointer() + Data.GetLength();

	//	Parse

	EStates iState = stateStart;
	char *pStartSection = NULL;
	char *pStart = NULL;
	CString sBeginText;
	CString sEndText;
	while (pPos < pPosEnd && iState != stateDone)
		{
		switch (iState)
			{
			case stateStart:
				//	Keep going until we find the first dash

				if (*pPos == '-')
					{
					pStartSection = pPos;
					iState = stateBeginLeadingDash;
					}
				break;

			case stateBeginLeadingDash:
				if (*pPos == 'B')
					{
					pStart = pPos;
					iState = stateBeginText;
					}
				break;

			case stateBeginText:
				if (*pPos == '-')
					{
					sBeginText = CString(pStart, pPos - pStart);
					iState = stateBeginTrailingDash;
					}
				break;

			case stateBeginTrailingDash:
				if (*pPos != '-' && *pPos != '\r' && *pPos != '\n')
					{
					pStart = pPos;
					iState = stateContent;
					}
				break;

			case stateContent:
				if (*pPos == '-')
					iState = stateEndLeadingDash;
				break;

			case stateEndLeadingDash:
				if (*pPos == 'E')
					{
					pStart = pPos;
					iState = stateEndText;
					}
				break;

			case stateEndText:
				if (*pPos == '-')
					{
					sEndText = CString(pStart, pPos - pStart);
					iState = stateEndTrailingDash;
					}
				break;

			case stateEndTrailingDash:
				if (*pPos != '-')
					{
					pPos--;
					iState = stateDone;
					}
				break;
			}

		//	Next

		pPos++;
		}

	//	We stop the loop under one of two conditions. Either we hit the end of
	//	the stream, or we hit the end of the segment (stateDone). If we're at 
	//	stateDone, then we advance a little bit in case we have another segment.

	if (iState == stateDone)
		{
		while (pPos < pPosEnd && strIsWhitespace(pPos))
			pPos++;
		}

	//	If we're at stateEndTrailingDash then it means that we hit the end while
	//	we were parsing the last trail of dashes. This is completely valid.

	else if (iState == stateEndTrailingDash)
		{
		}

	//	Otherwise, we could not complete the parsing. This is an error.

	else
		return false;

	//	Figure out the actual section type

	if (!strStartsWith(sBeginText, STR_BEGIN_PREFIX))
		return false;

	CString sBeginType = strSubString(sBeginText, STR_BEGIN_PREFIX.GetLength());

	//	Make sure it matches the end type

	if (!strStartsWith(sEndText, STR_END_PREFIX))
		return false;

	CString sEndType = strSubString(sEndText, STR_END_PREFIX.GetLength());
	if (!strEqualsNoCase(sBeginType, sEndType))
		return false;

	//	Return the data

	if (retsData)
		*retsData = CString(pStartSection, pPos - pStartSection);

	//	Return the type

	if (retsType)
		*retsType = sBeginType;

	//	pPos points to the start of the next section (or the end of the buffer).
	//	Returns an index to the end of the section

	if (retiEnd)
		*retiEnd = Min(Data.GetLength(), (int)(pPos - Data.GetPointer()));

	//	Success!

	return true;
	}

void CSSLCert::SortChain (TArray<CSSLCert> &Chain)

//	SortChain
//
//	Sorts the given array so the chain is in order from bottom cert to root cert.

	{
	int i, j;

	struct SInfo
		{
		SInfo (void) :
				iIssuer(-1),
				iCertified(-1)
			{ }

		int iIssuer;						//	Index into Chain of who issuer our cert
		int iCertified;						//	Index into Chain of who we certified
		};

	//	Loop over all certificates finding the issuer for each certificate.

	TArray<SInfo> CertInfo;
	CertInfo.InsertEmpty(Chain.GetCount());

	for (i = 0; i < Chain.GetCount(); i++)
		{
		//	See we have a cert for whomever issued our certificate.

		for (j = 0; j < Chain.GetCount(); j++)
			{
			if (Chain[i].GetIssuer() == Chain[j].GetSubject())
				{
				CertInfo[i].iIssuer = j;

				//	If this is not a self-signed certificate, remember
				//	that we certified someone.

				if (i != j)
					CertInfo[j].iCertified = i;

				break;
				}
			}
		}

	//	Find a certificate that didn't certify anyone else. This is the
	//	bottom certificate.

	int iStart = 0;
	for (i = 0; i < CertInfo.GetCount(); i++)
		if (CertInfo[i].iCertified == -1)
			{
			iStart = i;
			break;
			}

	//	Reconstruct the chain.

	TArray<CSSLCert> NewChain;
	int iNext = iStart;
	while (true)
		{
		NewChain.Insert(Chain[iNext]);

		//	If this is a self-signed certificate, we're done.

		if (CertInfo[iNext].iIssuer == -1 || CertInfo[iNext].iIssuer == iNext)
			break;

		//	If we've already inserts all certificates in the original chain, 
		//	then stop. This prevents infinite loops in the case of circular
		//	references.

		if (NewChain.GetCount() >= Chain.GetCount())
			break;

		//	Pick our issuer as the next cert

		iNext = CertInfo[iNext].iIssuer;
		}

	//	Done

	Chain = NewChain;
	}
