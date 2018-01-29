//	FoundationExceptions.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

enum ErrorCodes
	{
	errNone,						//	No error

	errFail,						//	Operation failed in a way that may need to
									//	be displayed to the user. (e.g., parsing
									//	error that must be shown to the user).
									//	m_sErrorInfo is error message.

	errCancel,						//	The user cancelled an operation in
									//	the middle. Upper layers must recover
									//	silently.

	errOutOfMemory,
	errOutOfDiskspace,
	errUnableToOpenFile,			//	m_sErrorInfo is file and reason for error
	errUnableToSaveFile,			//	m_sErrorInfo is file and reason for error
	errPathInvalid,					//	m_sErrorInfo is path and reason for error
	errDisk,						//	Operation failed because of bad disk or
									//	database corruption or other disk-related
									//	reason that the user cannot fix easily.
									//	m_sErrorInfo is user message.

	errNotFound,					//	Generic not found error
	errDuplicate,					//	Generic duplicate error
	errOS,							//	ErrorInfo is GetLastError code

	errHostNotFound,
	errBadProtocol,
	errPortNotFound,
	errConnectionLost,

	errProgrammerError,				//	m_sErrorInfo may have extra data
	errUnknownError,				//	No extra info available
	};

class CException
	{
	public:
		CException (ErrorCodes Code) : m_Code(Code), m_dwErrorInfo(0) { }
		CException (ErrorCodes Code, const CString &sErrorString) : m_Code(Code), m_dwErrorInfo(0), m_sErrorString(sErrorString) { }
		CException (ErrorCodes Code, DWORD dwErrorInfo, const CString &sErrorString) : m_Code(Code), m_dwErrorInfo(dwErrorInfo), m_sErrorString(sErrorString) { }

		inline ErrorCodes GetCode (void) const { return m_Code; }
		inline DWORD GetErrorInfo (void) const { return m_dwErrorInfo; }
		inline const CString &GetErrorString (void) const { return m_sErrorString; }

	private:
		ErrorCodes m_Code;
		DWORD m_dwErrorInfo;
		CString m_sErrorString;
	};

class CFileException : public CException
	{
	public:
		CFileException (ErrorCodes Code, const CString &sFilespec, DWORD dwErrorInfo, const CString &sErrorString) : CException(Code, dwErrorInfo, sErrorString),
				m_sFilespec(sFilespec)
			{ }

		inline const CString &GetFilespec (void) const { return m_sFilespec; }

	private:
		CString m_sFilespec;
	};