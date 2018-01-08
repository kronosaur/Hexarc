//	Wrappers.cpp
//
//	Wrapper functions for Big Integer Library
//	Copyright (c) 2011 by George Moromisato. All Rights Reserved.

#include "stdafx.h"

#ifdef USE_BIG_INT

CIPInteger::operator CString (void) const
	{
	CStringBuffer Output;

	if (m_Value.getSign() == BigInteger::negative)
		Output.Write("-", 1);

	BigUnsignedInABase Int(m_Value.getMagnitude(), 10);
	Int.WriteToStream(Output);

	//	Done

	CString sOutput;
	sOutput.TakeHandoff(Output);
	return sOutput;
	}

#endif