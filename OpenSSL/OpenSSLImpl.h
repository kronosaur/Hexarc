//	OpenSSLImpl.h
//
//	OpenSSL Classes and Utilities
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include <openssl/ssl.h>

class CMemoryBIO
	{
	public:
		CMemoryBIO (void);
		CMemoryBIO (const CString &sData);
		CMemoryBIO (const IMemoryBlock &Data);
		inline ~CMemoryBIO (void) { CleanUp(); }

		inline operator BIO * () const { return m_pBIO; }

		inline BIO *GetHandoff (void) { BIO *pTemp = m_pBIO; m_pBIO = NULL; return pTemp; }
		bool Write (const CString &sData);
		bool Write (const IMemoryBlock &Data);

		//	We don't support copy/assign

		CMemoryBIO (const CMemoryBIO &Src) = delete;
		inline CMemoryBIO &operator= (const CMemoryBIO &Src) = delete;

	private:
		void CleanUp (void);
		void Create (void);

		BIO *m_pBIO;
	};

class COpenSSL
	{
	public:
		COpenSSL (void) :
				m_bInitialized(false),
				m_pCtx(NULL)
			{ }

		~COpenSSL (void);

		OpenSSL_SSLPtr AllocSession (void);
		void FreeSession (OpenSSL_SSLPtr pSession);
		bool Init (CString *retsError = NULL);

		//	Conversions to OpenSSL native types

		static BIO *AsBIO (OpenSSL_BIOPtr pData) { return reinterpret_cast<BIO *>((void *)pData); }
		static EVP_PKEY *AsEVPPKey (OpenSSL_EKeyPtr pData) { return reinterpret_cast<EVP_PKEY *>((void *)pData); }
		static SSL *AsSSL (OpenSSL_SSLPtr pData) { return reinterpret_cast<SSL *>((void *)pData); }
		static SSL_CTX *AsSSLCTX (OpenSSL_SSLCtxPtr pData) { return reinterpret_cast<SSL_CTX *>((void *)pData); }
		static X509 *AsX509 (OpenSSL_X509Ptr pData) { return reinterpret_cast<X509 *>((void *)pData); }
		static X509_NAME *AsX509Name (OpenSSL_X509NamePtr pData) { return reinterpret_cast<X509_NAME *>((void *)pData); }

		static STACK_OF(X509) *AsX509Stack (OpenSSL_X509StackPtr pData) { return reinterpret_cast<STACK_OF(X509) *>((void *)pData); }

		static CString AsString (ASN1_STRING *pString);

	private:
		void CleanUp (void);

		static unsigned long GetThreadID (void);
		static void LockMutex (int iMode, int n, const char * lpFilename, int iLine);

		CCriticalSection m_cs;
		bool m_bInitialized;

		TArray<CMutex> m_Mutex;
		SSL_CTX *m_pCtx;
	};

extern COpenSSL g_SSLGlobal;
