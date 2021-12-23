//	Foundation.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Foundation provides common functions and classes plus all(most)
//	cross-platform concepts.
//
//	1. Include Foundation.h
//	2. Link with Foundation.lib
//	3. Link with version.lib, ws2_32.lib
//
//	BASIC TYPES
//
//	Use the following types
//
//	int			A 32-bit signed integer
//	DWORD		A 32-bit unsigned integer
//	LONGLONG	A 64-bit signed integer
//	DWORDLONG	A 64-bit unsigned integer
//	DWORD_PTR	An unsigned integer the size of a pointer (32 or 64 bits)
//
//	char		A 1-byte character
//	TCHAR		A 2-byte unicode character
//
//	LPSTR		A pointer to a null-terminated UTF-8 encoded sequence of characters.
//	LPTSTR		A pointer to a null-terminated UTF-16 encoded sequence of characters.
//
//	CHARACTER SET
//
//	Internally we store all text strings in UTF-8 encoding. We convert to UTF-16 before
//	passing to Windows OS functions.

#pragma once

//	Include basic library files required by Foundation

struct IUnknown;

#define _CRT_RAND_S
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <math.h>

//	Include operating system header files.

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601			//	Require Windows 7 or better (we need CancelIoEx)
#endif

//	LATER:
//	::inet_addr needs to be replaced with ::inet_pton
//	::gethostbyname needs to be replaced with ::getaddrinfo
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define _WINSOCKAPI_				//	Don't want Windows.h to include winsock.h
#include <windows.h>

#include <winsock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include "FoundationBase.h"
#include "FoundationDebug.h"
#include "FoundationSmartPtr.h"
#include "FoundationStrings.h"
#include "FoundationExceptions.h"
#include "FoundationArrays.h"
#include "FoundationMaps.h"
#include "FoundationHashMaps.h"
#include "FoundationUtilities.h"
#include "FoundationStreams.h"
#include "FoundationStreams64.h"
#include "FoundationStreamsTBuffer.h"

#include "FoundationColor.h"
#include "FoundationMath.h"
#include "FoundationMathComplex.h"
#include "FoundationMathGeometry.h"
#include "FoundationMathHexGrid.h"
#include "FoundationThreads.h"
#include "FoundationTime.h"
#include "FoundationIOInterfaces.h"

//	Include Foundation classes

#include "FoundationCompression.h"
#include "FoundationCounters.h"
#include "FoundationCrypto.h"
#include "FoundationDB.h"
#include "FoundationFileIO.h"
#include "FoundationGrids.h"
#include "FoundationMarkup.h"
#include "FoundationNetworkIO.h"
#include "FoundationIOCompletionPort.h"
#include "FoundationParsers.h"
#include "FoundationPipes.h"
#include "FoundationXML.h"

//	Graphics subsystem

#include "FoundationGraphicsCore.h"
#include "FoundationGraphicsImage8.h"
#include "FoundationGraphicsImage32.h"
#include "FoundationGraphicsJPEG.h"
#include "FoundationGraphicsPNG.h"
#include "FoundationGraphicsDraw.h"

class CFoundation
	{
	public:
		static constexpr DWORD BOOT_FLAG_COM =		0x00000001;

		struct SCPUInfo
			{
			int iLogicalProcessorCount = 0;
			};

		CFoundation (void);
		~CFoundation (void);

		static bool Boot (DWORD dwFlags = 0, CString *retsError = NULL);
		static SCPUInfo GetCPUInfo ();

	private:
		bool Startup (DWORD dwFlags, CString *retsError = NULL);
		void Shutdown (void);

		bool m_bInitialized = false;
		bool m_bCOMInitialized = false;

		void Test (void) { RECT rcRect; int x = ::RectWidth(&rcRect); }
	};

