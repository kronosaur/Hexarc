//	CAEONReanimator3D.cpp
//
//	CAEONReanimator3D Class
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#include "pch.h"
#include "LuminousAEON.h"

DECLARE_CONST_STRING(FIELD_ACTIVE_CAMERA_ID, "activeCameraID");
DECLARE_CONST_STRING(FIELD_ANIM, "anim");
DECLARE_CONST_STRING(FIELD_BACKGROUND, "background");
DECLARE_CONST_STRING(FIELD_COLOR, "color");
DECLARE_CONST_STRING(FIELD_COLOR_TEXTURE_ID, "colorTextureID");
DECLARE_CONST_STRING(FIELD_ENVIRONMENT, "environment");
DECLARE_CONST_STRING(FIELD_FLIP_Y, "flipY");
DECLARE_CONST_STRING(FIELD_FORMAT, "format");
DECLARE_CONST_STRING(FIELD_FPS, "fps");
DECLARE_CONST_STRING(FIELD_FRAME, "frame");
DECLARE_CONST_STRING(FIELD_FRAME_COUNT, "frameCount");
DECLARE_CONST_STRING(FIELD_FRAMES, "frames");
DECLARE_CONST_STRING(FIELD_GRID_ID, "gridID");
DECLARE_CONST_STRING(FIELD_ID, "id");
DECLARE_CONST_STRING(FIELD_MATERIAL, "material");
DECLARE_CONST_STRING(FIELD_MATERIAL_ID, "materialID");
DECLARE_CONST_STRING(FIELD_MATERIALS, "materials");
DECLARE_CONST_STRING(FIELD_MODE, "mode");
DECLARE_CONST_STRING(FIELD_OBJECTS, "objects");
DECLARE_CONST_STRING(FIELD_OPACITY, "opacity");
DECLARE_CONST_STRING(FIELD_PARENT_ID, "parentID");
DECLARE_CONST_STRING(FIELD_SEQ, "seq");
DECLARE_CONST_STRING(FIELD_START_FRAME, "startFrame");
DECLARE_CONST_STRING(FIELD_TEXTURES, "textures");
DECLARE_CONST_STRING(FIELD_TYPE, "type");
DECLARE_CONST_STRING(FIELD_TYPES, "types");
DECLARE_CONST_STRING(FIELD_VALUE, "value");
DECLARE_CONST_STRING(FIELD_VALUES, "values");

DECLARE_CONST_STRING(ANIM_DENSE, "dense");
DECLARE_CONST_STRING(ANIM_SPARSE, "sparse");
DECLARE_CONST_STRING(TYPENAME_TEXTURE3D, "texture3D");
DECLARE_CONST_STRING(TYPENAME_MATERIAL3D, "material3D");
DECLARE_CONST_STRING(TYPENAME_OBJ3D, "obj3D");
DECLARE_CONST_STRING(TYPENAME_REANIMATOR3D, "reanimator3D");
DECLARE_CONST_STRING(ERR_AMBIGUOUS_CUBE_MATERIAL, "A cube descriptor cannot specify both color and material.");
DECLARE_CONST_STRING(ERR_INVALID_MATERIAL, "Invalid or cross-scene Material3D reference.");
DECLARE_CONST_STRING(ERR_INVALID_TEXTURE, "Invalid, cross-scene, or unsupported Texture3D reference.");
DECLARE_CONST_STRING(ERR_UNKNOWN_PROPERTY, "Unknown property: %s.");
DECLARE_CONST_STRING(ERR_INVALID_SCENE, "Invalid Scene3D reference.");

//	Texture --------------------------------------------------------------------

static const ILuminousTexture3D* GetWrapperTexture (const CAEONTexture3D& Wrapper)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	return (pScene ? pScene->FindTexture(Wrapper.GetID()) : NULL);
	}

TDatumPropertyHandler<CAEONTexture3D> CAEONTexture3D::m_Properties = {
	{
		"id", "?", "Returns the texture ID.",
		[](const CAEONTexture3D& Obj, const CString&) { return CDatum((int)Obj.m_dwID); },
		NULL,
	},
	{
		"type", "?", "Returns the native texture type.",
		[](const CAEONTexture3D& Obj, const CString&)
			{
			const ILuminousTexture3D* pTexture = GetWrapperTexture(Obj);
			return (pTexture ? CDatum(pTexture->GetTextureType()) : CDatum());
			},
		NULL,
	},
	{
		"gridID", "?", "Returns the source image GridID.",
		[](const CAEONTexture3D& Obj, const CString&)
			{
			const ILuminousTexture3D* pTexture = GetWrapperTexture(Obj);
			return (pTexture ? CDatum(pTexture->GetGridID()) : CDatum());
			},
		NULL,
	},
	{
		"format", "?", "Returns the requested image export format.",
		[](const CAEONTexture3D& Obj, const CString&)
			{
			const ILuminousTexture3D* pTexture = GetWrapperTexture(Obj);
			return (pTexture ? CDatum(pTexture->GetFormat()) : CDatum());
			},
		NULL,
	},
	{
		"flipY", "?", "Returns whether the decoded image is flipped vertically.",
		[](const CAEONTexture3D& Obj, const CString&)
			{
			const ILuminousTexture3D* pTexture = GetWrapperTexture(Obj);
			return (pTexture ? CDatum(pTexture->GetFlipY()) : CDatum());
			},
		NULL,
	},
};

TDatumMethodHandler<CAEONTexture3D> CAEONTexture3D::m_Methods = { };

const CString& CAEONTexture3D::StaticGetTypename () { return TYPENAME_TEXTURE3D; }

CDatum CAEONTexture3D::Create (CDatum dScene, DWORD dwID)
	{
	if (!CAEONReanimator3D::Upconvert(dScene))
		throw CException(errFail);
	return CDatum(new CAEONTexture3D(dScene, dwID));
	}

CDatum CAEONTexture3D::GetDatatype () const { return CAEONTypes::Get(CAEONLuminous::TEXTURE3D_TYPE); }

TArray<IDatatype::SMemberDesc> CAEONTexture3D::GetMembers ()
	{
	TArray<IDatatype::SMemberDesc> Members;
	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);
	return Members;
	}

void CAEONTexture3D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONTexture3D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}

//	Material -------------------------------------------------------------------

static const ILuminousMaterial3D* GetWrapperMaterial (const CAEONMaterial3D& Wrapper)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	return (pScene ? pScene->FindMaterial(Wrapper.GetID()) : NULL);
	}

static ILuminousMaterial3D* GetMutableWrapperMaterial (CAEONMaterial3D& Wrapper)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	return (pScene ? pScene->FindMaterial(Wrapper.GetID()) : NULL);
	}

static bool InvalidMaterialProperty (const CString& sProperty, CString* retsError)
	{
	if (retsError) *retsError = strPattern("Invalid value for PhysicalMaterial3D.%s.", sProperty);
	return false;
	}

static CDatum GetWrapperMaterialTexture (const CAEONMaterial3D& Wrapper, Material3DMap iMap)
	{
	const ILuminousMaterial3D* pMaterial = GetWrapperMaterial(Wrapper);
	DWORD dwTextureID = (pMaterial ? pMaterial->GetTextureID(iMap) : 0);
	return (dwTextureID ? CAEONTexture3D::Create(Wrapper.GetScene(), dwTextureID) : CDatum());
	}

static bool SetWrapperMaterialTexture (CAEONMaterial3D& Wrapper, Material3DMap iMap, CDatum dValue, CString* retsError)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	ILuminousMaterial3D* pMaterial = (pScene ? pScene->FindMaterial(Wrapper.GetID()) : NULL);
	if (!pMaterial)
		return InvalidMaterialProperty(ILuminousMaterial3D::AsID(iMap), retsError);

	if (dValue.IsNil())
		return pMaterial->SetTextureID(iMap, 0);

	auto* pTexture = CAEONTexture3D::Upconvert(dValue);
	auto* pTextureScene = (pTexture ? CAEONReanimator3D::Upconvert(pTexture->GetScene()) : NULL);
	if (!pTexture || pTextureScene != pScene || !pScene->FindTexture(pTexture->GetID()) || !pMaterial->SetTextureID(iMap, pTexture->GetID()))
		{
		if (retsError) *retsError = ERR_INVALID_TEXTURE;
		return false;
		}

	return true;
	}

#define MATERIAL_SCALAR_PROPERTY(name, getter, setter, help) \
	{ name, "?", help, \
		[](const CAEONMaterial3D& Obj, const CString&) { const auto* pMaterial = GetWrapperMaterial(Obj); return (pMaterial ? CDatum(pMaterial->getter()) : CDatum()); }, \
		[](CAEONMaterial3D& Obj, const CString& sProperty, CDatum dValue, CString* retsError) { auto* pMaterial = GetMutableWrapperMaterial(Obj); return (pMaterial && dValue.IsNumber() && pMaterial->setter(dValue) ? true : InvalidMaterialProperty(sProperty, retsError)); } }

#define MATERIAL_COLOR_PROPERTY(name, getter, setter, help) \
	{ name, "?", help, \
		[](const CAEONMaterial3D& Obj, const CString&) { const auto* pMaterial = GetWrapperMaterial(Obj); return (pMaterial ? CAEONLuminous::AsDatum(pMaterial->getter()) : CDatum()); }, \
		[](CAEONMaterial3D& Obj, const CString& sProperty, CDatum dValue, CString* retsError) { auto* pMaterial = GetMutableWrapperMaterial(Obj); if (!pMaterial || CAEONTexture3D::Upconvert(dValue)) return InvalidMaterialProperty(sProperty, retsError); pMaterial->setter(CAEONLuminous::AsColor(dValue)); return true; } }

#define MATERIAL_VECTOR_PROPERTY(name, getter, setter, help) \
	{ name, "?", help, \
		[](const CAEONMaterial3D& Obj, const CString&) { const auto* pMaterial = GetWrapperMaterial(Obj); return (pMaterial ? CDatum(pMaterial->getter()) : CDatum()); }, \
		[](CAEONMaterial3D& Obj, const CString& sProperty, CDatum dValue, CString* retsError) { auto* pMaterial = GetMutableWrapperMaterial(Obj); if (!pMaterial || dValue.GetCount() != 2 || !dValue.GetElement(0).IsNumber() || !dValue.GetElement(1).IsNumber()) return InvalidMaterialProperty(sProperty, retsError); CVector2D vValue(dValue.GetElement(0), dValue.GetElement(1)); return (pMaterial->setter(vValue) ? true : InvalidMaterialProperty(sProperty, retsError)); } }

#define MATERIAL_MAP_PROPERTY(name, map, help) \
	{ name, "?", help, \
		[](const CAEONMaterial3D& Obj, const CString&) { return GetWrapperMaterialTexture(Obj, map); }, \
		[](CAEONMaterial3D& Obj, const CString&, CDatum dValue, CString* retsError) { return SetWrapperMaterialTexture(Obj, map, dValue, retsError); } }

TDatumPropertyHandler<CAEONMaterial3D> CAEONMaterial3D::m_Properties = {
	{
		"id", "?", "Returns the material ID.",
		[](const CAEONMaterial3D& Obj, const CString&) { return CDatum((int)Obj.m_dwID); },
		NULL,
	},
	{
		"type", "?", "Returns the native material type.",
		[](const CAEONMaterial3D& Obj, const CString&)
			{
			const ILuminousMaterial3D* pMaterial = GetWrapperMaterial(Obj);
			return (pMaterial ? CDatum(pMaterial->GetMaterialType()) : CDatum());
			},
		NULL,
	},
	MATERIAL_COLOR_PROPERTY("color", GetColor, SetColor, "Gets/sets the base-color multiplier."),
	MATERIAL_SCALAR_PROPERTY("opacity", GetOpacity, SetOpacity, "Gets/sets opacity from 0 to 1."),
	MATERIAL_SCALAR_PROPERTY("metalness", GetMetalness, SetMetalness, "Gets/sets metallic response from 0 to 1."),
	MATERIAL_SCALAR_PROPERTY("roughness", GetRoughness, SetRoughness, "Gets/sets microsurface roughness from 0 to 1."),
	MATERIAL_COLOR_PROPERTY("emissiveColor", GetEmissiveColor, SetEmissiveColor, "Gets/sets the emissive-color multiplier."),
	MATERIAL_SCALAR_PROPERTY("emissiveIntensity", GetEmissiveIntensity, SetEmissiveIntensity, "Gets/sets nonnegative emissive intensity."),
	{
		"alphaMode", "?", "Gets/sets opaque, mask, blend, or hash alpha handling.",
		[](const CAEONMaterial3D& Obj, const CString&) { const auto* pMaterial = GetWrapperMaterial(Obj); return (pMaterial ? CDatum(ILuminousMaterial3D::AsID(pMaterial->GetAlphaMode())) : CDatum()); },
		[](CAEONMaterial3D& Obj, const CString& sProperty, CDatum dValue, CString* retsError) { auto* pMaterial = GetMutableWrapperMaterial(Obj); auto iMode = ILuminousMaterial3D::AsAlphaMode(dValue.AsString()); return (pMaterial && pMaterial->SetAlphaMode(iMode) ? true : InvalidMaterialProperty(sProperty, retsError)); },
	},
	MATERIAL_SCALAR_PROPERTY("alphaCutoff", GetAlphaCutoff, SetAlphaCutoff, "Gets/sets the mask cutoff from 0 to 1."),
	{
		"doubleSided", "?", "Gets/sets whether both face orientations render.",
		[](const CAEONMaterial3D& Obj, const CString&) { const auto* pMaterial = GetWrapperMaterial(Obj); return (pMaterial ? CDatum(pMaterial->GetDoubleSided()) : CDatum()); },
		[](CAEONMaterial3D& Obj, const CString& sProperty, CDatum dValue, CString* retsError) { auto* pMaterial = GetMutableWrapperMaterial(Obj); if (!pMaterial) return InvalidMaterialProperty(sProperty, retsError); pMaterial->SetDoubleSided(!dValue.IsNil()); return true; },
	},
	MATERIAL_SCALAR_PROPERTY("bumpScale", GetBumpScale, SetBumpScale, "Gets/sets bump-map depth."),
	MATERIAL_VECTOR_PROPERTY("normalScale", GetNormalScale, SetNormalScale, "Gets/sets tangent-space normal-map XY scale."),
	MATERIAL_SCALAR_PROPERTY("displacementScale", GetDisplacementScale, SetDisplacementScale, "Gets/sets vertex displacement scale."),
	MATERIAL_SCALAR_PROPERTY("displacementBias", GetDisplacementBias, SetDisplacementBias, "Gets/sets vertex displacement bias."),
	MATERIAL_SCALAR_PROPERTY("aoMapIntensity", GetAOMapIntensity, SetAOMapIntensity, "Gets/sets nonnegative ambient-occlusion strength."),
	MATERIAL_SCALAR_PROPERTY("lightMapIntensity", GetLightMapIntensity, SetLightMapIntensity, "Gets/sets nonnegative baked-light strength."),
	MATERIAL_SCALAR_PROPERTY("anisotropy", GetAnisotropy, SetAnisotropy, "Gets/sets anisotropic response from 0 to 1."),
	MATERIAL_SCALAR_PROPERTY("anisotropyRotation", GetAnisotropyRotation, SetAnisotropyRotation, "Gets/sets anisotropy rotation in radians."),
	MATERIAL_COLOR_PROPERTY("attenuationColor", GetAttenuationColor, SetAttenuationColor, "Gets/sets transmission attenuation color."),
	MATERIAL_SCALAR_PROPERTY("attenuationDistance", GetAttenuationDistance, SetAttenuationDistance, "Gets/sets attenuation distance; zero means infinite."),
	MATERIAL_SCALAR_PROPERTY("clearcoat", GetClearcoat, SetClearcoat, "Gets/sets clearcoat strength from 0 to 1."),
	MATERIAL_SCALAR_PROPERTY("clearcoatRoughness", GetClearcoatRoughness, SetClearcoatRoughness, "Gets/sets clearcoat roughness from 0 to 1."),
	MATERIAL_VECTOR_PROPERTY("clearcoatNormalScale", GetClearcoatNormalScale, SetClearcoatNormalScale, "Gets/sets clearcoat normal-map XY scale."),
	MATERIAL_SCALAR_PROPERTY("dispersion", GetDispersion, SetDispersion, "Gets/sets nonnegative chromatic dispersion."),
	MATERIAL_SCALAR_PROPERTY("ior", GetIOR, SetIOR, "Gets/sets index of refraction from 1 to 2.333."),
	MATERIAL_SCALAR_PROPERTY("iridescence", GetIridescence, SetIridescence, "Gets/sets iridescence strength from 0 to 1."),
	MATERIAL_SCALAR_PROPERTY("iridescenceIOR", GetIridescenceIOR, SetIridescenceIOR, "Gets/sets thin-film index of refraction from 1 to 2.333."),
	MATERIAL_VECTOR_PROPERTY("iridescenceThicknessRange", GetIridescenceThicknessRange, SetIridescenceThicknessRange, "Gets/sets the nonnegative minimum and maximum film thickness."),
	MATERIAL_SCALAR_PROPERTY("sheen", GetSheen, SetSheen, "Gets/sets sheen strength from 0 to 1."),
	MATERIAL_COLOR_PROPERTY("sheenColor", GetSheenColor, SetSheenColor, "Gets/sets sheen color."),
	MATERIAL_SCALAR_PROPERTY("sheenRoughness", GetSheenRoughness, SetSheenRoughness, "Gets/sets sheen roughness from 0 to 1."),
	MATERIAL_COLOR_PROPERTY("specularColor", GetSpecularColor, SetSpecularColor, "Gets/sets dielectric specular color."),
	MATERIAL_SCALAR_PROPERTY("specularIntensity", GetSpecularIntensity, SetSpecularIntensity, "Gets/sets dielectric specular intensity from 0 to 1."),
	MATERIAL_SCALAR_PROPERTY("thickness", GetThickness, SetThickness, "Gets/sets nonnegative transmission volume thickness."),
	MATERIAL_SCALAR_PROPERTY("transmission", GetTransmission, SetTransmission, "Gets/sets transmission from 0 to 1."),
	MATERIAL_MAP_PROPERTY("colorMap", Material3DMap::Color, "Gets/sets the base-color texture."),
	MATERIAL_MAP_PROPERTY("opacityMap", Material3DMap::Opacity, "Gets/sets the opacity texture."),
	MATERIAL_MAP_PROPERTY("emissiveMap", Material3DMap::Emissive, "Gets/sets the emissive texture."),
	MATERIAL_MAP_PROPERTY("normalMap", Material3DMap::Normal, "Gets/sets the tangent-space normal texture."),
	MATERIAL_MAP_PROPERTY("bumpMap", Material3DMap::Bump, "Gets/sets the height texture used for bump shading."),
	MATERIAL_MAP_PROPERTY("displacementMap", Material3DMap::Displacement, "Gets/sets the vertex displacement texture."),
	MATERIAL_MAP_PROPERTY("roughnessMap", Material3DMap::Roughness, "Gets/sets the roughness texture."),
	MATERIAL_MAP_PROPERTY("metalnessMap", Material3DMap::Metalness, "Gets/sets the metalness texture."),
	MATERIAL_MAP_PROPERTY("aoMap", Material3DMap::AO, "Gets/sets the ambient-occlusion texture."),
	MATERIAL_MAP_PROPERTY("lightMap", Material3DMap::Light, "Gets/sets the baked-light texture."),
	MATERIAL_MAP_PROPERTY("anisotropyMap", Material3DMap::Anisotropy, "Gets/sets the anisotropy texture."),
	MATERIAL_MAP_PROPERTY("clearcoatMap", Material3DMap::Clearcoat, "Gets/sets the clearcoat-strength texture."),
	MATERIAL_MAP_PROPERTY("clearcoatNormalMap", Material3DMap::ClearcoatNormal, "Gets/sets the clearcoat normal texture."),
	MATERIAL_MAP_PROPERTY("clearcoatRoughnessMap", Material3DMap::ClearcoatRoughness, "Gets/sets the clearcoat roughness texture."),
	MATERIAL_MAP_PROPERTY("iridescenceMap", Material3DMap::Iridescence, "Gets/sets the iridescence-strength texture."),
	MATERIAL_MAP_PROPERTY("iridescenceThicknessMap", Material3DMap::IridescenceThickness, "Gets/sets the thin-film thickness texture."),
	MATERIAL_MAP_PROPERTY("sheenColorMap", Material3DMap::SheenColor, "Gets/sets the sheen-color texture."),
	MATERIAL_MAP_PROPERTY("sheenRoughnessMap", Material3DMap::SheenRoughness, "Gets/sets the sheen roughness texture."),
	MATERIAL_MAP_PROPERTY("specularColorMap", Material3DMap::SpecularColor, "Gets/sets the dielectric specular-color texture."),
	MATERIAL_MAP_PROPERTY("specularIntensityMap", Material3DMap::SpecularIntensity, "Gets/sets the dielectric specular-intensity texture."),
	MATERIAL_MAP_PROPERTY("thicknessMap", Material3DMap::Thickness, "Gets/sets the transmission thickness texture."),
	MATERIAL_MAP_PROPERTY("transmissionMap", Material3DMap::Transmission, "Gets/sets the transmission texture."),
};

TDatumMethodHandler<CAEONMaterial3D> CAEONMaterial3D::m_Methods = { };

const CString& CAEONMaterial3D::StaticGetTypename () { return TYPENAME_MATERIAL3D; }

CDatum CAEONMaterial3D::Create (CDatum dScene, DWORD dwID)
	{
	if (!CAEONReanimator3D::Upconvert(dScene))
		throw CException(errFail);
	return CDatum(new CAEONMaterial3D(dScene, dwID));
	}

CDatum CAEONMaterial3D::GetDatatype () const
	{
	const ILuminousMaterial3D* pMaterial = GetWrapperMaterial(*this);
	return CAEONTypes::Get(pMaterial ? CAEONLuminous::PHYSICAL_MATERIAL3D_TYPE : CAEONLuminous::MATERIAL3D_TYPE);
	}

TArray<IDatatype::SMemberDesc> CAEONMaterial3D::GetMembers ()
	{
	TArray<IDatatype::SMemberDesc> Members;
	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);
	return Members;
	}

void CAEONMaterial3D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONMaterial3D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}

static CDatum GetWrapperProperty (const CAEONObj3D& Wrapper, Obj3DProp iProp)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	const ILuminousObj3D* pObj = (pScene ? pScene->FindObj(Wrapper.GetID()) : NULL);
	if (!pObj)
		return CDatum();

	switch (ILuminousObj3D::GetPropertyDesc(iProp).iType)
		{
		case Obj3DPropType::Bool: return CDatum(pObj->GetPropertyBool(iProp));
		case Obj3DPropType::Color: return CAEONLuminous::AsDatum(pObj->GetPropertyColor(iProp));
		case Obj3DPropType::Scalar: return CDatum(pObj->GetPropertyScalar(iProp));
		case Obj3DPropType::String: return CDatum(pObj->GetPropertyString(iProp));
		case Obj3DPropType::Vector: return CDatum(pObj->GetPropertyVector(iProp));
		default: return CDatum();
		}
	}

static bool SetWrapperProperty (CAEONObj3D& Wrapper, Obj3DProp iProp, CDatum dValue)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	return (pScene ? pScene->SetObjProperty(Wrapper.GetID(), iProp, dValue) : false);
	}

static CDatum GetWrapperColor (const CAEONObj3D& Wrapper)
	{
	auto* pScene = CAEONReanimator3D::Upconvert(Wrapper.GetScene());
	const ILuminousObj3D* pObj = (pScene ? pScene->FindObj(Wrapper.GetID()) : NULL);
	if (!pObj)
		return CDatum();

	DWORD dwTextureID = (!pObj->GetMaterialID() ? pObj->GetColorTextureID() : 0);
	return (dwTextureID ? CAEONTexture3D::Create(Wrapper.GetScene(), dwTextureID) : CAEONLuminous::AsDatum(pObj->GetPropertyColor(Obj3DProp::Color)));
	}

#define OBJ3D_PROPERTY_VECTOR(name, prop, help) \
	{ name, "?", help, \
		[](const CAEONObj3D& Obj, const CString&) { return GetWrapperProperty(Obj, prop); }, \
		[](CAEONObj3D& Obj, const CString&, CDatum dValue, CString*) { return SetWrapperProperty(Obj, prop, dValue); } }

#define OBJ3D_PROPERTY_SCALAR(name, prop, help) OBJ3D_PROPERTY_VECTOR(name, prop, help)
#define OBJ3D_PROPERTY_COLOR(name, prop, help) OBJ3D_PROPERTY_VECTOR(name, prop, help)
#define OBJ3D_PROPERTY_STRING(name, prop, help) OBJ3D_PROPERTY_VECTOR(name, prop, help)

TDatumPropertyHandler<CAEONObj3D> CAEONObj3D::m_Properties = {
	{
		"id", "?", "Returns the object ID.",
		[](const CAEONObj3D& Obj, const CString&) { return CDatum((int)Obj.m_dwID); },
		NULL,
	},
	{
		"type", "?", "Returns the native object type.",
		[](const CAEONObj3D& Obj, const CString&)
			{
			auto* pScene = CAEONReanimator3D::Upconvert(Obj.m_dScene);
			const ILuminousObj3D* pObj = (pScene ? pScene->FindObj(Obj.m_dwID) : NULL);
			return (pObj ? CDatum(pObj->GetObjType()) : CDatum());
			},
		NULL,
	},
	OBJ3D_PROPERTY_VECTOR("pos", Obj3DProp::Pos, "Gets/sets position."),
	OBJ3D_PROPERTY_VECTOR("rotation", Obj3DProp::Rot, "Gets/sets XYZ Euler rotation in radians."),
	OBJ3D_PROPERTY_VECTOR("scale", Obj3DProp::Scale, "Gets/sets scale."),
	OBJ3D_PROPERTY_VECTOR("visible", Obj3DProp::Visible, "Gets/sets visibility."),
	OBJ3D_PROPERTY_SCALAR("opacity", Obj3DProp::Opacity, "Gets/sets opacity."),
	{
		"material", "?", "Gets/sets the material for a compatible object.",
		[](const CAEONObj3D& Obj, const CString&)
			{
			auto* pScene = CAEONReanimator3D::Upconvert(Obj.m_dScene);
			const ILuminousObj3D* pObj = (pScene ? pScene->FindObj(Obj.m_dwID) : NULL);
			DWORD dwMaterialID = (pObj ? pObj->GetMaterialID() : 0);
			return (dwMaterialID ? CAEONMaterial3D::Create(Obj.m_dScene, dwMaterialID) : CDatum());
			},
		[](CAEONObj3D& Obj, const CString&, CDatum dValue, CString* retsError)
			{
			auto* pScene = CAEONReanimator3D::Upconvert(Obj.m_dScene);
			if (!pScene || !pScene->SetObjMaterial(Obj.m_dwID, dValue))
				{
				if (retsError) *retsError = ERR_INVALID_MATERIAL;
				return false;
				}
			return true;
			},
	},
	{
		"color", "?", "Gets/sets a cube color or texture, or a light color.",
		[](const CAEONObj3D& Obj, const CString&) { return GetWrapperColor(Obj); },
		[](CAEONObj3D& Obj, const CString&, CDatum dValue, CString* retsError)
			{
			auto* pScene = CAEONReanimator3D::Upconvert(Obj.m_dScene);
			if (!pScene || !pScene->SetObjColor(Obj.m_dwID, dValue))
				{
				if (retsError) *retsError = ERR_INVALID_TEXTURE;
				return false;
				}
			return true;
			},
	},
	OBJ3D_PROPERTY_SCALAR("intensity", Obj3DProp::Intensity, "Gets/sets light intensity."),
	OBJ3D_PROPERTY_SCALAR("distance", Obj3DProp::Distance, "Gets/sets light distance."),
	OBJ3D_PROPERTY_SCALAR("decay", Obj3DProp::Decay, "Gets/sets light decay."),
	OBJ3D_PROPERTY_SCALAR("fov", Obj3DProp::FOV, "Gets/sets camera field of view."),
	OBJ3D_PROPERTY_SCALAR("near", Obj3DProp::Near, "Gets/sets camera near plane."),
	OBJ3D_PROPERTY_SCALAR("far", Obj3DProp::Far, "Gets/sets camera far plane."),
	OBJ3D_PROPERTY_STRING("gridID", Obj3DProp::GridID, "Gets/sets the GridID for an imported model."),
	OBJ3D_PROPERTY_STRING("resourcePath", Obj3DProp::ResourcePath, "Gets/sets the GridID containing an imported model's external resources."),
};

#undef OBJ3D_PROPERTY_VECTOR
#undef OBJ3D_PROPERTY_SCALAR
#undef OBJ3D_PROPERTY_COLOR
#undef OBJ3D_PROPERTY_STRING

TDatumMethodHandler<CAEONObj3D> CAEONObj3D::m_Methods = {
	{
		"addKeyframe", "*", ".addKeyframe(prop, frame, desc) -> true/false", 0,
		[](CAEONObj3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			auto* pScene = CAEONReanimator3D::Upconvert(Obj.m_dScene);
			if (!pScene)
				{
				retResult.dResult = ERR_INVALID_SCENE;
				return false;
				}

			Obj3DProp iProp = ILuminousObj3D::ParseProperty(LocalEnv.GetArgument(1).AsStringView());
			if (iProp == Obj3DProp::Unknown)
				{
				retResult.dResult = strPattern(ERR_UNKNOWN_PROPERTY, LocalEnv.GetArgument(1).AsString());
				return false;
				}

			retResult.dResult = pScene->AnimateObjProperty(Obj.m_dwID, iProp, (int)LocalEnv.GetArgument(2), LocalEnv.GetArgument(3));
			return true;
			},
	},
};

const CString& CAEONObj3D::StaticGetTypename () { return TYPENAME_OBJ3D; }

CDatum CAEONObj3D::Create (CDatum dScene, DWORD dwID)
	{
	if (!CAEONReanimator3D::Upconvert(dScene))
		throw CException(errFail);
	return CDatum(new CAEONObj3D(dScene, dwID));
	}

CDatum CAEONObj3D::GetDatatype () const { return CAEONTypes::Get(CAEONLuminous::OBJECT3D_TYPE); }

TArray<IDatatype::SMemberDesc> CAEONObj3D::GetMembers ()
	{
	TArray<IDatatype::SMemberDesc> Members;
	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);
	return Members;
	}

void CAEONObj3D::DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized)
	{
	m_dScene = CDatum::DeserializeAEON(Stream, Serialized);
	m_dwID = Stream.ReadDWORD();
	}

void CAEONObj3D::SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	m_dScene.SerializeAEON(Stream, Serialized);
	Stream.Write(m_dwID);
	}

//	Scene ----------------------------------------------------------------------

TDatumPropertyHandler<CAEONReanimator3D> CAEONReanimator3D::m_Properties = {
	{
		"activeCamera", "?", "Gets/sets the active perspective camera.",
		[](const CAEONReanimator3D& Obj, const CString&)
			{
			DWORD dwID = Obj.m_Model.GetActiveCameraID();
			if (!dwID) return CDatum();
			CDatum dSelf = CDatum::raw_AsComplex(const_cast<CAEONReanimator3D*>(&Obj));
			return CAEONObj3D::Create(dSelf, dwID);
			},
		[](CAEONReanimator3D& Obj, const CString&, CDatum dValue, CString*)
			{
			Obj.m_Model.SetActiveCameraID(ParseID(dValue));
			return true;
			},
	},
	{
		"background", "?", "Gets/sets the scene background.",
		[](const CAEONReanimator3D& Obj, const CString&) { return CAEONLuminous::AsDatum(Obj.m_Model.GetBackgroundColor()); },
		[](CAEONReanimator3D& Obj, const CString&, CDatum dValue, CString*) { Obj.m_Model.SetBackgroundColor(CAEONLuminous::AsColor(dValue)); return true; },
	},
	{
		"environment", "?", "Gets/sets a solid color used for image-based scene lighting.",
		[](const CAEONReanimator3D& Obj, const CString&) { return CAEONLuminous::AsDatum(Obj.m_Model.GetEnvironmentColor()); },
		[](CAEONReanimator3D& Obj, const CString&, CDatum dValue, CString*) { Obj.m_Model.SetEnvironmentColor(CAEONLuminous::AsColor(dValue)); return true; },
	},
	{
		"fps", "?", "Gets/sets frames per second.",
		[](const CAEONReanimator3D& Obj, const CString&) { return CDatum(Obj.m_Model.GetFPS()); },
		[](CAEONReanimator3D& Obj, const CString&, CDatum dValue, CString*) { Obj.m_Model.SetFPS((int)dValue); return true; },
	},
	{
		"materials", "?", "Returns all scene materials.",
		[](const CAEONReanimator3D& Obj, const CString&)
			{
			CDatum dSelf = CDatum::raw_AsComplex(const_cast<CAEONReanimator3D*>(&Obj));
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Model.GetMaterialCount(); i++)
				dResult.Append(CAEONMaterial3D::Create(dSelf, Obj.m_Model.GetMaterial(i).GetID()));
			return dResult;
			},
		NULL,
	},
	{
		"textures", "?", "Returns all scene textures.",
		[](const CAEONReanimator3D& Obj, const CString&)
			{
			CDatum dSelf = CDatum::raw_AsComplex(const_cast<CAEONReanimator3D*>(&Obj));
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Model.GetTextureCount(); i++)
				dResult.Append(CAEONTexture3D::Create(dSelf, Obj.m_Model.GetTexture(i).GetID()));
			return dResult;
			},
		NULL,
	},
	{
		"mode", "?", "Gets/sets animation mode.",
		[](const CAEONReanimator3D& Obj, const CString&) { return CDatum(CLuminousScene3D::AsID(Obj.m_Model.GetMode())); },
		[](CAEONReanimator3D& Obj, const CString&, CDatum dValue, CString* retsError)
			{
			auto iMode = CLuminousScene3D::AsMode(dValue.AsStringView());
			if (iMode == CLuminousScene3D::EMode::Unknown)
				{
				if (retsError) *retsError = strPattern(ERR_UNKNOWN_PROPERTY, dValue.AsString());
				return false;
				}
			Obj.m_Model.SetMode(iMode);
			return true;
			},
	},
	{
		"objects", "?", "Returns all scene objects.",
		[](const CAEONReanimator3D& Obj, const CString&)
			{
			CDatum dSelf = CDatum::raw_AsComplex(const_cast<CAEONReanimator3D*>(&Obj));
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < Obj.m_Model.GetObjCount(); i++)
				dResult.Append(CAEONObj3D::Create(dSelf, Obj.m_Model.GetObj(i).GetID()));
			return dResult;
			},
		NULL,
	},
};

TDatumMethodHandler<CAEONReanimator3D> CAEONReanimator3D::m_Methods = {
	{
		"addKeyframe", "*", ".addKeyframe(id, prop, frame, desc) -> true/false", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			DWORD dwID = ParseID(LocalEnv.GetArgument(1));
			Obj3DProp iProp = ILuminousObj3D::ParseProperty(LocalEnv.GetArgument(2).AsStringView());
			if (iProp == Obj3DProp::Unknown)
				{
				retResult.dResult = strPattern(ERR_UNKNOWN_PROPERTY, LocalEnv.GetArgument(2).AsString());
				return false;
				}
			retResult.dResult = Obj.AnimateProperty(dwID, iProp, (int)LocalEnv.GetArgument(3), LocalEnv.GetArgument(4));
			return true;
			},
	},
	{
		"advanceFrame", "*", ".advanceFrame(count?) -> true", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			Obj.m_Model.AdvanceFrame(LocalEnv.GetCount() > 1 ? (int)LocalEnv.GetArgument(1) : 1);
			retResult.dResult = CDatum(true);
			return true;
			},
	},
	{
		"create3DS", "$Object3DType:gridID=?|gridID=?,desc=?", ".create3DS(gridID, desc?) -> Object3DType", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.Create3DSObj(
				LocalEnv.GetArgument(0),
				LocalEnv.GetArgument(1).AsString(),
				LocalEnv.GetCount() > 2 ? LocalEnv.GetArgument(2) : CDatum());
			return true;
			},
	},
	{
		"createCamera", "$Object3DType:|desc=?", ".createCamera(desc?) -> Object3DType", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.CreateCameraObj(LocalEnv.GetArgument(0), LocalEnv.GetCount() > 1 ? LocalEnv.GetArgument(1) : CDatum());
			return true;
			},
	},
	{
		"createCube", "$Object3DType:|desc=?", ".createCube(desc?) -> Object3DType", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			CString sError;
			retResult.dResult = Obj.CreateCubeObj(LocalEnv.GetArgument(0), LocalEnv.GetCount() > 1 ? LocalEnv.GetArgument(1) : CDatum(), &sError);
			if (!sError.IsEmpty())
				{
				retResult.dResult = sError;
				return false;
				}
			return true;
			},
	},
	{
		"createGLTF", "$Object3DType:gridID=?|gridID=?,desc=?", ".createGLTF(gridID, desc?) -> Object3DType", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.CreateGLTFObj(
				LocalEnv.GetArgument(0),
				LocalEnv.GetArgument(1).AsString(),
				LocalEnv.GetCount() > 2 ? LocalEnv.GetArgument(2) : CDatum());
			return true;
			},
		},
	{
		"createImageTexture", "$Texture3DType:gridID=?|gridID=?,options=?", ".createImageTexture(gridID, options?) -> Texture3DType", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.CreateImageTexture(
				LocalEnv.GetArgument(0),
				LocalEnv.GetArgument(1).AsString(),
				LocalEnv.GetCount() > 2 ? LocalEnv.GetArgument(2) : CDatum());
			return true;
			},
	},
	{
		"createPhysicalMaterial", "$PhysicalMaterial3D:|desc=?", ".createPhysicalMaterial(desc?) -> PhysicalMaterial3D", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			CString sError;
			retResult.dResult = Obj.CreatePhysicalMaterial(LocalEnv.GetArgument(0), LocalEnv.GetCount() > 1 ? LocalEnv.GetArgument(1) : CDatum(), &sError);
			if (!sError.IsEmpty())
				{
				retResult.dResult = sError;
				return false;
				}
			return true;
			},
	},
	{
		"createPointLight", "$Object3DType:|desc=?", ".createPointLight(desc?) -> Object3DType", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.CreatePointLightObj(LocalEnv.GetArgument(0), LocalEnv.GetCount() > 1 ? LocalEnv.GetArgument(1) : CDatum());
			return true;
			},
	},
	{
		"removeObject", "*", ".removeObject(obj) -> true/false", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = CDatum(Obj.RemoveObj(ParseID(LocalEnv.GetArgument(1))));
			return true;
			},
	},
	{
		"renderHTMLCanvasCommands", "*", ".renderHTMLCanvasCommands() -> desc", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			retResult.dResult = Obj.RenderAsHTMLCanvasCommands((SequenceNumber)LocalEnv.GetArgument(1));
			return true;
			},
	},
	{
		"setAt", "*", ".setAt(id, prop, value) -> true/false", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			Obj3DProp iProp = ILuminousObj3D::ParseProperty(LocalEnv.GetArgument(2).AsStringView());
			if (iProp == Obj3DProp::Unknown)
				{
				retResult.dResult = strPattern(ERR_UNKNOWN_PROPERTY, LocalEnv.GetArgument(2).AsString());
				return false;
				}
			retResult.dResult = Obj.SetObjProperty(ParseID(LocalEnv.GetArgument(1)), iProp, LocalEnv.GetArgument(3));
			return true;
			},
	},
	{
		"setKeyframe", "*", ".setKeyframe(frame, type) -> true; .setKeyframe() clears", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			if (LocalEnv.GetCount() == 1)
				Obj.m_Model.ClearKeyframe();
			else
				{
				auto iType = IAnimator3D::AsType(LocalEnv.GetArgument(2).AsStringView());
				if (iType == IAnimator3D::Type::Unknown) iType = IAnimator3D::Type::Linear;
				Obj.m_Model.SetKeyframe((int)LocalEnv.GetArgument(1), iType);
				}
			retResult.dResult = CDatum(true);
			return true;
			},
	},
	{
		"trimKeyframes", "*", ".trimKeyframes(frame) -> true", 0,
		[](CAEONReanimator3D& Obj, IInvokeCtx&, const CString&, CHexeStackEnv& LocalEnv, CDatum, CDatum, SAEONInvokeResult& retResult)
			{
			Obj.m_Model.TrimKeyframes((int)LocalEnv.GetArgument(1));
			retResult.dResult = CDatum(true);
			return true;
			},
	},
};

CDatum CAEONReanimator3D::Create () { return CDatum(new CAEONReanimator3D); }
const CString& CAEONReanimator3D::StaticGetTypename () { return TYPENAME_REANIMATOR3D; }
CDatum CAEONReanimator3D::GetDatatype () const { return CAEONTypes::Get(CAEONLuminous::SCENE3D_TYPE); }

TArray<IDatatype::SMemberDesc> CAEONReanimator3D::GetMembers ()
	{
	TArray<IDatatype::SMemberDesc> Members;
	m_Properties.AccumulateMembers(Members);
	m_Methods.AccumulateMembers(Members);
	return Members;
	}

bool CAEONReanimator3D::OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream)
	{
	m_Model = CLuminousScene3D::CreateFromStream(Stream);
	return true;
	}

int CAEONReanimator3D::OpCompare (CDatum::Types iValueType, CDatum dValue) const
	{
	const IAEONReanimator* pOther = dValue.GetReanimatorInterface();
	return (pOther ? RenderAsHTMLCanvasCommands().OpCompare(pOther->RenderAsHTMLCanvasCommands()) : KeyCompare(AsString(), dValue.AsString()));
	}

bool CAEONReanimator3D::OpIsEqual (CDatum::Types iValueType, CDatum dValue) const
	{
	const IAEONReanimator* pOther = dValue.GetReanimatorInterface();
	return (pOther && RenderAsHTMLCanvasCommands().OpIsEqual(pOther->RenderAsHTMLCanvasCommands()));
	}

DWORD CAEONReanimator3D::ParseID (CDatum dValue)
	{
	if (auto* pObj = CAEONObj3D::Upconvert(dValue)) return pObj->GetID();
	else return (DWORD)dValue;
	}

CDatum CAEONReanimator3D::Create3DSObj (CDatum dSelf, const CString& sGridID, CDatum dDesc)
	{
	DWORD dwParentID = ParseID(dDesc.GetElement(FIELD_PARENT_ID));
	ILuminousObj3D& Obj = m_Model.Create3DS(sGridID, dwParentID);
	if (!dDesc.IsNil()) SetObjProperties(Obj, dDesc);
	return CAEONObj3D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator3D::CreateCameraObj (CDatum dSelf, CDatum dDesc)
	{
	DWORD dwParentID = ParseID(dDesc.GetElement(FIELD_PARENT_ID));
	ILuminousObj3D& Obj = m_Model.CreatePerspectiveCamera(dwParentID);
	if (!dDesc.IsNil()) SetObjProperties(Obj, dDesc);
	return CAEONObj3D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator3D::CreateCubeObj (CDatum dSelf, CDatum dDesc, CString* retsError)
	{
	CDatum dMaterial = dDesc.GetElement(FIELD_MATERIAL);
	if (!dMaterial.IsNil() && !dDesc.GetElement(FIELD_COLOR).IsNil())
		{
		if (retsError) *retsError = ERR_AMBIGUOUS_CUBE_MATERIAL;
		return CDatum();
		}

	DWORD dwMaterialID = 0;
	if (!dMaterial.IsNil())
		{
		auto* pMaterial = CAEONMaterial3D::Upconvert(dMaterial);
		auto* pMaterialScene = (pMaterial ? CAEONReanimator3D::Upconvert(pMaterial->GetScene()) : NULL);
		if (!pMaterial || pMaterialScene != this || !FindMaterial(pMaterial->GetID()))
			{
			if (retsError) *retsError = ERR_INVALID_MATERIAL;
			return CDatum();
			}
		dwMaterialID = pMaterial->GetID();
		}

	DWORD dwParentID = ParseID(dDesc.GetElement(FIELD_PARENT_ID));
	ILuminousObj3D& Obj = m_Model.CreateCube(dwParentID);
	if (!dDesc.IsNil()) SetObjProperties(Obj, dDesc);
	if (dwMaterialID) m_Model.SetObjMaterial(Obj.GetID(), dwMaterialID);
	return CAEONObj3D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator3D::CreateGLTFObj (CDatum dSelf, const CString& sGridID, CDatum dDesc)
	{
	DWORD dwParentID = ParseID(dDesc.GetElement(FIELD_PARENT_ID));
	ILuminousObj3D& Obj = m_Model.CreateGLTF(sGridID, dwParentID);
	if (!dDesc.IsNil()) SetObjProperties(Obj, dDesc);
	return CAEONObj3D::Create(dSelf, Obj.GetID());
	}

CDatum CAEONReanimator3D::CreateImageTexture (CDatum dSelf, const CString& sGridID, CDatum dOptions)
	{
	CDatum dFlipY;
	bool bFlipY = (!dOptions.FindElement(FIELD_FLIP_Y, &dFlipY) || !dFlipY.IsNil());
	ILuminousTexture3D& Texture = m_Model.CreateImageTexture(sGridID, dOptions.GetElement(FIELD_FORMAT).AsString(), bFlipY);
	return CAEONTexture3D::Create(dSelf, Texture.GetID());
	}

CDatum CAEONReanimator3D::CreatePhysicalMaterial (CDatum dSelf, CDatum dDesc, CString* retsError)
	{
	ILuminousMaterial3D& Material = m_Model.CreatePhysicalMaterial();
	CDatum dResult = CAEONMaterial3D::Create(dSelf, Material.GetID());
	auto* pWrapper = CAEONMaterial3D::Upconvert(dResult);
	if (!pWrapper)
		throw CException(errFail);

	for (int i = 0; i < dDesc.GetCount(); i++)
		if (!pWrapper->SetProperty(dDesc.GetKey(i), dDesc.GetElement(i), retsError))
			{
			m_Model.RemoveMaterial(Material.GetID());
			return CDatum();
			}

	return dResult;
	}

CDatum CAEONReanimator3D::CreatePointLightObj (CDatum dSelf, CDatum dDesc)
	{
	DWORD dwParentID = ParseID(dDesc.GetElement(FIELD_PARENT_ID));
	ILuminousObj3D& Obj = m_Model.CreatePointLight(dwParentID);
	if (!dDesc.IsNil()) SetObjProperties(Obj, dDesc);
	return CAEONObj3D::Create(dSelf, Obj.GetID());
	}

bool CAEONReanimator3D::AnimateProperty (DWORD dwID, Obj3DProp iProp, int iFrame, CDatum dDesc)
	{
	ILuminousObj3D* pObj = m_Model.FindObj(dwID);
	return (pObj && AnimateProperty(*pObj, iProp, iFrame, dDesc));
	}

bool CAEONReanimator3D::AnimateProperty (ILuminousObj3D& Obj, Obj3DProp iProp, int iFrame, CDatum dDesc)
	{
	if (iFrame < 0) return false;
	if (iProp == Obj3DProp::GridID || iProp == Obj3DProp::ResourcePath) return false;
	auto iType = IAnimator3D::AsType(dDesc.GetElement(FIELD_TYPE).AsStringView());
	CDatum dValue = dDesc.GetElement(FIELD_VALUE);

	switch (ILuminousObj3D::GetPropertyDesc(iProp).iType)
		{
		case Obj3DPropType::Bool:
			return (iType == IAnimator3D::Type::Constant && Obj.AnimateBoolConstant(iProp, iFrame, !dValue.IsNil()));
		case Obj3DPropType::Color:
			return (iType == IAnimator3D::Type::Constant && Obj.AnimateColorConstant(iProp, iFrame, CAEONLuminous::AsColor(dValue)));
		case Obj3DPropType::Scalar:
			if (iType == IAnimator3D::Type::Constant) return Obj.AnimateScalarConstant(iProp, iFrame, dValue);
			else if (iType == IAnimator3D::Type::Linear) return Obj.AnimateScalarLinear(iProp, iFrame, dValue);
			else return false;
		case Obj3DPropType::String:
			return (iType == IAnimator3D::Type::Constant && Obj.AnimateStringConstant(iProp, iFrame, dValue.AsStringView()));
		case Obj3DPropType::Vector:
			{
			CVector3D Value(dValue.GetElement(0), dValue.GetElement(1), dValue.GetElement(2));
			if (iType == IAnimator3D::Type::Constant) return Obj.AnimateVectorConstant(iProp, iFrame, Value);
			else if (iType == IAnimator3D::Type::Linear) return Obj.AnimateVectorLinear(iProp, iFrame, Value);
			else return false;
			}
		default:
			return false;
		}
	}

bool CAEONReanimator3D::SetObjProperty (DWORD dwID, Obj3DProp iProp, CDatum dValue)
	{
	if (iProp == Obj3DProp::Color)
		return SetObjColor(dwID, dValue);

	ILuminousObj3D* pObj = m_Model.FindObj(dwID);
	return (pObj && SetObjProperty(*pObj, iProp, dValue));
	}

bool CAEONReanimator3D::SetObjColor (DWORD dwID, CDatum dValue)
	{
	ILuminousObj3D* pObj = m_Model.FindObj(dwID);
	if (!pObj)
		return false;
	if (pObj->GetMaterialID())
		return false;

	if (auto* pTexture = CAEONTexture3D::Upconvert(dValue))
		{
		auto* pTextureScene = CAEONReanimator3D::Upconvert(pTexture->GetScene());
		if (pTextureScene != this || !FindTexture(pTexture->GetID()))
			return false;

		return m_Model.SetObjColorTexture(dwID, pTexture->GetID());
		}

	if (pObj->GetColorTextureID() && !pObj->SetColorTextureID(0))
		return false;

	return SetObjProperty(*pObj, Obj3DProp::Color, dValue);
	}

bool CAEONReanimator3D::SetObjMaterial (DWORD dwID, CDatum dValue)
	{
	if (dValue.IsNil())
		return m_Model.SetObjMaterial(dwID, 0);

	auto* pMaterial = CAEONMaterial3D::Upconvert(dValue);
	auto* pMaterialScene = (pMaterial ? CAEONReanimator3D::Upconvert(pMaterial->GetScene()) : NULL);
	if (!pMaterial || pMaterialScene != this || !FindMaterial(pMaterial->GetID()))
		return false;

	return m_Model.SetObjMaterial(dwID, pMaterial->GetID());
	}

bool CAEONReanimator3D::SetObjProperty (ILuminousObj3D& Obj, Obj3DProp iProp, CDatum dValue)
	{
	Obj3DPropType iPropType = ILuminousObj3D::GetPropertyDesc(iProp).iType;
	bool bKeyframeArray = false;
	if (!m_Model.IsStreamMode() && (dValue.GetBasicType() == CDatum::typeArray || dValue.GetBasicType() == CDatum::typeTensor) && dValue.GetCount() > 0)
		bKeyframeArray = !dValue.GetElement(0).GetElement(FIELD_FRAME).IsNil();

	if (bKeyframeArray)
		{
		Obj.RemoveAnimation(iProp);
		for (int i = 0; i < dValue.GetCount(); i++)
			if (!AnimateProperty(Obj, iProp, (int)dValue.GetElement(i).GetElement(FIELD_FRAME), dValue.GetElement(i))) return false;
		return true;
		}
	else if (m_Model.IsKeyframeMode())
		{
		CDatum dDesc(CDatum::typeStruct);
		dDesc.SetElement(FIELD_TYPE, IAnimator3D::AsID(m_Model.GetKeyframeType()));
		dDesc.SetElement(FIELD_VALUE, dValue);
		return AnimateProperty(Obj, iProp, m_Model.GetKeyframeFrame(), dDesc);
		}

	if (!m_Model.IsStreamMode()) Obj.RemoveAnimation(iProp);
	switch (iPropType)
		{
		case Obj3DPropType::Bool: return Obj.SetPropertyBool(iProp, !dValue.IsNil());
		case Obj3DPropType::Color: return Obj.SetPropertyColor(iProp, CAEONLuminous::AsColor(dValue));
		case Obj3DPropType::Scalar: return Obj.SetPropertyScalar(iProp, dValue);
		case Obj3DPropType::String: return Obj.SetPropertyString(iProp, dValue.AsStringView());
		case Obj3DPropType::Vector: return Obj.SetPropertyVector(iProp, CVector3D(dValue.GetElement(0), dValue.GetElement(1), dValue.GetElement(2)));
		default: return false;
		}
	}

bool CAEONReanimator3D::SetObjProperties (ILuminousObj3D& Obj, CDatum dData)
	{
	for (int i = 0; i < dData.GetCount(); i++)
		{
		CString sKey = dData.GetKey(i);
		if (strEquals(sKey, FIELD_PARENT_ID)) continue;
		Obj3DProp iProp = ILuminousObj3D::ParseProperty(sKey);
		if (iProp != Obj3DProp::Unknown
				&& !(iProp == Obj3DProp::Color ? SetObjColor(Obj.GetID(), dData.GetElement(i)) : SetObjProperty(Obj, iProp, dData.GetElement(i))))
			return false;
		}
	return true;
	}

CDatum CAEONReanimator3D::CompactValue (Obj3DPropType iType, CDatum dValue)
	{
	if (iType != Obj3DPropType::Vector) return dValue;
	CDatum dResult(CDatum::typeArray);
	dResult.Append(dValue.GetElement(0));
	dResult.Append(dValue.GetElement(1));
	dResult.Append(dValue.GetElement(2));
	return dResult;
	}

CDatum CAEONReanimator3D::RenderConstProperty (const ILuminousObj3D& Obj, Obj3DProp iProp, Obj3DPropType iType) const
	{
	switch (iType)
		{
		case Obj3DPropType::Bool: return CDatum(Obj.GetPropertyBool(iProp));
		case Obj3DPropType::Color: return CAEONLuminous::AsDatum(Obj.GetPropertyColor(iProp));
		case Obj3DPropType::Scalar: return CDatum(Obj.GetPropertyScalar(iProp));
		case Obj3DPropType::String: return CDatum(Obj.GetPropertyString(iProp));
		case Obj3DPropType::Vector: return CDatum(Obj.GetPropertyVector(iProp));
		default: return CDatum();
		}
	}

CDatum CAEONReanimator3D::RenderAnimatedProperty (const IAnimator3D& Animator)
	{
	const auto& Frames = Animator.GetKeyframes();
	auto GetValue = [&](int i) -> CDatum
		{
		switch (Animator.GetPropertyType())
			{
			case Obj3DPropType::Bool: return CDatum(Animator.GetKeyframesBool()[i]);
			case Obj3DPropType::Color: return CAEONLuminous::AsDatum(Animator.GetKeyframesColor()[i]);
			case Obj3DPropType::Scalar: return CDatum(Animator.GetKeyframesScalar()[i]);
			case Obj3DPropType::String: return CDatum(Animator.GetKeyframesString()[i]);
			case Obj3DPropType::Vector: return CompactValue(Obj3DPropType::Vector, CDatum(Animator.GetKeyframesVector()[i]));
			default: throw CException(errFail);
			}
		};

	bool bDense = (Frames.GetCount() > 0);
	for (int i = 0; i < Frames.GetCount() && bDense; i++)
		if (Frames[i].iType != IAnimator3D::Type::Constant || (i > 0 && Frames[i].iFrame != Frames[i - 1].iFrame + 1)) bDense = false;

	CDatum dResult(CDatum::typeStruct);
	CDatum dValues(CDatum::typeArray);
	for (int i = 0; i < Frames.GetCount(); i++) dValues.Append(GetValue(i));

	if (bDense)
		{
		dResult.SetElement(FIELD_ANIM, ANIM_DENSE);
		dResult.SetElement(FIELD_START_FRAME, Frames[0].iFrame);
		dResult.SetElement(FIELD_VALUES, dValues);
		}
	else
		{
		CDatum dFrames(CDatum::typeArray);
		CDatum dTypes(CDatum::typeArray);
		for (int i = 0; i < Frames.GetCount(); i++)
			{
			dFrames.Append(Frames[i].iFrame);
			dTypes.Append(Frames[i].iType == IAnimator3D::Type::Linear ? 1 : (Frames[i].iType == IAnimator3D::Type::Blink ? 2 : 0));
			}
		dResult.SetElement(FIELD_ANIM, ANIM_SPARSE);
		dResult.SetElement(FIELD_FRAMES, dFrames);
		dResult.SetElement(FIELD_TYPES, dTypes);
		dResult.SetElement(FIELD_VALUES, dValues);
		}
	return dResult;
	}

CDatum CAEONReanimator3D::RenderObj (const ILuminousObj3D& Obj) const
	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_ID, Obj.GetID());
	if (Obj.GetParent()) dResult.SetElement(FIELD_PARENT_ID, Obj.GetParent()->GetID());
	dResult.SetElement(FIELD_SEQ, Obj.GetSeq());
	dResult.SetElement(FIELD_TYPE, Obj.GetObjType());
	if (Obj.GetMaterialID()) dResult.SetElement(FIELD_MATERIAL_ID, Obj.GetMaterialID());
	else if (Obj.GetColorTextureID()) dResult.SetElement(FIELD_COLOR_TEXTURE_ID, Obj.GetColorTextureID());

	auto Props = Obj.GetPropertiesToRender();
	for (int i = 0; i < Props.GetCount(); i++)
		dResult.SetElement(Props[i].sID, Props[i].pAnimator ? RenderAnimatedProperty(*Props[i].pAnimator) : CompactValue(Props[i].iType, RenderConstProperty(Obj, Props[i].iProp, Props[i].iType)));
	return dResult;
	}

static CDatum RenderMaterial (const ILuminousMaterial3D& Material)
	{
	auto RenderVector2 = [](const CVector2D& vValue)
		{
		CDatum dValue(CDatum::typeArray);
		dValue.Append(vValue.X());
		dValue.Append(vValue.Y());
		return dValue;
		};

	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_ID, Material.GetID());
	dResult.SetElement(FIELD_SEQ, Material.GetSeq());
	dResult.SetElement(FIELD_TYPE, Material.GetMaterialType());
	dResult.SetElement(FIELD_COLOR, CAEONLuminous::AsDatum(Material.GetColor()));
	dResult.SetElement(FIELD_OPACITY, Material.GetOpacity());
	dResult.SetElement(CString("metalness"), Material.GetMetalness());
	dResult.SetElement(CString("roughness"), Material.GetRoughness());
	dResult.SetElement(CString("emissiveColor"), CAEONLuminous::AsDatum(Material.GetEmissiveColor()));
	dResult.SetElement(CString("emissiveIntensity"), Material.GetEmissiveIntensity());
	dResult.SetElement(CString("alphaMode"), ILuminousMaterial3D::AsID(Material.GetAlphaMode()));
	dResult.SetElement(CString("alphaCutoff"), Material.GetAlphaCutoff());
	dResult.SetElement(CString("doubleSided"), Material.GetDoubleSided());
	dResult.SetElement(CString("bumpScale"), Material.GetBumpScale());
	dResult.SetElement(CString("normalScale"), RenderVector2(Material.GetNormalScale()));
	dResult.SetElement(CString("displacementScale"), Material.GetDisplacementScale());
	dResult.SetElement(CString("displacementBias"), Material.GetDisplacementBias());
	dResult.SetElement(CString("aoMapIntensity"), Material.GetAOMapIntensity());
	dResult.SetElement(CString("lightMapIntensity"), Material.GetLightMapIntensity());
	dResult.SetElement(CString("anisotropy"), Material.GetAnisotropy());
	dResult.SetElement(CString("anisotropyRotation"), Material.GetAnisotropyRotation());
	dResult.SetElement(CString("attenuationColor"), CAEONLuminous::AsDatum(Material.GetAttenuationColor()));
	dResult.SetElement(CString("attenuationDistance"), Material.GetAttenuationDistance());
	dResult.SetElement(CString("clearcoat"), Material.GetClearcoat());
	dResult.SetElement(CString("clearcoatRoughness"), Material.GetClearcoatRoughness());
	dResult.SetElement(CString("clearcoatNormalScale"), RenderVector2(Material.GetClearcoatNormalScale()));
	dResult.SetElement(CString("dispersion"), Material.GetDispersion());
	dResult.SetElement(CString("ior"), Material.GetIOR());
	dResult.SetElement(CString("iridescence"), Material.GetIridescence());
	dResult.SetElement(CString("iridescenceIOR"), Material.GetIridescenceIOR());
	dResult.SetElement(CString("iridescenceThicknessRange"), RenderVector2(Material.GetIridescenceThicknessRange()));
	dResult.SetElement(CString("sheen"), Material.GetSheen());
	dResult.SetElement(CString("sheenColor"), CAEONLuminous::AsDatum(Material.GetSheenColor()));
	dResult.SetElement(CString("sheenRoughness"), Material.GetSheenRoughness());
	dResult.SetElement(CString("specularColor"), CAEONLuminous::AsDatum(Material.GetSpecularColor()));
	dResult.SetElement(CString("specularIntensity"), Material.GetSpecularIntensity());
	dResult.SetElement(CString("thickness"), Material.GetThickness());
	dResult.SetElement(CString("transmission"), Material.GetTransmission());

	for (int i = 0; i < (int)Material3DMap::Count; i++)
		if (DWORD dwTextureID = Material.GetTextureID((Material3DMap)i))
			dResult.SetElement(strPattern("%sID", ILuminousMaterial3D::AsID((Material3DMap)i)), dwTextureID);

	return dResult;
	}

static CDatum RenderTexture (const ILuminousTexture3D& Texture)
	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_ID, Texture.GetID());
	dResult.SetElement(FIELD_SEQ, Texture.GetSeq());
	dResult.SetElement(FIELD_TYPE, Texture.GetTextureType());
	dResult.SetElement(FIELD_GRID_ID, Texture.GetGridID());
	if (!Texture.GetFormat().IsEmpty()) dResult.SetElement(FIELD_FORMAT, Texture.GetFormat());
	dResult.SetElement(FIELD_FLIP_Y, Texture.GetFlipY());
	return dResult;
	}

CDatum CAEONReanimator3D::RenderAsHTMLCanvasCommands (SequenceNumber Seq) const
	{
	CDatum dResult(CDatum::typeStruct);
	dResult.SetElement(FIELD_ACTIVE_CAMERA_ID, m_Model.GetActiveCameraID());
	dResult.SetElement(FIELD_BACKGROUND, CAEONLuminous::AsDatum(m_Model.GetBackgroundColor()));
	dResult.SetElement(FIELD_ENVIRONMENT, CAEONLuminous::AsDatum(m_Model.GetEnvironmentColor()));
	dResult.SetElement(FIELD_FPS, m_Model.GetFPS());
	dResult.SetElement(FIELD_FRAME_COUNT, m_Model.GetFrameCount());
	dResult.SetElement(FIELD_MODE, CLuminousScene3D::AsID(m_Model.GetMode()));
	dResult.SetElement(FIELD_SEQ, m_Model.GetSeq());
	CDatum dTextures(CDatum::typeArray);
	for (int i = 0; i < m_Model.GetTextureCount(); i++) dTextures.Append(RenderTexture(m_Model.GetTexture(i)));
	dResult.SetElement(FIELD_TEXTURES, dTextures);
	CDatum dMaterials(CDatum::typeArray);
	for (int i = 0; i < m_Model.GetMaterialCount(); i++) dMaterials.Append(RenderMaterial(m_Model.GetMaterial(i)));
	dResult.SetElement(FIELD_MATERIALS, dMaterials);
	CDatum dObjects(CDatum::typeArray);
	for (int i = 0; i < m_Model.GetObjCount(); i++) dObjects.Append(RenderObj(m_Model.GetObj(i)));
	dResult.SetElement(FIELD_OBJECTS, dObjects);
	return dResult;
	}
