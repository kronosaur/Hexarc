//	FoundationIOInterfaces.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

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
