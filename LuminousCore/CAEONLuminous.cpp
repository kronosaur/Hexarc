//	CAEONLuminous.cpp
//
//	CAEONLuminous Class
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

bool CAEONLuminous::m_bInitialized = false;

bool CAEONLuminous::Boot ()

//	Boot
//
//	Boot the AEON factory classes.

	{
	if (!m_bInitialized)
		{
		CAEONLuminousBitmap::RegisterFactory();
		CAEONLuminousCanvas::RegisterFactory();

		//	Done

		m_bInitialized = true;
		}

	return true;
	}
