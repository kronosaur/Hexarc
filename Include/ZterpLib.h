//	ZterpLib.h
//
//	ZterpLib Classes
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"
#include "GridLang.h"

class CGLZterpLibrary
	{
	public:
		static bool Boot ();
		static CDatum CreateZterpStory (CStringView sSessionID);
		static TSharedPtr<IASTNode> GetDefinitions ();
		static void Register ();

		static DWORD ZTERP_STORY_TYPE;

	private:
		static bool m_bBooted;
		static bool m_bRegistered;
	};

