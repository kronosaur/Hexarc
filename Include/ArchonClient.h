//	ArchonClient.h
//
//	Functions and classes for interacting with archons
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#pragma once

#include "AEON.h"

//	Aeon Helpers ---------------------------------------------------------------

class CAeonInterface
	{
	public:
		static CString EncodeFilePathComponent (const CString &sValue);
		static CString FilespecToFilePath (const CString &sFilespec);
		static bool ParseFilePath (const CString &sFilePath, const CString &sRoot, int iOffset, const CDateTime &IfModifiedAfter, CString *retsAddr, CString *retsMsg, CDatum *retdPayload);
	};

//	AI1 Protocol ---------------------------------------------------------------

class CAI1Protocol
	{
	public:
		static CDatum CreateAuthDescSHAPassword (const CString &sUsername, const CString &sPassword);
		static CDatum CreateSHAPassword_V1 (const CString &sUsername, const CString &sPassword);
		static CDatum CreateSHAPasswordChallenge (void);
		static CDatum CreateSHAPasswordChallengeResponse (CDatum dAuthSHAPassword, CDatum dChallenge);
		static CDatum CreateSHAPasswordChallengeResponse_V1 (const CString &sUsername, const CString &sPassword, CDatum dChallenge);
	};

class CAI1Stream
	{
	public:
		void GetNext (CString *retsCommand, CDatum *retdPayload);
		bool HasMore (bool *retbOverflow = NULL);
		inline void Mark (void) { m_dCachedPayload.Mark(); }

		inline void ReadFromEsper (CDatum dData) { m_Buffer.Write((const CString &)dData); }
		void ReadFromSocket (CSocket &theSocket);
		static void WriteToEsper (const CString &sCommand, CDatum dPayload, CDatum *retdData);

	private:
		void ParseMessage (int iLength, CString *retsCommand, CDatum *retdPayload);

		CCircularBuffer m_Buffer;

		CString m_sCachedCommand;
		CDatum m_dCachedPayload;
	};

//	Crypto Helpers -------------------------------------------------------------

class CCryptosaurInterface
	{
	public:
		static void CreateAuthToken (CDatum dData, const CDateTime &ExpireTime, const CIPInteger &SecretKey, CString *retsToken);
		static void CreateChallengeCredentials (const CString &sUsername, const CString &sPassword, CDatum dChallenge, CDatum *retCredentials);
		static void CreateCredentials (const CString &sUsername, const CString &sPassword, CIPInteger *retCredentials);
		static CDatum SignData (CDatum dData, const CIPInteger &SecretKey);
		static bool ValidateAuthToken (const CString &sToken, const CIPInteger &SecretKey, CDatum *retdData);
		static CString UsernameToKey (const CString &sUsername) { return strToLower(sUsername); }
	};

//	Esper Helpers --------------------------------------------------------------

class CEsperInterface
	{
	public:
		static void AddHeadersToMessage (CDatum dHeaders, CHTTPMessage *retMessage);
		static DWORD ConnectionToFriendlyID (CDatum dConnection);
		static bool ConvertBodyToDatum (const CHTTPMessage &Message, CDatum *retdBody);
		static bool ConvertFormURLEncodedToDatum (const CString &sText, CDatum *retdValue);
		static CDatum ConvertHeadersToDatum (const CHTTPMessage &Message);
		static CDatum DecodeHTTPResponse (const CHTTPMessage &Message, bool bRawBody);
		static void EncodeHTTPRequest (const CString &sMethod, const CString &sHost, const CString &sPath, CDatum dHeaders, CDatum dBody, CHTTPMessage *retMessage);
		static bool HTTP (const CString &sMethod, const CString &sURL, CDatum dHeaders, CDatum dBody, CDatum dOptions, CDatum *retdResult);
	};

class CEsperBodyBuilder : public IMediaTypeBuilder
	{
	public:
		CEsperBodyBuilder (void);

		inline bool GetBody (CDatum *retdBody) { *retdBody = m_dBody; return m_bParseSuccess; }
		CDatum GetRawBody (void);
		void Mark (void);
		inline void ResetBody (void) { m_dBody = CDatum(); ResetMultipartTemps(); }

		//	IMediaTypeBuilder interface

		virtual void Append (void *pPos, int iLength) override;
		virtual bool CreateMedia (IMediaTypePtr *retpBody) override;
		virtual int GetLength (void) const override { return m_iBodyRead; }
		virtual void Init (const CString &sMediaType) override;

	private:
		enum EStates
			{
			//	URL-encoded Form

			stateFormBuild,					//	Accumulate into m_Body

			//	JSON

			stateJSONBuild,					//	Accumulate into m_Body

			//	Multipart form

			stateMultipartFirstBoundary,	//	Waiting for the first boundary
			stateMultipartHeader,
			stateMultipartStartContent,
			stateMultipartMoreContent,
			stateMultipartDone,

			//	Text build

			stateTextBuild,					//	Accumulate into m_Body
			stateTextDone,

			//	Error

			stateError,						//	m_dBody is error
			};

		enum EParseResults
			{
			resultNeedMoreData,
			resultNotFound,
			resultFound,
			resultError,

			resultFoundBoundary,
			resultFoundLastBoundary,
			};

		bool CreateMultipartDatum (const CString &sPartType, char *pPos, char *pPosEnd, CDatum *retdData);
		EParseResults FindMultipartBoundary (char *pPos, char *pPosEnd, char **retpBoundaryStart, CString *retsPartialBoundary = NULL) const;
		EParseResults ParseMultipartHeader (CString *retsField, CString *retsValue);
		EParseResults ParseMultipartBoundary (void);
		bool ProcessMultipartFirstBoundary (void);
		bool ProcessMultipartHeader (void);
		bool ProcessMultipartMoreContent (void *pPos, int iLength);
		bool ProcessMultipartStartContent (void);
		void ResetMultipartTemps (void);

		CString m_sMediaType;
		EStates m_iState;
		CMemoryBuffer m_Body;
		int m_iParsePos;
		int m_iBodyRead;
		CComplexStruct *m_pStruct;

		//	Multipart form

		CString m_sBoundary;
		CString m_sPartType;
		CString m_sPartName;
		CMemoryBuffer m_PartContent;
		CString m_sPartialBoundary;
		CComplexBinaryFile *m_pPartContent;
		CDatum m_dPartContent;

		//	Result

		CDatum m_dBody;
		bool m_bParseSuccess;
	};

typedef TSharedPtr<CEsperBodyBuilder> CEsperBodyBuilderPtr;

class CEsperMultipartParser
	{
	public:
		CEsperMultipartParser (const CString &sMediaType, IMemoryBlock &Block) : 
				m_sMediaType(sMediaType),
				m_Block(Block),
				m_pResult(NULL)
			{ }
		~CEsperMultipartParser (void);

		bool ParseAsDatum (CDatum *retdBody);

	private:
		bool ParseToBoundary (char *pPos, char *pPosEnd, const CString &sBoundary, const CString &sPartType, CDatum *retdData, char **retpPos) const;

		CString m_sMediaType;
		IMemoryBlock &m_Block;
		CComplexStruct *m_pResult;
	};

//	Transpace Helpers ----------------------------------------------------------

class CTranspaceInterface
	{
	public:
		static bool ParseAddress (const CString &sAddress, CString *retsNamespace = NULL, CString *retsPath = NULL, CDatum *retdParams = NULL);

	private:
		static void ParseAddressPart (CString &sPart, CString *retResult);
	};