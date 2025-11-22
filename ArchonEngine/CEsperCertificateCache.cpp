//	CEsperCertificateCache.cpp
//
//	CEsperCertificateCache class
//	Copyright (c) 2017 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool CEsperCertificateCache::AddCertificate (const CString &sDomain, CDatum dCert, CString *retsError)

//	AddCertificate
//
//	Adds a certificate from a Cryptosaur structure

	{
	return false;
	}

bool CEsperCertificateCache::AddCertificate (const CString &sDomain, CSSLCert &Cert, CSSLEnvelopeKey &Key, CSSLCtx *retCtx, CString *retsError)

//	AddCertificate
//
//	Adds a certificate and returns the context

	{
	CSmartLock Lock(m_cs);

	//	Check to see if some other thread already added the certificate.

	if (GetSSLCtx(sDomain, retCtx))
		return true;

	//	Create an SSL context with the appropriate certificates

	CSSLCtx NewCtx;
	if (!NewCtx.Init(retsError))
		return false;

	if (!NewCtx.SetCertificate(Cert, Key, retsError))
		return false;

	//	Set the entry

	m_Certs.SetAt(sDomain, NewCtx);

	//	Return it, if necessary
	
	if (retCtx)
		*retCtx = NewCtx;

	return true;
	}

bool CEsperCertificateCache::GetSSLCtx (const CString &sDomain, CSSLCtx *retCtx) const

//	GetSSLCtx
//
//	Looks up the domain and returns an SSL context set up for that domain.

	{
	CSmartLock Lock(m_cs);

	CSSLCtx *pCtx = m_Certs.GetAt(sDomain);
	if (pCtx == NULL)
		return false;

	//	LATER: Check to see if it has expired, if it has, then we return false.


	//	Return it.

	*retCtx = *pCtx;
	return true;
	}
