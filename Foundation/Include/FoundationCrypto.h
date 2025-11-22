//	FoundationCrypto.h
//
//	Foundation header file
//	Copyright (c) 2011 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

enum EPasswordFlags 
	{
	CRYPTOPASS_MIXED_CASE =				0x00000001,
	CRYPTOPASS_NUMBERS =				0x00000002,
	CRYPTOPASS_SYMBOLS =				0x00000004,
	};

typedef BYTE DIGEST [20];

//	Cryptographic Algorithms

class CCryptoDigest
	{
	public:
		CCryptoDigest (void) { }

		void AddData (void *pData, int iLength);
		void AddData (IMemoryBlock &Data);
		void CalcDigest (CIPInteger *retDigest);
		void CalcDigest (BYTE *retDigest);
		void Reset (void) { m_bInitialized = false; }

	private:
		struct SHA_CTX
			{
			DWORD digest[5] = { 0 };		//	Message digest
			DWORD countLo = 0;				//	64-bit count
			DWORD countHi = 0;
			DWORD data[16] = { 0 };			//	SHS data buffer

			bool bBigEndian = false;
			};

		void SHAInit(SHA_CTX *);
		void SHAUpdate(SHA_CTX *, BYTE *buffer, int count);
		void SHAFinal(BYTE *output, SHA_CTX *);

		bool m_bInitialized = false;
		SHA_CTX m_Ctx;
	};

CStringBuffer cryptoBLAKE2 (const IMemoryBlock& Data, int iPos = 0, int iLength = -1);
void cryptoCreateDigest (IMemoryBlock &Data, CIPInteger *retd);
void cryptoCreateMAC (IMemoryBlock &Data, const CIPInteger &Key, CIPInteger *retMAC);
void cryptoRandom (int iCount, CIPInteger *retx);
CString cryptoRandomCode (int iChars, DWORD dwFlags = 0);
CString cryptoRandomCode29 (int iChars, DWORD dwFlags = 0);
CString cryptoRandomCodeBlock (int iChars);
CString cryptoRandomUserPassword (int iChars, DWORD dwFlags = (CRYPTOPASS_MIXED_CASE | CRYPTOPASS_NUMBERS));

