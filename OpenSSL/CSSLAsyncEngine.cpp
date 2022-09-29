//	CSSLAsyncEngine.cpp
//
//	CSSLAsyncEngine class
//	Copyright (c) 2014 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_AU_PREFIX,					"Au=");
DECLARE_CONST_STRING(FIELD_ENC_PREFIX,					"Enc=");
DECLARE_CONST_STRING(FIELD_KX_PREFIX,					"Kx=");
DECLARE_CONST_STRING(FIELD_MAC_PREFIX,					"Mac=");

DECLARE_CONST_STRING(ERR_CONNECT_INVALID_STATE,			"Connect: Invalid state.");
DECLARE_CONST_STRING(ERR_PROCESS_INVALID_STATE,			"Process: Invalid state.");
DECLARE_CONST_STRING(ERR_OUT_OF_MEMORY,					"SSL: Out of memory.");
DECLARE_CONST_STRING(ERR_CONNECT_FAILED,				"SSL: Connect failed: %x.");
DECLARE_CONST_STRING(ERR_READ_FAILED,					"SSL: Read failed: %x.");
DECLARE_CONST_STRING(ERR_WRITE_FAILED,					"SSL: Write failed: %x.");
DECLARE_CONST_STRING(ERR_ACCEPT_FAILED,					"SSL: Accept failed error = %d.");

const int BUFFER_SIZE =									16 * 1024;

CSSLAsyncEngine::CSSLAsyncEngine (CSSLCtx *pSSLCtx) :
		m_pSSLCtx(pSSLCtx),
		m_Buffer(BUFFER_SIZE)

//	CSSLAsyncEngine constructor

	{
	}

CSSLAsyncEngine::~CSSLAsyncEngine (void)

//	CSSLAsyncEngine destructor

	{
	if (m_pSSL)
		{
		if (m_pSSLCtx) m_pSSLCtx->FreeSSL(m_pSSL); else g_SSLGlobal.FreeSession(m_pSSL);
		m_pSSL = NULL;
		}
	}

void CSSLAsyncEngine::Accept (void)

//	Accept
//
//	Asks the engine to set itself to accept mode

	{
	switch (m_iState)
		{
		case stateNone:
			DebugLog("SSL State = stateAccepting");

			m_iState = stateAccepting;
			break;

		default:
			DebugLog("SSL State = stateError");

			m_iState = stateError;
			m_sError = ERR_CONNECT_INVALID_STATE;
			break;
		}
	}

void CSSLAsyncEngine::Connect (void)

//	Connect
//
//	Asks the engine to connect

	{
	switch (m_iState)
		{
		case stateNone:
			DebugLog("SSL State = stateConnecting");

			m_iState = stateConnecting;
			break;

		default:
			DebugLog("SSL State = stateError");

			m_iState = stateError;
			m_sError = ERR_CONNECT_INVALID_STATE;
			break;
		}
	}

#ifdef DEBUG
void CSSLAsyncEngine::DebugLog (const CString& sLine) const

//	DebugLog
//
//	Output log.

	{
	if (m_bDebugLog)
		printf("%s\n", (LPSTR)sLine);
	}
#endif

bool CSSLAsyncEngine::GetConnectionStatus (SConnectionStatus *retStatus) const

//	GetConnectionStatus
//
//	Returns the connection status. If we return FALSE, we have not yet 
//	connected.

	{
	int i;

	if (m_pSSL == NULL)
		return false;

	if (m_iState == stateNone 
			|| m_iState == stateConnecting 
			|| m_iState == stateError)
		return false;

	retStatus->sProtocol = CString(SSL_get_version(COpenSSL::AsSSL(m_pSSL)));
	
	const SSL_CIPHER *cipher = SSL_get_current_cipher(COpenSSL::AsSSL(m_pSSL));
	retStatus->sCipherName = CString(cipher->name);

	CString sDesc(256);
	SSL_CIPHER_description(cipher, sDesc.GetPointer(), sDesc.GetLength());

	TArray<CString> Fields;
	strSplit(sDesc, NULL_STR, &Fields, -1, SSP_FLAG_WHITESPACE_SEPARATOR);

	for (i = 0; i < Fields.GetCount(); i++)
		{
		if (strStartsWith(Fields[i], FIELD_AU_PREFIX))
			retStatus->sAuthentication = strSubString(Fields[i], FIELD_AU_PREFIX.GetLength());
		else if (strStartsWith(Fields[i], FIELD_ENC_PREFIX))
			retStatus->sEncryption = strSubString(Fields[i], FIELD_ENC_PREFIX.GetLength());
		else if (strStartsWith(Fields[i], FIELD_KX_PREFIX))
			retStatus->sKeyExchange = strSubString(Fields[i], FIELD_KX_PREFIX.GetLength());
		else if (strStartsWith(Fields[i], FIELD_MAC_PREFIX))
			retStatus->sMAC = strSubString(Fields[i], FIELD_MAC_PREFIX.GetLength());
		}

	//	Done

	return true;
	}

bool CSSLAsyncEngine::Init (bool bAsServer, CString *retsError)

//	Init
//
//	Initializes m_pSSL

	{
	if (m_pSSL)
		return true;

	ASSERT(m_pInput == NULL);
	ASSERT(m_pOutput == NULL);

	if (m_pSSLCtx)
		m_pSSL = m_pSSLCtx->AllocSSL();
	else
		m_pSSL = g_SSLGlobal.AllocSession();
	if (m_pSSL == NULL)
		{
		if (retsError) *retsError = ERR_OUT_OF_MEMORY;
		return false;
		}

	//	Allocate input and output memory buffers

	m_pInput = BIO_new(BIO_s_mem());
	if (m_pInput == NULL)
		{
		if (m_pSSLCtx) m_pSSLCtx->FreeSSL(m_pSSL); else g_SSLGlobal.FreeSession(m_pSSL);
		m_pSSL = NULL;

		if (retsError) *retsError = ERR_OUT_OF_MEMORY;
		return false;
		}

	m_pOutput = BIO_new(BIO_s_mem());
	if (m_pOutput == NULL)
		{
		BIO_free_all(COpenSSL::AsBIO(m_pInput));
		m_pInput = NULL;

		if (m_pSSLCtx) m_pSSLCtx->FreeSSL(m_pSSL); else g_SSLGlobal.FreeSession(m_pSSL);
		m_pSSL = NULL;

		if (retsError) *retsError = ERR_OUT_OF_MEMORY;
		return false;
		}

	//	Associate the buffers. After this, m_pvSSL takes 
	//	ownership of the input/output buffers.

	SSL_set_bio(COpenSSL::AsSSL(m_pSSL), COpenSSL::AsBIO(m_pInput), COpenSSL::AsBIO(m_pOutput));

	//	Done

	return true;
	}

CSSLAsyncEngine::EResults CSSLAsyncEngine::Process (CString *retsError)

//	Process
//
//	The engine processes until it blocks and returns a result indicating
//	what the engine needs to unblock.

	{
	//	Continue processing until we block.

	while (true)
		{
		switch (m_iState)
			{
			case stateNone:
				DebugLog("SSL State = stateConnecting");

				m_iState = stateConnecting;
				continue;

			case stateAccepting:
				{
				//	Allocate our session object

				if (!Init(true, retsError))
					return resError;

				//	Connect

				int err = SSL_accept(COpenSSL::AsSSL(m_pSSL));
				if (err > 0)
					{
					DebugLog("SSL State = stateReady");

					m_iState = stateReady;
					return resReady;
					}

				//	Otherwise, we need data to complete the handshake

				else
					{
					int iError = SSL_get_error(COpenSSL::AsSSL(m_pSSL), err);
					if (iError == SSL_ERROR_WANT_READ)
						return resReceiveData;
					else if (iError == SSL_ERROR_WANT_WRITE)
						return resSendData;
					else
						{
						if (retsError) *retsError = strPattern(ERR_ACCEPT_FAILED, iError);
						return resError;
						}
					}
				}

			case stateConnecting:
				{
				//	Allocate our session object

				if (!Init(false, retsError))
					return resError;

				//	Make sure we set the host name on the SSL object because we need 
				//	it in the TLS handshake for SNI.

				if (!m_sHostname.IsEmpty())
					{
					int res = SSL_set_tlsext_host_name(COpenSSL::AsSSL(m_pSSL), (LPSTR)m_sHostname);
					}

				//	Connect

				int err = SSL_connect(COpenSSL::AsSSL(m_pSSL));
				if (err > 0)
					{
					DebugLog("SSL State = stateReady");

					m_iState = stateReady;
					return resReady;
					}

				//	Otherwise, we need data to complete the handshake

				else
					{
					int iError = SSL_get_error(COpenSSL::AsSSL(m_pSSL), err);
					if (iError == SSL_ERROR_WANT_READ)
						return resReceiveData;
					else if (iError == SSL_ERROR_WANT_WRITE)
						return resSendData;
					else
						{
						if (retsError) *retsError = strPattern(ERR_CONNECT_FAILED, iError);
						return resError;
						}
					}
				}

			case stateReading:
				{
				m_Buffer.SetLength(BUFFER_SIZE);
				int iBytesRead = SSL_read(COpenSSL::AsSSL(m_pSSL), m_Buffer.GetPointer(), m_Buffer.GetLength());

				DebugLog(strPattern("SSL_read: %d bytes (buffer size=%d)", iBytesRead, m_Buffer.GetLength()));

				if (iBytesRead > 0)
					{
					m_bReadRetried = false;
					m_Buffer.SetLength(iBytesRead);
					return resReady;
					}

				//	Otherwise, we need data to complete read

				else
					{
					int iError = SSL_get_error(COpenSSL::AsSSL(m_pSSL), iBytesRead);
					switch (iError)
						{
						case SSL_ERROR_WANT_READ:
							{
							m_bReadRetried = false;
							DebugLog("SSL wants to read");
							return resReceiveData;
							}

						case SSL_ERROR_WANT_WRITE:
							{
							m_bReadRetried = false;
							DebugLog("SSL wants to write");
							return resSendData;
							}

						case SSL_ERROR_SSL:
							{
							//	LATER: Under certain conditions, we get this 
							//	error. Empirically, the error goes away if we
							//	read more data from the socket and pass it to 
							//	the SSL engine. It's almost as if SSL_read
							//	has a bug in which it returns this error instead
							//	of SSL_ERROR_WANT_READ.
							//
							//	It's also possible/likely that we're not calling
							//	SSL correctly and are missing some signal that 
							//	we should read more. But for now this is the 
							//	best fix I have.

							if (!m_bReadRetried)
								{
								DebugLog("SSL_ERROR_SSL, reading more...\n");
								m_bReadRetried = true;
								return resReceiveData;
								}
							else
								{
								CStringBuffer Buffer;

								int iMoreError;
								while (iMoreError = ERR_get_error())
									{
									if (Buffer.GetLength() > 0)
										Buffer.Write("\n");

									CString sError(ERR_reason_error_string(iMoreError));
									Buffer.Write(sError);
									}

								if (retsError) *retsError = CString(std::move(Buffer));
								return resError;
								}
							}

						default:
							if (retsError) *retsError = strPattern(ERR_READ_FAILED, iError);
							return resError;
						}
					}
				}

			case stateWriting:
				{
				int iBytesWritten = SSL_write(COpenSSL::AsSSL(m_pSSL), m_Buffer.GetPointer(), m_Buffer.GetLength());

				DebugLog(strPattern("SSL_write: %d bytes (wrote %d)\n", m_Buffer.GetLength(), iBytesWritten));
			
				if (iBytesWritten > 0)
					return resReady;

				//	Otherwise, we need data to complete read

				else
					{
					int iError = SSL_get_error(COpenSSL::AsSSL(m_pSSL), iBytesWritten);
					if (iError == SSL_ERROR_WANT_READ)
						{
						DebugLog("SSL wants to read");
						return resReceiveData;
						}
					else if (iError == SSL_ERROR_WANT_WRITE)
						{
						DebugLog("SSL wants to write\n");
						return resSendData;
						}
					else
						{
						if (retsError) *retsError = strPattern(ERR_WRITE_FAILED, iError);
						return resError;
						}
					}
				}

			case stateReady:
				return resReady;

			case stateError:
				if (retsError)
					*retsError = m_sError;
				return resError;

			default:
				DebugLog("SSL State = stateError");

				m_iState = stateError;
				m_sError = ERR_PROCESS_INVALID_STATE;
				if (retsError)
					*retsError = m_sError;
				return resError;
			}
		}
	}

bool CSSLAsyncEngine::ProcessHasDataToSend (void)

//	ProcessHasDataToSend
//
//	Returns TRUE if we have data to send

	{
	return (BIO_ctrl_pending(COpenSSL::AsBIO(m_pOutput)) > 0);
	}

void CSSLAsyncEngine::ProcessReceiveData (IMemoryBlock &Data)

//	ProcessReceiveData
//
//	Gives raw data to the engine for processing.

	{
	int iResult = BIO_write(COpenSSL::AsBIO(m_pInput), Data.GetPointer(), Data.GetLength());

	if (iResult <= 0)
		DebugLog("BIO_write error");
	else if (iResult < Data.GetLength())
		DebugLog("BIO_write did not write enough");
	}

void CSSLAsyncEngine::ProcessSendData (IByteStream &Data)

//	ProcessSendData
//
//	Retrieves raw data from the engine for transmission.

	{
	void *pData;
	int iLength = BIO_get_mem_data(COpenSSL::AsBIO(m_pOutput), &pData);
	Data.Write(pData, iLength);

	DebugLog(strPattern("Data Length: %d\n", iLength));

	BIO_reset(COpenSSL::AsBIO(m_pOutput));
	}

void CSSLAsyncEngine::Receive (void)

//	Receive
//
//	Asks the engine to receive data

	{
	m_iState = stateReading;
	}

void CSSLAsyncEngine::Send (IMemoryBlock &Data)

//	Send
//
//	Asks the engine to send data

	{
	m_Buffer.SetLength(Data.GetLength());
	utlMemCopy(Data.GetPointer(), m_Buffer.GetPointer(), Data.GetLength());
	m_iState = stateWriting;
	}
