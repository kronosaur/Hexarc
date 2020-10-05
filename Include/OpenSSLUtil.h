//	OpenSSLUtil.h
//
//	OpenSSL Classes and Utilities
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	1. Requires Foundation
//	3. Include OpenSSLLib.h
//	4. Link with OpenSSL.lib
//
//	NOTES
//
//	This project requires that we compile OpenSSL static libraries. Use the 
//	instructions in INSTALL.W64 to compile it.
//
//	After a successful compilation of OpenSSL, copy the following files:
//
//	{openssl}\inc32\openssl\*.*		-> Etherium\Include\openssl
//	{openssl}\out32\libeay32.lib	-> Etherium\Lib\openssl
//	{openssl}\out32\ssleay32.lib	-> Etherium\Lib\openssl
//	{openssl}\tmp32\lib.pdb			-> Etherium\Lib\openssl
//
//	Note that we currently don't separate 32-bit from 64-bit libraries, so if
//	compiling for 32-bits, these files need to be recopied.
//
//	OpenSSL compiled on 9/13/2015 from version 1.0.2d

#pragma once

#include "Foundation.h"

class CSSLCtx;

//	These pointer types hide the real pointer defined by OpenSSL. There are
//	internal functions to cast these to the real OpenSSL types.

template <class VALUE> class TOpenSSL_Ptr
	{
	public:
		TOpenSSL_Ptr (void *pPtr = NULL) :
				m_pPtr(pPtr)
			{ }

		inline operator void * () const { return m_pPtr; }
		inline TOpenSSL_Ptr<VALUE> &operator = (void *pSrc) { m_pPtr = pSrc; return *this; }

	private:
		void *m_pPtr;
	};

class OpenSSL_BIOClass { };			typedef TOpenSSL_Ptr<OpenSSL_BIOClass> OpenSSL_BIOPtr;
class OpenSSL_EKeyClass { };		typedef TOpenSSL_Ptr<OpenSSL_EKeyClass> OpenSSL_EKeyPtr;
class OpenSSL_SSLClass { };			typedef TOpenSSL_Ptr<OpenSSL_SSLClass> OpenSSL_SSLPtr;
class OpenSSL_SSLCtxClass { };		typedef TOpenSSL_Ptr<OpenSSL_SSLCtxClass> OpenSSL_SSLCtxPtr;
class OpenSSL_X509Class { };		typedef TOpenSSL_Ptr<OpenSSL_X509Class> OpenSSL_X509Ptr;
class OpenSSL_X509NameClass { };	typedef TOpenSSL_Ptr<OpenSSL_X509NameClass> OpenSSL_X509NamePtr;

class OpenSSL_X509Stack { };		typedef TOpenSSL_Ptr<OpenSSL_X509Stack> OpenSSL_X509StackPtr;

class CDistinguishedName
	{
	public:
		enum EPart
			{
			unknownPart,

			C,								//	Country
			CN,								//	Common name
			L,								//	Locality (City)
			O,								//	Organization
			OU,								//	Organizational unit
			ST,								//	State

			emailAddress,					//	Email address extension
			};

		CDistinguishedName (void) { }
		CDistinguishedName (const CString &sName);
		CDistinguishedName (OpenSSL_X509NamePtr pName);

		bool operator== (const CDistinguishedName &Src) const;
		bool operator!= (const CDistinguishedName &Src) const;

		void CopyTo (OpenSSL_X509NamePtr pName) const;
		CString GetDN (void) const;
		CString GetPart (EPart iPart) const;
		inline bool IsEmpty (void) const { return (m_DN.GetCount() == 0); }

		static void DeleteAll (OpenSSL_X509NamePtr pName);
		static const CString &GetPartName (EPart iPart);
		static const CDistinguishedName &Null (void) { return m_Null; }
		static EPart ParsePart (const CString &sPart);

	private:
		struct SPart
			{
			EPart iPart;
			CString sField;
			CString sFieldLong;
			CString sValue;
			};

		static bool AddPart (const CString &sPart, const CString &sValue, TArray<SPart> &DN);
		static bool Parse (const CString &sName, TArray<SPart> &DN);
		static CString ParseValue (const CString &sValue);

		TArray<SPart> m_DN;

		static CString EncodeValue (const CString &sValue);

		static CDistinguishedName m_Null;
	};

class CSSLEnvelopeKey
	{
	public:
		CSSLEnvelopeKey (void) : m_pData(NULL) { }
		CSSLEnvelopeKey (const CSSLEnvelopeKey &Src) { Copy(Src); }
		~CSSLEnvelopeKey (void) { CleanUp(); }

		inline operator OpenSSL_EKeyPtr () const { return (m_pData ? m_pData->pKey : NULL); }

		CSSLEnvelopeKey &operator= (const CSSLEnvelopeKey &Src) { CleanUp(); Copy(Src); return *this; }

		const CString &GetPEMData (void) const { return (m_pData ? m_pData->sPEMData : NULL_STR); }
		bool Init (CString *retsError = NULL);
		bool InitFromPEM (IMemoryBlock &Data, const CString &sPassphrase, bool bKeepRaw, CString *retsError = NULL);
		inline bool IsEmpty (void) const { return m_pData == NULL; }

		static bool IsKeyPEMSection (const CString &sType);

	private:
		struct SEKey
			{
			SEKey (void) :
					pKey(NULL),
					iRefCount(1)
				{ }

			OpenSSL_EKeyPtr pKey;
			int iRefCount;

			CString sPEMData;
			};

		void CleanUp (void);
		void Copy (const CSSLEnvelopeKey &Src);

		SEKey *m_pData;

		static CCriticalSection m_cs;
	};

class CSSLCert
	{
	public:
		struct SDesc
			{
			CDistinguishedName Subject;
			CDistinguishedName Issuer;
			CTimeSpan ValidTime;			//	Valid for this amount of time
			CSSLEnvelopeKey Key;			//	Key for certificate
			};

		CSSLCert (void);
		CSSLCert (const CSSLCert &Src) { Copy(Src); }
		explicit CSSLCert (OpenSSL_X509Ptr pX509);
		~CSSLCert (void) { CleanUp(); }

		inline operator OpenSSL_X509Ptr () const { return (m_pData ? m_pData->pX509 : NULL); }

		CSSLCert &operator= (const CSSLCert &Src) { CleanUp(); Copy(Src); return *this; }

		const CDistinguishedName &GetIssuer (void) const;
		const CString &GetPEMData (void) const { return (m_pData ? m_pData->sPEMData : NULL_STR); }
		const CDistinguishedName &GetSubject (void) const;
		bool InitFromPEM (IMemoryBlock &Data, bool bKeepRaw, CString *retsError);
		bool InitSelfSigned (const SDesc &Desc, CString *retsError = NULL);

		static bool HasPEMSection (IMemoryBlock &Data, int iOffset);
		static bool IsCertificateSection (const CString &sType);
		static bool ParsePEMSection (IMemoryBlock &Data, int iOffset, CString *retsData, CString *retsType = NULL, int *retiEnd = NULL);
		static void SortChain (TArray<CSSLCert> &Chain);

	private:
		struct SCert
			{
			SCert (void) :
					pX509(NULL),
					iRefCount(1)
				{ }

			OpenSSL_X509Ptr pX509;
			int iRefCount;

			CDistinguishedName Subject;
			CDistinguishedName Issuer;

			CString sPEMData;
			};

		void CleanUp (void);
		void Copy (const CSSLCert &Src);

		SCert *m_pData;

		static CCriticalSection m_cs;

	friend class CSSLCertStack;
	};

class CSSLCertStack
	{
	public:
		CSSLCertStack (void);
		CSSLCertStack (const CSSLCertStack &Src) { Copy(Src); }
		~CSSLCertStack (void) { CleanUp(); }

		CSSLCertStack &operator= (const CSSLCertStack &Src) { CleanUp(); Copy(Src); return *this; }

		CSSLCert GetCertificate (int iIndex) const;
		int GetCount (void) const;
		bool InitFromPEM (IMemoryBlock &Data, CString *retsError = NULL);

	private:
		struct SStack
			{
			OpenSSL_X509StackPtr pStack;
			int iRefCount;

			//	We keep a parallel set of pointers so we can return our own 
			//	object types. We don't release the refcounts on these certs until
			//	we free this structure.
			
			TArray<CSSLCert> Stack;
			};

		void CleanUp (void);
		void Copy (const CSSLCertStack &Src);

		SStack *m_pData;

		static CCriticalSection m_cs;
	};

typedef bool (*SSL_SNI_FUNC) (DWORD_PTR dwData, const CString &sServerName, CSSLCtx *retNewCtx);

class CSSLCtx
	{
	public:
		CSSLCtx (void);
		CSSLCtx (const CSSLCtx &Src) { Copy(Src); }
		~CSSLCtx (void) { CleanUp(); }

		inline operator OpenSSL_SSLCtxPtr () const { return (m_pData ? m_pData->pCtx : NULL); }

		CSSLCtx &operator= (const CSSLCtx &Src) { CleanUp(); Copy(Src); return *this; }

		OpenSSL_SSLPtr AllocSSL (void);
		void FreeSSL (OpenSSL_SSLPtr pSSL);
		bool Init (CString *retsError = NULL);
		inline bool IsEmpty (void) const { return (m_pData == NULL); }
		bool SetCertificate (CSSLCert &Cert, CSSLEnvelopeKey &Key, CString *retsError = NULL);
		void SetSNICallback (SSL_SNI_FUNC pfFunc, DWORD_PTR dwData);

	private:
		struct SSSLCtx
			{
			SSSLCtx (void) :
					pCtx(NULL),
					iRefCount(1),
					pfOnSNI(NULL),
					dwOnSNIData(0)
				{ }

			OpenSSL_SSLCtxPtr pCtx;
			int iRefCount;

			SSL_SNI_FUNC pfOnSNI;
			DWORD_PTR dwOnSNIData;
			};

		void CleanUp (void);
		void Copy (const CSSLCtx &Src);
		int OnServerNameIndication (OpenSSL_SSLPtr pSSL, int *ad, DWORD_PTR dwData);

		SSSLCtx *m_pData;

		static CCriticalSection m_cs;
		static int OnServerNameIndicationThunk (OpenSSL_SSLPtr pSSL, int *ad, DWORD_PTR dwData) { return ((CSSLCtx *)dwData)->OnServerNameIndication(pSSL, ad, dwData); }
	};

class CSSLAsyncEngine
	{
	public:
		enum EResults
			{
			resNone,
			resError,						//	An error happened

			resConnect,						//	Not yet connected. Call Connect.
			resSendData,					//	Call ProcessSendData to get data to send
			resReceiveData,					//	Call ProcessReceiveData to give data to engine
			resReady,						//	Ready to send or receive data (call Send or Receive)
			};

		struct SConnectionStatus
			{
			CString sProtocol;				//	"SSLv3", "TLSv1", etc.
			CString sCipherName;			//	"RC4-MD5", etc.
			CString sKeyExchange;			//	"DH", "RSA", etc.
			CString sAuthentication;		//	"RSA", etc.
			CString sEncryption;			//	"RC4(128)"
			CString sMAC;					//	"MD5", "SHA1"
			};

		CSSLAsyncEngine (CSSLCtx *pSSLCtx = NULL);
		~CSSLAsyncEngine (void);

		void Accept (void);
		void Connect (void);
		inline const IMemoryBlock &GetBuffer (void) const { return m_Buffer; }
		int GetInternalState (void) const { return (int)m_iState; }
		void Send (IMemoryBlock &Data);
		void Receive (void);

		EResults Process (CString *retsError);
		bool ProcessHasDataToSend (void);
		void ProcessSendData (IByteStream &Data);
		void ProcessReceiveData (IMemoryBlock &Data);

		bool GetConnectionStatus (SConnectionStatus *retStatus) const;

	private:
		enum EStates
			{
			stateNone,
			stateError,

			stateAccepting,					//	Accepting from a client
			stateConnecting,				//	Connect to server
			stateWriting,					//	Writing data
			stateReading,					//	Reading data
			stateReady,
			};

		bool Init (bool bAsServer, CString *retsError);

		EStates m_iState;
		CSSLCtx *m_pSSLCtx;					//	SSL_CTX object
		OpenSSL_SSLPtr m_pSSL;				//	SSL session object
		OpenSSL_BIOPtr m_pInput;			//	SSL reads from this object
		OpenSSL_BIOPtr m_pOutput;			//	SSL writes to this object
		CString m_sError;

		CBuffer m_Buffer;
	};

class CSSLSocketStream : public IByteStream
	{
	public:
		CSSLSocketStream (void) : 
				m_bConnected(false)
			{ }

		~CSSLSocketStream (void);

		bool Connect (const CString &sHostname, DWORD dwPort, bool bUseSSL, CString *retsError);
		inline bool Connect (const CString &sHostname, DWORD dwPort, CString *retsError = NULL) { return Connect(sHostname, dwPort, true, retsError); }
		void Disconnect (void);

		//	IByteStream interface

		virtual int GetPos (void) override { throw CException(errFail); }
		virtual int GetStreamLength (void) override { throw CException(errFail); }
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override { throw CException(errFail); }
		virtual int Write (const void *pData, int iLength) override;

	private:
		bool SelectWait (int iResult);

		CSocket m_Socket;
		bool m_bConnected;

		OpenSSL_SSLPtr m_pSSL;
		OpenSSL_BIOPtr m_pBIO;
	};

CHTTPMessage httpRequest (const CString &sURL, const CString &sMethod, const TSortMap<CString, CString> *pHeaders = NULL, const IMemoryBlock *pBuffer = NULL);
