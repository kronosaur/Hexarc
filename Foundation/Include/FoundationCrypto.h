//	FoundationCrypto.h
//
//	Foundation header file
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.
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
		CCryptoDigest (void) : m_bInitialized(false)
			{ }

		void AddData (void *pData, int iLength);
		void AddData (IMemoryBlock &Data);
		void CalcDigest (CIPInteger *retDigest);
		void CalcDigest (BYTE *retDigest);
		inline void Reset (void) { m_bInitialized = false; }

	private:
		struct SHA_CTX
			{
			DWORD digest[5];			//	Message digest
			DWORD countLo;				//	64-bit count
			DWORD countHi;
			DWORD data[16];             //	SHS data buffer

			bool bBigEndian;
			};

		void SHAInit(SHA_CTX *);
		void SHAUpdate(SHA_CTX *, BYTE *buffer, int count);
		void SHAFinal(BYTE *output, SHA_CTX *);

		bool m_bInitialized;
		SHA_CTX m_Ctx;
	};

void cryptoCreateDigest (IMemoryBlock &Data, CIPInteger *retd);
void cryptoCreateMAC (IMemoryBlock &Data, const CIPInteger &Key, CIPInteger *retMAC);
void cryptoRandom (int iCount, CIPInteger *retx);
CString cryptoRandomCode (int iChars, DWORD dwFlags = 0);
CString cryptoRandomUserPassword (int iChars, DWORD dwFlags = (CRYPTOPASS_MIXED_CASE | CRYPTOPASS_NUMBERS));

