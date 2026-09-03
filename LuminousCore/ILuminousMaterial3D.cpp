//	ILuminousMaterial3D.cpp
//
//	ILuminousMaterial3D Class
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#include "pch.h"

DECLARE_CONST_STRING(ALPHA_BLEND, "blend");
DECLARE_CONST_STRING(ALPHA_HASH, "hash");
DECLARE_CONST_STRING(ALPHA_MASK, "mask");
DECLARE_CONST_STRING(ALPHA_OPAQUE, "opaque");
DECLARE_CONST_STRING(MAP_ANISOTROPY, "anisotropyMap");
DECLARE_CONST_STRING(MAP_AO, "aoMap");
DECLARE_CONST_STRING(MAP_BUMP, "bumpMap");
DECLARE_CONST_STRING(MAP_CLEARCOAT, "clearcoatMap");
DECLARE_CONST_STRING(MAP_CLEARCOAT_NORMAL, "clearcoatNormalMap");
DECLARE_CONST_STRING(MAP_CLEARCOAT_ROUGHNESS, "clearcoatRoughnessMap");
DECLARE_CONST_STRING(MAP_COLOR, "colorMap");
DECLARE_CONST_STRING(MAP_DISPLACEMENT, "displacementMap");
DECLARE_CONST_STRING(MAP_EMISSIVE, "emissiveMap");
DECLARE_CONST_STRING(MAP_IRIDESCENCE, "iridescenceMap");
DECLARE_CONST_STRING(MAP_IRIDESCENCE_THICKNESS, "iridescenceThicknessMap");
DECLARE_CONST_STRING(MAP_LIGHT, "lightMap");
DECLARE_CONST_STRING(MAP_METALNESS, "metalnessMap");
DECLARE_CONST_STRING(MAP_NORMAL, "normalMap");
DECLARE_CONST_STRING(MAP_OPACITY, "opacityMap");
DECLARE_CONST_STRING(MAP_ROUGHNESS, "roughnessMap");
DECLARE_CONST_STRING(MAP_SHEEN_COLOR, "sheenColorMap");
DECLARE_CONST_STRING(MAP_SHEEN_ROUGHNESS, "sheenRoughnessMap");
DECLARE_CONST_STRING(MAP_SPECULAR_COLOR, "specularColorMap");
DECLARE_CONST_STRING(MAP_SPECULAR_INTENSITY, "specularIntensityMap");
DECLARE_CONST_STRING(MAP_THICKNESS, "thicknessMap");
DECLARE_CONST_STRING(MAP_TRANSMISSION, "transmissionMap");
DECLARE_CONST_STRING(TYPE_PHYSICAL, "physical");

const CString& ILuminousMaterial3D::AsID (Material3DAlphaMode iMode)
	{
	switch (iMode)
		{
		case Material3DAlphaMode::Opaque: return ALPHA_OPAQUE;
		case Material3DAlphaMode::Mask: return ALPHA_MASK;
		case Material3DAlphaMode::Blend: return ALPHA_BLEND;
		case Material3DAlphaMode::Hash: return ALPHA_HASH;
		default: throw CException(errFail);
		}
	}

const CString& ILuminousMaterial3D::AsID (Material3DMap iMap)
	{
	switch (iMap)
		{
		case Material3DMap::Color: return MAP_COLOR;
		case Material3DMap::Opacity: return MAP_OPACITY;
		case Material3DMap::Emissive: return MAP_EMISSIVE;
		case Material3DMap::Normal: return MAP_NORMAL;
		case Material3DMap::Bump: return MAP_BUMP;
		case Material3DMap::Displacement: return MAP_DISPLACEMENT;
		case Material3DMap::Roughness: return MAP_ROUGHNESS;
		case Material3DMap::Metalness: return MAP_METALNESS;
		case Material3DMap::AO: return MAP_AO;
		case Material3DMap::Light: return MAP_LIGHT;
		case Material3DMap::Anisotropy: return MAP_ANISOTROPY;
		case Material3DMap::Clearcoat: return MAP_CLEARCOAT;
		case Material3DMap::ClearcoatNormal: return MAP_CLEARCOAT_NORMAL;
		case Material3DMap::ClearcoatRoughness: return MAP_CLEARCOAT_ROUGHNESS;
		case Material3DMap::Iridescence: return MAP_IRIDESCENCE;
		case Material3DMap::IridescenceThickness: return MAP_IRIDESCENCE_THICKNESS;
		case Material3DMap::SheenColor: return MAP_SHEEN_COLOR;
		case Material3DMap::SheenRoughness: return MAP_SHEEN_ROUGHNESS;
		case Material3DMap::SpecularColor: return MAP_SPECULAR_COLOR;
		case Material3DMap::SpecularIntensity: return MAP_SPECULAR_INTENSITY;
		case Material3DMap::Thickness: return MAP_THICKNESS;
		case Material3DMap::Transmission: return MAP_TRANSMISSION;
		default: throw CException(errFail);
		}
	}

Material3DAlphaMode ILuminousMaterial3D::AsAlphaMode (const CString& sValue)
	{
	if (strEqualsNoCase(sValue, ALPHA_OPAQUE)) return Material3DAlphaMode::Opaque;
	else if (strEqualsNoCase(sValue, ALPHA_MASK)) return Material3DAlphaMode::Mask;
	else if (strEqualsNoCase(sValue, ALPHA_BLEND)) return Material3DAlphaMode::Blend;
	else if (strEqualsNoCase(sValue, ALPHA_HASH)) return Material3DAlphaMode::Hash;
	else return (Material3DAlphaMode)-1;
	}

TUniquePtr<ILuminousMaterial3D> ILuminousMaterial3D::CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream, DWORD dwSceneVersion)
	{
	DWORD dwImpl = Stream.ReadDWORD();
	DWORD dwID = Stream.ReadDWORD();

	TUniquePtr<ILuminousMaterial3D> pMaterial;
	switch (dwImpl)
		{
		case IMPL_PHYSICAL:
			pMaterial.Set(new CPhysicalMaterial3D(Scene, dwID));
			break;

		default:
			throw CException(errFail);
		}

	pMaterial->m_Seq = Stream.ReadDWORDLONG();
	pMaterial->m_Color.Read(Stream);
	pMaterial->m_rOpacity = Stream.ReadDouble();

	if (dwSceneVersion >= 5)
		{
		pMaterial->m_iAlphaMode = (Material3DAlphaMode)Stream.ReadDWORD();
		pMaterial->m_rAlphaCutoff = Stream.ReadDouble();
		pMaterial->m_rMetalness = Stream.ReadDouble();
		pMaterial->m_rRoughness = Stream.ReadDouble();
		pMaterial->m_EmissiveColor.Read(Stream);
		pMaterial->m_rEmissiveIntensity = Stream.ReadDouble();
		pMaterial->m_bDoubleSided = (Stream.ReadDWORD() != 0);
		pMaterial->m_vNormalScale.Read(Stream);
		pMaterial->m_rBumpScale = Stream.ReadDouble();
		pMaterial->m_rDisplacementScale = Stream.ReadDouble();
		pMaterial->m_rDisplacementBias = Stream.ReadDouble();
		pMaterial->m_rAOMapIntensity = Stream.ReadDouble();
		pMaterial->m_rLightMapIntensity = Stream.ReadDouble();
		pMaterial->m_rAnisotropy = Stream.ReadDouble();
		pMaterial->m_rAnisotropyRotation = Stream.ReadDouble();
		pMaterial->m_AttenuationColor.Read(Stream);
		pMaterial->m_rAttenuationDistance = Stream.ReadDouble();
		pMaterial->m_rClearcoat = Stream.ReadDouble();
		pMaterial->m_rClearcoatRoughness = Stream.ReadDouble();
		pMaterial->m_vClearcoatNormalScale.Read(Stream);
		pMaterial->m_rDispersion = Stream.ReadDouble();
		pMaterial->m_rIOR = Stream.ReadDouble();
		pMaterial->m_rIridescence = Stream.ReadDouble();
		pMaterial->m_rIridescenceIOR = Stream.ReadDouble();
		pMaterial->m_vIridescenceThicknessRange.Read(Stream);
		pMaterial->m_rSheen = Stream.ReadDouble();
		pMaterial->m_SheenColor.Read(Stream);
		pMaterial->m_rSheenRoughness = Stream.ReadDouble();
		pMaterial->m_SpecularColor.Read(Stream);
		pMaterial->m_rSpecularIntensity = Stream.ReadDouble();
		pMaterial->m_rThickness = Stream.ReadDouble();
		pMaterial->m_rTransmission = Stream.ReadDouble();

		DWORD dwMapCount = Stream.ReadDWORD();
		if (dwMapCount != (DWORD)Material3DMap::Count)
			throw CException(errFail);
		for (int i = 0; i < (int)Material3DMap::Count; i++)
			pMaterial->m_TextureIDs[i] = Stream.ReadDWORD();
		}

	if (!pMaterial->IsValid())
		throw CException(errFail);

	return pMaterial;
	}

bool ILuminousMaterial3D::IsValid () const
	{
	auto IsFinite = [](double rValue) { return std::isfinite(rValue); };
	auto IsUnit = [=](double rValue) { return IsFinite(rValue) && rValue >= 0.0 && rValue <= 1.0; };
	auto IsNonNegative = [=](double rValue) { return IsFinite(rValue) && rValue >= 0.0; };
	auto IsVectorFinite = [=](const CVector2D& vValue) { return IsFinite(vValue.X()) && IsFinite(vValue.Y()); };

	return ((int)m_iAlphaMode >= (int)Material3DAlphaMode::Opaque
			&& (int)m_iAlphaMode <= (int)Material3DAlphaMode::Hash
			&& IsUnit(m_rAlphaCutoff)
			&& IsUnit(m_rAnisotropy)
			&& IsFinite(m_rAnisotropyRotation)
			&& IsNonNegative(m_rAttenuationDistance)
			&& IsNonNegative(m_rAOMapIntensity)
			&& IsFinite(m_rBumpScale)
			&& IsUnit(m_rClearcoat)
			&& IsVectorFinite(m_vClearcoatNormalScale)
			&& IsUnit(m_rClearcoatRoughness)
			&& IsNonNegative(m_rDispersion)
			&& IsFinite(m_rDisplacementBias)
			&& IsFinite(m_rDisplacementScale)
			&& IsNonNegative(m_rEmissiveIntensity)
			&& IsFinite(m_rIOR) && m_rIOR >= 1.0 && m_rIOR <= 2.333
			&& IsUnit(m_rIridescence)
			&& IsFinite(m_rIridescenceIOR) && m_rIridescenceIOR >= 1.0 && m_rIridescenceIOR <= 2.333
			&& IsVectorFinite(m_vIridescenceThicknessRange)
			&& m_vIridescenceThicknessRange.X() >= 0.0
			&& m_vIridescenceThicknessRange.X() <= m_vIridescenceThicknessRange.Y()
			&& IsNonNegative(m_rLightMapIntensity)
			&& IsUnit(m_rMetalness)
			&& IsVectorFinite(m_vNormalScale)
			&& IsUnit(m_rOpacity)
			&& IsUnit(m_rRoughness)
			&& IsUnit(m_rSheen)
			&& IsUnit(m_rSheenRoughness)
			&& IsUnit(m_rSpecularIntensity)
			&& IsNonNegative(m_rThickness)
			&& IsUnit(m_rTransmission));
	}

void ILuminousMaterial3D::Modified ()
	{
	m_pScene->OnMaterialModified(*this);
	}

#define SET_MATERIAL_UNIT(method, member) \
	bool ILuminousMaterial3D::method (double rValue) \
		{ \
		if (!std::isfinite(rValue) || rValue < 0.0 || rValue > 1.0) return false; \
		if (member == rValue) return true; \
		member = rValue; Modified(); return true; \
		}

#define SET_MATERIAL_NONNEGATIVE(method, member) \
	bool ILuminousMaterial3D::method (double rValue) \
		{ \
		if (!std::isfinite(rValue) || rValue < 0.0) return false; \
		if (member == rValue) return true; \
		member = rValue; Modified(); return true; \
		}

#define SET_MATERIAL_FINITE(method, member) \
	bool ILuminousMaterial3D::method (double rValue) \
		{ \
		if (!std::isfinite(rValue)) return false; \
		if (member == rValue) return true; \
		member = rValue; Modified(); return true; \
		}

SET_MATERIAL_UNIT(SetAlphaCutoff, m_rAlphaCutoff)
SET_MATERIAL_UNIT(SetAnisotropy, m_rAnisotropy)
SET_MATERIAL_FINITE(SetAnisotropyRotation, m_rAnisotropyRotation)
SET_MATERIAL_NONNEGATIVE(SetAttenuationDistance, m_rAttenuationDistance)
SET_MATERIAL_NONNEGATIVE(SetAOMapIntensity, m_rAOMapIntensity)
SET_MATERIAL_FINITE(SetBumpScale, m_rBumpScale)
SET_MATERIAL_UNIT(SetClearcoat, m_rClearcoat)
SET_MATERIAL_UNIT(SetClearcoatRoughness, m_rClearcoatRoughness)
SET_MATERIAL_NONNEGATIVE(SetDispersion, m_rDispersion)
SET_MATERIAL_FINITE(SetDisplacementBias, m_rDisplacementBias)
SET_MATERIAL_FINITE(SetDisplacementScale, m_rDisplacementScale)
SET_MATERIAL_NONNEGATIVE(SetEmissiveIntensity, m_rEmissiveIntensity)
SET_MATERIAL_UNIT(SetIridescence, m_rIridescence)
SET_MATERIAL_NONNEGATIVE(SetLightMapIntensity, m_rLightMapIntensity)
SET_MATERIAL_UNIT(SetMetalness, m_rMetalness)
SET_MATERIAL_UNIT(SetOpacity, m_rOpacity)
SET_MATERIAL_UNIT(SetRoughness, m_rRoughness)
SET_MATERIAL_UNIT(SetSheen, m_rSheen)
SET_MATERIAL_UNIT(SetSheenRoughness, m_rSheenRoughness)
SET_MATERIAL_UNIT(SetSpecularIntensity, m_rSpecularIntensity)
SET_MATERIAL_NONNEGATIVE(SetThickness, m_rThickness)
SET_MATERIAL_UNIT(SetTransmission, m_rTransmission)

bool ILuminousMaterial3D::SetAlphaMode (Material3DAlphaMode iMode)
	{
	if ((int)iMode < (int)Material3DAlphaMode::Opaque || (int)iMode > (int)Material3DAlphaMode::Hash)
		return false;
	if (m_iAlphaMode == iMode) return true;
	m_iAlphaMode = iMode;
	Modified();
	return true;
	}

#define SET_MATERIAL_COLOR(method, member) \
	void ILuminousMaterial3D::method (const CLuminousColor& Color) \
		{ member = Color; Modified(); }

SET_MATERIAL_COLOR(SetAttenuationColor, m_AttenuationColor)
SET_MATERIAL_COLOR(SetColor, m_Color)
SET_MATERIAL_COLOR(SetEmissiveColor, m_EmissiveColor)
SET_MATERIAL_COLOR(SetSheenColor, m_SheenColor)
SET_MATERIAL_COLOR(SetSpecularColor, m_SpecularColor)

void ILuminousMaterial3D::SetDoubleSided (bool bValue)
	{
	if (m_bDoubleSided == bValue) return;
	m_bDoubleSided = bValue;
	Modified();
	}

bool ILuminousMaterial3D::SetIOR (double rValue)
	{
	if (!std::isfinite(rValue) || rValue < 1.0 || rValue > 2.333)
		return false;
	if (m_rIOR == rValue) return true;
	m_rIOR = rValue;
	Modified();
	return true;
	}

bool ILuminousMaterial3D::SetIridescenceIOR (double rValue)
	{
	if (!std::isfinite(rValue) || rValue < 1.0 || rValue > 2.333)
		return false;
	if (m_rIridescenceIOR == rValue) return true;
	m_rIridescenceIOR = rValue;
	Modified();
	return true;
	}

bool ILuminousMaterial3D::SetIridescenceThicknessRange (const CVector2D& vValue)
	{
	if (!std::isfinite(vValue.X()) || !std::isfinite(vValue.Y()) || vValue.X() < 0.0 || vValue.X() > vValue.Y())
		return false;
	if (m_vIridescenceThicknessRange == vValue) return true;
	m_vIridescenceThicknessRange = vValue;
	Modified();
	return true;
	}

#define SET_MATERIAL_VECTOR(method, member) \
	bool ILuminousMaterial3D::method (const CVector2D& vValue) \
		{ \
		if (!std::isfinite(vValue.X()) || !std::isfinite(vValue.Y())) return false; \
		if (member == vValue) return true; \
		member = vValue; Modified(); return true; \
		}

SET_MATERIAL_VECTOR(SetClearcoatNormalScale, m_vClearcoatNormalScale)
SET_MATERIAL_VECTOR(SetNormalScale, m_vNormalScale)

bool ILuminousMaterial3D::SetTextureID (Material3DMap iMap, DWORD dwTextureID)
	{
	if ((int)iMap < 0 || (int)iMap >= (int)Material3DMap::Count)
		return false;
	if (dwTextureID && !m_pScene->FindTexture(dwTextureID))
		return false;
	if (m_TextureIDs[(int)iMap] == dwTextureID)
		return true;

	m_TextureIDs[(int)iMap] = dwTextureID;
	Modified();
	return true;
	}

void ILuminousMaterial3D::Write (IByteStream& Stream) const
	{
	Stream.Write(GetImpl());
	Stream.Write(GetID());
	Stream.Write(m_Seq);
	m_Color.Write(Stream);
	Stream.Write(m_rOpacity);
	Stream.Write((DWORD)m_iAlphaMode);
	Stream.Write(m_rAlphaCutoff);
	Stream.Write(m_rMetalness);
	Stream.Write(m_rRoughness);
	m_EmissiveColor.Write(Stream);
	Stream.Write(m_rEmissiveIntensity);
	Stream.Write((DWORD)(m_bDoubleSided ? 1 : 0));
	m_vNormalScale.Write(Stream);
	Stream.Write(m_rBumpScale);
	Stream.Write(m_rDisplacementScale);
	Stream.Write(m_rDisplacementBias);
	Stream.Write(m_rAOMapIntensity);
	Stream.Write(m_rLightMapIntensity);
	Stream.Write(m_rAnisotropy);
	Stream.Write(m_rAnisotropyRotation);
	m_AttenuationColor.Write(Stream);
	Stream.Write(m_rAttenuationDistance);
	Stream.Write(m_rClearcoat);
	Stream.Write(m_rClearcoatRoughness);
	m_vClearcoatNormalScale.Write(Stream);
	Stream.Write(m_rDispersion);
	Stream.Write(m_rIOR);
	Stream.Write(m_rIridescence);
	Stream.Write(m_rIridescenceIOR);
	m_vIridescenceThicknessRange.Write(Stream);
	Stream.Write(m_rSheen);
	m_SheenColor.Write(Stream);
	Stream.Write(m_rSheenRoughness);
	m_SpecularColor.Write(Stream);
	Stream.Write(m_rSpecularIntensity);
	Stream.Write(m_rThickness);
	Stream.Write(m_rTransmission);
	Stream.Write((DWORD)Material3DMap::Count);
	for (int i = 0; i < (int)Material3DMap::Count; i++)
		Stream.Write(m_TextureIDs[i]);
	}

const CString& CPhysicalMaterial3D::OnGetMaterialType () const
	{
	return TYPE_PHYSICAL;
	}
