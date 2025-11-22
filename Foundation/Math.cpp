//	Math.cpp
//
//	Math functions and classes
//	Copyright (c) 2012 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"
#include <Math.h>

double mathRound (double rValue, double rMultiple)

//	mathRound
//
//	Rounds number

	{
	if (rMultiple == 0.0)
		{
		if (rValue > 0.0)
			return floor(rValue + 0.5);
		else
			return ceil(rValue - 0.5);
		}
	else
		{
		if (rValue > 0.0)
			return floor(rValue * rMultiple + 0.5) / rMultiple;
		else
			return ceil(rValue * rMultiple - 0.5) / rMultiple;
		}
	}
