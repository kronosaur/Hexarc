/* spExtras.h */

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

/* Interface to extra BigDigits "sp" functions that operate with single digits */

#ifndef SPEXTRAS_H_
#define SPEXTRAS_H_ 1

#include "bigdigits.h"

int spModMult(DIGIT_T *a, DIGIT_T x, DIGIT_T y, DIGIT_T m);
	/* Computes a = (x * y) mod m */

int spModInv(DIGIT_T *inv, DIGIT_T u, DIGIT_T v);
	/* Computes inv = u^-1 mod v */

DIGIT_T spGcd(DIGIT_T x, DIGIT_T y);
	/* Returns gcd(x, y) */

int spIsPrime(DIGIT_T w, size_t t);
	/* Returns true if w is a probable prime, else false; t tests */

/* int spModExp(DIGIT_T *exp, DIGIT_T x, DIGIT_T n, DIGIT_T d); */
	/* Computes exp = x^n mod d */

/* Two alternatives for spModExp:
   define USE_KNUTH_MODEXP before <spExtras.h> to use Knuth method, 
   otherwise defaults to use Binary left-to-right method */
#ifdef USE_KNUTH_MODEXP
#define spModExp spModExpK
#else
#define spModExp spModExpB
#endif	/* USE_KNUTH_MODEXP */

int spModExpK(DIGIT_T *exp, DIGIT_T x, DIGIT_T n, DIGIT_T d);
int spModExpB(DIGIT_T *exp, DIGIT_T x, DIGIT_T n, DIGIT_T d);

#endif /* SPEXTRAS_H_ */

