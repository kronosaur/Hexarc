//	CGLDefaultEnvironment.cpp
//
//	CGLDefaultEnvironment Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"

void CGLDefaultEnvironment::Output (const CString &sPort, CDatum dValue)
	{
	const CString &sLine = dValue;
	printf("%s\n", (LPSTR)sLine);
	}
