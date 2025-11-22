//	CScalarAnimator3D.cpp
//
//	CScalarAnimator3D Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

void CScalarAnimator3D::OnRead (IByteStream& Stream)
	{
	int Count = Stream.ReadInt();
	m_Values.DeleteAll();
	m_Values.InsertEmpty(Count);
	for (int i = 0; i < Count; i++)
		m_Values[i] = Stream.ReadDouble();
	}

void CScalarAnimator3D::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_Values.GetCount());
	for (int i = 0; i < m_Values.GetCount(); i++)
		Stream.Write(m_Values[i]);
	}
