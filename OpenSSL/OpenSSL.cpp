//	OpenSSL.cpp
//
//	OpenSSL
//	Copyright (c) 2013 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

#include <openssl/evp.h>

void SHA256Hash(unsigned char digest[EVP_MAX_MD_SIZE], char *stringToHash)
{
 OpenSSL_add_all_digests();
 
 const EVP_MD *md = EVP_get_digestbyname("sha256");
 
 EVP_MD_CTX context;
 EVP_MD_CTX_init(&context);
 EVP_DigestInit_ex(&context, md, NULL); 
 EVP_DigestUpdate(&context, (unsigned char *)stringToHash, strlen(stringToHash));
 
 unsigned int digestSz;
 EVP_DigestFinal_ex(&context, digest, &digestSz);
 EVP_MD_CTX_cleanup(&context);
 
 EVP_cleanup();
}

