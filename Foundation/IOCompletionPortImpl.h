//	IOCompletionPortImpl.h
//
//	Classes used by Foundation
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#pragma once

class CIOCPSimpleSignal : public IIOCPEntry
	{
	public:
		CIOCPSimpleSignal (const CString &sType) : IIOCPEntry(sType)
			{ }
	};

