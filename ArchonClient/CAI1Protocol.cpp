//	CAI1Protocol.cpp
//
//	CAI1Protocol class
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(AUTH_TYPE_SHA1,					"SHA1")

DECLARE_CONST_STRING(FIELD_ACTUAL,						"actual")
DECLARE_CONST_STRING(FIELD_CREDENTIALS,					"credentials")
DECLARE_CONST_STRING(FIELD_TYPE,						"type")

CDatum CAI1Protocol::CreateAuthDescSHAPassword (const CString &sUsername, const CString &sPassword)

//	CreateAuthDescSHAPassword
//
//	Creates an authDesc structure with the hash of the password

	{
	//	Create credentials

	CIPInteger Credentials;
	CCryptosaurInterface::CreateCredentials(sUsername, sPassword, &Credentials);
	CDatum dCredentials;
	CDatum::CreateIPIntegerFromHandoff(Credentials, &dCredentials);

	//	Create an authDesc structure

	CComplexStruct *pAuthDesc = new CComplexStruct;
	pAuthDesc->SetElement(FIELD_TYPE, AUTH_TYPE_SHA1);
	pAuthDesc->SetElement(FIELD_ACTUAL, CDatum(CDatum::constTrue));
	pAuthDesc->SetElement(FIELD_CREDENTIALS, dCredentials);

	return CDatum(pAuthDesc);
	}

CDatum CAI1Protocol::CreateSHAPassword_V1 (const CString &sUsername, const CString &sPassword)

//	CreateSHAPassword
//
//	Creates a hash of username:password

	{
	CStringBuffer Source(strPattern("%s:%s", sUsername, sPassword));
	CIPInteger Hash;
	cryptoCreateDigest(Source, &Hash);
	CDatum dHash;
	CDatum::CreateIPIntegerFromHandoff(Hash, &dHash);

	return dHash;
	}

CDatum CAI1Protocol::CreateSHAPasswordChallenge (void)

//	CreateSHAPasswordChallenge
//
//	Creates a random number to be used as a challenge

	{
	CIPInteger Challenge;
	cryptoRandom(32, &Challenge);
	CDatum dChallenge;
	CDatum::CreateIPIntegerFromHandoff(Challenge, &dChallenge);

	return dChallenge;
	}

CDatum CAI1Protocol::CreateSHAPasswordChallengeResponse (CDatum dAuthSHAPassword, CDatum dChallenge)

//	CreateSHAPasswordChallengeResponse
//
//	Creates a response with the password and the challenge

	{
	//	Generate a buffer that concatenates the hashed password and the
	//	challenge.

	CBuffer Combination;

	const CIPInteger &HashedPassword = dAuthSHAPassword;
	const CIPInteger &Challenge = dChallenge;

	HashedPassword.WriteBytes(Combination);
	Combination.Write(":", 1);
	Challenge.WriteBytes(Combination);

	//	Generate a hash of the concatenation

	CIPInteger Hash;
	cryptoCreateDigest(Combination, &Hash);

	//	Return it as a datum

	CDatum dHash;
	CDatum::CreateIPIntegerFromHandoff(Hash, &dHash);

	return dHash;
	}

CDatum CAI1Protocol::CreateSHAPasswordChallengeResponse_V1 (const CString &sUsername, const CString &sPassword, CDatum dChallenge)

//	CreateSHAPasswordChallengeResponse
//
//	Creates a response with the password and the challenge

	{
	return CreateSHAPasswordChallengeResponse(CreateSHAPassword_V1(sUsername, sPassword), dChallenge);
	}
