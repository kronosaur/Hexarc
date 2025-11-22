//	LuminousScene3D.h
//
//	LuminousCore Classes
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

class CLuminousScene3D;

//	NOTE: These values should never be persisted. Use a string identifier when
//	persisting. [Which means it is OK to renumber.]
//
//	These values must match the table in ILuminousObj3D.cpp

enum class Obj3DProp
	{
	Unknown =						0,

	Visible =						1,		//	Boolean
	Opacity =						2,		//	0-1.0

	Pos =							3,		//	Object position (vector)
	Scale =							4,		//	Object scale 1.0 = no change (vector)
	Rot =							5,		//	3D Rotation (radians)
	RotCenter =						6,		//	Center of rotation (vector)

	Count =							7,
	};

enum class Obj3DPropType
	{
	Unknown,

	Bool,									//	Boolean value
	Color,									//	CLuminousColor
	Scalar,									//	A double precision scalar
	String,									//	ID or text
	Vector,									//	A 3D vector
	};

class IAnimator3D
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

		IAnimator3D (Obj3DProp iProp) : m_iProp(iProp) { }

		static TUniquePtr<IAnimator3D> CreateFromStream (IByteStream& Stream);

		virtual ~IAnimator3D () { }

		void AddKeyframeBool (const SKeyframeDesc& Desc, bool bValue) { m_Keyframes.Insert(Desc); AddBoolValue(bValue); }
		void AddKeyframeColor (const SKeyframeDesc& Desc, const CLuminousColor& Value) { m_Keyframes.Insert(Desc); AddColorValue(Value); }
		void AddKeyframeScalar (const SKeyframeDesc& Desc, double rValue) { m_Keyframes.Insert(Desc); AddScalarValue(rValue); }
		void AddKeyframeString (const SKeyframeDesc& Desc, const CString& sValue) { m_Keyframes.Insert(Desc); AddStringValue(sValue); }
		void AddKeyframeVector (const SKeyframeDesc& Desc, const CVector3D& vValue) { m_Keyframes.Insert(Desc); AddVectorValue(vValue); }
		static CString AsID (Type iType);
		static Type AsType (const CString& sID);
		virtual TUniquePtr<IAnimator3D> Clone () const = 0;
		int GetFrameCount () const { return GetKeyframeLast().iFrame; }
		int GetKeyframeCount () const { return m_Keyframes.GetCount(); }
		const TArray<SKeyframeDesc> &GetKeyframes () const { return m_Keyframes; }
		virtual const TArray<bool>& GetKeyframesBool () const { return m_NullBool; }
		virtual const TArray<CLuminousColor>& GetKeyframesColor () const { return m_NullColor; }
		SKeyframeDesc& GetKeyframeLast () { return (m_Keyframes.GetCount() > 0 ? m_Keyframes[m_Keyframes.GetCount() - 1] : m_NullKeyframe); }
		const SKeyframeDesc& GetKeyframeLast () const { return (m_Keyframes.GetCount() > 0 ? m_Keyframes[m_Keyframes.GetCount() - 1] : m_NullKeyframe); }
		virtual const TArray<double>& GetKeyframesScalar () const { return m_NullScalar; }
		virtual const TArray<CString>& GetKeyframesString () const { return m_NullString; }
		virtual const TArray<CVector3D>& GetKeyframesVector () const { return m_NullVector; }
		Obj3DProp GetProperty () const { return m_iProp; }
		virtual Obj3DPropType GetPropertyType () const = 0;
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
		virtual void AddVectorValue (const CVector3D& vValue) { }
		virtual void OnRead (IByteStream& Stream) { }
		virtual void OnWrite (IByteStream& Stream) const { }

		Obj3DProp m_iProp = Obj3DProp::Unknown;

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

		virtual void AddColorValue (const CLuminousColor& Color) override { m_Values.Insert(Color); }
		virtual const TArray<CLuminousColor>& GetKeyframesColor () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
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

		virtual void AddVectorValue (const CVector3D& vValue) override { m_Values.Insert(vValue); }
		virtual const TArray<CVector3D>& GetKeyframesVector () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
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

		ILuminousObj3D (CLuminousScene3D& Scene, DWORD dwID, ILuminousObj3D* pParent) : m_Scene(Scene), m_dwID(dwID), m_pParent(pParent) { }
		virtual ~ILuminousObj3D () { }

		static TUniquePtr<ILuminousObj3D> CreateFromStream (CLuminousScene3D& Scene, IByteStream& Stream, TSortMap<DWORD, DWORD> &retParents);

		bool AnimateBoolConstant (Obj3DProp iProp, int iFrame, bool bValue);
		bool AnimateColorConstant (Obj3DProp iProp, int iFrame, const CLuminousColor& Value);
		bool AnimateScalarConstant (Obj3DProp iProp, int iFrame, double rValue);
		bool AnimateScalarLinear (Obj3DProp iProp, int iFrame, double rValue);
		bool AnimateStringConstant (Obj3DProp iProp, int iFrame, const CString& sValue);
		bool AnimateVectorConstant (Obj3DProp iProp, int iFrame, const CVector3D& Value);
		bool AnimateVectorLinear (Obj3DProp iProp, int iFrame, const CVector3D& vValue);
		TUniquePtr<ILuminousObj3D> Clone () const { return OnClone(); }
		int GetFrameCount () const { return m_Animators.GetFrameCount(); }
		DWORD GetID () const { return m_dwID; }
		virtual DWORD GetImpl () const = 0;
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
		static Obj3DProp ParseProperty (const CString& sProperty);
		bool RemoveAnimation (Obj3DProp iProp) { return m_Animators.RemoveAnimation(iProp); }
		void SetParent (ILuminousObj3D* pParent) { m_pParent = pParent; }
		bool SetPropertyBool (Obj3DProp iProp, bool bValue);
		bool SetPropertyColor (Obj3DProp iProp, const CLuminousColor& Value);
		bool SetPropertyScalar (Obj3DProp iProp, double rValue);
		bool SetPropertyString (Obj3DProp iProp, const CString& sValue);
		bool SetPropertyVector (Obj3DProp iProp, const CVector3D& Value);
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void Write (IByteStream& Stream) const;

		static const SPropertyDesc& GetPropertyDesc (Obj3DProp iProp);

	protected:

		static constexpr DWORD IMPL_RECTANGLE = 0x00000001;

		static void AccumulatePropertyToRender (const SPropertyDesc& Desc, const IAnimator3D* pAnimator, TArray<SPropertyRenderCtx>& Result)
			{ Result.Insert({ Desc.iProp, Desc.iType, Desc.sID, pAnimator }); }

	private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const { }
		virtual TUniquePtr<ILuminousObj3D> OnClone () const = 0;
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
		virtual bool OnSetPropertyVector (Obj3DProp iProp, const CVector3D& Value) { return false; }
		virtual bool OnSetPropertyString (Obj3DProp iProp, const CString& sValue) { return false; }
		virtual void OnWrite (IByteStream& Stream) const { }

		CLuminousScene3D& m_Scene;
		ILuminousObj3D* m_pParent = NULL;
		DWORD m_dwID = 0;
		CVector3D m_vPos;					//	Position relative to parent origin
		CVector3D m_vScale = CVector3D(1.0, 1.0, 1.0);	//	Scale
		CVector3D m_vRotCenter;				//	Center of rotation relative to local origin
		double m_rRotation = 0.0;			//	Rotation (radians)
		double m_rOpacity = 1.0;			//	Opacity (0-1.0)
		bool m_bVisible = true;

		CAnimatorSet3D m_Animators;

		SequenceNumber m_Seq = 0;

		static TArray<SPropertyDesc> m_Properties;
		static TSortMap<CString, Obj3DProp> m_PropLookup;
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
			};

		static CLuminousScene3D CreateFromStream (IByteStream& Stream);

		void Play (int iStartFrame = 0);
		bool IsPlaying () const { return m_dwStartTime != 0; }
		void Stop ();

		static const CString& AsID (EMode iMode);
		static EMode AsMode (const CString& sValue);
		ILuminousObj3D* FindObj (DWORD dwID) { auto* pObj = m_Objs.GetAt(dwID); return (pObj ? (ILuminousObj3D*)(*pObj) : NULL); }
		const ILuminousObj3D* FindObj (DWORD dwID) const { return const_cast<CLuminousScene3D*>(this)->FindObj(dwID); }
		CLuminousColor GetBackgroundColor () const { return m_Background; }
		DWORDLONG GetCurTime () const { return (IsPlaying() ? ::sysGetTickCount64() : 0); }
		int GetFPS () const { return m_iFPS; }
		int GetFrameCount () const { return m_iFrameCount; }
		ILuminousObj3D& GetObj (int iIndex) { return *m_Objs[iIndex]; }
		const ILuminousObj3D& GetObj (int iIndex) const { return *m_Objs[iIndex]; }
		int GetObjCount () const { return m_Objs.GetCount(); }
		const CVector3D& GetOrigin () const { return m_vOrigin; }
		double GetHeight () const { return m_vExtent.Y(); }
		EMode GetMode () const { return m_iMode; }
		SequenceNumber GetSeq () const { return m_Seq; }
		int GetStartFrame () const { return m_iStartFrame; }
		DWORDLONG GetStartTime () const { return m_dwStartTime; }
		double GetWidth () const { return m_vExtent.X(); }
		SequenceNumber IncSeq () { return ++m_Seq; }
		void OnObjModified (ILuminousObj3D& Obj);
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

		void RecalcAnimation ();

		int m_iFPS = DEFAULT_FPS;
		int m_iFrameCount = -1;				//	-1 = infinite (otherwise, stop or repeat at this frame).
		CVector3D m_vExtent = CVector3D(DEFAULT_WIDTH, DEFAULT_HEIGHT, 0.0);
		CVector3D m_vOrigin = CVector3D(DEFAULT_WIDTH / 2.0, DEFAULT_HEIGHT / 2.0, 0.0);
		EMode m_iMode = EMode::Default;

		CLuminousColor m_Background = CLuminousColor();
		TSortMap<DWORD, TUniquePtr<ILuminousObj3D>> m_Objs;
		DWORD m_dwNextID = 1;
		SequenceNumber m_Seq = 1;

		DWORDLONG m_dwStartTime = 0;		//	Tick on which we started playing (0 = not playing)
		int m_iStartFrame = 0;				//	Frame on which we started playing
	};
