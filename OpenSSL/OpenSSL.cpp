//	OpenSSL.cpp
//
//	OpenSSL
//	Copyright (c) 2013 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#include "openssl/evp.h"

void SHA256Hash (unsigned char digest[EVP_MAX_MD_SIZE], char *stringToHash)
	{
	EVP_MD_CTX *ctx = EVP_MD_CTX_new();
	if (!ctx)
		{
		// handle error (memory allocation failure)
		return;
		}

	// Initialize for SHA-256
	if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1)
		{
		// handle error
		EVP_MD_CTX_free(ctx);
		return;
		}

	// Hash the input string
	if (EVP_DigestUpdate(ctx, stringToHash, strlen(stringToHash)) != 1)
		{
		// handle error
		EVP_MD_CTX_free(ctx);
		return;
		}

	// Retrieve the final digest
	unsigned int digestLen = 0;
	if (EVP_DigestFinal_ex(ctx, digest, &digestLen) != 1)
		{
		// handle error
		EVP_MD_CTX_free(ctx);
		return;
		}

	// Clean up
	EVP_MD_CTX_free(ctx);
	}

