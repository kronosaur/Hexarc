//	FoundationIOInterfaces.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

//	Interface ------------------------------------------------------------------
//
//	An I/O interface is a protocol for abstracting input and output for module.
//	For example, we might hand an I/O interface to a module so that it can 
//	generate output without it having to know where the output is going.

class IIOInterface
	{
	public:
		virtual ~IIOInterface (void) { }
	};

//	Console Interface ----------------------------------------------------------
//
//	A console interface is a line-oriented interface. We provide methods for
//	requesting input and for submitting output.

class IIOConsole : public IIOInterface
	{
	public:
		virtual ~IIOConsole (void) { }

		inline CString GetInputLine (void) { return OnGetInputLine(); }
		inline bool HasInput (void) const { return OnHasInput(); }
		void Output (const CString &sText);
		inline void OutputLine (const CString &sLine) { return OnOutputLine(sLine); }

	protected:

		//	Sub-classes must implement these methods

		virtual CString OnGetInputLine (void) = 0;
		virtual bool OnHasInput (void) const = 0;
		virtual void OnOutputLine (const CString &sLine) = 0;
	};

//	Log Interface --------------------------------------------------------------

class ILogService
	{
	public:
		enum ELogClasses
			{
			logDebug,
			logInfo,
			logWarning,
			logError,
			};

		virtual ~ILogService (void) { }

		virtual void Write (const CString &sLine, ELogClasses iClass = logInfo) { }
	};

//  Progress Interface ---------------------------------------------------------

class IProgressEvents
    {
    public:
        virtual ~IProgressEvents (void) { }

        virtual void OnProgressStart (void) { }
        virtual void OnProgress (int iPercent, const CString &sStatus = NULL_STR) { }
		virtual void OnProgressLogError (const CString &sError) { }
        virtual void OnProgressDone (void) { }
    };

