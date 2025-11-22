//	DebugPEM.cpp
//
//	Debug PEM files
//	Copyright (c) 2017 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include "OpenSSLUtil.h"

int ExecuteDebugPEMFile (const CString &sFilespec)

//	ExecuteDebugPEMFile
//
//	Dump PEM file stats

	{
	CString sError;
	CFileBuffer File;

	if (!File.OpenReadOnly(sFilespec, &sError))
		{
		printf("ERROR: %s\n", (LPSTR)sError);
		return 1;
		}

	CSSLCertStack CertStack;
	if (!CertStack.InitFromPEM(File, &sError))
		{
		printf("ERROR: %s\n",  (LPSTR)sError);
		return 1;
		}

	if (CertStack.GetCount() == 0)
		{
		printf("ERROR: No certificates found.\n");
		return 1;
		}

	CSSLCert Cert = CertStack.GetCertificate(0);
	CString sSubject = Cert.GetSubject().GetDN();

	printf("Subject = %s\n", (LPSTR)sSubject);

	CSSLEnvelopeKey Key;
	if (!Key.InitFromPEM(File, NULL_STR, false, &sError))
		{
		printf("ERROR: %s\n",  (LPSTR)sError);
		return 1;
		}

	printf("Key loaded.\n");

	return 0;
	}
