//	FoundationExceptions.h
//
//	Foundation header file
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
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

	errEoS,							//	End of stream
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
	errCrashTrace,					//	Trace error (m_sErrorInfo is function name)

	errProgrammerError,				//	m_sErrorInfo may have extra data
	errUnknownError,				//	No extra info available
	};

class CException
	{
	public:
		CException (ErrorCodes Code) : m_Code(Code)
			{
			switch (Code)
				{
				case errCrashTrace:
					m_sErrorString = CString(__FUNCTION__);
					break;

				default:
					m_sErrorString = strPattern("Exception %d in %s", (int)Code, CString(__FUNCTION__));
					break;
				}
			}

		CException (ErrorCodes Code, const CString &sErrorString) : m_Code(Code), m_sErrorString(sErrorString) { }
		CException (ErrorCodes Code, DWORD dwErrorInfo, const CString &sErrorString) : m_Code(Code), m_dwErrorInfo(dwErrorInfo), m_sErrorString(sErrorString) { }

		ErrorCodes GetCode (void) const { return m_Code; }
		DWORD GetErrorInfo (void) const { return m_dwErrorInfo; }
		const CString &GetErrorString (void) const { return m_sErrorString; }

	private:
		ErrorCodes m_Code = errUnknownError;
		DWORD m_dwErrorInfo = 0;
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

#define DEBUG_TRY					try {
#define DEBUG_CATCH					} catch (CException e) { throw CException(errCrashTrace, strPattern("%s\n%s", (e.GetErrorString().IsEmpty() ? CString("EXCEPTION") : e.GetErrorString()), CString(__FUNCTION__))); } catch (...) { throw CException(errCrashTrace, strPattern("Exception in %s", CString(__FUNCTION__))); }

