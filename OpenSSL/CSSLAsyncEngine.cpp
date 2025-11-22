//	CSSLAsyncEngine.cpp
//
//	CSSLAsyncEngine class
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_AU_PREFIX,					"Au=");
DECLARE_CONST_STRING(FIELD_ENC_PREFIX,					"Enc=");
DECLARE_CONST_STRING(FIELD_KX_PREFIX,					"Kx=");
DECLARE_CONST_STRING(FIELD_MAC_PREFIX,					"Mac=");

DECLARE_CONST_STRING(STR_CONNECTED,						"Connected.");
DECLARE_CONST_STRING(STR_HANDSHAKE_START,				"Handshake start.");

DECLARE_CONST_STRING(ERR_CONNECT_INVALID_STATE,			"Connect: Invalid state.");
DECLARE_CONST_STRING(ERR_PROCESS_INVALID_STATE,			"Process: Invalid state.");
DECLARE_CONST_STRING(ERR_OUT_OF_MEMORY,					"SSL: Out of memory.");
DECLARE_CONST_STRING(ERR_CONNECT_FAILED,				"SSL: Connect failed: %x.");
DECLARE_CONST_STRING(ERR_READ_FAILED,					"SSL: Read failed: %x.");
DECLARE_CONST_STRING(ERR_WRITE_FAILED,					"SSL: Write failed: %x.");
DECLARE_CONST_STRING(ERR_ACCEPT_FAILED,					"SSL: Accept failed error = %d.");
DECLARE_CONST_STRING(ERR_CANT_SET_HOST,					"SSL: Unable to set SNI hostname.");

const int BUFFER_SIZE =									160 * 1024;

int CSSLAsyncEngine::m_SSLCtxIndex = -1;
CIODiagnostics CSSLAsyncEngine::m_NullDiagnostics;

CSSLAsyncEngine::CSSLAsyncEngine (CSSLCtx *pSSLCtx, CIODiagnostics& Diag) :
		m_pSSLCtx(pSSLCtx),
		m_Buffer(BUFFER_SIZE),
		m_Diagnostics(Diag)

//	CSSLAsyncEngine constructor

	{
	if (m_SSLCtxIndex == -1)
		{
		m_SSLCtxIndex = SSL_CTX_get_ex_new_index(0, NULL, NULL, NULL, NULL);
		if (m_SSLCtxIndex == -1)
			throw CException(errFail);
		}
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
CString CSSLAsyncEngine::DebugGetBufferState () const
	{
	int iLeftToRead = (int)BIO_ctrl_pending(COpenSSL::AsBIO(m_pInput));
	int iLeftToWrite = (int)BIO_ctrl_pending(COpenSSL::AsBIO(m_pOutput));
	return strPattern("Left to Read: %d; Left to Write: %d", iLeftToRead, iLeftToWrite);
	}

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
	retStatus->sCipherName = CString(SSL_CIPHER_get_name(cipher));

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

CString CSSLAsyncEngine::GetSSLError () const
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

	return CString(std::move(Buffer));
	}

CSSLAsyncEngine& CSSLAsyncEngine::GetThis (const void* pSSL)
	{
	return *(CSSLAsyncEngine*)SSL_get_ex_data((const SSL*)pSSL, m_SSLCtxIndex);
	}

static void SSLInfoCallback (const SSL *ssl, int where, int ret)
	{
	CSSLAsyncEngine& Engine = CSSLAsyncEngine::GetThis(ssl);
	CIODiagnostics& Diag = Engine.GetDiagnostics();
	if (!Diag.IsEnabled())
		return;

	if (where & SSL_CB_HANDSHAKE_START)
		{
		// This is the start of a (re)handshake
		Diag.Log(STR_HANDSHAKE_START);
		}

	if (where & SSL_CB_LOOP)
		{
		// For example, states like "SSLv3 read server certificate request A"
		// can tell you the server asked for a client cert.
		const char *str = SSL_state_string_long(ssl);
		Diag.Log(strPattern("SSL state: %s", CString(str)));
		}

#ifdef DEBUG
	if (where & SSL_CB_ALERT)
		{
		const char *str = SSL_alert_type_string_long(ret);
		const char *desc = SSL_alert_desc_string_long(ret);
		Diag.Log(strPattern("SSL alert: %s %s", CString(str), CString(desc)));
		}
#endif
	}

bool CSSLAsyncEngine::Init (bool bAsServer, CString *retsError)

//	Init
//
//	Initializes m_pSSL

	{
	if (m_pSSL)
		return true;

	ERR_clear_error();

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

	SSL_set_ex_data(COpenSSL::AsSSL(m_pSSL), m_SSLCtxIndex, this);
	SSL_set_info_callback(COpenSSL::AsSSL(m_pSSL), SSLInfoCallback);

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

	//	Make sure we set the host name on the SSL object because we need 
	//	it in the TLS handshake for SNI.

	if (!m_sHostname.IsEmpty())
		{
#ifdef DEBUG_SSL
		printf("Setting hostname: %s\n", (LPSTR)m_sHostname);
#endif
		int res = SSL_set_tlsext_host_name(COpenSSL::AsSSL(m_pSSL), (LPSTR)m_sHostname);
		if (res != 1)
			{
			if (retsError) *retsError = ERR_CANT_SET_HOST;
			return false;
			}
		}

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

				ERR_clear_error();
				int err = SSL_accept(COpenSSL::AsSSL(m_pSSL));
				if (err > 0)
					{
					DebugLog("SSL State = stateReady");

					m_iState = stateReady;
					return resReadyConnect;
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

#ifdef DEBUG_SSL
				printf("Connecting to %s\n", (LPSTR)m_sHostname);
#endif

				//	Connect

				ERR_clear_error();
				int err = SSL_connect(COpenSSL::AsSSL(m_pSSL));
				if (err > 0)
					{
					m_Diagnostics.Log(STR_CONNECTED);

					m_iState = stateReady;
					return resReadyConnect;
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

			case stateWorking:
				{
				if (m_bWriting)
					{
					ERR_clear_error();
					int iBytesWritten = SSL_write(COpenSSL::AsSSL(m_pSSL), m_Buffer.GetPointer(), m_Buffer.GetLength());

					m_Diagnostics.LogFn([iBytesWritten]() { return strPattern("SSL_write: %d bytes", iBytesWritten); });
			
					if (iBytesWritten > 0)
						{
						m_bWriting = false;
						if (!m_bReading)
							m_iState = stateReady;

						return resReadyWrite;
						}

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
							CString sError = GetSSLError();
							m_Diagnostics.Log(strPattern("SSL_write error: %s", sError));

							if (retsError) *retsError = strPattern(ERR_WRITE_FAILED, iError);
							return resError;
							}
						}
					}

				if (m_bReading)
					{
#ifdef DEBUG_SSL
					printf("SSL_read buffer: %llx\n", (DWORDLONG)m_Buffer.GetPointer());
#endif
					m_Buffer.SetLength(BUFFER_SIZE);
					ERR_clear_error();
					int iBytesRead = SSL_read(COpenSSL::AsSSL(m_pSSL), m_Buffer.GetPointer(), m_Buffer.GetLength());

					m_Diagnostics.LogFn([iBytesRead]() { return strPattern("SSL_read: %d bytes", iBytesRead); });

					if (iBytesRead > 0)
						{
						m_bReading = false;
						if (!m_bWriting)
							m_iState = stateReady;

						m_bReadRetried = false;
						m_Buffer.SetLength(iBytesRead);
						return resReadyRead;
						}

					//	Otherwise, we need data to complete read

					else
						{
						int iError = SSL_get_error(COpenSSL::AsSSL(m_pSSL), iBytesRead);
						switch (iError)
							{
							case SSL_ERROR_WANT_READ:
								{
#ifdef DEBUG_SSL
								printf("SSL_ERROR_WANT_READ\n");
#endif
								m_bReadRetried = false;
								DebugLog("SSL wants to read");
								return resReceiveData;
								}

							case SSL_ERROR_WANT_WRITE:
								{
#ifdef DEBUG_SSL
								printf("SSL_ERROR_WANT_WRITE\n");
#endif
								m_bReadRetried = false;
								DebugLog("SSL wants to write");
								return resSendData;
								}

							case SSL_ERROR_SSL:
								{
								CString sError = GetSSLError();
								m_Diagnostics.Log(strPattern("SSL_read error: %s", sError));

								if (retsError) *retsError = sError;
								return resError;
								}

							default:
								{
								CString sError = GetSSLError();
								m_Diagnostics.Log(strPattern("SSL_read unknown error: %s", sError));

								if (retsError) *retsError = strPattern(ERR_READ_FAILED, iError);
								return resError;
								}
							}
						}
					}

				//	Should never get here because we can't be in stateWorking
				//	unless we're either reading or writing.
				continue;
				}

			case stateReady:
				return resReadyIdle;

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
#ifdef DEBUG_SSL
	printf("BIO_ctrl_pending: %lld\n", BIO_ctrl_pending(COpenSSL::AsBIO(m_pOutput)));
#endif
	return (BIO_ctrl_pending(COpenSSL::AsBIO(m_pOutput)) > 0);
	}

void CSSLAsyncEngine::ProcessReceiveData (IMemoryBlock &Data)

//	ProcessReceiveData
//
//	Gives raw data to the engine for processing.

	{
#ifdef DEBUG_SSL
	printf("BIO_Write: %d bytes\n", Data.GetLength());
#endif

	int iResult = BIO_write(COpenSSL::AsBIO(m_pInput), Data.GetPointer(), Data.GetLength());

	if (iResult <= 0)
		DebugLog("BIO_write error");
	else if (iResult < Data.GetLength())
		DebugLog("BIO_write did not write enough");

#ifdef DEBUG_SSL
	if (iResult != Data.GetLength())
		printf("BIO_write: %d bytes (wrote %d)\n", Data.GetLength(), iResult);
#endif
	}

void CSSLAsyncEngine::ProcessSendData (IByteStream &Data)

//	ProcessSendData
//
//	Retrieves raw data from the engine for transmission.

	{
#ifdef DEBUG_SSL
	printf("BIO_get_mem_data\n");
#endif

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
	m_iState = stateWorking;
	m_bReading = true;
	}

void CSSLAsyncEngine::Send (IMemoryBlock &Data)

//	Send
//
//	Asks the engine to send data

	{
	m_Buffer.SetLength(Data.GetLength());
	utlMemCopy(Data.GetPointer(), m_Buffer.GetPointer(), Data.GetLength());
	m_iState = stateWorking;
	m_bWriting = true;
	}
