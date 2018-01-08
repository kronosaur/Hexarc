/* $Id: t_bdSimple.c $ */

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

/* A very simple test of some "bd" functions */

#include <stdio.h>
#include "bigd.h"

int main(void)
{
	BIGD u, v, w;

	/* Display the BigDigits version number */
	printf("Version=%d\n", bdVersion());

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

	/* Free all objects we made */
	bdFree(&u);
	bdFree(&v);
	bdFree(&w);

	return 0;
}
