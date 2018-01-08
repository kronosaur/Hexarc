//	CMsgProcessKeepAlive.cpp
//
//	CMsgProcessKeepAlive class
//	Copyright (c) 2016 by Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_ARC_KEEP_ALIVE,				"Arc.keepAlive")

const DWORDLONG KEEP_ALIVE_THRESHOLD =                  1000 * 10;  //  10 seconds

void CMsgProcessKeepAlive::OnProgress (int iPercent, const CString &sStatus)

//  OnProgress
//
//  Progress has happened

    {
    DWORDLONG dwNow = ::sysGetTickCount64();

    //  If it's been a while since our last update, send a keep-alive message
    //  so that we don't time out.

    if ((dwNow - m_dwStart) > KEEP_ALIVE_THRESHOLD)
        {
        m_dwStart = dwNow;
        m_Ctx.SendMessageReply(MSG_ARC_KEEP_ALIVE, CDatum());
        }
    }

