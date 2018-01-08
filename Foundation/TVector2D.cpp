//	TVector2D.cpp
//
//	TVector2D class
//	Copyright (c) 2012 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

template <class VALUE> VALUE TVector2D<VALUE>::Polar (VALUE *retrRadius) const
	{
	if (retrRadius)
		*retrRadius = Length();

	return mathAngleMod(atan2(m_y, m_x));
	}

//	Specialization: double -----------------------------------------------------

template <> void TVector2D<double>::Randomize (double xMin, double xMax, double yMin, double yMax)
	{
	m_x = mathRandom(xMin, xMax);
	m_y = mathRandom(yMin, yMax);
	}

void DummyDouble (void)
	{
	TVector2D<double> Temp;

	//	Forces linker to create function

	Temp.Polar();
	}
