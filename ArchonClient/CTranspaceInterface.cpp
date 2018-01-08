//	CTranspaceInterface.cpp
//
//	CTranspaceInterface class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

bool CTranspaceInterface::ParseAddress (const CString &sAddress, CString *retsNamespace, CString *retsPath, CDatum *retdParams)

//	ParseAddress
//
//	Parses a Transpace Address.
//	See: http://kronosaur.com/design/wiki/index.php/Transpace_Addressing_System
//
//	We parse addresses of the following form:
//
//	@Luminous.command/RenderCloud.ars?sizeX=256&sizeY=256
//	|---------------||--------------| |-----------------|
//	 namespace		  path			   params
//
//	@Aeon.command/Multiverse.files/Temp/Test.jpg
//	|-----------||-----------------------------|
//	 namespace    path
//
//	#Multiverse.fileCache/Anacreon/Scenario1.ars
//	|-------------------||---------------------|
//	 namespace            path
//
//	/Astro.files/nebulae/m42.jpg
//	|--------------------------|
//	 path
//
//	Namespaces, path segments, and params must contain only alphanumerics and - _ . ~
//	%xx can be used to encode arbitrary characters
//	+ can be used to encode a space

	{
	char *pPos = sAddress.GetParsePointer();
	char *pPosEnd = pPos + sAddress.GetLength();

	//	First parse the namespace. A namespace is optional.

	char *pStart = pPos;
	while (pPos < pPosEnd && *pPos != '/')
		pPos++;

	CString sNamespace(pStart, pPos - pStart);
	if (retsNamespace)
		ParseAddressPart(sNamespace, retsNamespace);

	//	Now parse the path. A path is required.

	pStart = pPos;
	while (pPos < pPosEnd && *pPos != '?')
		pPos++;

	CString sPath(pStart, pPos - pStart);
	if (sPath.IsEmpty())
		return false;

	if (retsPath)
		ParseAddressPart(sPath, retsPath);

	//	Finally, parse parameters

	if (pPos < pPosEnd)
		{
		//	Prepare output

		CComplexStruct *pParams = NULL;
		if (retdParams)
			pParams = new CComplexStruct;

		//	Skip '?'

		pPos++;

		//	Keep parsing until the end

		while (pPos < pPosEnd)
			{
			//	Parse the field

			pStart = pPos;
			while (pPos < pPosEnd && *pPos != '=')
				pPos++;

			CString sField(pStart, pPos - pStart);

			//	Skip '='

			if (pPos == pPosEnd)
				return false;

			pPos++;

			//	Parse value

			pStart = pPos;
			while (pPos < pPosEnd && *pPos != '&')
				pPos++;

			CString sValue(pStart, pPos - pStart);

			//	Skip '&'

			if (pPos < pPosEnd)
				pPos++;

			//	Add to structure

			if (retdParams)
				{
				ParseAddressPart(sField, &sField);
				ParseAddressPart(sValue, &sValue);

				pParams->SetElement(sField, sValue);
				}
			}

		//	Done

		if (retdParams)
			*retdParams = CDatum(pParams);
		}
	else
		{
		if (retdParams)
			*retdParams = CDatum();
		}

	//	Done

	return true;
	}

void CTranspaceInterface::ParseAddressPart (CString &sPart, CString *retResult)

//	ParseAddressPart
//
//	Parses sPart and decodes and special characters.
//	NOTE: We destroy sPart.

	{
	char *pPos = sPart.GetParsePointer();
	char *pPosEnd = pPos + sPart.GetLength();

	//	Check to see if sPart has any special characters.

	while (pPos < pPosEnd && *pPos != '+' && *pPos != '%')
		pPos++;

	//	If we don't have special characters then we can just return the original
	//	string.

	if (pPos == pPosEnd)
		{
		if (retResult != &sPart)
			retResult->TakeHandoff(sPart);
		return;
		}

	//	Otherwise we use standard URL parsing

	retResult->TakeHandoff(urlDecode(sPart));
	}
