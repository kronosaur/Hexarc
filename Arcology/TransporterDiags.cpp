//	TransporterDiags.cpp
//
//	Transporter Diagnostics
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data")

void CDrHouseEngine::MsgPortCacheDump (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgPortCacheDump
//
//	Diagnostics.portCacheDump

	{
	int i;

	CMessageTransporter &Transporter = GetProcessCtx()->GetTransporter();
	TArray<CMessagePort *> Ports = Transporter.GetPortCacheList();

	CDatum dResult(CDatum::typeArray);
	dResult.GrowToFit(Ports.GetCount());
	for (i = 0; i < Ports.GetCount(); i++)
		dResult.Append(Ports[i]->GetStatus());

	SendMessageReply(MSG_REPLY_DATA, dResult, Msg);
	}
