//	IMemoryBlock64.cpp
//
//	IMemoryBlock64 class
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

IByteStream64::EBOM IMemoryBlock64::ReadBOM (DWORDLONG dwPos) const

//	ReadBOM
//
//	FF FE		UTF16LE
//	FE FF		UTF16BE

	{
	if (dwPos + 2 > GetLength())
		return IByteStream64::EBOM::None;

	const char* pPos = GetPointer() + dwPos;

	if (pPos[0] == (char)0xFF && pPos[1] == (char)0xFE)
		return IByteStream64::EBOM::UTF16LE;
	else if (pPos[0] == (char)0xFE && pPos[1] == (char)0xFF)
		return IByteStream64::EBOM::UTF16BE;
	else
		return IByteStream64::EBOM::None;
	}

