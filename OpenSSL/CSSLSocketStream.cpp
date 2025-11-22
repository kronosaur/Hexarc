//	CSSLSocketStream.cpp
//
//	CSSLSocketStream class
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(ERR_CANT_ALLOC_SSL_SESSION,		"Unable to allocate SSL session.")
DECLARE_CONST_STRING(ERR_CANT_SSL_CONNECT,				"Unable to connect SSL session.")
DECLARE_CONST_STRING(ERR_UNABLE_TO_CONNECT,				"Unable to connect to server at %s:%d.")

CSSLSocketStream::~CSSLSocketStream (void)

//	CSSLSocketStream destructor

	{
	Disconnect();
	if (m_pSSL)
		g_SSLGlobal.FreeSession(m_pSSL);
	}

bool CSSLSocketStream::Connect (const CString &sHostname, DWORD dwPort, bool bUseSSL, CString *retsError)

//	Connect
//
//	Connects to the given server and port.

	{
	if (!m_bConnected)
		{
		//	Always connect with the socket (use non-blocking mode so that we can
		//	timeout)

		m_Socket.SetBlockingMode(false);
		if (!m_Socket.Connect(sHostname, dwPort))
			{
			if (retsError)
				*retsError = strPattern(ERR_UNABLE_TO_CONNECT, sHostname, dwPort);
			return false;
			}

		//	If we want SSL, we start an SSL session

		if (bUseSSL
				&& m_pSSL == NULL)
			{
			m_pSSL = g_SSLGlobal.AllocSession();
			if (m_pSSL == NULL)
				{
				if (retsError)
					*retsError = ERR_CANT_ALLOC_SSL_SESSION;
				m_Socket.Disconnect();
				return false;
				}

			//	NOTE: In 64-bit Windows, a SOCKET is a DWORD_PTR, but Windows 
			//	guarantees that the SOCKET will fit in 32-bits, so it is OK to cast.

			m_pBIO = BIO_new_socket((int)(SOCKET)m_Socket, BIO_NOCLOSE);
			SSL_set_bio(COpenSSL::AsSSL(m_pSSL), COpenSSL::AsBIO(m_pBIO), COpenSSL::AsBIO(m_pBIO));

			//	Make sure we set the host name on the SSL object because we need 
			//	it in the TLS handshake for SNI.

			int res = SSL_set_tlsext_host_name(COpenSSL::AsSSL(m_pSSL), (LPSTR)sHostname);

			//	Connect

			while (true)
				{
				int err = SSL_connect(COpenSSL::AsSSL(m_pSSL));
				if (err <= 0)
					{
					if (!SelectWait(err))
						{
						err = SSL_get_error(COpenSSL::AsSSL(m_pSSL), err);
						if (retsError)
							*retsError = ERR_CANT_SSL_CONNECT;
						g_SSLGlobal.FreeSession(m_pSSL);
						m_pSSL = NULL;
						m_Socket.Disconnect();
						return false;
						}

					//	Try again
					continue;
					}

				//	Success
				break;
				}
			}

		m_bConnected = true;
		}

	return true;
	}

void CSSLSocketStream::Disconnect (void)

//	Disconnect
//
//	Disconnects

	{
	if (m_bConnected)
		{
		if (m_pSSL)
			SSL_shutdown(COpenSSL::AsSSL(m_pSSL));

		m_Socket.Disconnect();

		if (m_pSSL)
			{
			g_SSLGlobal.FreeSession(m_pSSL);
			m_pSSL = NULL;
			}

		m_bConnected = false;
		}
	}

int CSSLSocketStream::Read (void *pData, int iLength)

//	Read
//
//	Reads and returns the amount of data read.

	{
	//	If we have an SSL session, read through that

	if (m_pSSL)
		{
		int iBytesRead;
		while (true)
			{
			if ((iBytesRead = SSL_read(COpenSSL::AsSSL(m_pSSL), pData, iLength)) <= 0)
				{
				if (!SelectWait(iBytesRead))
					return 0;

				//	The socket it ready; try again.

				continue;
				}

			//	Success

			break;
			}

		return iBytesRead;
		}

	//	Otherwise, we read through a normal socket

	else
		return m_Socket.Read(pData, iLength);
	}

bool CSSLSocketStream::SelectWait (int iResult)

//	SelectWait
//
//	Wait for the socket, based on the result of an SSL function. We return
//	TRUE if the socket is ready, FALSE for errors and timeouts.

	{
	int iError = SSL_get_error(COpenSSL::AsSSL(m_pSSL), iResult);

	if (iError == SSL_ERROR_NONE)
		return true;
	else if (iError == SSL_ERROR_WANT_READ)
		{
		if (!m_Socket.SelectWaitRead())
			return false;
		}
	else if (iError == SSL_ERROR_WANT_WRITE)
		{
		if (!m_Socket.SelectWaitWrite())
			return false;
		}
	else
		//	Some other error
		return false;

	return true;
	}

int CSSLSocketStream::Write (const void *pData, int iLength)

//	Write
//
//	Writes and returns the amount of data written.

	{
	//	If we have an SSL session, write through that

	if (m_pSSL)
		{
		int iBytesSent;
		while (true)
			{
			if ((iBytesSent = SSL_write(COpenSSL::AsSSL(m_pSSL), pData, iLength)) <= 0)
				{
				if (!SelectWait(iBytesSent))
					return 0;

				//	The socket it ready; try again.

				continue;
				}

			//	Success

			break;
			}

		return iBytesSent;
		}

	//	Otherwise, we write through a normal socket

	else
		return m_Socket.Write(pData, iLength);
	}
