//	GLDefaultEvn.h
//
//	CGridLangDefaultEnvironment class
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CGLDefaultEnvironment : public IGridLangEnvironment
	{
	public:
		virtual void Output (const CString &sPort, CDatum dValue) override;
	};
