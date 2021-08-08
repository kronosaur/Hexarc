//	ILogService.cpp
//
//	ILogService classes
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

class CDefaultLogService : public ILogService
	{
	public:
		virtual void Write (const CString &sLine, ELogClasses iClass = logInfo) override { printf("%s\n", (LPSTR)sLine); }
	};

static CDefaultLogService DefaultService;

ILogService &ILogService::Default ()
	{
	return DefaultService;
	}
