//	GridWhaleNotifications.h
//
//	GridWhale Classes and Functions
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

class CNotificationEntry
	{
	public:

		enum class EStatus
			{
			Unknown,

			New,					//	New notification
			Read,					//	Notification has been read
			};

		enum class EType
			{
			Unknown,

			System,					//	System notification
			Program,				//	Program-generated notification
			User,					//	User-generated notification (e.g., invitation, etc.).
			Message,				//	User direct message notification
			Mention,				//	User mention notification
			Interaction,			//	User interaction notification (e.g., likes, reactions)
			};

		CNotificationEntry () {}
		explicit CNotificationEntry (CDatum dEntry);

		CDatum AsDatum () const;
		const CGridName& GetActor () const { return m_Actor; }
		CStringView GetID () const { return m_sID; }
		CStringView GetTitle () const { return m_sTitle; }
		CStringView GetURL () const { return m_sURL; }
		EType GetType () const { return m_iType; }
		void SetActor (const CGridName& Actor) { m_Actor = Actor; }
		void SetCreatedOn (const CDateTime& CreatedOn) { m_CreatedOn = CreatedOn; }
		void SetID (CStringView sID) { m_sID = sID; }
		void SetProgramID (CStringView sProgramID) { m_sProgramID = sProgramID; }
		void SetStatus (EStatus iStatus) { m_iStatus = iStatus; }
		void SetTitle (CStringView sTitle) { m_sTitle = sTitle; }
		void SetURL (CStringView sURL) { m_sURL = sURL; }
		void SetUsername (const CGridName& Username) { m_Username = Username; }

		static CString AsID (EStatus iStatus);
		static CString AsID (EType iType);
		static EStatus AsStatus (CStringView sValue);
		static EType AsType (CStringView sValue);

	private:

		CString m_sID;				//	ID of the notification, unique globally.
		CGridName m_Username;		//	User associated with the notification.
		EStatus m_iStatus = EStatus::Unknown;
		CString m_sProgramID;		//	Program that generated the notification.
		CString m_sEventID;			//	Event ID (unique within user and program).
		CDateTime m_CreatedOn;		//	Date/time the event happened.
		CDateTime m_ReadOn;			//	Date/time the event was read.
		CString m_sURL;				//	URL associated with the notification.

		EType m_iType = EType::Unknown;
		CString m_sTitle;			//	Title of the notification.
		CGridName m_Actor;			//	Actor that caused the notification.
	};


