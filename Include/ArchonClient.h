//	ArchonClient.h
//
//	Functions and classes for interacting with archons
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.

#pragma once

#include "AEON.h"

//	Aeon Helpers ---------------------------------------------------------------

class CAeonInterface
	{
	public:
		static CString CreateTableFilePath (const CString &sTable, const CString &sFilePath);
		static CString EncodeFilePathComponent (const CString &sValue);
		static CString FilespecToFilePath (const CString &sFilespec);
		static bool ParseFilePath (const CString &sFilePath, const CString &sRoot, int iOffset, const CDateTime &IfModifiedAfter, CString *retsAddr, CString *retsMsg, CDatum *retdPayload);
		static bool ParseTableFilePath (const CString &sPath, CString *retsTable, CString *retsFilePath, CString *retsError);
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

		inline void ReadFromEsper (CDatum dData) { m_Buffer.Write(dData.AsStringView()); }
		void ReadFromSocket (CSocket &theSocket);
		static void WriteToEsper (const CString &sCommand, CDatum dPayload, CDatum *retdData);

	private:
		void ParseMessage (int iLength, CString *retsCommand, CDatum *retdPayload);

		CCircularBuffer m_Buffer;

		CString m_sCachedCommand;
		CDatum m_dCachedPayload;
	};

//	CAMP1Protocol

class CAMP1Protocol
	{
	public:

		static constexpr int MAX_COMMAND_LENGTH = 64;
		static constexpr int MAX_HEADER_LENGTH
			= 8							//	AMP/1.00
			+ 1							//	space
			+ MAX_COMMAND_LENGTH		//	command
			+ 1							//	space
			+ 20						//	data length
			+ 2;						//	CRLF

		static bool GetHeader (const IMemoryBlock& Data, CString* retsCommand = NULL, DWORD* retdwDataLen = NULL, const char** retpPartialData = NULL, DWORD* retdwPartialDataLen = NULL);
		static CString MakeAUTH0Message (CStringView sMachineName, const CIPInteger& SecretKey);
		static CString MakeMessage (CStringView sMsg, const IMemoryBlock& Data = CBuffer());

	private:

		static bool ParseHeaderWord (const char* pPos, const char* pPosEnd, CString* retsWord = NULL, const char** retpPos = NULL);

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

		virtual void Append (const void *pPos, int iLength) override;
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

		bool CreateMultipartDatum (char *pPos, char *pPosEnd, CDatum *retdData) const;
		EParseResults FindMultipartBoundary (char *pPos, char *pPosEnd, char **retpBoundaryStart, CString *retsPartialBoundary = NULL) const;
		EParseResults ParseMultipartHeader (CString *retsField, CString *retsValue);
		EParseResults ParseMultipartBoundary (void);
		bool ProcessMultipartFirstBoundary (void);
		bool ProcessMultipartHeader (void);
		bool ProcessMultipartMoreContent (const void *pPos, int iLength);
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
		CString m_sPartFilename;
		CMemoryBuffer m_PartContent;
		CString m_sPartialBoundary;
		CComplexBinaryFile *m_pPartContent;
		CDatum m_dPartContent;

		//	Result

		CDatum m_dBody;
		bool m_bParseSuccess;
	};

typedef TSharedPtr<CEsperBodyBuilder> CEsperBodyBuilderPtr;

//	Transpace Helpers ----------------------------------------------------------

class CTranspaceInterface
	{
	public:
		static bool ParseAddress (const CString &sAddress, CString *retsNamespace = NULL, CString *retsPath = NULL, CDatum *retdParams = NULL);

	private:
		static void ParseAddressPart (CString &sPart, CString *retResult);
	};