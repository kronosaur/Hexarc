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
	MaxPoints =					10,		//	Trail size limit

	CornerRadius =				11,
	CornerRadiusBottomLeft =	12,
	CornerRadiusBottomRight =	13,
	CornerRadiusTopLeft =		14,
	CornerRadiusTopRight =		15,
	FillColor =				16,
	LineColor =				17,
	LineWidth =				18,
	Points =					19,		//	Trail points (array of CVector2D)
	LinePoints =			20,		//	Line points (array of CVector2D)
	PointCount =				21,		//	Polyline size

	Count =					22,
	};

enum class ObjPropType
	{
	Unknown,

	Bool,								//	Boolean value
	Color,							//	CLuminousColor
	Scalar,						//	A double precision scalar
	String,						//	ID or text
	Vector,						//	A 2D vector
	VectorQueue,				//	A queue of 2D vectors
	VectorList,					//	A fixed list of 2D vectors
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
		void AddKeyframeVectorQueue (const SKeyframeDesc& Desc, const TArray<CVector2D>& Value) { m_Keyframes.Insert(Desc); AddVectorQueueValue(Value); }
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
		virtual const TArray<TArray<CVector2D>>& GetKeyframesVectorQueue () const { return m_NullVectorQueue; }
		Obj2DProp GetProperty () const { return m_iProp; }
		virtual ObjPropType GetPropertyType () const = 0;
		void TrimBefore (int iFrame);
		void Write (IByteStream& Stream) const;

	private:

		static constexpr DWORD IMPL_BOOL = 0x00000001;
		static constexpr DWORD IMPL_COLOR = 0x00000002;
		static constexpr DWORD IMPL_SCALAR = 0x00000003;
		static constexpr DWORD IMPL_STRING = 0x00000004;
		static constexpr DWORD IMPL_VECTOR = 0x00000005;
		static constexpr DWORD IMPL_VECTOR_QUEUE = 0x00000006;

		DWORD GetImplID () const;

		virtual void AddBoolValue (bool bValue) { }
		virtual void AddColorValue (const CLuminousColor& Color) { }
		virtual void AddScalarValue (double rValue) { }
		virtual void AddStringValue (const CString& sValue) { }
		virtual void AddVectorValue (const CVector2D& vValue) { }
		virtual void AddVectorQueueValue (const TArray<CVector2D>& Value) { }
		virtual void OnRead (IByteStream& Stream) { }
		virtual void OnTrimValues (int iIndex, int iCount) { }
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
		static TArray<TArray<CVector2D>> m_NullVectorQueue;
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
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
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
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
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
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
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
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
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
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<CVector2D> m_Values;
	};

class CVectorQueueAnimator2D : public IAnimator2D
	{
	public:

		CVectorQueueAnimator2D (Obj2DProp iProp) : IAnimator2D(iProp) { }

		virtual TUniquePtr<IAnimator2D> Clone () const override { return TUniquePtr<IAnimator2D>(new CVectorQueueAnimator2D(*this)); }
		virtual ObjPropType GetPropertyType () const override;

	private:

		virtual void AddVectorQueueValue (const TArray<CVector2D>& Value) override { m_Values.Insert(Value); }
		virtual const TArray<TArray<CVector2D>>& GetKeyframesVectorQueue () const override { return m_Values; }
		virtual void OnRead (IByteStream& Stream) override;
		virtual void OnTrimValues (int iIndex, int iCount) override { m_Values.Delete(iIndex, iCount); }
		virtual void OnWrite (IByteStream& Stream) const override;

		TArray<TArray<CVector2D>> m_Values;
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
		IAnimator2D& GetAnimatorVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& InitialValue);
		int GetFrameCount () const;
		const IAnimator2D* FindAnimator (Obj2DProp iProp) const { auto* pAnimator = m_Animators.GetAt(iProp); return (pAnimator ? (const IAnimator2D*)(*pAnimator) : NULL); }
		bool RemoveAnimation (Obj2DProp iProp);
		void TrimAllBefore (int iFrame);
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
		bool AnimateVectorQueueConstant (Obj2DProp iProp, int iFrame, const TArray<CVector2D>& Value);
		bool AnimateVectorQueueLinear (Obj2DProp iProp, int iFrame, const TArray<CVector2D>& Value);
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
		const TArray<CVector2D>& GetPropertyVectorQueue (Obj2DProp iProp) const;
		SequenceNumber GetSeq () const { return m_Seq; }
		static Obj2DProp ParseProperty (const CString& sProperty);
		bool RemoveAnimation (Obj2DProp iProp) { return m_Animators.RemoveAnimation(iProp); }
		void SetParent (ILuminousObj2D* pParent) { m_pParent = pParent; }
		void TrimKeyframesBefore (int iFrame) { m_Animators.TrimAllBefore(iFrame); }
		bool SetPropertyBool (Obj2DProp iProp, bool bValue);
		bool SetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value);
		bool SetPropertyScalar (Obj2DProp iProp, double rValue);
		bool SetPropertyString (Obj2DProp iProp, const CString& sValue);
		bool SetPropertyVector (Obj2DProp iProp, const CVector2D& Value);
		bool SetPropertyVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& Value);
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void Write (IByteStream& Stream) const;

		void MarkPropertyDirty (Obj2DProp iProp) { m_dwDirtyProps |= (1 << (int)iProp); }
		DWORD GetDirtyProps () const { return m_dwDirtyProps; }
		void ClearDirtyProps () { m_dwDirtyProps = 0; }

		static const SPropertyDesc& GetPropertyDesc (Obj2DProp iProp);

	protected:

		static constexpr DWORD IMPL_RECTANGLE = 0x00000001;
		static constexpr DWORD IMPL_CIRCLE = 0x00000002;
		static constexpr DWORD IMPL_TRAIL = 0x00000003;
		static constexpr DWORD IMPL_LINE = 0x00000004;

		static void AccumulatePropertyToRender (const SPropertyDesc& Desc, const IAnimator2D* pAnimator, TArray<SPropertyRenderCtx>& Result)
			{ Result.Insert({ Desc.iProp, Desc.iType, Desc.sID, pAnimator }); }

	
		CLuminousScene2D& GetScene () { return m_Scene; }
		const CLuminousScene2D& GetScene () const { return m_Scene; }
		static const TArray<CVector2D>& GetNullVectorQueueValue () { return m_NullVectorQueueValue; }

private:

		virtual void OnAccumulatePropertiesToRender (TArray<SPropertyRenderCtx>& Result) const { }
		virtual TUniquePtr<ILuminousObj2D> OnClone () const = 0;
		virtual const CString& OnGetObjType () const = 0;
		virtual bool OnGetPropertyBool (Obj2DProp iProp) const { return false; }
		virtual CLuminousColor OnGetPropertyColor (Obj2DProp iProp) const { return CLuminousColor(); }
		virtual double OnGetPropertyScalar (Obj2DProp iProp) const { return 0.0; }
		virtual CString OnGetPropertyString (Obj2DProp iProp) const { return NULL_STR; }
		virtual CVector2D OnGetPropertyVector (Obj2DProp iProp) const { return CVector2D(); }
		virtual const TArray<CVector2D>& OnGetPropertyVectorQueue (Obj2DProp iProp) const { return m_NullVectorQueueValue; }
		virtual void OnRead (IByteStream& Stream) { }
		virtual bool OnSetPropertyBool (Obj2DProp iProp, bool bValue) { return false; }
		virtual bool OnSetPropertyColor (Obj2DProp iProp, const CLuminousColor& Value) { return false; }
		virtual bool OnSetPropertyScalar (Obj2DProp iProp, double rValue) { return false; }
		virtual bool OnSetPropertyVector (Obj2DProp iProp, const CVector2D& Value) { return false; }
		virtual bool OnSetPropertyString (Obj2DProp iProp, const CString& sValue) { return false; }
		virtual bool OnSetPropertyVectorQueue (Obj2DProp iProp, const TArray<CVector2D>& Value) { return false; }
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
		DWORD m_dwDirtyProps = 0;

		SequenceNumber m_Seq = 0;

		static TArray<SPropertyDesc> m_Properties;
		static TSortMap<CString, Obj2DProp> m_PropLookup;
		static TArray<CVector2D> m_NullVectorQueueValue;
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
			Stream,
			};

		enum class EOrigin
			{
			Unknown,

			Center,								//	(0,0) at center; +X right, +Y down
			UpperLeft,							//	(0,0) at upper-left; +X right, +Y down
			LowerLeft,							//	(0,0) at lower-left; +X right, +Y up
			};

		static CLuminousScene2D CreateFromStream (IByteStream& Stream);

		CLuminousScene2D () { }
		CLuminousScene2D (const CLuminousScene2D& Src) { Copy(Src); }
		CLuminousScene2D (CLuminousScene2D&& Src) noexcept = default;

		CLuminousScene2D& operator= (const CLuminousScene2D& Src) { CleanUp(); Copy(Src); return *this; }
		CLuminousScene2D& operator= (CLuminousScene2D&& Src) noexcept = default;

		static const CString& AsID (EMode iMode);
		static EMode AsMode (const CString& sValue);
		ILuminousObj2D& CreateCircle (DWORD dwParentID);
		ILuminousObj2D& CreateRectangle (DWORD dwParentID);
		ILuminousObj2D& CreateTrail (DWORD dwParentID);
		ILuminousObj2D& CreateLine (DWORD dwParentID);
		ILuminousObj2D* FindObj (DWORD dwID) { auto* pObj = m_Objs.GetAt(dwID); return (pObj ? (ILuminousObj2D*)(*pObj) : NULL); }
		const ILuminousObj2D* FindObj (DWORD dwID) const { return const_cast<CLuminousScene2D*>(this)->FindObj(dwID); }
		CLuminousColor GetBackgroundColor () const { return m_Background; }
		int GetFPS () const { return m_iFPS; }
		int GetFrameCount () const { return m_iFrameCount; }
		ILuminousObj2D& GetObj (int iIndex) { return *m_Objs[iIndex]; }
		const ILuminousObj2D& GetObj (int iIndex) const { return *m_Objs[iIndex]; }
		int GetObjCount () const { return m_Objs.GetCount(); }
		EOrigin GetOrigin () const { return m_iOrigin; }
		const CVector2D& GetExtents () const { return m_vExtents; }
		EMode GetMode () const { return m_iMode; }
		SequenceNumber GetSeq () const { return m_Seq; }
		SequenceNumber IncSeq () { return ++m_Seq; }
		void OnObjModified (ILuminousObj2D& Obj);
		bool RemoveObj (DWORD dwID);
		void SetBackgroundColor (const CLuminousColor& Color) { m_Background = Color; IncSeq(); }
		void SetKeyframe (int iFrame, IAnimator2D::Type iType) { m_iKeyframeFrame = iFrame; m_iKeyframeType = iType; }
		void ClearKeyframe () { m_iKeyframeFrame = -1; m_iKeyframeType = IAnimator2D::Type::Unknown; }
		bool IsKeyframeMode () const { return m_iKeyframeFrame >= 0; }
		int GetKeyframeFrame () const { return m_iKeyframeFrame; }
		IAnimator2D::Type GetKeyframeType () const { return m_iKeyframeType; }
		void SetExtents (const CVector2D& vExtents) { m_vExtents = vExtents; IncSeq(); }
		void SetFPS (int iFPS);
		void SetMode (EMode iMode);
		void SetOrigin (EOrigin iOrigin) { m_iOrigin = iOrigin; IncSeq(); }
		static const CString& AsOriginID (EOrigin iOrigin);
		static EOrigin AsOrigin (const CString& sValue);
		bool IsStreamMode () const { return m_iMode == EMode::Stream; }
		int GetStreamFrame () const { return m_iStreamFrame; }
		void AdvanceFrame (int iCount = 1);
		void TrimKeyframes (int iFrame);
		void SetSeq (SequenceNumber Seq) { m_Seq = Seq; }
		void Write (IByteStream& Stream) const;

	private:

		static constexpr DWORD SERIALIZED_VERSION = 3;

		static constexpr int DEFAULT_FPS = 30;

		void CleanUp () { m_Objs.DeleteAll(); }
		void Copy (const CLuminousScene2D& Src);
		void RecalcAnimation ();

		int m_iFPS = DEFAULT_FPS;
		int m_iFrameCount = -1;				//	-1 = infinite (otherwise, stop or repeat at this frame).
		EOrigin m_iOrigin = EOrigin::Center;
		CVector2D m_vExtents;				//	Logical extents (0,0 = raw pixels, scaleMode ignored)
		EMode m_iMode = EMode::Default;

		CLuminousColor m_Background = CLuminousColor();
		TSortMap<DWORD, TUniquePtr<ILuminousObj2D>> m_Objs;
		DWORD m_dwNextID = 1;
		SequenceNumber m_Seq = 1;

		//	Frame pointer: when set, property assignments on objects create
		//	keyframes instead of setting constant values.

		int m_iKeyframeFrame = -1;			//	-1 = not in keyframe mode
		IAnimator2D::Type m_iKeyframeType = IAnimator2D::Type::Unknown;

		//	Stream mode (EMode::Stream): properties set on objects are tracked
		//	as dirty; advanceFrame() creates constant keyframes for dirty properties.

		int m_iStreamFrame = 0;
	};
