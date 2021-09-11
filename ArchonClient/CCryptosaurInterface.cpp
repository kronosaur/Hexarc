//	CCryptosaurInterface.cpp
//
//	CCryptosaurInterface class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(CONST_NO_EXPIRATION,				"infinite")

void CCryptosaurInterface::CreateAuthToken (CDatum dData, const CDateTime &ExpireTime, const CIPInteger &SecretKey, CString *retsToken)

//	CreateAuthToken
//
//	Creates an authentication token. An auth token is a string with the
//	following elements:
//
//	&{data}&{expire-time}&{MAC}
//
//	{data} is the AEONScript serialized form of dData
//	{expire-time} is an integer string representing seconds since midnight 1/1/2001.
//	{MAC} is the MAC of the preceeding string encoded in Base64.

	{
	CStringBuffer Token;

	//	Compute the number of seconds since midnight 1/1/2001.

	CString sSeconds;
	if (ExpireTime.IsValid())
		{
		CDateTime Origin(1, 1, 2001);
		CTimeSpan Span = timeSpan(Origin, ExpireTime);
		DWORD dwSeconds = Span.Seconds();
		sSeconds = strFromInt(dwSeconds);
		}
	else
		sSeconds = CONST_NO_EXPIRATION;

	//	Encode

	Token.Write("&", 1);
	dData.Serialize(CDatum::EFormat::AEONScript, Token);
	Token.Write("&", 1);
	Token.Write(sSeconds);
	Token.Write("&", 1);

	//	Now generate a MAC

	CIPInteger MAC;
	cryptoCreateMAC(Token, SecretKey, &MAC);

	//	Serialize as Base64

	CBase64Encoder Encoder(&Token);
	MAC.Serialize(Encoder);
	Encoder.Close();

	//	Done

	*retsToken = CString::CreateFromHandoff(Token);
	}

void CCryptosaurInterface::CreateChallengeCredentials (const CString &sUsername, const CString &sPassword, CDatum dChallenge, CDatum *retCredentials)

//	CreateChallengeCredentials
//
//	Creates a challenge response

	{
	CIPInteger Credentials;
	CreateCredentials(sUsername, sPassword, &Credentials);

	CDatum dCredentials;
	CDatum::CreateIPIntegerFromHandoff(Credentials, &dCredentials);

	*retCredentials = CAI1Protocol::CreateSHAPasswordChallengeResponse(dCredentials, dChallenge);
	}

void CCryptosaurInterface::CreateCredentials (const CString &sUsername, const CString &sPassword, CIPInteger *retCredentials)

//	CreateCredentials
//
//	Creates hashed credentials from a username and password.

	{
	CStringBuffer Buffer(strPattern("%s:HEXARC01:%s", strToLower(sUsername), sPassword));
	cryptoCreateDigest(Buffer, retCredentials);
	}

CDatum CCryptosaurInterface::SignData (CDatum dData, const CIPInteger &SecretKey)

//	SignData
//
//	Creates a signature (SHA1 hash) out of some data and a secret key

	{
	//	Serialize the data

	CBuffer Buffer;
	dData.Serialize(CDatum::EFormat::AEONScript, Buffer);

	//	Sign

	CIPInteger Signature;
	cryptoCreateMAC(Buffer, SecretKey, &Signature);

	//	Done

	CDatum dSignature;
	if (!CDatum::CreateIPIntegerFromHandoff(Signature, &dSignature))
		return CDatum();

	return dSignature;
	}

bool CCryptosaurInterface::ValidateAuthToken (const CString &sToken, const CIPInteger &SecretKey, CDatum *retdData)

//	ValidateAuthToken
//
//	Validates an authentication token. Returns TRUE if this is a valid token
//	(signature is valid, and it has not expired yet).

	{
	CBuffer Buffer(sToken);
	char *pBufferEnd = Buffer.GetPointer() + Buffer.GetLength();

	char chChar;
	Buffer.Read(&chChar, 1);
	if (chChar != '&')
		return false;

	//	Get the data

	CDatum::Deserialize(CDatum::EFormat::AEONScript, Buffer, retdData);

	Buffer.Read(&chChar, 1);
	if (chChar != '&')
		return false;

	//	Get the time string

	char *pTime = Buffer.GetPointer() + Buffer.GetPos();
	char *pPos = pTime;
	while (*pPos != '&' && pPos < pBufferEnd)
		pPos++;

	if (*pPos != '&')
		return false;

	CString sTime = CString(pTime, (int)(pPos - pTime));
	Buffer.Read(NULL, (int)(pPos - pTime));
	if (!strEquals(sTime, CONST_NO_EXPIRATION))
		{
		DWORD dwSeconds = strToInt(sTime, 0);

		//	Check to see if the time has expired

		CDateTime Origin(1, 1, 2001);
		CTimeSpan Span = timeSpan(Origin, CDateTime(CDateTime::Now));
		DWORD dwSecondsNow = Span.Seconds();
		if (dwSecondsNow > dwSeconds)
			return false;
		}

	//	Continue reading

	Buffer.Read(&chChar, 1);
	if (chChar != '&')
		return false;

	//	At this point we've read the entire message. Remember the length so that
	//	we can recreate the MAC.

	int iMessageLength = Buffer.GetPos();

	//	Get the MAC

	CBase64Decoder Decoder(&Buffer);
	CIPInteger MAC;
	if (!CIPInteger::Deserialize(Decoder, &MAC))
		return false;

	//	Recreate the MAC

	CBuffer Token(sToken, 0, iMessageLength);
	CIPInteger VerifyMAC;
	cryptoCreateMAC(Token, SecretKey, &VerifyMAC);

	//	MACs should equal

	return (MAC == VerifyMAC);
	}
