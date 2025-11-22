//	CSSLCtx.cpp
//
//	CSSLCtx class
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_SET_CERTIFICATE,			"Unable to set certificate on SSL CTX.")
DECLARE_CONST_STRING(ERR_CANT_SET_PRIVATE_KEY,			"Unable to set private key on SSL CTX.")

CCriticalSection CSSLCtx::m_cs;

CSSLCtx::CSSLCtx (void) :
		m_pData(NULL)

//	CSSLCtx constructor

	{
	}

OpenSSL_SSLPtr CSSLCtx::AllocSSL (void)

//	AllocSSL
//
//	Allocates an SSL object. Callers must free it.

	{
	if (m_pData == NULL)
		return NULL;

	return SSL_new(COpenSSL::AsSSLCTX(m_pData->pCtx));
	}

void CSSLCtx::FreeSSL (OpenSSL_SSLPtr pSSL)

//	FreeSSL
//
//	Free object allocated by AllocSSL

	{
	SSL_free(COpenSSL::AsSSL(pSSL));
	}

void CSSLCtx::CleanUp (void)

//	CleanUp
//
//	Free everything

	{
	if (m_pData)
		{
		CSmartLock Lock(m_cs);

		if (--m_pData->iRefCount == 0)
			{
			SSL_CTX_free(COpenSSL::AsSSLCTX(m_pData->pCtx));
			delete m_pData;
			}

		m_pData = NULL;
		}
	}

void CSSLCtx::Copy (const CSSLCtx &Src)

//	Copy
//
//	Copy from source

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

bool CSSLCtx::Init (CString *retsError)

//	Init
//
//	Initialize

	{
	CleanUp();

	if (!g_SSLGlobal.Init(retsError))
		return false;

	m_pData = new SSSLCtx;

	//	Initialize with SSLv23_method so that we negotiate with the client for
	//	the appropriate protocol.
	//	See: http://stackoverflow.com/questions/23709664/openssl-let-the-server-and-client-negotiate-the-method
	
	const SSL_METHOD *meth = SSLv23_method();
	m_pData->pCtx = SSL_CTX_new(meth);
	if (m_pData->pCtx == NULL)
		{
		delete m_pData;
		m_pData = NULL;
		return false;
		}

	//	Set options to deprecate SSL methods which have security problems.
	//	See: https://raymii.org/s/tutorials/Strong_SSL_Security_On_Apache2.html

	const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
	SSL_CTX_set_options(COpenSSL::AsSSLCTX(m_pData->pCtx), flags);

	//	Restrict the set of ciphers used to protect against various attacks.
	//	See: http://stackoverflow.com/questions/30319394/openssl-certificate-generation-for-dhe-exchange/30332153#30332153

	SSL_CTX_set_cipher_list(COpenSSL::AsSSLCTX(m_pData->pCtx), "HIGH:!aNULL:!RC4:!MD5");

	return true;
	}

int CSSLCtx::OnServerNameIndication (OpenSSL_SSLPtr pSSL, int *ad, DWORD_PTR dwData)

//	OnServerNameIndication
//
//	This is called on an SSL connection when we've received a server name from 
//	the client. We need to switch context to one with the appropriate certificates.

	{
	//	Get the server name

	const char *servername = SSL_get_servername(COpenSSL::AsSSL(pSSL), TLSEXT_NAMETYPE_host_name);
	if (servername == NULL)
		return SSL_TLSEXT_ERR_OK;

	CString sServerName(servername);

	//	Get information from our data to do a callback. We need to lock in case
	//	some other thread tries to change things.

	CSmartLock Lock(m_cs);

	if (m_pData == NULL || m_pData->pfOnSNI == NULL)
		return SSL_TLSEXT_ERR_NOACK;

	SSL_SNI_FUNC pfOnSNI = m_pData->pfOnSNI;
	DWORD_PTR dwOnSNIData = m_pData->dwOnSNIData;

	//	Now unlock and make do the callback

	Lock.Unlock();

	CSSLCtx NewCtx;
	if (!pfOnSNI(dwOnSNIData, sServerName, &NewCtx))
		return SSL_TLSEXT_ERR_NOACK;

	//	Switch the context (this is OK outside of a lock because OpenSSL does 
	//	its own locking).

	SSL_set_SSL_CTX(COpenSSL::AsSSL(pSSL), COpenSSL::AsSSLCTX(NewCtx));

	return SSL_TLSEXT_ERR_OK;
	}

void CSSLCtx::SetSNICallback (SSL_SNI_FUNC pfFunc, DWORD_PTR dwData)

//	SetSNICallback
//
//	Sets a callback for Server Name Indicator.

	{
	CSmartLock Lock(m_cs);

	if (m_pData == NULL)
		return;

	m_pData->pfOnSNI = pfFunc;
	m_pData->dwOnSNIData = dwData;

	SSL_CTX_set_tlsext_servername_arg(COpenSSL::AsSSLCTX(m_pData->pCtx), this);
	SSL_CTX_set_tlsext_servername_callback(COpenSSL::AsSSLCTX(m_pData->pCtx), OnServerNameIndicationThunk);
	}

bool CSSLCtx::SetCertificate (CSSLCert &Cert, CSSLEnvelopeKey &Key, CString *retsError)

//	SetCertificate
//
//	Sets the certificate and the corresponding private key.

	{
	if (m_pData == NULL)
		return false;

	//	This function makes copy of the X509 certificate, so we still need to
	//	free our copy.

	if (!SSL_CTX_use_certificate(COpenSSL::AsSSLCTX(m_pData->pCtx), COpenSSL::AsX509(Cert)))
		{
		if (retsError) *retsError = ERR_CANT_SET_CERTIFICATE;
		return false;
		}

	//	Add the private key that belongs to the certificate.

	if (!SSL_CTX_use_PrivateKey(COpenSSL::AsSSLCTX(m_pData->pCtx), COpenSSL::AsEVPPKey(Key)))
		{
		if (retsError) *retsError = ERR_CANT_SET_PRIVATE_KEY;
		return false;
		}

	//	Done

	return true;
	}
