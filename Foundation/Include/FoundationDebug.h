//	FoundationDebug.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

//	Define DEBUG

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif

//	ASSERT macro

#ifndef ASSERT
#ifdef DEBUG
#define ASSERT(exp)		((exp) ? true : (DebugBreak(), false))
#else
#define ASSERT(exp)
#endif
#endif

//	The following macro "NoEmptyFile()" can be put into a file 
//	in order suppress the MS Visual C++ Linker warning 4221 
// 
//	warning LNK4221: no public symbols found; archive member will be inaccessible 
// 
//	Thanks to: http://stackoverflow.com/users/14904/adisak
//	See: http://stackoverflow.com/questions/1822887/what-is-the-best-way-to-eliminate-ms-visual-c-linker-warning-warning-lnk422
 
#define NoEmptyFile()   namespace { char NoEmptyFileDummy##__LINE__; } 

//	Leak detection
//
//	Thanks to Visual Leak Detector
//	See: http://vld.codeplex.com/

#ifdef DEBUG
//#define DEBUG_LEAKS
#endif

#ifdef DEBUG_LEAKS
#include "vld.h"
#endif