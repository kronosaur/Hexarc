/* $Id: t_bdDSA.c $ */

/******************** SHORT COPYRIGHT NOTICE**************************
This source code is part of the BigDigits multiple-precision
arithmetic library Version 2.2 originally written by David Ireland,
copyright (c) 2001-8 D.I. Management Services Pty Limited, all rights
reserved. It is provided "as is" with no warranties. You may use
this software under the terms of the full copyright notice
"bigdigitsCopyright.txt" that should have been included with this
library or can be obtained from <www.di-mgt.com.au/bigdigits.html>.
This notice must always be retained in any copy.
******************* END OF COPYRIGHT NOTICE***************************/
/*
	Last updated:
	$Date: 2008-07-31 12:54:00 $
	$Revision: 2.2.0 $
	$Author: dai $
*/

/* EXAMPLE OF THE DSA from APPENDIX 5 FIPS PUB 186-2 
   "DIGITAL SIGNATURE STANDARD (DSS)", 27 January 2000 */

#if _MSC_VER >= 1100
	/* Detect memory leaks in MSVC++ */ 
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#else
	#include <stdlib.h>
#endif

#include <stdio.h>
#include <assert.h>
#include "bigd.h"


static void pr_msg(char *msg, BIGD b)
{
	printf("%s", msg);
	bdPrint(b, BD_PRINT_TRIM | BD_PRINT_NL);
}


int main(void)
{
	BIGD p, q, g;
	BIGD x, k;
	BIGD hashm;
	BIGD k1, y, tmp;
	BIGD r, s;
	BIGD w, u1, u2, v, gg, yy;

/* MSVC memory leak checking stuff */
#if _MSC_VER >= 1100
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDOUT );
#endif

	printf("EXAMPLE OF THE DSA from APPENDIX 5 FIPS PUB 186-2 'DIGITAL SIGNATURE STANDARD'\n");

	/* Initialise variables */
	p = bdNew();
	q = bdNew();
	g = bdNew();
	x = bdNew();
	k = bdNew();
	k1 = bdNew();
	y = bdNew();
	tmp = bdNew();
	hashm= bdNew();
	r= bdNew();
	s= bdNew();
	w = bdNew();
	u1 = bdNew();
	u2 = bdNew();
	v = bdNew();
	gg = bdNew();
	yy = bdNew();

	/* INPUT */
	/* Convert integers from hex format to BIGD (spaces are ignored) */
	bdConvFromHex(p, "8df2a494 492276aa 3d25759b b06869cb eac0d83a fb8d0cf7"
		"cbb8324f 0d7882e5 d0762fc5 b7210eaf c2e9adac 32ab7aac"
		"49693dfb f83724c2 ec0736ee 31c80291");
	bdConvFromHex(q, "c773218c 737ec8ee 993b4f2d ed30f48e dace915f");
	bdConvFromHex(g, "626d0278 39ea0a13 413163a5 5b4cb500 299d5522 956cefcb"
		"3bff10f3 99ce2c2e 71cb9de5 fa24babf 58e5b795 21925c9c"
		"c42e9f6f 464b088c c572af53 e6d78802");
	bdConvFromHex(x, "2070b322 3dba372f de1c0ffc 7b2e3b49 8b260614");
	bdConvFromHex(k, "358dad57 1462710f 50e254cf 1a376b2b deaadfbf");
	/* M = ASCII form of "abc" (See FIPS PUB 180-1, Appendix A)
	   (SHA-1)(M) = <pre-computed value> */
	bdConvFromHex(hashm, "a9993e36 4706816a ba3e2571 7850c26c 9cd0d89d");

	pr_msg("p=\n", p);
	pr_msg("q=", q);
	pr_msg("g=\n", g);
	pr_msg("x=\n", x);
	pr_msg("k=", k);
	pr_msg("SHA-1(M)=", hashm);

	/* COMPUTATION BY SIGNER */
	/* (p, q, g) are public, common values
	   x is signer's private key
	   k is a random integer 0 < k < q
	   Keep (x, k) secret
	*/
	/* Compute k' = k^-1 mod q */
	bdModInv(k1, k, q);
	pr_msg("k^-1=", k1);
	/* Compute y = g^x mod p */
	bdModExp(y, g, x, p);
	pr_msg("y=\n", y);
	/* Compute r = (g^k mod p) mod q */
	bdModExp(tmp, g, k, p);
	bdModulo(r, tmp, q);
	pr_msg("r=", r);
	/* Compute s = (k^-1(SHA-1(M) + xr)) mod q */
	bdModMult(tmp, x, r, q);
	bdAdd(tmp, tmp, hashm);
	bdModMult(s, k1, tmp, q);
	pr_msg("s=", s);

	/* VERIFICATION BY RECEIVER */
	/* (p, q, g) are public, common values
	   y is signer's public key
	   (r,s) = signature(M)
	   Receiver receives (M', r', s').
	   Receiver recomputes SHA-1(M') for received message M'
	*/

	/* Check 0 < r' < q and 0 < s' < q */
	assert(bdCompare(q, r) > 0);
	assert(bdCompare(q, s) > 0);
	/* Compute w = s^-1 mod q */
	bdModInv(w, s, q);
	pr_msg("w=", w);
	/* Compute u1 = ((SHA-1(M'))w) mod q */
	bdModMult(u1, hashm, w, q);
	pr_msg("u1=", u1);
	/* Compute u2 = ((r')w) mod q */
	bdModMult(u2, r, w, q);
	pr_msg("u2=", u2);
	/* Compute g^u1 mod p */
	bdModExp(gg, g, u1, p);
	pr_msg("g^u1 mod p=\n", gg);
	/* Compute y^u2 mod p */
	bdModExp(yy, y, u2, p);
	pr_msg("y^u2 mod p=\n", yy);
	/* Compute v = (((g^u1)(y^u2)) mod p) mod q */
	bdModMult(tmp, gg, yy, p);
	bdModulo(v, tmp, q);
	pr_msg("v=", v);
	/* Verify that v = r' */
	if (bdIsEqual(v, r))
		printf("Signature verified OK\n");
	else
		printf("Signature verification FAILED!\n");

	assert(bdIsEqual(v, r));

	/* Clean up */
	bdFree(&p);
	bdFree(&q);
	bdFree(&g);
	bdFree(&x);
	bdFree(&k);
	bdFree(&k1);
	bdFree(&y);
	bdFree(&tmp);
	bdFree(&hashm);
	bdFree(&r);
	bdFree(&s);
	bdFree(&w);
	bdFree(&u1);
	bdFree(&u2);
	bdFree(&v);
	bdFree(&gg);
	bdFree(&yy);

	printf("OK, successfully completed tests.\n");

	return 0;
}
