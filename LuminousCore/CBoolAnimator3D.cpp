//	CBoolAnimator3D.cpp
//
//	CBoolAnimator3D Class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

void CBoolAnimator3D::OnRead (IByteStream& Stream)
	{
	int Count = Stream.ReadInt();
	m_Values.DeleteAll();
	m_Values.InsertEmpty(Count);
	for (int i = 0; i < Count; i++)
		m_Values[i] = Stream.ReadChar() != 0;
	}

void CBoolAnimator3D::OnWrite (IByteStream& Stream) const
	{
	Stream.Write(m_Values.GetCount());
	for (int i = 0; i < m_Values.GetCount(); i++)
		Stream.WriteChar((char)(m_Values[i] ? 1 : 0));
	}
