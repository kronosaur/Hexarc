//	FoundationPipes.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CNamedPipe
	{
	public:
		static constexpr DWORD FLAG_READ_OVERLAPPED =	0x00000001;
		static constexpr DWORD FLAG_WRITE_OVERLAPPED =	0x00000002;
		static constexpr DWORD FLAG_READ_INHERIT =		0x00000004;
		static constexpr DWORD FLAG_WRITE_INHERIT =		0x00000008;

		~CNamedPipe ();

		bool Create (const CString &sName, DWORD dwFlags = 0);

		HANDLE GetReadHandle () { return m_hRead; }
		HANDLE GetWriteHandle () { return m_hWrite; }
		CManualEvent &HasDataEvent () { return m_ReadEvent; }
		CString Read ();
		CManualEvent &WantsDataEvent () { return m_WriteEvent; }

	private:
		static constexpr int BUFFER_SIZE = 4096;

		static CString ComposeName (const CString &sText);
		bool AsyncRead ();

		CString m_sName;

		HANDLE m_hRead = INVALID_HANDLE_VALUE;
		CManualEvent m_ReadEvent;
		OVERLAPPED m_OverlappedRead;
		CBuffer m_ReadBuffer;
		bool m_bAsyncRead = false;
		bool m_bWaitingForRead = false;

		HANDLE m_hWrite = INVALID_HANDLE_VALUE;
		CManualEvent m_WriteEvent;
		OVERLAPPED m_OverlappedWrite;
		bool m_bAsyncWrite = false;
		bool m_bWaitingForWrite = false;

		static volatile LONG m_dwSN;
	};
