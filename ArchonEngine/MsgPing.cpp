//	MsgPing.cpp
//
//	CExarchEngine class
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

DECLARE_CONST_STRING(FIELD_ADDRESS,						"address")
DECLARE_CONST_STRING(FIELD_TIME,						"time")

DECLARE_CONST_STRING(MSG_OK,							"OK")
DECLARE_CONST_STRING(MSG_ARC_PING,						"Arc.ping")
DECLARE_CONST_STRING(MSG_ERROR_TIMEOUT,					"Error.timeout")
DECLARE_CONST_STRING(MSG_ERROR_UNABLE_TO_COMPLY,		"Error.unableToComply")
DECLARE_CONST_STRING(MSG_LOG_DEBUG,						"Log.debug")
DECLARE_CONST_STRING(MSG_LOG_ERROR,						"Log.error")
DECLARE_CONST_STRING(MSG_REPLY_DATA,					"Reply.data");

DECLARE_CONST_STRING(PORT_EXARCH_COMMAND,				"Exarch.command")

static constexpr DWORD MESSAGE_TIMEOUT =				15 * 1000;

class CPingSession : public ISessionHandler
	{
	public:
		CPingSession (const TArray<CString> &Addresses);

		//	ISessionHandler virtuals

		virtual void OnMark (void) override { }
		virtual bool OnProcessMessage (const SArchonMessage &Msg) override;
		virtual bool OnStartSession (const SArchonMessage &Msg, DWORD dwTicket) override;

	private:
		static constexpr DWORD NO_RESULT = 0xffffffff;

		struct SEntry
			{
			DWORDLONG dwStartTick = 0;
			DWORD dwPingTime = NO_RESULT;
			CString sError;
			};

		bool IsComplete (void) const;
		void SendReply (void);

		CCriticalSection m_cs;
		TSortMap<CString, SEntry> m_Result;
	};

void CExarchEngine::MsgPing (const SArchonMessage &Msg, const CHexeSecurityCtx *pSecurityCtx)

//	MsgPing
//
//	Exarch.ping [{machineName}]

	{
	TArray<CString> Addresses;

	CStringView sMachineName = Msg.dPayload.GetElement(0);
	if (sMachineName.IsEmpty())
		{
		for (int i = 0; i < m_MecharcologyDb.GetMachineCount(); i++)
			{
			SMachineDesc MachineDesc;
			if (m_MecharcologyDb.GetMachine(i, &MachineDesc) && !MachineDesc.sName.IsEmpty())
				{
				Addresses.Insert(GetTransporter().GenerateAddress(PORT_EXARCH_COMMAND, NULL_STR, MachineDesc.sName));
				}
			}
		}
	else
		{
		SMachineDesc MachineDesc;
		if (!m_MecharcologyDb.FindMachineByPartialName(sMachineName, &MachineDesc))
			{
			SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unknown machine: %s", sMachineName), Msg);
			return;
			}

		Addresses.Insert(GenerateMachineAddress(MachineDesc.sName, PORT_EXARCH_COMMAND));
		}

	StartSession(Msg, new CPingSession(Addresses));
	}

//	CPingSession ---------------------------------------------------------------

CPingSession::CPingSession (const TArray<CString> &Addresses)

//	CPingSession constructor

	{
	for (int i = 0; i < Addresses.GetCount(); i++)
		m_Result.SetAt(Addresses[i]);
	}

bool CPingSession::IsComplete (void) const

//	IsComplete
//
//	Returns TRUE if we have all results. Callers must lock.

	{
	for (int i = 0; i < m_Result.GetCount(); i++)
		if (m_Result[i].dwPingTime == NO_RESULT)
			return false;

	return true;
	}

bool CPingSession::OnProcessMessage (const SArchonMessage &Msg)

//	OnProcessMessage
//
//	Handle a message.

	{
	if (strEquals(Msg.sMsg, MSG_ERROR_TIMEOUT))
		{
		//	If we get a time-out then we stop waiting and just return a reply.

		SendReply();
		return false;
		}
	else if (IsError(Msg))
		{
		//	LATER: For now we can't tell who returned the error, so we just 
		//	abort the entire operation.

		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, Msg.dPayload.GetElement(0).AsStringView());
		return false;
		}
	else if (strEquals(Msg.sMsg, MSG_REPLY_DATA))
		{
		CSmartLock Lock(m_cs);

		CStringView sAddress = Msg.dPayload.GetElement(0);
		SEntry *pEntry = m_Result.GetAt(sAddress);
		if (pEntry && pEntry->dwPingTime == NO_RESULT)
			{
			pEntry->dwPingTime = (DWORD)::sysGetTicksElapsed(pEntry->dwStartTick);
			}

		//	If we're not complete yet, keep waiting

		if (!IsComplete())
			return true;

		//	Otherwise we're done; send a reply

		SendReply();
		return false;
		}
	else
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, strPattern("Unexpected message: %s", Msg.sMsg));
		return false;
		}
	}

bool CPingSession::OnStartSession (const SArchonMessage &Msg, DWORD dwTicket)

//	OnStartSession
//
//	Start.

	{
	if (m_Result.GetCount() == 0)
		{
		SendMessageReplyError(MSG_ERROR_UNABLE_TO_COMPLY, "Internal Error");
		return false;
		}

	//	Send messages to all addresses.

	CString sReplyAddr = GenerateAddress(PORT_EXARCH_COMMAND);

	for (int i = 0; i < m_Result.GetCount(); i++)
		{
		const CString &sAddress = m_Result.GetKey(i);
		m_Result[i].dwStartTick = ::sysGetTickCount64();

		CDatum dPayload(CDatum::typeArray);
		dPayload.Append(sAddress);

		//	We set a timeout only on the first message because sessions don't
		//	currently allow for multiple timeouts.

		if (!SendMessageCommand(sAddress, MSG_ARC_PING, sReplyAddr, dPayload, (i == 0 ? MESSAGE_TIMEOUT : 0)))
			GetProcessCtx()->Log(MSG_LOG_ERROR, strPattern("Unable to send Arc.ping to %s", sAddress));
		}

	//	Wait for results

	return true;
	}

void CPingSession::SendReply (void)

//	SendReply
//
//	Composes a reply showing ping times for all addresses. Callers must lock.

	{
	CDatum dResult(CDatum::typeArray);

	for (int i = 0; i < m_Result.GetCount(); i++)
		{
		CDatum dEntry(CDatum::typeStruct);

		CString sMachineName;
		GetProcessCtx()->GetTransporter().ParseAddress(m_Result.GetKey(i), NULL, NULL, &sMachineName);
		dEntry.SetElement(FIELD_ADDRESS, sMachineName);

		if (m_Result[i].dwPingTime == NO_RESULT)
			dEntry.SetElement(FIELD_TIME, MSG_ERROR_TIMEOUT);
		else
			dEntry.SetElement(FIELD_TIME, m_Result[i].dwPingTime);

		dResult.Append(dEntry);
		}

	SendMessageReply(MSG_REPLY_DATA, dResult);
	}
