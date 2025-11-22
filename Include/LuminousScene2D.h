//	LuminousScene2D.h
//
//	LuminousCore Classes
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

class CLuminousScene2D;

//	NOTE: These values should never be persisted. Use a string identifier when
//	persisting. [Which means it is OK to renumber.]
//
//	These values must match the table in ILuminousObj2D.cpp

enum class Obj2DProp
	{
	Unknown =						0,

	Visible =						1,		//	Boolean
	Opacity =						2,		//	0-1.0

	Pos =							3,		//	Object position (vector)
	Scale =							4,		//	Object scale 1.0 = no change (vector)
	Rot =							5,		//	2D Rotation (radians)
	RotCenter =						6,		//	Center of rotation (vector)

	Height =						7,		//	E.g., height of a rectangle
	Radius =						8,		//	E.g., radius of a circle
	Width =							9,		//	E.g., width of a rectangle

	CornerRadius =					10,
	CornerRadiusBottomLeft =		11,
	CornerRadiusBottomRight =		12,
	CornerRadiusTopLeft =			13,
	CornerRadiusTopRight =			14,
	FillColor =						15,
	LineColor =						16,
	LineWidth =						17,

	Count =							18,
	};

enum class ObjPropType
	{
	Unknown,

	Bool,									//	Boolean value
	Color,									//	CLuminousColor
	Scalar,									//	A double precision scalar
	String,									//	ID or text
	Vector,									//	A 2D vector
	};

class IAnimator2D
	{
	public:

		enum class Type
			{
			Unknown,

			Blink,									//	Blink on and off
			Constant,								//	Constant value
			Linear,									//	Linear interpolation
			};

		struct SKeyframeDesc
			{
			int iFrame = 0;
			Type iType = Type::Unknown;

			//	ObjAnimateType::Blink
			int iBlinkInterval = 0;
			};

		IAnimator2D (Obj2DProp iProp) : m_iProp(iProp) { }

		static TUniquePtr<IAnimator2D> CreateFromStream (IByteStream& Stream);

		virtual ~IAnimator2D () { }

		void AddKeyframeBool (const SKeyframeDesc& Desc, bool bValue) { m_Keyframes.Insert(Desc); AddBoolValue(bValue); }
		void AddKeyframeColor (const SKeyframeDesc& Desc, const CLuminousColor& Value) { m_Keyframes.Insert(Desc); AddColorValue(Value); }
		void AddKeyframeScalar (const SKeyframeDesc& Desc, double rValue) { m_Keyframes.Insert(Desc); AddScalarValue(rValue); }
		void AddKeyframeString (const SKeyframeDesc& Desc, const CString& sValue) { m_Keyframes.Insert(Desc); AddStringValue(sValue); }
		void AddKeyframeVector (const SKeyframeDesc& Desc, const CVector2D& vValue) { m_Keyframes.Insert(Desc); AddVectorValue(vValue); }
		static CString AsID (Type iType);
		static Type AsType (const CString& sID);
		virtual TUniquePtr<IAnimator2D> Clone () const = 0;
		int GetFrameCount () const { return GetKeyframeLast().iFrame; }
		int GetKeyframeCount () const { return m_Keyframes.GetCount(); }
		const TArray<SKeyframeDesc> &GetKeyframes () const { return m_Keyframes; }
		virtual const TArray<bool>& GetKeyframesBool () const { return m_NullBool; }
		virtual const TArray<CLuminousColor>& GetKeyframesColor () const { return m_NullColor; }
		SKeyframeDesc& GetKeyframeLast () { return (m_Keyframes.GetCount() > 0 ? m_Keyframes[m_Keyframes.GetCount() - 1] : m_NullKeyframe); }
		const SKeyframeDesc& GetKeyframeLast () const { return (m_Keyframes.GetCount() > 0 ? m_Keyframes[m_Keyframes.GetCount() - 1] : m_NullKeyframe); }
		virtual const TArray<double>& GetKeyframesScalar () const { return m_NullScalar; }
		virtual const TArray<CString>& GetKeyframesString () const { return m_NullString; }
		virtual const TArray<CVector2D>& GetKeyframesVector () const { return m_NullVector; }
		Obj2DProp GetProperty () const { return m_iProp; }
		virtual ObjPropType GetPropertyType () const = 0;
		void Write (IByteStream& Stream) const;

	private:

		static constexpr DWORD IMPL_BOOL = 0x00000001;
		static constexpr DWORD IMPL_COLOR = 0x00000002;
		static constexpr DWORD IMPL_SCALAR = 0x00000003;
		static constexpr DWORD IMPL_STRING = 0x00000004;
		static constexpr DWORD IMPL_VECTOR = 0x00000005;

		DWORD GetImplID () const;

		virtual void AddBoolValue (bool bValue) { }
		virtual void AddColorValue (const CLuminousColor& Color) { }
		virtual void AddScalarValue (double rValue) { }
		virtual void AddStringValue (const CString& sValue) { }
		virtual void AddVectorValue (const CVector2D& vValue) { }
		virtual void OnRead (IByteStream& Stream) { }
		virtual void OnWrite (IByteStream& Stream) const { }

		Obj2DProp m_iProp = Obj2DProp::Unknown;

		//	Each keyframe describes the animation of the property from the end
		//	of the last frame to the end of this frame (which may never end).
		//
		//	For some animation types, like Linear, we need a previous frame so
		//	that we know the initial value. In that case, we must guarantee a
		//	0-sized constant keyframe at the start.

		TArray<SKeyframeDesc> m_Keyframes;

		static TArray<bool> m_NullBool;
		static TArray<CLuminousColor> m_NullColor;
		static TArray<double> m_NullScalar;
		static TArray<CString> m_NullString;
		static TArray<CVector2D> m_NullVector;
		static SKeyframeDesc m_NullKeyframe;
	};

class CBoolAnimator2D : public IAnimator2D
	{
	public:

		CBoolAnimator2D (Obj2DProp iProp) : IAnimator2D(iProp) { }

		virtual TUniquePtr<IAnimator2D> Clone () const override { return TUniquePtr<IAnimator2D>(new CBoolAnimator2D(*this)); }
		virtual ObjPropType GetPropertyType () const override { return ObjPropType::Bool; }

	private:

		virtual void AddBoolValue (bool bValue) override { m_Values.Insert(bValue); }
		virtual const TArray<bool>& GetKeyframesBool () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<bool> m_Values;
	};

class CColorAnimator2D : public IAnimator2D
	{
	public:

		CColorAnimator2D (Obj2DProp iProp) : IAnimator2D(iProp) { }

		virtual TUniquePtr<IAnimator2D> Clone () const override { return TUniquePtr<IAnimator2D>(new CColorAnimator2D(*this)); }
		virtual ObjPropType GetPropertyType () const override { return ObjPropType::Color; }

	private:

		virtual void AddColorValue (const CLuminousColor& Color) override { m_Values.Insert(Color); }
		virtual const TArray<CLuminousColor>& GetKeyframesColor () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<CLuminousColor> m_Values;
	};

class CScalarAnimator2D : public IAnimator2D
	{
	public:
	
		CScalarAnimator2D (Obj2DProp iProp) : IAnimator2D(iProp) { }
	
		virtual TUniquePtr<IAnimator2D> Clone () const override { return TUniquePtr<IAnimator2D>(new CScalarAnimator2D(*this)); }
		virtual ObjPropType GetPropertyType () const override { return ObjPropType::Scalar; }
	
	private:
	
		virtual void AddScalarValue (double rValue) override { m_Values.Insert(rValue); }
		virtual const TArray<double>& GetKeyframesScalar () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<double> m_Values;
	};

class CStringAnimator2D : public IAnimator2D
	{
	public:
	
		CStringAnimator2D (Obj2DProp iProp) : IAnimator2D(iProp) { }
		
		virtual TUniquePtr<IAnimator2D> Clone () const override { return TUniquePtr<IAnimator2D>(new CStringAnimator2D(*this)); }
		virtual ObjPropType GetPropertyType () const override { return ObjPropType::String; }
	
	private:

		virtual void AddStringValue (const CString& sValue) override { m_Values.Insert(sValue); }
		virtual const TArray<CString>& GetKeyframesString () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<CString> m_Values;
	};

class CVectorAnimator2D : public IAnimator2D
	{
	public:
	
		CVectorAnimator2D (Obj2DProp iProp) : IAnimator2D(iProp) { }
	
		virtual TUniquePtr<IAnimator2D> Clone () const override { return TUniquePtr<IAnimator2D>(new CVectorAnimator2D(*this)); }
		virtual ObjPropType GetPropertyType () const override { return ObjPropType::Vector; }
	
	private:

		virtual void AddVectorValue (const CVector2D& vValue) override { m_Values.Insert(vValue); }
		virtual const TArray<CVector2D>& GetKeyframesVector () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<CVector2D> m_Values;
	};

class CAnimatorSet2D
	{
	public:

		CAnimatorSet2D () { }
		CAnimatorSet2D (const CAnimatorSet2D& Src) { Copy(Src); }

		CAnimatorSet2D (CAnimatorSet2D&& Src) noexcept = default;

		static CAnimatorSet2D CreateFromStream (IByteStream& Stream);

		CAnimatorSet2D& operator= (CAnimatorSet2D&& Src) noexcept = default;
		CAnimatorSet2D& operator= (const CAnimatorSet2D& Src) { m_Animators.DeleteAll(); Copy(Src); return *this; }

		IAnimator2D& GetAnimatorBool (Obj2DProp iProp, bool bInitialValue);
		IAnimator2D& GetAnimatorColor (Obj2DProp iProp, const CLuminousColor& InitialValue);
		IAnimator2D& GetAnimatorScalar (Obj2DProp iProp, double rInitialValue);
		IAnimator2D& GetAnimatorString (Obj2DProp iProp, const CString& sInitialValue);
		IAnimator2D& GetAnimatorVector (Obj2DProp iProp, const CVector2D& InitialValue);
		int GetFrameCount () const;
		const IAnimator2D* FindAnimator (Obj2DProp iProp) const { auto* pAnimator = m_Animators.GetAt(iProp); return (pAnimator ? (const IAnimator2D*)(*pAnimator) : NULL); }
		bool RemoveAnimation (Obj2DProp iProp);
		void Write (IByteStream& Stream) const;

	private:

		void Copy (const CAnimatorSet2D& Src);

		TSortMap<Obj2DProp, TUniquePtr<IAnimator2D>> m_Animators;
	};

class ILuminousObj2D
	{
	public:

		struct SPropertyDesc
			{
			Obj2DProp iProp = Obj2DProp::Unknown;
			ObjPropType iType = ObjPropType::Unknown;
			CString sID;
			};

		struct SPropertyRenderCtx
			{
			Obj2DProp iProp = Obj2DProp::Unknown;
			ObjPropType iType = ObjPropType::Unknown;
			CString sID;

			const IAnimator2D* pAnimator = NULL;
			};

		ILuminousObj2D (CLuminousScene2D& Scene, DWORD dwID, ILuminousObj2D* pParent) : m_Scene(Scene), m_dwID(dwID), m_pParent(pParent) { }
		virtual ~ILuminousObj2D () { }

		static TUniquePtr<ILuminousObj2D> CreateFromStream (CLuminousScene2D& Scene, IByteStream& Stream, TSortMap<DWORD, DWORD> &retParents);

		bool AnimateBoolConstant (Obj2DProp iProp, int iFrame, bool bValue);
		bool AnimateColorConstant (Obj2DProp iProp, int iFrame, const CLuminousColor& Value);
		bool AnimateScalarConstant (Obj2DProp iProp, int iFrame, double rValue);
		bool AnimateScalarLinear (Obj2DProp iProp, int iFrame, double rValue);
		bool AnimateStringConstant (Obj2DProp iProp, int iFrame, const CString& sValue);
		bool AnimateVectorConstant (Obj2DProp iProp, int iFrame, const CVector2D& Value);
		bool AnimateVectorLinear (Obj2DProp iProp, int iFrame, const CVector2D& vValue);
		TUniquePtr<ILuminousObj2D> Clone () const { return OnClone(); }
		int GetFrameCount () const { return m_Animators.GetFrameCount(); }
		DWORD GetID () const { return m_dwID; }
		virtual DWORD GetImpl () const = 0;
		const CString& GetObjType () const { return OnGetObjType(); }
		const ILuminousObj2D* GetParent () const { return m_pParent; }
		TArray<SPropertyRenderCtx> GetPropertiesToRender () const;
		const IAnimator2D* GetPropertyAnimator (Obj2DProp iProp) const { return m_Animators.FindAnimator(iProp); }
		bool GetPropertyBool (Obj2DProp iProp) const;
		CLuminousColor GetPropertyColor (Obj2DProp iProp) const;
		double GetPropertyScalar (Obj2DProp iProp) const;
		CString GetPropertyString (Obj2DProp iProp) const;
		CVector2D GetPropertyVector (Obj2DProp iProp) const;
		SequenceNumber GetSeq () const { return m_Seq; }
		static Obj2DProp ParseProperty (const CString& sProperty);
		bool RemoveAnimation (Obj2DProp iProp) { return m_Animators.RemoveAnimation(iProp); }
		void SetParent (ILuminousObj2D* pParent) { m_pParent = pParent; }
		bool SetPropertyBool (Obj2DProp iProp, bool bValue);
		bool SetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value);
		bool SetPropertyScalar (Obj2DProp iProp, double rValue);
		bool SetPropertyString (Obj2DProp iProp, const CString& sValue);
		bool SetPropertyVector (Obj2DProp iProp, const CVector2D& Value);
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void Write (IByteStream& Stream) const;

		static const SPropertyDesc& GetPropertyDesc (Obj2DProp iProp);

	protected:

		static constexpr DWORD IMPL_RECTANGLE = 0x00000001;

		static void AccumulatePropertyToRender (const SPropertyDesc& Desc, const IAnimator2D* pAnimator, TArray<SPropertyRenderCtx>& Result)
			{ Result.Insert({ Desc.iProp, Desc.iType, Desc.sID, pAnimator }); }

	private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const { }
		virtual TUniquePtr<ILuminousObj2D> OnClone () const = 0;
		virtual const CString& OnGetObjType () const = 0;
		virtual bool OnGetPropertyBool (Obj2DProp iProp) const { return false; }
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const { return CLuminousColor(); }
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const { return 0.0; }
		virtual CString OnGetPropertyString (Obj2DProp iProp) const { return NULL_STR; }
		virtual CVector2D OnGetPropertyVector (Obj2DProp iProp) const { return CVector2D(); }
		virtual void OnRead (IByteStream& Stream) { }
		virtual bool OnSetPropertyBool (Obj2DProp iProp, bool bValue) { return false; }
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) { return false; }
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) { return false; }
		virtual bool OnSetPropertyVector (Obj2DProp iProp, const CVector2D& Value) { return false; }
		virtual bool OnSetPropertyString (Obj2DProp iProp, const CString& sValue) { return false; }
		virtual void OnWrite (IByteStream& Stream) const { }

		CLuminousScene2D& m_Scene;
		ILuminousObj2D* m_pParent = NULL;
		DWORD m_dwID = 0;
		CVector2D m_vPos;					//	Position relative to parent origin
		CVector2D m_vScale = CVector2D(1.0, 1.0);	//	Scale
		CVector2D m_vRotCenter;				//	Center of rotation relative to local origin
		double m_rRotation = 0.0;			//	Rotation (radians)
		double m_rOpacity = 1.0;			//	Opacity (0-1.0)
		bool m_bVisible = true;

		CAnimatorSet2D m_Animators;

		SequenceNumber m_Seq = 0;

		static TArray<SPropertyDesc> m_Properties;
		static TSortMap<CString, Obj2DProp> m_PropLookup;
	};

class CLuminousScene2D
	{
	public:

		enum class EMode
			{
			Unknown,

			Default,
			Loop,
			Realtime,
			};

		static CLuminousScene2D CreateFromStream (IByteStream& Stream);

		CLuminousScene2D () { }
		CLuminousScene2D (const CLuminousScene2D& Src) { Copy(Src); }
		CLuminousScene2D (CLuminousScene2D&& Src) noexcept = default;

		CLuminousScene2D& operator= (const CLuminousScene2D& Src) { CleanUp(); Copy(Src); return *this; }
		CLuminousScene2D& operator= (CLuminousScene2D&& Src) noexcept = default;

		void Play (int iStartFrame = 0);
		bool IsPlaying () const { return m_dwStartTime != 0; }
		void Stop ();

		static const CString& AsID (EMode iMode);
		static EMode AsMode (const CString& sValue);
		ILuminousObj2D& CreateRectangle (DWORD dwParentID);
		ILuminousObj2D* FindObj (DWORD dwID) { auto* pObj = m_Objs.GetAt(dwID); return (pObj ? (ILuminousObj2D*)(*pObj) : NULL); }
		const ILuminousObj2D* FindObj (DWORD dwID) const { return const_cast<CLuminousScene2D*>(this)->FindObj(dwID); }
		CLuminousColor GetBackgroundColor () const { return m_Background; }
		DWORDLONG GetCurTime () const { return (IsPlaying() ? ::sysGetTickCount64() : 0); }
		int GetFPS () const { return m_iFPS; }
		int GetFrameCount () const { return m_iFrameCount; }
		ILuminousObj2D& GetObj (int iIndex) { return *m_Objs[iIndex]; }
		const ILuminousObj2D& GetObj (int iIndex) const { return *m_Objs[iIndex]; }
		int GetObjCount () const { return m_Objs.GetCount(); }
		const CVector2D& GetOrigin () const { return m_vOrigin; }
		double GetHeight () const { return m_vExtent.Y(); }
		EMode GetMode () const { return m_iMode; }
		SequenceNumber GetSeq () const { return m_Seq; }
		int GetStartFrame () const { return m_iStartFrame; }
		DWORDLONG GetStartTime () const { return m_dwStartTime; }
		double GetWidth () const { return m_vExtent.X(); }
		SequenceNumber IncSeq () { return ++m_Seq; }
		void OnObjModified (ILuminousObj2D& Obj);
		void SetBackgroundColor (const CLuminousColor& Color) { m_Background = Color; IncSeq(); }
		void SetMode (EMode iMode);
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void SetStartFrame (int iFrame) { m_iStartFrame = iFrame; IncSeq(); }
		void Write (IByteStream& Stream) const;

	private:

		static constexpr DWORD SERIALIZED_VERSION = 1;

		static constexpr int DEFAULT_FPS = 60;
		static constexpr double DEFAULT_WIDTH = 1620;
		static constexpr double DEFAULT_HEIGHT = 1002;

		void CleanUp () { m_Objs.DeleteAll(); }
		void Copy (const CLuminousScene2D& Src);
		void RecalcAnimation ();

		int m_iFPS = DEFAULT_FPS;
		int m_iFrameCount = -1;				//	-1 = infinite (otherwise, stop or repeat at this frame).
		CVector2D m_vExtent = CVector2D(DEFAULT_WIDTH, DEFAULT_HEIGHT);
		CVector2D m_vOrigin = CVector2D(DEFAULT_WIDTH / 2.0, DEFAULT_HEIGHT / 2.0);
		EMode m_iMode = EMode::Default;

		CLuminousColor m_Background = CLuminousColor();
		TSortMap<DWORD, TUniquePtr<ILuminousObj2D>> m_Objs;
		DWORD m_dwNextID = 1;
		SequenceNumber m_Seq = 1;

		DWORDLONG m_dwStartTime = 0;		//	Tick on which we started playing (0 = not playing)
		int m_iStartFrame = 0;				//	Frame on which we started playing
	};
