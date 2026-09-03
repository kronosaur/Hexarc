//	LuminousScene3D.h
//
//	LuminousCore Classes
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

class CLuminousScene3D;

class ILuminousTexture3D
	{
	public:
		ILuminousTexture3D (CLuminousScene3D& Scene, DWORD dwID) : m_pScene(&Scene), m_dwID(dwID) { }
		virtual ~ILuminousTexture3D () { }

		static TUniquePtr<ILuminousTexture3D> CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream);
		virtual bool GetFlipY () const { return true; }
		virtual const CString& GetFormat () const { return NULL_STR; }
		virtual const CString& GetGridID () const { return NULL_STR; }
		DWORD GetID () const { return m_dwID; }
		virtual DWORD GetImpl () const = 0;
		SequenceNumber GetSeq () const { return m_Seq; }
		const CString& GetTextureType () const { return OnGetTextureType(); }
		void SetScene (CLuminousScene3D& Scene) { m_pScene = &Scene; }
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void Write (IByteStream& Stream) const;

	protected:
		static constexpr DWORD IMPL_IMAGE = 0x00000001;

	private:
		virtual const CString& OnGetTextureType () const = 0;
		virtual void OnRead (IByteStream& Stream) { }
		virtual void OnWrite (IByteStream& Stream) const { }

		CLuminousScene3D* m_pScene = NULL;
		DWORD m_dwID = 0;
		SequenceNumber m_Seq = 0;
	};

class CImageTexture3D : public ILuminousTexture3D
	{
	public:
		CImageTexture3D (CLuminousScene3D& Scene, DWORD dwID, const CString& sGridID = NULL_STR, const CString& sFormat = NULL_STR, bool bFlipY = true) :
				ILuminousTexture3D(Scene, dwID),
				m_sGridID(sGridID),
				m_sFormat(sFormat),
				m_bFlipY(bFlipY)
			{ }

		virtual bool GetFlipY () const override { return m_bFlipY; }
		virtual const CString& GetFormat () const override { return m_sFormat; }
		virtual const CString& GetGridID () const override { return m_sGridID; }
		virtual DWORD GetImpl () const override { return IMPL_IMAGE; }

	private:
		virtual const CString& OnGetTextureType () const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		CString m_sGridID;
		CString m_sFormat;
		bool m_bFlipY = true;
	};

//	These values are persisted in Scene3D streams. Do not reorder them.

enum class Material3DAlphaMode
	{
	Opaque,
	Mask,
	Blend,
	Hash,
	};

//	These values are persisted positionally in Scene3D version 5. Append new
//	roles immediately before Count and bump the scene version.

enum class Material3DMap
	{
	Color,
	Opacity,
	Emissive,
	Normal,
	Bump,
	Displacement,
	Roughness,
	Metalness,
	AO,
	Light,
	Anisotropy,
	Clearcoat,
	ClearcoatNormal,
	ClearcoatRoughness,
	Iridescence,
	IridescenceThickness,
	SheenColor,
	SheenRoughness,
	SpecularColor,
	SpecularIntensity,
	Thickness,
	Transmission,
	Count,
	};

class ILuminousMaterial3D
	{
	public:
		ILuminousMaterial3D (CLuminousScene3D& Scene, DWORD dwID) : m_pScene(&Scene), m_dwID(dwID) { }
		virtual ~ILuminousMaterial3D () { }

		static TUniquePtr<ILuminousMaterial3D> CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream, DWORD dwSceneVersion);
		static const CString& AsID (Material3DAlphaMode iMode);
		static const CString& AsID (Material3DMap iMap);
		static Material3DAlphaMode AsAlphaMode (const CString& sValue);
		double GetAlphaCutoff () const { return m_rAlphaCutoff; }
		Material3DAlphaMode GetAlphaMode () const { return m_iAlphaMode; }
		double GetAnisotropy () const { return m_rAnisotropy; }
		double GetAnisotropyRotation () const { return m_rAnisotropyRotation; }
		CLuminousColor GetAttenuationColor () const { return m_AttenuationColor; }
		double GetAttenuationDistance () const { return m_rAttenuationDistance; }
		double GetAOMapIntensity () const { return m_rAOMapIntensity; }
		double GetBumpScale () const { return m_rBumpScale; }
		double GetClearcoat () const { return m_rClearcoat; }
		CVector2D GetClearcoatNormalScale () const { return m_vClearcoatNormalScale; }
		double GetClearcoatRoughness () const { return m_rClearcoatRoughness; }
		CLuminousColor GetColor () const { return m_Color; }
		double GetDispersion () const { return m_rDispersion; }
		double GetDisplacementBias () const { return m_rDisplacementBias; }
		double GetDisplacementScale () const { return m_rDisplacementScale; }
		bool GetDoubleSided () const { return m_bDoubleSided; }
		CLuminousColor GetEmissiveColor () const { return m_EmissiveColor; }
		double GetEmissiveIntensity () const { return m_rEmissiveIntensity; }
		DWORD GetID () const { return m_dwID; }
		virtual DWORD GetImpl () const = 0;
		double GetIOR () const { return m_rIOR; }
		double GetIridescence () const { return m_rIridescence; }
		double GetIridescenceIOR () const { return m_rIridescenceIOR; }
		CVector2D GetIridescenceThicknessRange () const { return m_vIridescenceThicknessRange; }
		double GetLightMapIntensity () const { return m_rLightMapIntensity; }
		const CString& GetMaterialType () const { return OnGetMaterialType(); }
		double GetMetalness () const { return m_rMetalness; }
		CVector2D GetNormalScale () const { return m_vNormalScale; }
		double GetOpacity () const { return m_rOpacity; }
		double GetRoughness () const { return m_rRoughness; }
		SequenceNumber GetSeq () const { return m_Seq; }
		double GetSheen () const { return m_rSheen; }
		CLuminousColor GetSheenColor () const { return m_SheenColor; }
		double GetSheenRoughness () const { return m_rSheenRoughness; }
		CLuminousColor GetSpecularColor () const { return m_SpecularColor; }
		double GetSpecularIntensity () const { return m_rSpecularIntensity; }
		double GetThickness () const { return m_rThickness; }
		DWORD GetTextureID (Material3DMap iMap) const { return m_TextureIDs[(int)iMap]; }
		double GetTransmission () const { return m_rTransmission; }
		bool SetAlphaCutoff (double rValue);
		bool SetAlphaMode (Material3DAlphaMode iMode);
		bool SetAnisotropy (double rValue);
		bool SetAnisotropyRotation (double rValue);
		void SetAttenuationColor (const CLuminousColor& Color);
		bool SetAttenuationDistance (double rValue);
		bool SetAOMapIntensity (double rValue);
		bool SetBumpScale (double rValue);
		bool SetClearcoat (double rValue);
		bool SetClearcoatNormalScale (const CVector2D& vValue);
		bool SetClearcoatRoughness (double rValue);
		void SetColor (const CLuminousColor& Color);
		bool SetDispersion (double rValue);
		bool SetDisplacementBias (double rValue);
		bool SetDisplacementScale (double rValue);
		void SetDoubleSided (bool bValue);
		void SetEmissiveColor (const CLuminousColor& Color);
		bool SetEmissiveIntensity (double rValue);
		bool SetIOR (double rValue);
		bool SetIridescence (double rValue);
		bool SetIridescenceIOR (double rValue);
		bool SetIridescenceThicknessRange (const CVector2D& vValue);
		bool SetLightMapIntensity (double rValue);
		bool SetMetalness (double rValue);
		bool SetNormalScale (const CVector2D& vValue);
		bool SetOpacity (double rOpacity);
		bool SetRoughness (double rValue);
		void SetScene (CLuminousScene3D& Scene) { m_pScene = &Scene; }
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		bool SetSheen (double rValue);
		void SetSheenColor (const CLuminousColor& Color);
		bool SetSheenRoughness (double rValue);
		void SetSpecularColor (const CLuminousColor& Color);
		bool SetSpecularIntensity (double rValue);
		bool SetThickness (double rValue);
		bool SetTextureID (Material3DMap iMap, DWORD dwTextureID);
		bool SetTransmission (double rValue);
		void Write (IByteStream& Stream) const;

	protected:
		static constexpr DWORD IMPL_PHYSICAL = 0x00000001;

	private:
		virtual const CString& OnGetMaterialType () const = 0;
		bool IsValid () const;
		void Modified ();

		CLuminousScene3D* m_pScene = NULL;
		DWORD m_dwID = 0;
		Material3DAlphaMode m_iAlphaMode = Material3DAlphaMode::Opaque;
		double m_rAlphaCutoff = 0.5;
		double m_rAnisotropy = 0.0;
		double m_rAnisotropyRotation = 0.0;
		CLuminousColor m_AttenuationColor = CLuminousColor(CRGBA32(0xff, 0xff, 0xff));
		double m_rAttenuationDistance = 0.0;
		double m_rAOMapIntensity = 1.0;
		double m_rBumpScale = 1.0;
		double m_rClearcoat = 0.0;
		CVector2D m_vClearcoatNormalScale = CVector2D(1.0, 1.0);
		double m_rClearcoatRoughness = 0.0;
		CLuminousColor m_Color = CLuminousColor(CRGBA32(0xff, 0xff, 0xff));
		double m_rDispersion = 0.0;
		double m_rDisplacementBias = 0.0;
		double m_rDisplacementScale = 1.0;
		bool m_bDoubleSided = false;
		CLuminousColor m_EmissiveColor = CLuminousColor(CRGBA32(0x00, 0x00, 0x00));
		double m_rEmissiveIntensity = 1.0;
		double m_rIOR = 1.5;
		double m_rIridescence = 0.0;
		double m_rIridescenceIOR = 1.3;
		CVector2D m_vIridescenceThicknessRange = CVector2D(100.0, 400.0);
		double m_rLightMapIntensity = 1.0;
		double m_rMetalness = 0.0;
		CVector2D m_vNormalScale = CVector2D(1.0, 1.0);
		double m_rOpacity = 1.0;
		double m_rRoughness = 1.0;
		SequenceNumber m_Seq = 0;
		double m_rSheen = 0.0;
		CLuminousColor m_SheenColor = CLuminousColor(CRGBA32(0x00, 0x00, 0x00));
		double m_rSheenRoughness = 1.0;
		CLuminousColor m_SpecularColor = CLuminousColor(CRGBA32(0xff, 0xff, 0xff));
		double m_rSpecularIntensity = 1.0;
		double m_rThickness = 0.0;
		DWORD m_TextureIDs[(int)Material3DMap::Count] = { };
		double m_rTransmission = 0.0;
	};

class CPhysicalMaterial3D : public ILuminousMaterial3D
	{
	public:
		CPhysicalMaterial3D (CLuminousScene3D& Scene, DWORD dwID) : ILuminousMaterial3D(Scene, dwID) { }
		virtual DWORD GetImpl () const override { return IMPL_PHYSICAL; }

	private:
		virtual const CString& OnGetMaterialType () const override;
	};

//	These values are not persisted. Property string IDs are persisted instead.

enum class Obj3DProp
	{
	Unknown = 0,
	Visible,
	Opacity,
	Pos,
	Scale,
	Rot,
	Color,
	Intensity,
	Distance,
	Decay,
	FOV,
	Near,
	Far,
	GridID,
	ResourcePath,
	Count,
	};

enum class Obj3DPropType
	{
	Unknown,
	Bool,
	Color,
	Scalar,
	String,
	Vector,
	};

class IAnimator3D
	{
	public:
		enum class Type
			{
			Unknown,
			Blink,
			Constant,
			Linear,
			};

		struct SKeyframeDesc
			{
			int iFrame = 0;
			Type iType = Type::Unknown;
			int iBlinkInterval = 0;
			};

		IAnimator3D (Obj3DProp iProp) : m_iProp(iProp) { }
		virtual ~IAnimator3D () { }

		static TUniquePtr<IAnimator3D> CreateFromStream (IByteStream& Stream);
		void AddKeyframeBool (const SKeyframeDesc& Desc, bool bValue) { m_Keyframes.Insert(Desc); AddBoolValue(bValue); }
		void AddKeyframeColor (const SKeyframeDesc& Desc, const CLuminousColor& Value) { m_Keyframes.Insert(Desc); AddColorValue(Value); }
		void AddKeyframeScalar (const SKeyframeDesc& Desc, double rValue) { m_Keyframes.Insert(Desc); AddScalarValue(rValue); }
		void AddKeyframeString (const SKeyframeDesc& Desc, const CString& sValue) { m_Keyframes.Insert(Desc); AddStringValue(sValue); }
		void AddKeyframeVector (const SKeyframeDesc& Desc, const CVector3D& Value) { m_Keyframes.Insert(Desc); AddVectorValue(Value); }
		static CString AsID (Type iType);
		static Type AsType (const CString& sID);
		virtual TUniquePtr<IAnimator3D> Clone () const = 0;
		int GetFrameCount () const { return GetKeyframeLast().iFrame; }
		const TArray<SKeyframeDesc>& GetKeyframes () const { return m_Keyframes; }
		virtual const TArray<bool>& GetKeyframesBool () const { return m_NullBool; }
		virtual const TArray<CLuminousColor>& GetKeyframesColor () const { return m_NullColor; }
		const SKeyframeDesc& GetKeyframeLast () const { return (m_Keyframes.GetCount() > 0 ? m_Keyframes[m_Keyframes.GetCount() - 1] : m_NullKeyframe); }
		virtual const TArray<double>& GetKeyframesScalar () const { return m_NullScalar; }
		virtual const TArray<CString>& GetKeyframesString () const { return m_NullString; }
		virtual const TArray<CVector3D>& GetKeyframesVector () const { return m_NullVector; }
		Obj3DProp GetProperty () const { return m_iProp; }
		virtual Obj3DPropType GetPropertyType () const = 0;
		void TrimBefore (int iFrame);
		void Write (IByteStream& Stream) const;

	private:
		static constexpr DWORD IMPL_BOOL = 0x00000001;
		static constexpr DWORD IMPL_COLOR = 0x00000002;
		static constexpr DWORD IMPL_SCALAR = 0x00000003;
		static constexpr DWORD IMPL_STRING = 0x00000004;
		static constexpr DWORD IMPL_VECTOR = 0x00000005;

		DWORD GetImplID () const;
		virtual void AddBoolValue (bool bValue) { }
		virtual void AddColorValue (const CLuminousColor& Value) { }
		virtual void AddScalarValue (double rValue) { }
		virtual void AddStringValue (const CString& sValue) { }
		virtual void AddVectorValue (const CVector3D& Value) { }
		virtual void OnRead (IByteStream& Stream) { }
		virtual void OnTrimValues (int iIndex, int iCount) { }
		virtual void OnWrite (IByteStream& Stream) const { }

		Obj3DProp m_iProp = Obj3DProp::Unknown;
		TArray<SKeyframeDesc> m_Keyframes;
		static TArray<bool> m_NullBool;
		static TArray<CLuminousColor> m_NullColor;
		static TArray<double> m_NullScalar;
		static TArray<CString> m_NullString;
		static TArray<CVector3D> m_NullVector;
		static SKeyframeDesc m_NullKeyframe;
	};

class CBoolAnimator3D : public IAnimator3D
	{
	public:
		CBoolAnimator3D (Obj3DProp iProp) : IAnimator3D(iProp) { }
		virtual TUniquePtr<IAnimator3D> Clone () const override { return TUniquePtr<IAnimator3D>(new CBoolAnimator3D(*this)); }
		virtual Obj3DPropType GetPropertyType () const override { return Obj3DPropType::Bool; }
	private:
		virtual void AddBoolValue (bool bValue) override { m_Values.Insert(bValue); }
		virtual const TArray<bool>& GetKeyframesBool () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;
		TArray<bool> m_Values;
	};

class CColorAnimator3D : public IAnimator3D
	{
	public:
		CColorAnimator3D (Obj3DProp iProp) : IAnimator3D(iProp) { }
		virtual TUniquePtr<IAnimator3D> Clone () const override { return TUniquePtr<IAnimator3D>(new CColorAnimator3D(*this)); }
		virtual Obj3DPropType GetPropertyType () const override { return Obj3DPropType::Color; }
	private:
		virtual void AddColorValue (const CLuminousColor& Value) override { m_Values.Insert(Value); }
		virtual const TArray<CLuminousColor>& GetKeyframesColor () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;
		TArray<CLuminousColor> m_Values;
	};

class CScalarAnimator3D : public IAnimator3D
	{
	public:
		CScalarAnimator3D (Obj3DProp iProp) : IAnimator3D(iProp) { }
		virtual TUniquePtr<IAnimator3D> Clone () const override { return TUniquePtr<IAnimator3D>(new CScalarAnimator3D(*this)); }
		virtual Obj3DPropType GetPropertyType () const override { return Obj3DPropType::Scalar; }
	private:
		virtual void AddScalarValue (double rValue) override { m_Values.Insert(rValue); }
		virtual const TArray<double>& GetKeyframesScalar () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;
		TArray<double> m_Values;
	};

class CStringAnimator3D : public IAnimator3D
	{
	public:
		CStringAnimator3D (Obj3DProp iProp) : IAnimator3D(iProp) { }
		virtual TUniquePtr<IAnimator3D> Clone () const override { return TUniquePtr<IAnimator3D>(new CStringAnimator3D(*this)); }
		virtual Obj3DPropType GetPropertyType () const override { return Obj3DPropType::String; }
	private:
		virtual void AddStringValue (const CString& sValue) override { m_Values.Insert(sValue); }
		virtual const TArray<CString>& GetKeyframesString () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;
		TArray<CString> m_Values;
	};

class CVectorAnimator3D : public IAnimator3D
	{
	public:
		CVectorAnimator3D (Obj3DProp iProp) : IAnimator3D(iProp) { }
		virtual TUniquePtr<IAnimator3D> Clone () const override { return TUniquePtr<IAnimator3D>(new CVectorAnimator3D(*this)); }
		virtual Obj3DPropType GetPropertyType () const override { return Obj3DPropType::Vector; }
	private:
		virtual void AddVectorValue (const CVector3D& Value) override { m_Values.Insert(Value); }
		virtual const TArray<CVector3D>& GetKeyframesVector () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;
		TArray<CVector3D> m_Values;
	};

class CAnimatorSet3D
	{
	public:
		CAnimatorSet3D () { }
		CAnimatorSet3D (const CAnimatorSet3D& Src) { Copy(Src); }
		CAnimatorSet3D (CAnimatorSet3D&& Src) noexcept = default;
		static CAnimatorSet3D CreateFromStream (IByteStream& Stream);
		CAnimatorSet3D& operator= (CAnimatorSet3D&& Src) noexcept = default;
		CAnimatorSet3D& operator= (const CAnimatorSet3D& Src) { m_Animators.DeleteAll(); Copy(Src); return *this; }
		IAnimator3D& GetAnimatorBool (Obj3DProp iProp, bool bInitialValue);
		IAnimator3D& GetAnimatorColor (Obj3DProp iProp, const CLuminousColor& InitialValue);
		IAnimator3D& GetAnimatorScalar (Obj3DProp iProp, double rInitialValue);
		IAnimator3D& GetAnimatorString (Obj3DProp iProp, const CString& sInitialValue);
		IAnimator3D& GetAnimatorVector (Obj3DProp iProp, const CVector3D& InitialValue);
		int GetFrameCount () const;
		const IAnimator3D* FindAnimator (Obj3DProp iProp) const { auto* pAnimator = m_Animators.GetAt(iProp); return (pAnimator ? (const IAnimator3D*)(*pAnimator) : NULL); }
		bool RemoveAnimation (Obj3DProp iProp);
		void TrimAllBefore (int iFrame);
		void Write (IByteStream& Stream) const;
	private:
		void Copy (const CAnimatorSet3D& Src);
		TSortMap<Obj3DProp, TUniquePtr<IAnimator3D>> m_Animators;
	};

class ILuminousObj3D
	{
	public:
		struct SPropertyDesc
			{
			Obj3DProp iProp = Obj3DProp::Unknown;
			Obj3DPropType iType = Obj3DPropType::Unknown;
			CString sID;
			};
		struct SPropertyRenderCtx
			{
			Obj3DProp iProp = Obj3DProp::Unknown;
			Obj3DPropType iType = Obj3DPropType::Unknown;
			CString sID;
			const IAnimator3D* pAnimator = NULL;
			};

		ILuminousObj3D (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent) : m_pScene(&Scene), m_dwID(dwID), m_pParent(pParent) { }
		virtual ~ILuminousObj3D () { }
		static TUniquePtr<ILuminousObj3D> CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream, TSortMap<DWORD, DWORD>& retParents);
		bool AnimateBoolConstant (Obj3DProp iProp, int iFrame, bool bValue);
		bool AnimateColorConstant (Obj3DProp iProp, int iFrame, const CLuminousColor& Value);
		bool AnimateScalarConstant (Obj3DProp iProp, int iFrame, double rValue);
		bool AnimateScalarLinear (Obj3DProp iProp, int iFrame, double rValue);
		bool AnimateStringConstant (Obj3DProp iProp, int iFrame, const CString& sValue);
		bool AnimateVectorConstant (Obj3DProp iProp, int iFrame, const CVector3D& Value);
		bool AnimateVectorLinear (Obj3DProp iProp, int iFrame, const CVector3D& Value);
		int GetFrameCount () const { return m_Animators.GetFrameCount(); }
		DWORD GetColorTextureID () const { return OnGetColorTextureID(); }
		DWORD GetID () const { return m_dwID; }
		virtual DWORD GetImpl () const = 0;
		DWORD GetMaterialID () const { return OnGetMaterialID(); }
		const CString& GetObjType () const { return OnGetObjType(); }
		const ILuminousObj3D* GetParent () const { return m_pParent; }
		TArray<SPropertyRenderCtx> GetPropertiesToRender () const;
		const IAnimator3D* GetPropertyAnimator (Obj3DProp iProp) const { return m_Animators.FindAnimator(iProp); }
		bool GetPropertyBool (Obj3DProp iProp) const;
		CLuminousColor GetPropertyColor (Obj3DProp iProp) const;
		double GetPropertyScalar (Obj3DProp iProp) const;
		CString GetPropertyString (Obj3DProp iProp) const;
		CVector3D GetPropertyVector (Obj3DProp iProp) const;
		SequenceNumber GetSeq () const { return m_Seq; }
		DWORD GetDirtyProps () const { return m_dwDirtyProps; }
		static const SPropertyDesc& GetPropertyDesc (Obj3DProp iProp);
		static Obj3DProp ParseProperty (const CString& sProperty);
		bool RemoveAnimation (Obj3DProp iProp) { return m_Animators.RemoveAnimation(iProp); }
		void SetParent (ILuminousObj3D* pParent) { m_pParent = pParent; }
		void SetScene (CLuminousScene3D& Scene) { m_pScene = &Scene; }
		bool SetPropertyBool (Obj3DProp iProp, bool bValue);
		bool SetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value);
		bool SetPropertyScalar (Obj3DProp iProp, double rValue);
		bool SetPropertyString (Obj3DProp iProp, const CString& sValue);
		bool SetPropertyVector (Obj3DProp iProp, const CVector3D& Value);
		bool SetColorTextureID (DWORD dwTextureID) { return OnSetColorTextureID(dwTextureID); }
		bool SetMaterialID (DWORD dwMaterialID) { return OnSetMaterialID(dwMaterialID); }
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void MarkPropertyDirty (Obj3DProp iProp) { m_dwDirtyProps |= (1 << (int)iProp); }
		void ClearDirtyProps () { m_dwDirtyProps = 0; }
		void TrimKeyframesBefore (int iFrame) { m_Animators.TrimAllBefore(iFrame); }
		void Write (IByteStream& Stream) const;

	protected:
		static constexpr DWORD IMPL_CUBE = 0x00000001;
		static constexpr DWORD IMPL_POINT_LIGHT = 0x00000002;
		static constexpr DWORD IMPL_PERSPECTIVE_CAMERA = 0x00000003;
		static constexpr DWORD IMPL_MODEL_3DS = 0x00000004;
		static constexpr DWORD IMPL_MODEL_GLTF = 0x00000005;
		static void AccumulatePropertyToRender (const SPropertyDesc& Desc, const IAnimator3D* pAnimator, TArray<SPropertyRenderCtx>& Result)
			{ Result.Insert({ Desc.iProp, Desc.iType, Desc.sID, pAnimator }); }

	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const { }
		virtual DWORD OnGetColorTextureID () const { return 0; }
		virtual DWORD OnGetMaterialID () const { return 0; }
		virtual const CString& OnGetObjType () const = 0;
		virtual bool OnGetPropertyBool (Obj3DProp iProp) const { return false; }
		virtual CLuminousColor OnGetPropertyColor (Obj3DProp iProp) const { return CLuminousColor(); }
		virtual double OnGetPropertyScalar (Obj3DProp iProp) const { return 0.0; }
		virtual CString OnGetPropertyString (Obj3DProp iProp) const { return NULL_STR; }
		virtual CVector3D OnGetPropertyVector (Obj3DProp iProp) const { return CVector3D(); }
		virtual void OnRead (IByteStream& Stream) { }
		virtual bool OnSetPropertyBool (Obj3DProp iProp, bool bValue) { return false; }
		virtual bool OnSetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value) { return false; }
		virtual bool OnSetPropertyScalar (Obj3DProp iProp, double rValue) { return false; }
		virtual bool OnSetPropertyString (Obj3DProp iProp, const CString& sValue) { return false; }
		virtual bool OnSetPropertyVector (Obj3DProp iProp, const CVector3D& Value) { return false; }
		virtual bool OnSetColorTextureID (DWORD dwTextureID) { return false; }
		virtual bool OnSetMaterialID (DWORD dwMaterialID) { return false; }
		virtual void OnWrite (IByteStream& Stream) const { }

		CLuminousScene3D* m_pScene = NULL;
		ILuminousObj3D* m_pParent = NULL;
		DWORD m_dwID = 0;
		CVector3D m_vPos;
		CVector3D m_vScale = CVector3D(1.0, 1.0, 1.0);
		CVector3D m_vRotation;
		double m_rOpacity = 1.0;
		bool m_bVisible = true;
		CAnimatorSet3D m_Animators;
		DWORD m_dwDirtyProps = 0;
		SequenceNumber m_Seq = 0;
		static TArray<SPropertyDesc> m_Properties;
		static TSortMap<CString, Obj3DProp> m_PropLookup;
	};

class CObj3DModel3DS : public ILuminousObj3D
	{
	public:
		CObj3DModel3DS (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent, const CString& sGridID = NULL_STR) : ILuminousObj3D(Scene, dwID, pParent), m_sGridID(sGridID) { }
		virtual DWORD GetImpl () const override { return IMPL_MODEL_3DS; }

	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual const CString& OnGetObjType () const override;
		virtual CString OnGetPropertyString (Obj3DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual bool OnSetPropertyString (Obj3DProp iProp, const CString& sValue) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		CString m_sGridID;
		CString m_sResourcePath;
	};

class CObj3DModelGLTF : public ILuminousObj3D
	{
	public:
		CObj3DModelGLTF (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent, const CString& sGridID = NULL_STR) : ILuminousObj3D(Scene, dwID, pParent), m_sGridID(sGridID) { }
		virtual DWORD GetImpl () const override { return IMPL_MODEL_GLTF; }

	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual const CString& OnGetObjType () const override;
		virtual CString OnGetPropertyString (Obj3DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual bool OnSetPropertyString (Obj3DProp iProp, const CString& sValue) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		CString m_sGridID;
	};

class CObj3DCube : public ILuminousObj3D
	{
	public:
		CObj3DCube (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent) : ILuminousObj3D(Scene, dwID, pParent) { }
		virtual DWORD GetImpl () const override { return IMPL_CUBE; }
	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual DWORD OnGetColorTextureID () const override { return m_dwColorTextureID; }
		virtual DWORD OnGetMaterialID () const override { return m_dwMaterialID; }
		virtual const CString& OnGetObjType () const override;
		virtual CLuminousColor OnGetPropertyColor (Obj3DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual bool OnSetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetColorTextureID (DWORD dwTextureID) override { m_dwColorTextureID = dwTextureID; return true; }
		virtual bool OnSetMaterialID (DWORD dwMaterialID) override { m_dwMaterialID = dwMaterialID; return true; }
		virtual void OnWrite (IByteStream& Stream) const override;
		CLuminousColor m_Color = CLuminousColor(CRGBA32(0x80, 0x80, 0x80));
		DWORD m_dwColorTextureID = 0;
		DWORD m_dwMaterialID = 0;
	};

class CObj3DPointLight : public ILuminousObj3D
	{
	public:
		CObj3DPointLight (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent) : ILuminousObj3D(Scene, dwID, pParent) { }
		virtual DWORD GetImpl () const override { return IMPL_POINT_LIGHT; }
	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual const CString& OnGetObjType () const override;
		virtual CLuminousColor OnGetPropertyColor (Obj3DProp iProp) const override;
		virtual double OnGetPropertyScalar (Obj3DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual bool OnSetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value) override;
		virtual bool OnSetPropertyScalar (Obj3DProp iProp, double rValue) override;
		virtual void OnWrite (IByteStream& Stream) const override;
		CLuminousColor m_Color = CLuminousColor(CRGBA32(0xff, 0xff, 0xff));
		double m_rIntensity = 5.0;
		double m_rDistance = 0.0;
		double m_rDecay = 2.0;
	};

class CObj3DPerspectiveCamera : public ILuminousObj3D
	{
	public:
		CObj3DPerspectiveCamera (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent) : ILuminousObj3D(Scene, dwID, pParent) { }
		virtual DWORD GetImpl () const override { return IMPL_PERSPECTIVE_CAMERA; }
	private:
		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const override;
		virtual const CString& OnGetObjType () const override;
		virtual double OnGetPropertyScalar (Obj3DProp iProp) const override;
		virtual void OnRead (IByteStream& Stream) override;
		virtual bool OnSetPropertyScalar (Obj3DProp iProp, double rValue) override;
		virtual void OnWrite (IByteStream& Stream) const override;
		double m_rFOV = 75.0;
		double m_rNear = 0.1;
		double m_rFar = 1000.0;
	};

class CLuminousScene3D
	{
	public:
		enum class EMode
			{
			Unknown,
			Default,
			Loop,
			Realtime,
			Stream,
			};

		CLuminousScene3D ();
		CLuminousScene3D (CLuminousScene3D&& Src) noexcept { Move(std::move(Src)); }
		CLuminousScene3D& operator= (CLuminousScene3D&& Src) noexcept { if (this != &Src) Move(std::move(Src)); return *this; }
		static CLuminousScene3D CreateFromStream (IByteStream& Stream);
		static const CString& AsID (EMode iMode);
		static EMode AsMode (const CString& sValue);
		void AdvanceFrame (int iCount = 1);
		void ClearKeyframe () { m_iKeyframeFrame = -1; m_iKeyframeType = IAnimator3D::Type::Unknown; }
		ILuminousObj3D& Create3DS (const CString& sGridID, DWORD dwParentID = 0);
		ILuminousObj3D& CreateCube (DWORD dwParentID = 0);
		ILuminousObj3D& CreateGLTF (const CString& sGridID, DWORD dwParentID = 0);
		ILuminousTexture3D& CreateImageTexture (const CString& sGridID, const CString& sFormat = NULL_STR, bool bFlipY = true);
		ILuminousMaterial3D& CreatePhysicalMaterial ();
		ILuminousObj3D& CreatePerspectiveCamera (DWORD dwParentID = 0);
		ILuminousObj3D& CreatePointLight (DWORD dwParentID = 0);
		ILuminousMaterial3D* FindMaterial (DWORD dwID) { auto* pMaterial = m_Materials.GetAt(dwID); return (pMaterial ? (ILuminousMaterial3D*)(*pMaterial) : NULL); }
		const ILuminousMaterial3D* FindMaterial (DWORD dwID) const { return const_cast<CLuminousScene3D*>(this)->FindMaterial(dwID); }
		ILuminousObj3D* FindObj (DWORD dwID) { auto* pObj = m_Objs.GetAt(dwID); return (pObj ? (ILuminousObj3D*)(*pObj) : NULL); }
		const ILuminousObj3D* FindObj (DWORD dwID) const { return const_cast<CLuminousScene3D*>(this)->FindObj(dwID); }
		ILuminousTexture3D* FindTexture (DWORD dwID) { auto* pTexture = m_Textures.GetAt(dwID); return (pTexture ? (ILuminousTexture3D*)(*pTexture) : NULL); }
		const ILuminousTexture3D* FindTexture (DWORD dwID) const { return const_cast<CLuminousScene3D*>(this)->FindTexture(dwID); }
		DWORD GetActiveCameraID () const { return m_dwActiveCameraID; }
		CLuminousColor GetBackgroundColor () const { return m_Background; }
		CLuminousColor GetEnvironmentColor () const { return m_Environment; }
		int GetFPS () const { return m_iFPS; }
		int GetFrameCount () const { return m_iFrameCount; }
		int GetKeyframeFrame () const { return m_iKeyframeFrame; }
		IAnimator3D::Type GetKeyframeType () const { return m_iKeyframeType; }
		ILuminousMaterial3D& GetMaterial (int iIndex) { return *m_Materials[iIndex]; }
		const ILuminousMaterial3D& GetMaterial (int iIndex) const { return *m_Materials[iIndex]; }
		int GetMaterialCount () const { return m_Materials.GetCount(); }
		EMode GetMode () const { return m_iMode; }
		ILuminousObj3D& GetObj (int iIndex) { return *m_Objs[iIndex]; }
		const ILuminousObj3D& GetObj (int iIndex) const { return *m_Objs[iIndex]; }
		int GetObjCount () const { return m_Objs.GetCount(); }
		ILuminousTexture3D& GetTexture (int iIndex) { return *m_Textures[iIndex]; }
		const ILuminousTexture3D& GetTexture (int iIndex) const { return *m_Textures[iIndex]; }
		int GetTextureCount () const { return m_Textures.GetCount(); }
		SequenceNumber GetSeq () const { return m_Seq; }
		int GetStreamFrame () const { return m_iStreamFrame; }
		SequenceNumber IncSeq () { return ++m_Seq; }
		bool IsKeyframeMode () const { return m_iKeyframeFrame >= 0; }
		bool IsStreamMode () const { return m_iMode == EMode::Stream; }
		void OnMaterialModified (ILuminousMaterial3D& Material);
		void OnObjModified (ILuminousObj3D& Obj);
		bool RemoveMaterial (DWORD dwID);
		bool RemoveObj (DWORD dwID);
		void SetActiveCameraID (DWORD dwID);
		void SetBackgroundColor (const CLuminousColor& Color) { m_Background = Color; IncSeq(); }
		void SetEnvironmentColor (const CLuminousColor& Color) { m_Environment = Color; IncSeq(); }
		void SetFPS (int iFPS);
		void SetKeyframe (int iFrame, IAnimator3D::Type iType) { m_iKeyframeFrame = iFrame; m_iKeyframeType = iType; }
		void SetMode (EMode iMode);
		bool SetObjColorTexture (DWORD dwObjID, DWORD dwTextureID);
		bool SetObjMaterial (DWORD dwObjID, DWORD dwMaterialID);
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void TrimKeyframes (int iFrame);
		void Write (IByteStream& Stream) const;

	private:
		static constexpr DWORD SERIALIZED_VERSION = 5;
		static constexpr DWORD SERIALIZED_VERSION_ENVIRONMENT = 2;
		static constexpr DWORD SERIALIZED_VERSION_MATERIALS = 3;
		static constexpr DWORD SERIALIZED_VERSION_TEXTURES = 4;
		static constexpr int DEFAULT_FPS = 30;
		void Move (CLuminousScene3D&& Src) noexcept;
		void RecalcAnimation ();
		int m_iFPS = DEFAULT_FPS;
		int m_iFrameCount = -1;
		EMode m_iMode = EMode::Default;
		CLuminousColor m_Background;
		CLuminousColor m_Environment;
		TSortMap<DWORD, TUniquePtr<ILuminousMaterial3D>> m_Materials;
		TSortMap<DWORD, TUniquePtr<ILuminousObj3D>> m_Objs;
		TSortMap<DWORD, TUniquePtr<ILuminousTexture3D>> m_Textures;
		DWORD m_dwNextMaterialID = 1;
		DWORD m_dwNextID = 1;
		DWORD m_dwNextTextureID = 1;
		DWORD m_dwActiveCameraID = 0;
		SequenceNumber m_Seq = 1;
		int m_iKeyframeFrame = -1;
		IAnimator3D::Type m_iKeyframeType = IAnimator3D::Type::Unknown;
		int m_iStreamFrame = 0;
	};
