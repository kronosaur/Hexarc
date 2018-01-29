//	FoundationBase.h
//
//	Foundation header file
//	Copyright (c) 2010 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IByteStream
	{
	public:
		virtual ~IByteStream (void) { }
		virtual int GetPos (void) = 0;
		virtual int GetStreamLength (void) = 0;
		virtual int Read (void *pData, int iLength) = 0;
		virtual void Seek (int iPos, bool bFromEnd = false) = 0;
		virtual int Write (void *pData, int iLength) = 0;

		//	For shared stream access
		virtual bool Lock (int iPos, int iLength) { ASSERT(false); return false; }
		virtual void Unlock (int iPos, int iLength) { ASSERT(false); }
	};

class IMemoryBlock
	{
	public:
		virtual ~IMemoryBlock (void) { }
		virtual int GetLength (void) const = 0;
		virtual char *GetPointer (void) const = 0;

	};

