//	CHTTPMessageBodyBuilder.cpp
//
//	CHTTPMessageBodyBuilder class
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

const int DEFAULT_SIZE =					64 * 1024;

CHTTPMessageBodyBuilder::CHTTPMessageBodyBuilder (void) : m_Body(DEFAULT_SIZE)

//	CHTTPMessageBodyBuild constructor

	{
	}

void CHTTPMessageBodyBuilder::Append (void *pPos, int iLength)

//	Append
//
//	Append the data

	{
	m_Body.Write(pPos, iLength);
	}

bool CHTTPMessageBodyBuilder::CreateMedia (IMediaTypePtr *retpBody)

//	CreateMedia
//
//	Creates the media buffer

	{
	IMediaTypePtr pBody(new CRawMediaType);
	pBody->DecodeFromBuffer(m_sMediaType, m_Body);
	*retpBody = pBody;
	return true;
	}

void CHTTPMessageBodyBuilder::Init (const CString &sMediaType)

//	Init
//
//	Initializes

	{
	m_sMediaType = sMediaType;
	m_Body.SetLength(0);
	}
