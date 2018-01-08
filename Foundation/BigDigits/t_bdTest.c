/* $Id: t_bdTest.c $ */

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

/* Various tests of "bd" functions, not exhaustive */

#if _MSC_VER >= 1100
	/* Detect memory leaks in MSVC++ */ 
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#else
	#include <stdlib.h>
#endif

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "bigd.h"
#include "bigdRand.h"

static void pr_msg(const char *msg, BIGD b)
/* Display a message followed by a BIGD value */
{
	printf("%s", msg);
	bdPrint(b, BD_PRINT_NL);
}

/*	CAUTION: with the pr_msg or pr_bytes functions, do not pass a 
	format argument (eg %d) in `msg' or you will crash your program!
*/

static void pr_bytesmsg(const char *msg, unsigned char *bytes, size_t nbytes)
/* Display a message followed by the hex values of a byte array */
{
	size_t i;
	printf("%s", msg);
	for (i = 0; i < nbytes; i++)
		printf("%02x", bytes[i]);
	printf("\n");
}	

int my_rand(unsigned char *bytes, size_t nbytes, const unsigned char *seed, size_t seedlen)
/* Our own (very insecure) random generator call-back function using good old rand() 
   This demonstrates the required format for BD_RANDFUNC
   -- replace this in practice with your own cryptographically-secure function.
*/
{
	unsigned int myseed;
	size_t i;
	int offset;

	/* Use time - then blend in seed, if any */
	myseed = (unsigned)time(NULL);
	if (seed)
	{
		for (offset = 0, i = 0; i < seedlen; i++, offset = (offset + 1) % sizeof(unsigned))
			myseed ^= ((unsigned int)seed[i] << (offset * 8));
	}

	srand(myseed);
	while (nbytes--)
	{
		*bytes++ = rand() & 0xFF;
	}

	return 0;
}

int fermat_test(BIGD n)
{
/* Carries out a quick Fermat primality test on n > 4.
   Returns 1 if n is prime and 0 if composite or -1 if n < 5.
   This is not foolproof but we use it as a 
   check on our main bdIsPrime function.
   It will return a false positive for Fermat Liars, which
   are very rare.
*/
	BIGD a, e, r;
	int isprime = 1;

	/* For any integer a, 1<=a<=n-1, if a^(n-1) mod n != 1
	   then n is composite. */

	if (bdShortCmp(n, 5) < 0) return -1;

	a = bdNew();
	e = bdNew();
	r = bdNew();
	/* e = n -1 */
	bdSetEqual(e, n);
	bdDecrement(e);

	/* Set a = 2 and compute a^(n-1) mod n */
	bdSetShort(a, 2);
	bdModExp(r, a, e, n);
	if (bdShortCmp(r, 1) != 0)
		isprime = 0;

	/* a = 3 */
	bdSetShort(a, 3);
	bdModExp(r, a, e, n);
	if (bdShortCmp(r, 1) != 0)
		isprime = 0;

	/* a = 5 */
	bdSetShort(a, 5);
	bdModExp(r, a, e, n);
	if (bdShortCmp(r, 1) != 0)
		isprime = 0;

	bdFree(&a);
	bdFree(&e);
	bdFree(&r);

	return isprime;
}

int main(void)
{
	BIGD u, v, w, q, r, p, b, a;
	BIGD n, e, m, c, z, mz, cz, t;
	bdigit_t overflow;
	int cmp, res;
	unsigned char ff[64], bytes[16];
	int i, mc, jac;
	size_t nbytes;
	char s[128];

/* MSVC memory leak checking stuff */
#if _MSC_VER >= 1100
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_WARN, _CRTDBG_FILE_STDOUT );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ERROR, _CRTDBG_FILE_STDOUT );
#endif

	printf("Tests for BIGD functions\n");

	/* Initialise */
	u = bdNew();
	v = bdNew();
	w = bdNew();
	p = bdNew();
	q = bdNew();
	r = bdNew();

	printf("SIMPLE ARITHMETIC OPERATIONS...\n");
	pr_msg("At start w=", w);
	assert(bdIsZero(w));

	/* Addition */
	/* 1 + 1 = 2 */
	bdSetShort(u, 1);
	bdSetShort(v, 1);
	bdAdd(w, u, v);
	pr_msg("1+1=", w);
	assert(bdShortCmp(w, 2) == 0);

	/* Add with digit overflow */
	bdSetShort(u, 0xffffffff);
	bdSetShort(v, 0xffffffff);
	bdAdd(w, u, v);
	pr_msg("ffffffff+ffffffff=", w);
	/* ffffffff+ffffffff=00000001 fffffffe */
	assert(bdSizeof(w) == 2);

	/* Bigger random digits */
	bdSetRandTest(u, 10);
	bdSetRandTest(v, 10);
	bdAdd(w, u, v);
	pr_msg("u=\n", u);
	pr_msg("v=\n", v);
	pr_msg("w=u+v=\n", w);

	/* Subtract */
	bdSubtract(r, w, v);
	pr_msg("w-v=\n", r);
	assert(bdCompare(r, u) == 0);

	/* Multiplication */
	/* 2 * 3 = 6 */
	bdSetShort(u, 2);
	bdSetShort(v, 3);
	bdMultiply(w, u, v);
	pr_msg("2 * 3=", w);
	assert(bdShortCmp(w, 6) == 0);

	bdSetShort(u, 0xffffffff);
	bdSetShort(v, 0xffffffff);
	bdMultiply(w, u, v);
	pr_msg("ffffffff*ffffffff=", w);
	/* ffffffff*ffffffff=fffffffe 00000001 */
	assert(bdGetBit(w, 0) == 1);

	/* Use ShortMult */
	bdShortMult(p, u, 0xffffffff);
	pr_msg("(Short)ffffffff*ffffffff=", p);
	assert(bdCompare(p, w) == 0);

	/* Use larger random u, v */
	bdSetRandTest(u, 10);
	bdSetRandTest(v, 20);
	pr_msg("u=\n", u);
	pr_msg("v=\n", v);

	/* Use ShortMult by one: w = v * 1 */
	bdShortMult(w, v, 1);
	pr_msg("(Short)v*1=\n", w);
	assert(bdCompare(w, v) == 0);

	/* Use ShortMult by two */
	bdShortMult(w, u, 2);
	pr_msg("(Short)u*2=\n", w);
	assert(bdIsEven(w));

	/* Big multiplication */
	bdMultiply(w, u, v);
	pr_msg("w=u*v=\n", w);

	/* Divide expecting q = w/v == u and r = w%v == 0 */
	bdDivide(q, r, w, v);
	pr_msg("w/v=\n", q);
	pr_msg("w%v=", r);
	assert(bdIsEqual(q, u));
	assert(bdShortCmp(r, 0) == 0);

	/* Modulo */
	bdIncrement(w);
	bdModulo(r, w, v);
	pr_msg("w+1 mod v=", r);
	assert(bdShortCmp(r, 1) == 0);

	bdDecrement(w);
	bdModulo(r, w, u);
	pr_msg("w+1-1 mod u=", r);
	assert(bdShortCmp(r, 0) == 0);

	/* Use a short divisor q=w/2 */
	bdShortDiv(q, r, w, 2);
	pr_msg("(ShortDiv)w/2=\n", q);
	pr_msg("(ShortDiv)w%2=", r);

	printf("w is %s\n", bdIsOdd(w) ? "ODD" : "EVEN");
	bdShortMod(r, w, 2);
	pr_msg("w mod 2=", r);
	bdIncrement(w);
	printf("w+1 is %s\n", bdIsOdd(w) ? "ODD" : "EVEN");
	bdShortMod(r, w, 2);
	pr_msg("++w mod 2=", r);
	bdDecrement(w);

	/* Check with short mult u = q*2 */
	bdShortMult(u, q, 2);
	pr_msg("(ShortMult) (w/2) * 2=\n", u);

	printf("SQUARE AND SQUARE ROOT...\n");
	/* Square a random number w = u^2 */
	bdSetRandTest(u, 10);
	pr_msg("random u=\n", u);
	bdSquare(w, u);
	pr_msg("(Square) u^2=", w);
	/* check against product p = u * u */
	bdMultiply(p, u, u);
	pr_msg("u*u=", p);
	assert(bdIsEqual(p, w));
	/* Compute integer square root [new in v2.1] */
	bdSqrt(p, w);
	pr_msg("sqrt(w)=", p);
	assert(bdIsEqual(p, u));

	printf("MODULAR ARITHMETIC...\n");
	/* Check that (u mod n + v mod n) mod n == (u+v) mod n */
	bdSetRandTest(u, 10);
	bdSetRandTest(v, 10);
	bdSetRandTest(w, 10);
	pr_msg("(Modulo)\nNew random u=", u);
	pr_msg("v=", v);
	pr_msg("w=", w);
	/* r = u mod w */
	bdModulo(r, u, w);
	/* q = v mod w */
	bdModulo(q, v, w);
	/* q = u mod n + v mod n */
	bdAdd(q, q, r);
	/* r = (u mod w + v mod w) mod w */
	bdModulo(r, q, w);
	pr_msg("(u mod w + v mod w) mod w=\n", r);
	/* q = (u+v) mod w */
	bdSetEqual(p, u);
	bdAdd(p, p, v);
	bdModulo(q, p, w);
	pr_msg("(u+v) mod w=\n", q);
	assert(bdIsEqual(r, q));

	/* Clear and start again */
	bdSetZero(u);
	bdSetZero(v);
	bdSetZero(w);

	printf("ARITHMETIC BOUNDARY CONDITIONS...\n");
	/* This causes Divide algorithm to `Add Back' with 32-bit digits 
	   See Knuth 4.3.1 Algorithm D Step D6 */
	printf("Check `Add Back' in Divide algorithm\n");
	bdConvFromHex(u, "7fffffff 80000001 00000000 00000000");
	bdConvFromHex(v, "00000000 80000000 80000002 00000005");
	bdDivide(q, r, u, v);
	pr_msg("u=", u);
	pr_msg("v=", v);
	pr_msg("q=u/v=", q);
	pr_msg("r=u mod v=", r);
	/* check that qv + r = u */
	bdMultiply(w, q, v);
	bdAdd(w, w, r);
	pr_msg("qv+r=", w);
	assert(bdIsEqual(w, u));
	/* And check that r = u mod v >= (1-2/b)v 
	   See Knuth 4.3.1 Exercise 21 */
	bdConvFromHex(p, "1 00000000");
	bdSetEqual(w, p);
	bdShortSub(w, w, 2);
	bdMultiply(u, w, v);
	bdDivide(q, w, u, p);
	pr_msg("(1-2/b)=", q);
	cmp = bdCompare(r, q);
	printf("u mod v %s (1-2/b)v\n", (cmp >= 0 ? ">=" : "<"));


	/* Cope, sort of, with `negative' numbers */
	printf("Go ``negative''...\n");
	bdSetZero(u);
	overflow = bdShortSub(w, u, 2);
	pr_msg("w=0-2=", w);
	printf("overflow=%" PRIuBIGD "\n", overflow);
	overflow = bdIncrement(w);
	pr_msg("w+1=", w);
	printf("overflow=%" PRIuBIGD "\n", overflow);
	overflow = bdIncrement(w);
	pr_msg("(Note error!) w+1=", w);
	printf("overflow=%" PRIuBIGD "\n", overflow);

	bdSetZero(u);
	overflow = bdShortAdd(u, u, 1);
	pr_msg("u=0+1=", u);
	printf("overflow=%" PRIuBIGD "\n", overflow);

	bdSetZero(u);
	bdSetZero(v);
	bdAdd(w, u, v);
	pr_msg("0+0=", w);

	/* Set u = v = ffff...ffff */
	for (i = 0; i < sizeof(ff); i++)
		ff[i] = 0xff;
	bdConvFromOctets(u, ff, sizeof(ff));
	bdConvFromOctets(v, ff, sizeof(ff));
	pr_msg("u=", u);
	pr_msg("v=", v);

	/* Compute u * v */
	bdMultiply(w, u, v);
	pr_msg("w = u * v =\n", w);
	/* Set v = w / u */
	bdDivide(v, r, w, u);
	pr_msg("v = w / u =\n", v);
	pr_msg("rem=", r);
	assert(bdIsEqual(u, v));
	assert(bdShortCmp(r, 0) == 0);

	bdIncrement(u);
	bdDecrement(v);
	bdMultiply(w, u, v);
	pr_msg("w = ++u * --v =\n", w);
	bdDivide(w, r, u, v);
	pr_msg("q = u / v =", w);
	pr_msg("rem=", r);
	assert(bdShortCmp(w, 1) == 0);
	assert(bdShortCmp(r, 2) == 0);


	/* Clear and start again */
	bdSetZero(u);
	bdSetZero(v);
	bdSetZero(w);

	/* Set a new random value for v */
	bdSetRandTest(v, 10);
	pr_msg("Random v=\n", v);
	bdShortAdd(u, v, 1);
	pr_msg("u = v + 1 =\n", u);
	cmp = bdCompare(u, v);
	printf("bdCompare(u-v) = %d\n", cmp);
	assert(cmp > 0);
	cmp = bdCompare(v, u);
	printf("bdCompare(v-u) = %d\n", cmp);
	assert(cmp < 0);
	cmp = bdCompare(u, u);
	printf("bdCompare(u-u) = %d\n", cmp);
	assert(cmp == 0);

	/* Compare with short */
	bdSetShort(w, 0xfffffffe);
	pr_msg("w=", w);
	cmp = bdShortCmp(w, 1);
	assert(cmp > 0);
	printf("bdShortCmp(w-1) = %d\n", cmp);
	cmp = bdShortCmp(w, 0xffffffff);
	assert(cmp < 0);
	printf("bdShortCmp(w-0xffffffff) = %d\n", cmp);
	cmp = bdShortCmp(w, 0xfffffffd);
	printf("bdShortCmp(w-0xfffffffd) = %d\n", cmp);
	assert(cmp > 0);

	printf("BIT SHIFT OPERATIONS...\n");
	/* Try shifting bits */
	pr_msg("w      = ", w);
	bdFree(&u);
	u = bdNew();
	bdShiftLeft(u, w, 2);
	pr_msg("w << 2 = ", u);
	assert(bdGetBit(u, 33) == 1);
	bdShiftRight(v, u, 3);
	pr_msg("(w << 2) >> 3 = ", v);
	assert(bdGetBit(v, 31) == 0);

	/* Bigger shift */
	bdShiftLeft(u, w, 128);
	pr_msg("w << 128 = ", u);
	assert(bdGetBit(u, 159) == 1);
	bdShiftRight(v, u, 129);
	pr_msg("(w << 128) >> 129 = ", v);
	assert(bdGetBit(v, 31) == 0);

	/* Use a new variable */
	b = bdNew();
	bdSetZero(b);	/* overkill */
	bdSetBit(b, 510, 1);
	bdSetBit(b, 477, 1);
	bdSetBit(b, 31, 1);
	bdSetBit(b, 0, 1);
	pr_msg("b=\n", b);

	bdSetBit(b, 512, 1);
	bdSetBit(b, 31, 0);
	pr_msg("b'=\n", b);
	printf("Bit 512 is %d\n", bdGetBit(b, 512));
	assert(bdGetBit(b, 512) == 1);
	printf("Bit 511 is %d\n", bdGetBit(b, 511));
	assert(bdGetBit(b, 511) == 0);
	assert(bdGetBit(b, 477) == 1);
	/* NB one-based bit lengths so expecting 513 */
	printf("Bit Length = %u\n", bdBitLength(b));
	assert(bdBitLength(b) == 513);
	pr_msg("b'=\n", b);
	/* Destroy b */
	bdFree(&b);
	
	printf("RANDOM NUMBER GENERATION...\n");
	/* Create a random number using `my_rand' callback function */
	bdRandomSeeded(r, 508, (const unsigned char*)"", 0, my_rand);
	pr_msg("random=\n", r);
	/* NB bit length may be less than 508 */
	printf("%u bits\n", bdBitLength(r));
	assert(bdBitLength(r) <= 508);

	/* Create a random number using the internal RNG */
	bdRandomBits(r, 509);
	pr_msg("random=\n", r);
	printf("%u bits\n", bdBitLength(r));
	assert(bdBitLength(r) <= 509);

	printf("PRIME NUMBERS...\n");
	/* Generate two random primes */
	printf("Generating two primes...\n");
	bdGeneratePrime(p, 128, 5, (const unsigned char*)"1", 1, my_rand);
	pr_msg("prime p=\n", p);
	bdGeneratePrime(q, 128, 5, (const unsigned char*)"abcdef", 6, my_rand);
	pr_msg("prime q=\n", q);

	/* Check primality with more tests (64 vs 5) */
	/* NB vv small chance this could fail */
	printf("Checking prime: p ");
	res = bdIsPrime(p, 64);
	printf("%s\n", (res ? "is OK" : "is NOT prime!!"));
	/* And again using Fermat test */
	res = fermat_test(p);
	if (res == 0) printf("WARNING: p failed Fermat test.\n");

	/* w = product pq */
	bdMultiply(w, p, q);
	pr_msg("pq=\n", w);

	/* Product pq should not be prime */
	res = bdIsPrime(w, 5);
	printf("Composite pq %s\n", (res ? "IS prime" : "is NOT prime"));
	assert(res == 0);

	/* Check GCD: gcd(pq, p) == p */
	bdGcd(r, w, p);
	pr_msg("gcd(w, p) =\n", r);
	assert(bdIsEqual(r, p));

	/* --Modular inversion-- */
	printf("MODULAR INVERSION...\n");
	/* set v to be a small random multiplier */
	bdSetRandTest(v, 1);
	bdIncrement(v);	/* Make sure not zero */
	pr_msg("multiplier, v=", v);
	/* set u = vp - 1 */
	bdMultiply(u, v, p);
	bdDecrement(u);
	pr_msg("p=\n", p);
	pr_msg("u = vp - 1 =\n", u);

	/* compute w = u^-1 mod p */
	res = bdModInv(w, u, p);
	pr_msg("w = u^-1 mod p=\n", w);
	assert(res == 0);

	/* check that wu mod p == 1 */
	bdModMult(r, w, u, p);
	pr_msg("wu mod p = ", r);
	assert(bdShortCmp(r, 1) == 0);

	/* Try mod inversion that should fail */
	/* Set u = pq so that gcd(u, p) != 1 */
	bdMultiply(u, p, q);
	res = bdModInv(w, u, p);
	printf("w = (pq)^-1 mod p returns (expected) error %d\n", res);
	assert(res != 0);
	pr_msg("output, w=", w);

	/* Do some bit string operations */
	printf("BIT STRING OPERATIONS...\n");

	/* Use XOR to swop two variables without a temp variable, i.e.
	   a = a XOR b; b = a XOR b; a = a XOR b; */
	printf("Swop u and w:\n");
	/* Generate 2 random numbers (u,w) each 144 bits long */
	bdRandomBits(u, 144);
	bdRandomBits(w, 144);
	pr_msg("u= ", u);
	pr_msg("w= ", w);
	/* remember these for later (p=u, q=w) */
	bdSetEqual(p, u);
	bdSetEqual(q, w);
	/* Swop using XOR */
	bdXorBits(u, u, w);
	bdXorBits(w, u, w);
	bdXorBits(u, u, w);
	pr_msg("u'=", u);
	pr_msg("w'=", w);
	/* check they have swopped accurately */
	assert(bdIsEqual(u, q));
	assert(bdIsEqual(w, p));

	/* Try some simple AND and OR ops */
	printf("AND and OR ops:\n");
	/* Set every even bit in u and every odd bit in v */
	bdSetZero(u);
	bdSetZero(v);
	for (i = 0; i < 144 / 2; i++)
	{
		bdSetBit(u, (i * 2), 1);
		bdSetBit(v, (i * 2) + 1, 1);
	}
	pr_msg("u=      ", u);
	pr_msg("v=      ", v);
	bdAndBits(w, u, v);
	pr_msg("u AND v=", w);
	/* expected answer = zero */
	assert(bdIsZero(w));
	bdOrBits(w, u, v);
	pr_msg("u OR v= ", w);
	/* expected answer p = 2^144 - 1 */
	bdSetZero(p);
	bdSetBit(p, 144, 1);
	bdDecrement(p);
	pr_msg("2^144-1=", p);
	assert(bdIsEqual(p, w));

	/* Invert all bits */
	bdSetZero(p);
	bdSetZero(q);
	bdSetShort(p, 0xfffe);
	bdShiftLeft(p, p, 32);
	bdShortAdd(p, p, 0xfffe);
	pr_msg("p=    ", p);
	bdNotBits(q, p);
	pr_msg("NOT p=", q);
	/* check p AND (NOT p) == 0 */
	bdAndBits(w, p, q);
	pr_msg("p AND (NOT p)=", w);
	assert(bdIsZero(w));

	/* Compose a 400-bit string by concatenating random 160-bit sections 
	   and then truncating the result */
	printf("Bit string concatenation and truncation:\n");
	/*
	ALGORITHM: (based on a similar one in RFC 2631/ANSI X.42)
	Set m' = m/160 where / represents integer division with rounding upwards
	Set U = 0
	For i = 0 to m' - 1
		Set R = FUNC_160 and V = FUNC_160 where FUNC_160 generates a unique 160-bit value
		U = U + (R XOR V) * 2^(160 * i)
	Form q from U by computing U mod (2^m) and setting the most
		significant bit (the 2^(m-1) bit) and the least significant bit to 1. 
		In terms of boolean operations, q = U OR 2^(m-1) OR 1. 
		Note that 2^(m-1) < q < 2^m
	*/
	/* In this example, m=400 and m'=3, and we just generate 160-bit random values */
	mc = 3;
	bdSetZero(u);	/* Set U = 0 */
	for (i = 0; i < mc; i++)
	{
		bdRandomBits(r, 160);	/* R = FUNC_160 */
		bdRandomBits(v, 160);	/* V = FUNC_160 */
		bdXorBits(p, r, v);		/* p = (R XOR V) */
		/* w = p * 2^160i, i.e. shift p left by 160i bits */
		bdShiftLeft(w, p, (160 * i));
		bdOrBits(u, u, w);	/* q = q + w */
		printf("w(%d)=", i);
		pr_msg("", w);
	}
	pr_msg("u = w(0)+w(1)+w(2) =\n", u);
	bdModPowerOf2(u, 400);	/* U = U mod (2^m) */
	pr_msg("u = u mod (2^m) = // `ModPowerOf2' \n", u);
	/* q = U OR 2^(m-1) OR 1 */
	bdSetEqual(q, u);		/* q = U */
	bdSetBit(q, 400-1, 1);	/* q = q OR 2^(m-1) */
	bdSetBit(q, 0, 1);		/* q = q OR 1 */
	pr_msg("q = u OR 2^(m-1) OR 1 =\n", q);
	/* Check that 2^(m-1) < q < 2^m */
	printf("BitLength(q)=%d\n", bdBitLength(q));
	bdSetZero(p);
	bdSetBit(p, 400-1, 1);	/* p = 2^(m-1) */
	assert(bdCompare(p, q) < 1);	/* p < q */
	bdSetZero(r);
	bdSetBit(r, 400, 1);	/* r = 2^m */
	assert(bdCompare(q, r) < 1);	/* q < r */
	printf("OK, checked that 2^(m-1) < q < 2^m\n");
	

	/* Create a new set for Modular exponentiation test */
	printf("MODULAR EXPONENTIATION...\n");
	n = bdNew();
	e = bdNew();
	m = bdNew();
	c = bdNew();
	z = bdNew();
	mz = bdNew();
	cz = bdNew();
	t = bdNew();

	/* n is a prime modulus */
	printf("Generating a prime...\n");
	bdGeneratePrime(n, 513, 5, NULL, 0, my_rand);
	pr_msg("prime n=\n", n);
	printf("bdBitLength(n)=%u bdSizeof(n)=%u\n", bdBitLength(n), bdSizeof(n));
	/* Check primality using Fermat test */
	res = fermat_test(n);
	if (res == 0) printf("WARNING: n failed Fermat test.\n");

	/* exponent e is a single random digit */
	bdSetShort(e, bdRandDigit());
	pr_msg("e=", e);

	/* base m is some random number m < n */
	bdSetRandTest(m, bdSizeof(n) - 1);
	pr_msg("random m=\n", m);

	/* Compute c = m^e mod n */
	bdModExp(c, m, e, n);
	pr_msg("c = m^e mod n =\n", c);

	/* Now use a random z and check that 
	   c.z^e == (m.z)^e (mod n) */
	bdSetRandTest(z, bdSizeof(n));
	pr_msg("random z=\n", z);

	/* Compute cz = c.z^e mod n (Note use of temp t) */
	bdModExp(t, z, e, n);
	bdModMult(cz, c, t, n);
	pr_msg("c.z^e mod n=\n", cz);
	/* Compute mz = (m.z)^e mod n */
	bdModMult(t, m, z, n);
	bdModExp(mz, t, e, n);
	pr_msg("(m.z)^e mod n==\n", mz);
	if (!bdIsEqual(mz, cz))
		printf("ERROR: (c.z^e == (m.z)^e (mod n) DOES NOT MATCH m'\n");
	else
		printf("c.z^e == (m.z)^e (mod n) checks OK\n");
	assert(bdIsEqual(mz, cz));


	/* Do some conversions with hex and decimal strings */
	bdSetZero(u);
	bdConvFromHex(u, "DeadBeefCafeBabeBeddedDefacedDeafBeadFacade");
	pr_msg("From Hex: ", u);
	bdConvToHex(u, s, sizeof(s));
	printf("To Hex: %s\n", s);
	bdConvFromDecimal(u, "1234567890123456789012345678901234567890");
	pr_msg("From Decimal: ", u);
	bdConvToDecimal(u, s, sizeof(s));
	printf("To Decimal: %s\n", s);
	cmp = strcmp(s, "1234567890123456789012345678901234567890");
	assert(cmp == 0);

	/* 987654321 x 81 = 80000000001 */
	bdConvFromDecimal(u, "987654321");
	bdShortMult(v, u, 81);
	bdConvToDecimal(v, s, sizeof(s));
	printf("987654321 x 81 = %s\n", s);
	cmp = strcmp(s, "80000000001");
	assert(cmp == 0);

	/* 123456789 x 9 + 10 = 1111111111 */
	bdConvFromDecimal(u, "123456789");
	bdShortMult(v, u, 9);
	bdShortAdd(w, v, 10);
	bdConvToDecimal(w, s, sizeof(s));
	printf("123456789 x 9 + 10 = %s\n", s);
	cmp = strcmp(s, "1111111111");
	assert(cmp == 0);

	/* Convert an empty decimal string to a BIGD */
	bdSetShort(u, 0xfdfdfdfd);
	bdConvFromDecimal(u, "");
	pr_msg("bdConvFromDecimal(u, '')=", u);
	assert(bdIsZero(u));
	/* ditto an empty hex string */
	bdSetShort(u, 0xfdfdfdfd);
	bdConvFromHex(u, "");
	pr_msg("bdConvFromHex(u, '')=", u);
	assert(bdIsZero(u));
	/* Convert zero BIGD value to a decimal string */
	bdSetZero(u);
	bdConvToDecimal(u, s, sizeof(s));
	printf("Decimal zero = '%s'\n", s);
	cmp = strcmp(s, "0");
	assert(cmp == 0);
	/* ditto to a hex string */
	bdConvToHex(u, s, sizeof(s));
	printf("Hex zero = '%s'\n", s);
	cmp = strcmp(s, "0");
	assert(cmp == 0);
	/* Convert a zero BIGD to an array of octets */
	memset(bytes, 0xdf, sizeof(bytes));
	bdSetZero(u);
	nbytes = bdConvToOctets(u, bytes, sizeof(bytes));
	printf("bdConvToOctets returns %d: ", nbytes);
	/* show that all bytes are zero */
	pr_bytesmsg("", bytes, sizeof(bytes));
	assert(0 == bytes[sizeof(bytes)-1] && 0 == bytes[0]);
	/* Convert an zero-length array of octets to a BIGD */
	bdSetShort(u, 0xfdfdfdfd);
	bdConvFromOctets(u, bytes, 0);
	pr_msg("bdConvFromOctets(nbytes=0)=", u);
	assert(bdIsZero(u));

	bdFree(&n);
	/* Try computing some Jacobi and Legendre symbol values */
	a = bdNew();
	n = bdNew();
	bdSetShort(a, 158);
	bdSetShort(n, 235);
	jac = bdJacobi(a, n);
	printf("Jacobi(158, 235)=%d\n", jac);
	assert(-1 == jac);
	bdSetShort(a, 2183);
	bdSetShort(n, 9907);
	jac = bdJacobi(a, n);
	printf("Jacobi(2183, 9907)=%d\n", jac);
	assert(1 == jac);
	bdSetShort(a, 1001);
	jac = bdJacobi(a, n);
	printf("Jacobi(1001, 9907)=%d\n", jac);
	assert(-1 == jac);
	bdShortMult(a, n, 10000);
	jac = bdJacobi(a, n);
	printf("Jacobi(10000 * 9907, 9907)=%d\n", jac);
	assert(0 == jac);


	/* Show the current version number */
	printf("VERSION=%d\n", bdVersion());

	/* Clear up */
	bdFree(&u);
	bdFree(&v);
	bdFree(&w);
	bdFree(&q);
	bdFree(&r);
	bdFree(&p);

	bdFree(&a);
	bdFree(&n);
	bdFree(&e);
	bdFree(&m);
	bdFree(&c);
	bdFree(&z);
	bdFree(&mz);
	bdFree(&cz);
	bdFree(&t);

	printf("OK, successfully completed tests.\n");

	return 0;
}

