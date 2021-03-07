//	CGLDefaultEnvironment.cpp
//
//	CGLDefaultEnvironment Class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include <iostream>
//#include <locale>
#include <string>

bool CGLDefaultEnvironment::GetInput (const CString &sPort, const CString &sPrompt, CDatum *retdResult)
	{
	if (!sPrompt.IsEmpty())
		printf("%s", (LPSTR)sPrompt);

	std::wstring input;
	std::wcin >> input;
	*retdResult = CDatum(CString(CString16(input.c_str(), -1)));
	return true;
	}

void CGLDefaultEnvironment::Output (const CString &sPort, CDatum dValue)
	{
	const CString &sLine = dValue;
	printf("%s\n", (LPSTR)sLine);
	}
