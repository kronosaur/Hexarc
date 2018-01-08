//	CSSLEnvelopeKey.cpp
//
//	CSSLEnvelopeKey class
//	Copyright (c) 2017 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(TYPE_PRIVATE_KEY,					"PRIVATE KEY")

DECLARE_CONST_STRING(ERR_CANT_PARSE_KEY,				"Unable to parse key from PEM section.")
DECLARE_CONST_STRING(ERR_CANT_PARSE_PEM,				"Unable to parse PEM section.")
DECLARE_CONST_STRING(ERR_NO_KEY_FOUND,					"Unable to find key in PEM buffer.")
DECLARE_CONST_STRING(ERR_OUT_OF_MEMORY,					"Out of memory creating keys.")
DECLARE_CONST_STRING(ERR_CANT_GENERATE_RSA,				"Unable to generate 2048-bit RSA key.")

CCriticalSection CSSLEnvelopeKey::m_cs;

void CSSLEnvelopeKey::CleanUp (void)

//	CleanUp
//
//	Free resources

	{
	if (m_pData)
		{
		CSmartLock Lock(m_cs);

		if (--m_pData->iRefCount == 0)
			{
			EVP_PKEY_free(COpenSSL::AsEVPPKey(m_pData->pKey));
			delete m_pData;
			}

		m_pData = NULL;
		}
	}

void CSSLEnvelopeKey::Copy (const CSSLEnvelopeKey &Src)

//	Copy
//
//	Add a reference

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

bool CSSLEnvelopeKey::Init (CString *retsError)

//	Init
//
//	Creates a new public/private key pair

	{
	//	Make sure OpenSSL is initialized.

	if (!g_SSLGlobal.Init())
		return false;

	//	Create the key

    EVP_PKEY *pKey = EVP_PKEY_new();
	if (pKey == NULL)
		{
		if (retsError) *retsError = ERR_OUT_OF_MEMORY;
		return false;
		}

	//	Generate the RSA key and assign it.

	RSA *pRSA = RSA_generate_key(2048, RSA_F4, NULL, NULL);
	if (!EVP_PKEY_assign_RSA(pKey, pRSA))
		{
        EVP_PKEY_free(pKey);
		if (retsError) *retsError = ERR_CANT_GENERATE_RSA;
		return false;
		}

	//	We store the result

	CleanUp();

	//	Allocate the structure

	m_pData = new SEKey;
	m_pData->pKey = pKey;

	//	Done

	return true;
	}

bool CSSLEnvelopeKey::InitFromPEM (IMemoryBlock &Data, const CString &sPassphrase, bool bKeepRaw, CString *retsError)

//	InitFromPEM
//
//	Loads the first key from the given PEM file.

	{
	CleanUp();

	//	Make sure OpenSSL is initialized.

	if (!g_SSLGlobal.Init())
		return false;

	//	Keep parsing until we find a key

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

		//	We skip any types that we don't understand. This is not an error, 
		//	since we often store private keys and other things in a PEM file.

		if (!IsKeyPEMSection(sType))
			continue;

		//	Create BIO stream for the section

		CMemoryBIO SectionData(sData);

		//	Create the key

		EVP_PKEY *pKey = PEM_read_bio_PrivateKey(SectionData, NULL, NULL, sPassphrase.GetParsePointer());
		if (!pKey)
			{
			if (retsError) *retsError = ERR_CANT_PARSE_KEY;
			return false;
			}

		//	Create

		m_pData = new SEKey;
		m_pData->pKey = pKey;
		if (bKeepRaw)
			m_pData->sPEMData = sData;

		//	We load the first key we find, so we're done.

		break;
		}

	//	If we didn't find a key, then we return an error

	if (m_pData == NULL)
		{
		if (retsError) *retsError = ERR_NO_KEY_FOUND;
		return false;
		}

	//	Success

	return true;
	}

bool CSSLEnvelopeKey::IsKeyPEMSection (const CString &sType)

//	IsKeyPEMSection
//
//	Returns TRUE if this is a section we can load.

	{
	return (strEqualsNoCase(sType, TYPE_PRIVATE_KEY));
	}
