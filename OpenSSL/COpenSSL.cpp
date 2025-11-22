//	COpenSSL.cpp
//
//	COpenSSL class
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#pragma warning(disable:4099)			//	No need for a delete because we're placing object

DECLARE_CONST_STRING(ERR_UNABLE_TO_CREATE_MUTEX,		"Unable to create mutex.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_INITIALIZE,			"Unable to initialize OpenSSL.")

COpenSSL g_SSLGlobal;

COpenSSL::~COpenSSL (void)

//	COpenSSL destructor

	{
	CleanUp();
	}

OpenSSL_SSLPtr COpenSSL::AllocSession (void)

//	AllocSession
//
//	Allocates a session. Return NULL if allocation fails.

	{
	if (!Init())
		return NULL;

	void *pSession = SSL_new(m_pCtx);
	if (pSession == NULL)
		return NULL;

	return pSession;
	}

CString COpenSSL::AsString (ASN1_STRING *pString)

//	AsString
//
//	Converts an ASN1 string to a UTF-8 string.

	{
	BYTE *pUTF8;
	int iLen = ASN1_STRING_to_UTF8(&pUTF8, pString);
	if (iLen <= 0)
		return NULL_STR;

	//	Make a copy and free the original

	CString sUTF8((char *)pUTF8, iLen);
	OPENSSL_free(pUTF8);

	//	Done

	return sUTF8;
	}

void COpenSSL::CleanUp (void)

//	CleanUp
//
//	Free up all OpenSSL resources

	{
	CSmartLock Lock(m_cs);

	//	If already cleaned up, we're done

	if (!m_bInitialized)
		return;

	//	Free SSL ctx

	SSL_CTX_free(m_pCtx);
	m_pCtx = NULL;

	//	Clean up the callbacks

	CRYPTO_set_id_callback(NULL);
	CRYPTO_set_locking_callback(NULL);

	//	Done

	m_bInitialized = false;
	}

void COpenSSL::FreeSession (OpenSSL_SSLPtr pSession)

//	FreeSession
//
//	Discard the session

	{
	if (pSession)
		SSL_free(AsSSL(pSession));
	}

unsigned long COpenSSL::GetThreadID (void)

//	GetThreadID
//
//	Returns the current thread's ID

	{
	return ::GetCurrentThreadId();
	}

bool COpenSSL::Init (CString *retsError)

//	Init
//
//	Initialize all OpenSSL resources

	{
	CSmartLock Lock(m_cs);
	int i;

	//	If we're already initialized, then we're done

	if (m_bInitialized)
		return true;

	//	Initialize locks for OpenSSL (so that it can be thread-safe).

	m_Mutex.InsertEmpty(CRYPTO_num_locks());
	for (i = 0; i < m_Mutex.GetCount(); i++)
		{
		try 
			{
			m_Mutex[i].Create();
			}
		catch (...)
			{
			if (retsError) *retsError = ERR_UNABLE_TO_CREATE_MUTEX;
			return false;
			}
		}

	//	Set callback functions for threading

	CRYPTO_set_id_callback(GetThreadID);
	CRYPTO_set_locking_callback(LockMutex);

	//	Initialize the library

	if (!SSL_library_init())
		{
		CRYPTO_set_id_callback(NULL);
		CRYPTO_set_locking_callback(NULL);

		if (retsError) *retsError = ERR_UNABLE_TO_INITIALIZE;
		return false;
		}

	const SSL_METHOD *meth = SSLv23_method();
	SSL_load_error_strings();
	m_pCtx = SSL_CTX_new (meth);

	//	Set options to deprecate SSL methods which have security problems.
	//	See: https://raymii.org/s/tutorials/Strong_SSL_Security_On_Apache2.html

	const long flags = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_COMPRESSION;
	SSL_CTX_set_options(m_pCtx, flags);

	//	Restrict the set of ciphers used to protect against various attacks.
	//	See: http://stackoverflow.com/questions/30319394/openssl-certificate-generation-for-dhe-exchange/30332153#30332153

	SSL_CTX_set_cipher_list(m_pCtx, "HIGH:!aNULL:!RC4:!MD5");

	//	Done

	m_bInitialized = true;
	return true;
	}

void COpenSSL::LockMutex (int iMode, int n, const char * lpFilename, int iLine)

//	LockMutex
//
//	Locks or unlocks the nth mutex

	{
	if (iMode & CRYPTO_LOCK)
		g_SSLGlobal.m_Mutex[n].Lock();
	else
		g_SSLGlobal.m_Mutex[n].Unlock();
	}

