//	CAMP1Queue.cpp
//
//	CAMP1Queue class
//	Copyright (c) 2025 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

void CAMP1Queue::ReplayEvents (IAMP1CommunicatorEvents& Events)

//	ReplayEvents
//
// 	Replays all queued events to the given event handler.

	{
	for (int i = 0; i < m_Queue.GetCount(); i++)
		{
		SEntry& Entry = m_Queue[i];
		switch (Entry.iType)
			{
			case EEntryType::ClientConnected:
				Events.OnAMP1ClientConnected(Entry.sNodeID);
				break;

			case EEntryType::ClientDisconnected:
				Events.OnAMP1ClientDisconnected(Entry.sNodeID);
				break;

			case EEntryType::ConnectedToServer:
				Events.OnAMP1ConnectedToServer();
				break;

			case EEntryType::FatalError:
				Events.OnAMP1FatalError(Entry.sCommand);
				break;

			case EEntryType::Message:
				Events.OnAMP1Message(Entry.sNodeID, Entry.sCommand, std::move(Entry.Data));
				break;

			default:
				ASSERT(false);
			}
		}

	m_Queue.DeleteAll();
	}

