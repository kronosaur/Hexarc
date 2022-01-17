//	Utilities.h
//
//	HTTP Utilities
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CHTTPMultipartParser
	{
	public:
		CHTTPMultipartParser (const CString &sMediaType, IMemoryBlock &Block) : 
				m_sMediaType(sMediaType),
				m_Block(Block)
			{ }

		CHTTPMultipartParser (const CHTTPMultipartParser &Src) = delete;
		CHTTPMultipartParser (CHTTPMultipartParser &&Src) = delete;

		~CHTTPMultipartParser ();

		CHTTPMultipartParser &operator= (const CHTTPMultipartParser &Src) = delete;
		CHTTPMultipartParser &operator= (CHTTPMultipartParser &&Src) = delete;

		bool ParseAsDatum (CDatum &retdBody);

	private:
		bool ParseToBoundary (const char *pPos, const char *pPosEnd, const CString &sBoundary, const CString &sPartType, CDatum &retdData, const char **retpPos = NULL) const;

		CString m_sMediaType;
		IMemoryBlock &m_Block;
		CComplexStruct *m_pResult = NULL;
	};
