/* $Id: t_bdCPP.c $ */

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

#include "bigd.h"
#include "bigdRand.h"

#include <iostream>
using namespace std;
int main()
{
	int ver;
	ver = bdVersion();
	cout << "bdVersion = " << ver << "\n";

	BIGD u, v, w;

	/* Create new BIGD objects */
	u = bdNew();
	v = bdNew();
	w = bdNew();

	/* Compute 2 * 0xdeadbeefface */
	bdSetShort(u, 2);
	bdConvFromHex(v, "deadbeefface");
	bdMultiply(w, u, v);

	/* Display the result */
	bdPrint(u, BD_PRINT_TRIM);
	printf("* ");
	bdPrint(v, BD_PRINT_TRIM);
	printf("= ");
	bdPrint(w, BD_PRINT_NL);

	/* Add with digit overflow */
	/* ffffffff+ffffffff=00000001 fffffffe */
	bdSetShort(u, 0xffffffff);
	bdSetShort(v, 0xffffffff);
	bdAdd(w, u, v);
	printf("ffffffff+ffffffff=");
	bdPrint(w, BD_PRINT_NL);

	/* Generate some random digits using internal RNG */
	bdSetShort(u, bdRandDigit());
	printf("Random=");
	bdPrint(u, BD_PRINT_NL);
	bdRandomBits(u, 512);
	bdPrint(u, BD_PRINT_NL);

	/* Free all objects we made */
	bdFree(&u);
	bdFree(&v);
	bdFree(&w);

	return 0;
}
