//	FoundationNetworkIO.h
//
//	Foundation header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

struct SAddrInfo
	{
	int iFlags = 0;
	int iFamily = 0;
	int iSockType = 0;
	int iProtocol = 0;
	CString sName;

	void *pIterator = NULL;
	};

class CWSAddrInfo
	{
	public:
		CWSAddrInfo () { }
		CWSAddrInfo (const CWSAddrInfo &Src) = delete;
		CWSAddrInfo (CWSAddrInfo &&Src) noexcept { Move(Src); }

		~CWSAddrInfo () { CleanUp(); }

		operator const ADDRINFOW & () const { return CastADDRINFO(); }
		explicit operator bool () const noexcept { return (m_pAddrInfo != NULL); }

		CWSAddrInfo &operator= (const CWSAddrInfo &Src) = delete;
		CWSAddrInfo &operator= (CWSAddrInfo &&Src) noexcept { CleanUp(); Move(Src); return *this; }

		const ADDRINFOW &CastADDRINFO () const { if (!m_pAddrInfo) throw CException(errFail); return *m_pAddrInfo; }
		const ADDRINFOW *GetFirstIPInfo () const;
		bool GetNext (SAddrInfo &retInfo) const;

		static CWSAddrInfo Get (const CString &sHost, DWORD dwPort, CString *retsError = NULL);
		static CWSAddrInfo Get (DWORD dwPort, CString *retsError = NULL);
			
	private:
		void CleanUp () 
			{
			if (m_pAddrInfo)
				{
				::FreeAddrInfoW(m_pAddrInfo);
				m_pAddrInfo = NULL;
				}
			}

		void Move (CWSAddrInfo &Src) noexcept
			{
			m_pAddrInfo = Src.m_pAddrInfo;
			Src.m_pAddrInfo = NULL;
			}

		ADDRINFOW *m_pAddrInfo = NULL;
	};

class CWSAddrInfoStorage
	{
	public:
		operator const sockaddr & () const { return (const sockaddr &)m_Storage; }

		sockaddr &GetAddrRef () { return (sockaddr &)m_Storage; }
		int &GetAddrLenRef () { return m_iLength; }
		int GetLength () const { return m_iLength; }

	private:
		sockaddr_storage m_Storage;
		int m_iLength = sizeof(sockaddr_storage);
	};

class CSocket : public IByteStream
	{
	public:
		enum class EType
			{
			Unknown	=	0,
			TCP	=		1,
			UDP =		2,
			};

		CSocket (void) { }
		CSocket (const ADDRINFOW &AI);
		CSocket (const CSocket &Src) = delete;
		CSocket (CSocket &&Src) noexcept { Move(Src); }

		~CSocket (void);

		operator SOCKET () const { return m_hSocket; }

		CSocket &operator= (const CSocket &Src) = delete;
		CSocket &operator= (CSocket &&Src) noexcept { Close(); Move(Src); return *this; }

		bool AcceptConnection (CSocket &retSocket) const;
		void Close (void);
		static void CloseHandoffSocket (SOCKET hSocket);
		bool Connect (const CString &sHost, DWORD dwPort, EType iType = EType::TCP);
		bool Create (EType iType);
		void Disconnect (void);
		CString GetConnectionAddress (void) const;
		CString GetHostAddress (void) const;
		SOCKET Handoff (void) { SOCKET hTemp = m_hSocket; m_hSocket = INVALID_SOCKET; return hTemp; }
		bool IsValid (void) const { return m_hSocket != INVALID_SOCKET; }
		bool SelectWaitRead (void);
		bool SelectWaitWrite (void);
		void SetBlockingMode (bool bBlocking = true);

		static int GetGlobalSocketCount (void) { return m_iGlobalSocketCount; }
		static bool IsIPv6Addr (const CString &sHostName);
		static const CSocket &Null () { return m_Null; }

		//	IByteStream virtuals
		virtual int GetPos (void) override { return 0; }
		virtual int GetStreamLength (void) override { return 0; }
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override { }
		virtual int Write (const void *pData, int iLength) override;

		//	Helpers	(Needed because IByteStream::Write is hidden when we
		//	have the derived class)
		inline int Write (const CString &sString) { return Write((LPSTR)sString, sString.GetLength()); }

	private:
		bool Connect (SOCKADDR *pSockAddr, size_t iSockAddrLen);
		void Move (CSocket &Src) noexcept;
		static bool WSGetNameInfo (const SOCKADDR &SockAddr, size_t iSockAddrLen, bool bNumericHost = false, CString *retsHostname = NULL, CString *retsPort = NULL);
		bool WSGetAddressInfo (CWSAddrInfoStorage &retAI) const;

		SOCKET m_hSocket = INVALID_SOCKET;
		bool m_bConnected = false;
		bool m_bBlocking = true;

		static int m_iGlobalSocketCount;
		static const CSocket m_Null;
	};

class CSocketSet
	{
	public:
		bool AcceptConnection (CSocket &retSocket) const;
		void Close () { m_Sockets.DeleteAll(); FD_ZERO(&m_SockSet); }
		bool CreateListeners (DWORD dwPort, CSocket::EType iType = CSocket::EType::TCP);

	private:
		const CSocket &Select () const;

		TArray<CSocket> m_Sockets;
		mutable fd_set m_SockSet;
	};

//	HTTP -----------------------------------------------------------------------

enum EHTTPStatusCodes
	{
	http_CONTINUE =							100,
	http_SWITCHING_PROTOCOLS =				101,

	http_OK =								200,
	http_CREATED =							201,
	http_ACCEPTED =							202,
	http_NON_AUTHORITATIVE =				203,
	http_NO_CONTENT =						204,
	http_RESET_CONTENT =					205,
	http_PARTIAL_CONTENT =					206,

	http_MULTIPLE_CHOICES =					300,
	http_MOVED_PERMANENTLY =				301,
	http_FOUND =							302,
	http_SEE_OTHER =						303,
	http_NOT_MODIFIED =						304,
	http_USE_PROXY =						305,
	//	306 unused
	http_TEMPORARY_REDIRECT =				307,

	http_BAD_REQUEST =						400,
	http_UNAUTHORIZED =						401,
	//	402 reserved (Payment Required)
	http_FORBIDDEN =						403,
	http_NOT_FOUND =						404,
	http_NOT_ALLOWED =						405,
	http_NOT_ACCEPTABLE =					406,
	http_PROXY_AUTH_REQUIRED =				407,
	http_REQUEST_TIMEOUT =					408,
	http_CONFLICT =							409,
	http_GONE =								410,
	http_LENGTH_REQUIRED =					411,
	http_PRECONDITION_FAILED =				412,
	http_ENTITY_TOO_LARGE =					413,
	http_URI_TOO_LONG =						414,
	http_UNSUPPORTED_MEDIA_TYPE =			415,
	http_BAD_REQUEST_RANGE =				416,
	http_EXPECTATION_FAILED =				417,
	http_SHOULD_BE_ASHAMED_OF_YOURSELF =	418,

	http_INTERNAL_SERVER_ERROR =			500,
	http_NOT_IMPLEMENTED =					501,
	http_BAD_GATEWAY =						502,
	http_SERVICE_UNAVAILABLE =				503,
	http_GATEWAY_TIMEOUT =					504,
	http_VERSION_NOT_SUPPORTED =			505,
	};

enum EContentEncodingTypes
	{
	http_encodingIdentity,					//	No encoding
	http_encodingGzip,						//	gzip
	};

class IMediaType
	{
	public:
		inline IMediaType *AddRef (void) { m_dwRefCount++; return this; }
		inline void Delete (void) { if (--m_dwRefCount == 0) delete this; }

		virtual ~IMediaType (void) { }

		virtual bool DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer) = 0;
		virtual void EncodeContent (EContentEncodingTypes iEncoding) { }
		virtual bool EncodeToBuffer (IByteStream &Stream, DWORD dwOffset = 0, DWORD dwSize = 0xffffffff) const = 0;
		virtual const CString &GetMediaBuffer (void) const { return NULL_STR; }
		virtual EContentEncodingTypes GetMediaEncoding (void) const { return http_encodingIdentity; }
		const CString &GetMediaEncodingHeader (void) const;
		virtual DWORD GetMediaLength (void) const = 0;
		virtual const CString &GetMediaType (void) const = 0;

		static EContentEncodingTypes GetDefaultEncodingType (const CString &sMediaType);
		static CString MediaTypeFromExtension (const CString &sExtension);

	private:
		DWORD m_dwRefCount = 1;
	};

typedef TSharedPtr<IMediaType> IMediaTypePtr;

class IMediaTypeBuilder
	{
	public:
		IMediaTypeBuilder *AddRef (void) 
			{ 
			m_dwRefCount++; 
			return this; 
			}

		void Delete (void) 
			{ 
			if (--m_dwRefCount == 0) 
				delete this; 
			}

		virtual ~IMediaTypeBuilder (void) { }
		virtual void Append (void *pPos, int iLength) = 0;
		virtual bool CreateMedia (IMediaTypePtr *retpBody) = 0;
		virtual int GetLength (void) const = 0;
		virtual void Init (const CString &sMediaType) = 0;
		virtual bool IsEmpty (void) const { return (GetLength() == 0); }

	private:
		DWORD m_dwRefCount = 1;
	};

typedef TSharedPtr<IMediaTypeBuilder> IMediaTypeBuilderPtr;

class CHTTPMessageBodyBuilder : public IMediaTypeBuilder
	{
	public:
		CHTTPMessageBodyBuilder (void);

		//	IMediaTypeBuilder interface

		virtual void Append (void *pPos, int iLength) override;
		virtual bool CreateMedia (IMediaTypePtr *retpBody) override;
		virtual int GetLength (void) const override { return m_Body.GetLength(); }
		virtual void Init (const CString &sMediaType) override;

	private:
		CString m_sMediaType;
		CMemoryBuffer m_Body;
	};

class CHTTPMessage
	{
	public:
		enum Flags
			{
			//	WriteToBuffer
			FLAG_CHUNKED_ENCODING =			0x00000001,
			};

		struct SHeader
			{
			CString sField;
			CString sValue;
			};

		CHTTPMessage (void);

		void AddHeader (const CString &sField, const CString &sValue);
		void AddHeader (const CString &sField, const CDateTime &Value);
		inline void DeleteBodyBuilder (void) { m_pBodyBuilder = NULL; }
		bool Encode (EContentEncodingTypes iEncoding);
		bool FindHeader (const CString &sField, CString *retsValue = NULL) const;
		inline IMediaTypePtr GetBody (void) const { return m_pBody; }
		inline const CString &GetBodyBuffer (void) const { if (m_pBody) return m_pBody->GetMediaBuffer(); else return NULL_STR; }
		DWORD GetBodySize (void) const;
		CString GetCookie (const CString &sKey) const;
		EContentEncodingTypes GetDefaultEncoding (void) const;
		inline void GetHeader (int iIndex, CString *retsHeader, CString *retsValue = NULL) const { *retsHeader = m_Headers[iIndex].sField; if (retsValue) *retsValue = m_Headers[iIndex].sValue; }
		inline int GetHeaderCount (void) const { return m_Headers.GetCount(); }
		inline const CString &GetHTTPVersion (void) const { return m_sVersion; }
		inline const CString &GetMethod (void) const { return m_sMethod; }
		CString GetRequestedHost (void) const;
		CString GetRequestedPath (void) const;
		CString GetRequestedURL (void) const;
		inline DWORD GetStatusCode (void) const { return m_dwStatusCode; }
		inline const CString &GetStatusMsg (void) const { return m_sStatusMsg; }
		bool InitFromBuffer (const IMemoryBlock &Buffer, bool bNoBody = false);
		bool InitFromPartialBuffer (const IMemoryBlock &Buffer, bool bNoBody = false, TArray<CString> *pDebugOutput = NULL);
		void InitFromPartialBufferReset (IMediaTypeBuilderPtr pBodyBuilder = IMediaTypeBuilderPtr());
		bool InitFromSocket (SOCKET hSocket);
		bool InitFromStream (IByteStream &Stream);
		bool InitRequest (const CString &sMethod, const CString &sURL);
		bool InitResponse (DWORD dwStatusCode, const CString &sStatusMsg);
		bool IsEncodingAccepted (EContentEncodingTypes iEncoding);
		inline bool IsHTTP11 (void) const { return m_bHTTP11; }
		bool IsMessageComplete (void) const { return m_iState == stateDone; }
		bool IsMessagePartial (void) const { return (m_iState != stateDone && m_iState != stateStart); }
		bool ParseRequestedURL (CString *retsProtocol, CString *retsHost = NULL, CString *retsPath = NULL) const;
		void SetBody (IMediaTypePtr pBody);
		bool WriteChunkToBuffer (IByteStream &Stream, DWORD dwOffset, DWORD dwSize) const;
		bool WriteHeadersToBuffer (IByteStream &Stream, DWORD dwFlags = 0) const;
		bool WriteToBuffer (IByteStream &Stream) const;

		CString DebugGetInitState (void) const;

		static void ParseCookies (const CString &sValue, TSortMap<CString, CString> *retCookies);
		static bool ParseHeader (char *pPos, char *pEndPos, CString *retpField, CString *retpValue, char **retpPos, bool *retbHeadersDone);
		static bool ParseHeaderValue (const CString &sValue, CString *retsValue, TSortMap<CString, CString> *retFields);
		static CString StatusMessageFromStatusCode (DWORD dwStatusCode);

	private:
		enum States
			{
			stateStart,					//	Haven't parsed anything
			stateHeaders,				//	Parse the headers
			stateBody,					//	Parse the body
			stateChunk,					//	Parsing a chunk
			stateChunkEnd,				//	Parsing CRLF at end of chunk
			stateDone,					//	Found end of message
			};

		enum MessageTypes
			{
			typeUnknown,
			typeRequest,
			typeResponse,
			};

		struct SStatusMessageEntry
			{
			DWORD dwCode = 0;
			LPCSTR pszMessage = NULL;
			};

		bool ParseToken (char *pPos, char *pEndPos, char chDelimiter, char **retpPos, CString *retsToken) const;

		MessageTypes m_iType;
		CString m_sMethod;				//	If method is blank, then this is a response
		CString m_sURL;
		CString m_sVersion;
		DWORD m_dwStatusCode;
		CString m_sStatusMsg;
		TArray<SHeader> m_Headers;
		IMediaTypePtr m_pBody;
		bool m_bHTTP11;					//	If TRUE, HTTP/1.1

		//	Parse state
		States m_iState;
		int m_iChunkLeft;
		IMediaTypeBuilderPtr m_pBodyBuilder;
		CString m_sLeftOver;			//	Left over buffer from previous call

		static SStatusMessageEntry m_StatusMessageTable[];
	};

//	HTTP Media Types -----------------------------------------------------------

class CHTMLForm : public IMediaType
	{
	public:
		void AddField (const CString &sKey, const CString &sValue);

		//	IMediaType
		virtual bool DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer);
		virtual bool EncodeToBuffer (IByteStream &Stream, DWORD dwOffset = 0, DWORD dwSize = 0xffffffff) const;
		virtual DWORD GetMediaLength (void) const;
		virtual const CString &GetMediaType (void) const;

	private:
		struct SField
			{
			CString sKey;
			CString sValue;
			};

		bool EncodeText (IByteStream &Stream, const CString &sText) const;
		DWORD GetEncodedTextLength (const CString &sText) const;

		TArray<SField> m_Fields;
	};

class CMultipartData : public IMediaType
	{
	public:
		inline int GetCount (void) const { return m_Parts.GetCount(); }
		inline const CString &GetPartData (int iIndex) const { return m_Parts[iIndex].sData; }
		inline const CString &GetPartName (int iIndex) const { return m_Parts[iIndex].sName; }
		inline const CString &GetPartType (int iIndex) const { return m_Parts[iIndex].sContentType; }

		//	IMediaType
		virtual bool DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer);
		virtual bool EncodeToBuffer (IByteStream &Stream, DWORD dwOffset = 0, DWORD dwSize = 0xffffffff) const;
		virtual const CString &GetMediaBuffer (void) const { return NULL_STR; }
		virtual DWORD GetMediaLength (void) const { return 0; }
		virtual const CString &GetMediaType (void) const;

	private:
		struct SPart
			{
			CString sName;
			CString sContentType;
			CString sData;
			};

		bool ParseToBoundary (char *pPos, char *pPosEnd, const CString &sBoundary, CString *retsData, char **retpPos) const;

		TArray<SPart> m_Parts;
	};

class CRawMediaType : public IMediaType
	{
	public:
		CRawMediaType (void) : m_iEncoding(http_encodingIdentity)
			{ }

		//	IMediaType
		virtual bool DecodeFromBuffer (const CString &sMediaType, const IMemoryBlock &Buffer);
		virtual void EncodeContent (EContentEncodingTypes iEncoding);
		virtual bool EncodeToBuffer (IByteStream &Stream, DWORD dwOffset = 0, DWORD dwSize = 0xffffffff) const;
		virtual const CString &GetMediaBuffer (void) const { return m_sBody; }
		virtual EContentEncodingTypes GetMediaEncoding (void) const { return m_iEncoding; }
		virtual DWORD GetMediaLength (void) const;
		virtual const CString &GetMediaType (void) const;

	private:
		CString m_sMediaType;
		CString m_sBody;
		EContentEncodingTypes m_iEncoding;
	};

//	Utilities ------------------------------------------------------------------

void htmlWriteAttributeValue (const CString &sText, IByteStream &Output);
CString htmlWriteAttributeValue (const CString &sText);
void htmlWriteText (char *pPos, char *pPosEnd, IByteStream &Output);
void htmlWriteText (const CString &sText, IByteStream &Output);
CString htmlWriteText (const CString &sText);
CString urlAppend (const CString &sPath, const CString &sComponent);
CString urlDecode (const CString &sValue);
CString urlEncodeParam (const CString &sValue);
DWORD urlGetDefaultPort (const CString &sProtocol);
bool urlMatchPattern (const CString &sPattern, const CString &sURL);
bool urlParse (char *pStart, CString *retsProtocol = NULL, CString *retsHost = NULL, CString *retsPath = NULL, char **retpEnd = NULL);
bool urlParseHostPort (const CString &sProtocol, const CString &sHost, CString *retsHostname, DWORD *retdwPort = NULL);
