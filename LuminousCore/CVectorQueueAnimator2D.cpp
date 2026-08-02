//	CVectorQueueAnimator2D.cpp
//
//	CVectorQueueAnimator2D Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

ObjPropType CVectorQueueAnimator2D::GetPropertyType () const
	{
	return ILuminousObj2D::GetPropertyDesc(GetProperty()).iType;
	}

void CVectorQueueAnimator2D::OnRead (IByteStream& Stream)
	{
	int QueueCount = Stream.ReadInt();
	m_Values.DeleteAll();
	m_Values.InsertEmpty(QueueCount);
	for (int i = 0; i < QueueCount; i++)
		{
		int PointCount = Stream.ReadInt();
		m_Values[i].DeleteAll();
		m_Values[i].InsertEmpty(PointCount);
		for (int j = 0; j < PointCount; j++)
			m_Values[i][j].Read(Stream);
		}
	}

void CVectorQueueAnimator2D::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_Values.GetCount());
	for (int i = 0; i < m_Values.GetCount(); i++)
		{
		Stream.Write(m_Values[i].GetCount());
		for (int j = 0; j < m_Values[i].GetCount(); j++)
			m_Values[i][j].Write(Stream);
		}
	}
