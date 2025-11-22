//	CScalarAnimator2D.cpp
//
//	CScalarAnimator2D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

void CScalarAnimator2D::OnRead (IByteStream& Stream)
	{
	int Count = Stream.ReadInt();
	m_Values.DeleteAll();
	m_Values.InsertEmpty(Count);
	for (int i = 0; i < Count; i++)
		m_Values[i] = Stream.ReadDouble();
	}

void CScalarAnimator2D::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_Values.GetCount());
	for (int i = 0; i < m_Values.GetCount(); i++)
		Stream.Write(m_Values[i]);
	}
