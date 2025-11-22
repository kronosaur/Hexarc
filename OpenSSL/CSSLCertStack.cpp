//	CSSLCertStack.cpp
//
//	CSSLCertStack class
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_PARSE_PEM,				"Unable to parse PEM section.")
DECLARE_CONST_STRING(ERR_CANT_PARSE_X509,				"Unable to parse X509 certificate.")

CCriticalSection CSSLCertStack::m_cs;

CSSLCertStack::CSSLCertStack (void) :
		m_pData(NULL)

//	CSSLCertStack constructor

	{
	}

void CSSLCertStack::CleanUp (void)

//	CleanUp
//
//	Clean up

	{
	if (m_pData)
		{
		CSmartLock Lock(m_cs);

		if (--m_pData->iRefCount == 0)
			{
			//	NOTE: This frees the stack but NOT the individual cert
			//	objects.

			sk_X509_free(COpenSSL::AsX509Stack(m_pData->pStack));

			//	Delete the structure, which will drop the refcount on the
			//	individual certs and free them if needed.

			delete m_pData;
			}

		m_pData = NULL;
		}
	}

void CSSLCertStack::Copy (const CSSLCertStack &Src)

//	Copy
//
//	Make a copy

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

CSSLCert CSSLCertStack::GetCertificate (int iIndex) const

//	GetCertificate
//
//	Returns a certificate

	{
	if (m_pData == NULL || iIndex < 0 || iIndex >= m_pData->Stack.GetCount())
		return CSSLCert();

	return m_pData->Stack[iIndex];
	}

int CSSLCertStack::GetCount (void) const

//	GetCount
//
//	Returns the number of certificates on the stack

	{
	return (m_pData ? m_pData->Stack.GetCount() : 0);
	}

bool CSSLCertStack::InitFromPEM (IMemoryBlock &Data, CString *retsError)

//	InitFromPEM
//
//	Initializes from PEM

	{
	CleanUp();

	//	Make sure OpenSSL is initialized.

	if (!g_SSLGlobal.Init())
		return false;

	//	Allocate a stack

	m_pData = new SStack;
	m_pData->iRefCount = 1;
	m_pData->pStack = sk_X509_new_null();

	//	Keep parsing until we get all the certs or until we hit an error.

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

		if (!CSSLCert::IsCertificateSection(sType))
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

		//	Create a new certificate object

		CSSLCert NewCert(pX509);

		//	Add it to our array. [We copy it into the array, which will increase
		//	the refcount.]

		m_pData->Stack.Insert(NewCert);

		//	Add it to our stack

		sk_X509_push(COpenSSL::AsX509Stack(m_pData->pStack), pX509);
		}

	//	Success

	return true;
	}
