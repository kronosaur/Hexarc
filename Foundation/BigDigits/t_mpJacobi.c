/* $Id: t_mpJacobi.c $ */

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

#include <stdio.h>
#include <assert.h>
#include "bigdigits.h"

#define TEST_LEN (1024/8)

void mpPrintMsg(const char *msg, const DIGIT_T p[], size_t ndigits)
{
	printf("%s", msg);
	mpPrintTrimNL(p, ndigits);
}

char *strA = 
	"6BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
	"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
	"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
	"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
	"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
	"BBBBBBBBBBBAA9993E364706816ABA3E25717850"
	"C26C9CD0D89D33CC";

const char *strN = 
	"CCD34C2F4D95FFAD1420E666C07E39D1450A1330"
	"4C3F5891EDE57595C772A3691AB51D2BECE1476B"
	"8F22AE223365F183BC3EE2D4CACDBA3AD0C4D478"
	"1C523A10EFE6203D6F3BC226BF9A459727B8F122"
	"C482D8C86019F9A869329187096430A6C67CB103"
	"742BCBC66906AD23836EBABB511D5D80AB8CB599"
	"74E9AAC62D785C45";

int main(void)
{
	size_t ndigits = TEST_LEN;
	DIGIT_T a[TEST_LEN];
	DIGIT_T n[TEST_LEN];
	int j;

	/* Use example from X9.31 Appendix D.5.2 */
	mpConvFromHex(a, ndigits, strA);
	mpConvFromHex(n, ndigits, strN);
	mpPrintMsg("n=\n", n, ndigits);
	mpPrintMsg("a=\n", a, ndigits);
	j= mpJacobi(a, n, ndigits);
	printf("Jacobi(a/n)=%d\n", j);
	assert(j == -1);

	/* Divide a by 2 */
	mpShiftRight(a, a, 1, ndigits);
	mpPrintMsg("a=a/2=\n", a, ndigits);
	j= mpJacobi(a, n, ndigits);
	printf("Jacobi(a/n)=%d\n", j);
	assert(j == +1);

	/* Make n|a */
	mpShortMult(a, n, 7, ndigits);
	mpPrintMsg("a=n*7=\n", a, ndigits);
	j= mpJacobi(a, n, ndigits);
	printf("Jacobi(a/n)=%d\n", j);
	assert(j == 0);

	printf("Jacobi tests completed OK\n");

	return 0;
}
