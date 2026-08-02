//	OpenSSL.cpp
//
//	OpenSSL
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#include "openssl/crypto.h"
#include "openssl/evp.h"
#include "openssl/err.h"
#include "openssl/hmac.h"
#include "openssl/rand.h"

DECLARE_CONST_STRING(STR_OPEN_SSL_TOO_MANY_RANDOM_BYTES_REQUESTED,	"Too many random bytes requested.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_GENERATE_OPEN_SSL_RANDOM_BYTES,	"Unable to generate OpenSSL random bytes.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_ALLOCATE_OPEN_SSL_DIGEST_CONTEXT,	"Unable to allocate OpenSSL digest context.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_INITIALIZE_OPEN_SSL_DIGEST,	"Unable to initialize OpenSSL digest.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_UPDATE_OPEN_SSL_DIGEST,	"Unable to update OpenSSL digest.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_FINALIZE_OPEN_SSL_DIGEST,	"Unable to finalize OpenSSL digest.");
DECLARE_CONST_STRING(STR_OPEN_SSL_HMAC_KEY_IS_TOO_LARGE,	"HMAC key is too large.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_COMPUTE_OPEN_SSL_HMAC,	"Unable to compute OpenSSL HMAC.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_ALLOCATE_OPEN_SSL_HMAC_CONTEXT,	"Unable to allocate OpenSSL HMAC context.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_INITIALIZE_OPEN_SSL_HMAC,	"Unable to initialize OpenSSL HMAC.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_UPDATE_OPEN_SSL_HMAC,	"Unable to update OpenSSL HMAC.");
DECLARE_CONST_STRING(STR_OPEN_SSL_UNABLE_TO_FINALIZE_OPEN_SSL_HMAC,	"Unable to finalize OpenSSL HMAC.");

static bool CalcDigest (const EVP_MD *pDigestType, const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError);
static bool CalcDigest (const EVP_MD *pDigestType, const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError);
static bool CalcHMAC (const EVP_MD *pDigestType, const void *pKey, size_t iKeyLength, const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError);
static bool CalcHMAC (const EVP_MD *pDigestType, const void *pKey, size_t iKeyLength, const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError);
static void SetOpenSSLError (CString *retsError, const CString &sDefault);

bool cryptoMD5 (const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoMD5
//
//	Computes an MD5 digest.

	{
	return CalcDigest(EVP_md5(), pData, iLength, retDigest, retsError);
	}

bool cryptoMD5 (const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoMD5
//
//	Computes an MD5 digest.

	{
	return cryptoMD5(Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoMD5 (const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoMD5
//
//	Computes an MD5 digest.

	{
	return CalcDigest(EVP_md5(), Data, retDigest, retsError);
	}

bool cryptoSHA1 (const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA1
//
//	Computes a SHA-1 digest.

	{
	return CalcDigest(EVP_sha1(), pData, iLength, retDigest, retsError);
	}

bool cryptoSHA1 (const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA1
//
//	Computes a SHA-1 digest.

	{
	return cryptoSHA1(Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoSHA1 (const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA1
//
//	Computes a SHA-1 digest.

	{
	return CalcDigest(EVP_sha1(), Data, retDigest, retsError);
	}

bool cryptoSHA256 (const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA256
//
//	Computes a SHA-256 digest.

	{
	return CalcDigest(EVP_sha256(), pData, iLength, retDigest, retsError);
	}

bool cryptoSHA256 (const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA256
//
//	Computes a SHA-256 digest.

	{
	return cryptoSHA256(Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoSHA256 (const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA256
//
//	Computes a SHA-256 digest.

	{
	return CalcDigest(EVP_sha256(), Data, retDigest, retsError);
	}

bool cryptoSHA384 (const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA384
//
//	Computes a SHA-384 digest.

	{
	return CalcDigest(EVP_sha384(), pData, iLength, retDigest, retsError);
	}

bool cryptoSHA384 (const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA384
//
//	Computes a SHA-384 digest.

	{
	return cryptoSHA384(Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoSHA384 (const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA384
//
//	Computes a SHA-384 digest.

	{
	return CalcDigest(EVP_sha384(), Data, retDigest, retsError);
	}

bool cryptoSHA512 (const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA512
//
//	Computes a SHA-512 digest.

	{
	return CalcDigest(EVP_sha512(), pData, iLength, retDigest, retsError);
	}

bool cryptoSHA512 (const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA512
//
//	Computes a SHA-512 digest.

	{
	return cryptoSHA512(Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoSHA512 (const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoSHA512
//
//	Computes a SHA-512 digest.

	{
	return CalcDigest(EVP_sha512(), Data, retDigest, retsError);
	}

bool cryptoHMACSHA1 (const void *pKey, size_t iKeyLength, const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoHMACSHA1
//
//	Computes an HMAC-SHA1 digest.

	{
	return CalcHMAC(EVP_sha1(), pKey, iKeyLength, pData, iLength, retDigest, retsError);
	}

bool cryptoHMACSHA1 (const IMemoryBlock &Key, const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoHMACSHA1
//
//	Computes an HMAC-SHA1 digest.

	{
	return cryptoHMACSHA1(Key.GetPointer(), Key.GetLength(), Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoHMACSHA1 (const void *pKey, size_t iKeyLength, const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoHMACSHA1
//
//	Computes an HMAC-SHA1 digest.

	{
	return CalcHMAC(EVP_sha1(), pKey, iKeyLength, Data, retDigest, retsError);
	}

bool cryptoHMACSHA256 (const void *pKey, size_t iKeyLength, const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	cryptoHMACSHA256
//
//	Computes an HMAC-SHA256 digest.

	{
	return CalcHMAC(EVP_sha256(), pKey, iKeyLength, pData, iLength, retDigest, retsError);
	}

bool cryptoHMACSHA256 (const IMemoryBlock &Key, const IMemoryBlock &Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoHMACSHA256
//
//	Computes an HMAC-SHA256 digest.

	{
	return cryptoHMACSHA256(Key.GetPointer(), Key.GetLength(), Data.GetPointer(), Data.GetLength(), retDigest, retsError);
	}

bool cryptoHMACSHA256 (const void *pKey, size_t iKeyLength, const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	cryptoHMACSHA256
//
//	Computes an HMAC-SHA256 digest.

	{
	return CalcHMAC(EVP_sha256(), pKey, iKeyLength, Data, retDigest, retsError);
	}

bool cryptoRandomBytes (size_t iLength, CStringBuffer *retData, CString *retsError)

//	cryptoRandomBytes
//
//	Generates cryptographically secure random bytes.

	{
	if (retData == NULL)
		throw CException(errFail);

	retData->SetLength(0);
	if (iLength == 0)
		return true;
	else if (iLength > INT_MAX)
		{
		if (retsError) *retsError = STR_OPEN_SSL_TOO_MANY_RANDOM_BYTES_REQUESTED;
		return false;
		}

	retData->SetLength((int)iLength);
	if (RAND_bytes((unsigned char *)retData->GetPointer(), (int)iLength) != 1)
		{
		retData->SetLength(0);
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_GENERATE_OPEN_SSL_RANDOM_BYTES);
		return false;
		}

	return true;
	}

bool cryptoConstantTimeEqual (const void *pData1, size_t iLength1, const void *pData2, size_t iLength2)

//	cryptoConstantTimeEqual
//
//	Compares two byte sequences in constant time if they are the same length.

	{
	if (iLength1 != iLength2)
		return false;
	else if (iLength1 == 0)
		return true;
	else
		return (CRYPTO_memcmp(pData1, pData2, iLength1) == 0);
	}

bool cryptoConstantTimeEqual (const IMemoryBlock &Data1, const IMemoryBlock &Data2)

//	cryptoConstantTimeEqual
//
//	Compares two byte sequences in constant time if they are the same length.

	{
	return cryptoConstantTimeEqual(Data1.GetPointer(), Data1.GetLength(), Data2.GetPointer(), Data2.GetLength());
	}

void SHA256Hash (unsigned char digest[EVP_MAX_MD_SIZE], char *stringToHash)
	{
	CStringBuffer Buffer;
	if (cryptoSHA256(stringToHash, strlen(stringToHash), &Buffer))
		utlMemCopy(Buffer.GetPointer(), digest, Buffer.GetLength());
	}

static bool CalcDigest (const EVP_MD *pDigestType, const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	CalcDigest
//
//	Computes a digest.

	{
	if (retDigest == NULL)
		throw CException(errFail);

	retDigest->SetLength(0);

	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	if (!ctx)
		{
		if (retsError) *retsError = STR_OPEN_SSL_UNABLE_TO_ALLOCATE_OPEN_SSL_DIGEST_CONTEXT;
		return false;
		}

	if (EVP_DigestInit_ex(ctx, pDigestType, NULL) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_INITIALIZE_OPEN_SSL_DIGEST);
		EVP_MD_CTX_free(ctx);
		return false;
		}

	if (iLength > 0 && EVP_DigestUpdate(ctx, pData, iLength) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_UPDATE_OPEN_SSL_DIGEST);
		EVP_MD_CTX_free(ctx);
		return false;
		}

	unsigned char Digest[EVP_MAX_MD_SIZE];
	unsigned int digestLen = 0;
	if (EVP_DigestFinal_ex(ctx, Digest, &digestLen) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_FINALIZE_OPEN_SSL_DIGEST);
		EVP_MD_CTX_free(ctx);
		return false;
		}

	EVP_MD_CTX_free(ctx);

	retDigest->Write(Digest, (int)digestLen);
	return true;
	}

static bool CalcDigest (const EVP_MD *pDigestType, const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	CalcDigest
//
//	Computes a digest incrementally.

	{
	if (retDigest == NULL)
		throw CException(errFail);

	retDigest->SetLength(0);

	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	if (!ctx)
		{
		if (retsError) *retsError = STR_OPEN_SSL_UNABLE_TO_ALLOCATE_OPEN_SSL_DIGEST_CONTEXT;
		return false;
		}

	if (EVP_DigestInit_ex(ctx, pDigestType, NULL) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_INITIALIZE_OPEN_SSL_DIGEST);
		EVP_MD_CTX_free(ctx);
		return false;
		}

	if (!Data(
			[ctx, retsError](const void *pData, size_t iLength)
				{
				if (iLength > 0 && EVP_DigestUpdate(ctx, pData, iLength) != 1)
					{
					SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_UPDATE_OPEN_SSL_DIGEST);
					return false;
					}

				return true;
				},
			retsError))
		{
		EVP_MD_CTX_free(ctx);
		return false;
		}

	unsigned char Digest[EVP_MAX_MD_SIZE];
	unsigned int digestLen = 0;
	if (EVP_DigestFinal_ex(ctx, Digest, &digestLen) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_FINALIZE_OPEN_SSL_DIGEST);
		EVP_MD_CTX_free(ctx);
		return false;
		}

	EVP_MD_CTX_free(ctx);

	retDigest->Write(Digest, (int)digestLen);
	return true;
	}

static bool CalcHMAC (const EVP_MD *pDigestType, const void *pKey, size_t iKeyLength, const void *pData, size_t iLength, CStringBuffer *retDigest, CString *retsError)

//	CalcHMAC
//
//	Computes an HMAC digest.

	{
	if (retDigest == NULL)
		throw CException(errFail);

	retDigest->SetLength(0);
	if (iKeyLength > INT_MAX)
		{
		if (retsError) *retsError = STR_OPEN_SSL_HMAC_KEY_IS_TOO_LARGE;
		return false;
		}

	unsigned char Digest[EVP_MAX_MD_SIZE];
	unsigned int digestLen = 0;
	const void *pHMACKey = (iKeyLength > 0 ? pKey : "");
	const unsigned char *pHMACData = (iLength > 0 ? (const unsigned char *)pData : (const unsigned char *)"");
	if (HMAC(pDigestType, pHMACKey, (int)iKeyLength, pHMACData, iLength, Digest, &digestLen) == NULL)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_COMPUTE_OPEN_SSL_HMAC);
		return false;
		}

	retDigest->Write(Digest, (int)digestLen);
	return true;
	}

#pragma warning(push)
#pragma warning(disable: 4996)

static bool CalcHMAC (const EVP_MD *pDigestType, const void *pKey, size_t iKeyLength, const FCryptoByteProducer& Data, CStringBuffer *retDigest, CString *retsError)

//	CalcHMAC
//
//	Computes an HMAC digest incrementally.

	{
	if (retDigest == NULL)
		throw CException(errFail);

	retDigest->SetLength(0);
	if (iKeyLength > INT_MAX)
		{
		if (retsError) *retsError = STR_OPEN_SSL_HMAC_KEY_IS_TOO_LARGE;
		return false;
		}

	HMAC_CTX *ctx = HMAC_CTX_new();
	if (!ctx)
		{
		if (retsError) *retsError = STR_OPEN_SSL_UNABLE_TO_ALLOCATE_OPEN_SSL_HMAC_CONTEXT;
		return false;
		}

	const void *pHMACKey = (iKeyLength > 0 ? pKey : "");
	if (HMAC_Init_ex(ctx, pHMACKey, (int)iKeyLength, pDigestType, NULL) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_INITIALIZE_OPEN_SSL_HMAC);
		HMAC_CTX_free(ctx);
		return false;
		}

	if (!Data(
			[ctx, retsError](const void *pData, size_t iLength)
				{
				if (iLength > 0 && HMAC_Update(ctx, (const unsigned char *)pData, iLength) != 1)
					{
					SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_UPDATE_OPEN_SSL_HMAC);
					return false;
					}

				return true;
				},
			retsError))
		{
		HMAC_CTX_free(ctx);
		return false;
		}

	unsigned char Digest[EVP_MAX_MD_SIZE];
	unsigned int digestLen = 0;
	if (HMAC_Final(ctx, Digest, &digestLen) != 1)
		{
		SetOpenSSLError(retsError, STR_OPEN_SSL_UNABLE_TO_FINALIZE_OPEN_SSL_HMAC);
		HMAC_CTX_free(ctx);
		return false;
		}

	HMAC_CTX_free(ctx);

	retDigest->Write(Digest, (int)digestLen);
	return true;
	}

#pragma warning(pop)

static void SetOpenSSLError (CString *retsError, const CString &sDefault)

//	SetOpenSSLError
//
//	Sets error, if requested.

	{
	if (retsError == NULL)
		return;

	unsigned long dwError = ERR_get_error();
	if (dwError == 0)
		*retsError = sDefault;
	else
		{
		char szBuffer[256];
		ERR_error_string_n(dwError, szBuffer, sizeof(szBuffer));
		*retsError = CString(szBuffer);
		}
	}

