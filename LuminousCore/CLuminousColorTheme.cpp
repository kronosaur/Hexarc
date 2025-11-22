//	CLuminousColorTheme.cpp
//
//	CLuminousColorTheme Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

CLuminousColorTheme::CLuminousColorTheme (const std::initializer_list<SColorEntry>& Shades, const std::initializer_list<SRoleEntry>& Roles)

//	CLuminousColorTheme constructor

	{
	for (auto &Entry : Shades)
		{
		if ((int)Entry.iShade < (int)CLuminousColor::EShade::MAX && (int)Entry.iShade >= 0)
			m_Colors[(int)Entry.iShade] = Entry;
		}

	for (auto& Entry : Roles)
		{
		if ((int)Entry.iRole < (int)CLuminousColor::ERole::MAX && (int)Entry.iRole >= 0)
			m_RoleColors[(int)Entry.iRole] = CLuminousColor(Entry.iShade, Entry.iTone).GetSolidColor();
		}
	}

CRGBA32 CLuminousColorTheme::GetColor (CLuminousColor::EShade iColor, int iToneIndex) const

//	GetColor
//
//	Returns the RGB color for the given shade.

	{
	if ((int)iColor < 0 || (int)iColor >= (int)CLuminousColor::EShade::MAX || iToneIndex < 0 || iToneIndex > MAX_COLORS)
		return CRGBA32(0, 0, 0);

	return m_Colors[(int)iColor].Colors[iToneIndex];
	}

CRGBA32 CLuminousColorTheme::GetColor (CLuminousColor::ERole iRole) const

//	GetColor
//
//	Returns the RGB color for the given role.

	{
	if ((int)iRole < 0 || (int)iRole >= (int)CLuminousColor::ERole::MAX)
		return CRGBA32(0, 0, 0);

	return m_RoleColors[(int)iRole];
	}

