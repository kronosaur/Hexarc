//	LuminousAEON.h
//
//	Luminous AEON Integration
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "LuminousCore.h"
#include "LuminousHTMLCanvas.h"
#include "AEON.h"
#include "Hexe.h"

class CAEONLuminousBitmap : public TExternalDatum<CAEONLuminousBitmap>, public IAEONCanvas
	{
	public:

		CAEONLuminousBitmap () { }

		static CDatum Create (const CRGBA32Image& Src);
		static CDatum Create (CRGBA32Image&& Src);
		static CDatum Create (int cxWidth, int cyHeight);
		static CDatum Create (int cxWidth, int cyHeight, CRGBA32 rgbBackground);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual CString AsString () const override { return strPattern("Image %dx%d", m_Image.GetWidth(), m_Image.GetHeight()); }
		virtual size_t CalcMemorySize () const override { return (size_t)m_Image.GetWidth() * (size_t)m_Image.GetHeight() * sizeof(DWORD); }
		virtual const CRGBA32Image &CastCRGBA32Image () const override { return m_Image; }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeImage32; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CRGBA32Image *GetImageInterface () override { return &m_Image; }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsNil () const override { return m_Image.IsEmpty(); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static TArray<IDatatype::SMemberDesc> GetMembers ();

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CLuminousCanvasCtx m_DrawCtx;
		CRGBA32Image m_Image;
		CRGBA32 m_rgbBackground;

		static TDatumPropertyHandler<CAEONLuminousBitmap> m_Properties;
		static TDatumMethodHandler<CAEONLuminousBitmap> m_Methods;
	};

class CAEONLuminousCanvas : public TExternalDatum<CAEONLuminousCanvas>, public IAEONCanvas
	{
	public:

		CAEONLuminousCanvas () { }

		static CDatum Create ();
		static CDatum GenerateBeginUpdate ();
		static CDatum GenerateClearRectDesc (const CVector2D& vUL, const CVector2D& vLR);
		static CDatum GenerateImageDesc (const CVector2D& vPos, CDatum dImage);
		static CDatum GenerateSetResourceDesc (const CString& sResourceID, CDatum dImage);
		static CDatum GenerateShapeDesc (const CLuminousCanvasModel::SShapeOptions& Options);
		static CDatum GenerateTextDesc (const CLuminousCanvasModel::STextOptions& Options);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONLuminousCanvas(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual IAEONCanvas *GetCanvasInterface () override { return this; }
		virtual CDatum GetDatatype () const override { return CAEONTypes::Get(IDatatype::CANVAS); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return OpIsEqual(iValueType, dValue); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		//	IAEONCanvas

		virtual void DeleteAllGraphics () override { OnModify(); m_Model.DeleteAll(); }
		virtual int GetGraphicCount () const override { return m_Model.GetRenderCount(); }
		virtual SequenceNumber GetSeq () const override { return m_Seq; }
		virtual bool InsertGraphic (CDatum dDesc) override;
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const override;
		virtual void SetGraphicSeq (int iIndex, SequenceNumber Seq) override;
		virtual void SetGraphicSeq (SequenceNumber Seq) override { m_Model.SetSeq(Seq); m_Resources.SetSeq(Seq); }
		virtual void SetSeq (SequenceNumber Seq) override { m_Seq = Seq; }

		virtual void* raw_GetGraphicByID (DWORD dwID) const override;

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		CDatum CreateSprite (CDatum dDesc, CDatum dOptions);
		void OnModify ();
		static void SetSpriteOptions (ILuminousGraphic& Graphic, CDatum dOptions);

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CLuminousCanvasCtx m_DrawCtx;
		CLuminousCanvasModel m_Model;
		CLuminousCanvasResources m_Resources;

		SequenceNumber m_Seq = 0;

		static TDatumPropertyHandler<CAEONLuminousCanvas> m_Properties;
		static TDatumMethodHandler<CAEONLuminousCanvas> m_Methods;
	};

class CAEONCircle2D : public TExternalDatum<CAEONCircle2D>
	{
	public:

		CAEONCircle2D () { }
		CAEONCircle2D (CDatum dScene, DWORD dwID) :
				m_dScene(dScene),
				m_dwID(dwID)
			{ }

		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONCircle2D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static TArray<IDatatype::SMemberDesc> GetMembers ();

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CDatum m_dScene;
		DWORD m_dwID = 0;

		static TDatumPropertyHandler<CAEONCircle2D> m_Properties;
		static TDatumMethodHandler<CAEONCircle2D> m_Methods;
	};

class CAEONRect2D : public TExternalDatum<CAEONRect2D>
	{
	public:

		CAEONRect2D () { }
		CAEONRect2D (CDatum dScene, DWORD dwID) :
				m_dScene(dScene),
				m_dwID(dwID)
			{ }

		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONRect2D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static TArray<IDatatype::SMemberDesc> GetMembers ();

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CDatum m_dScene;
		DWORD m_dwID = 0;

		static TDatumPropertyHandler<CAEONRect2D> m_Properties;
		static TDatumMethodHandler<CAEONRect2D> m_Methods;
	};



class CAEONLine2D : public TExternalDatum<CAEONLine2D>
	{
	public:

		CAEONLine2D () { }
		CAEONLine2D (CDatum dScene, DWORD dwID) :
				m_dScene(dScene),
				m_dwID(dwID)
			{ }

		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static const CString &StaticGetTypename (void);

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONLine2D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static TArray<IDatatype::SMemberDesc> GetMembers ();

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CDatum m_dScene;
		DWORD m_dwID = 0;

		static TDatumPropertyHandler<CAEONLine2D> m_Properties;
		static TDatumMethodHandler<CAEONLine2D> m_Methods;
	};
class CAEONTrail2D : public TExternalDatum<CAEONTrail2D>
	{
	public:

		CAEONTrail2D () { }
		CAEONTrail2D (CDatum dScene, DWORD dwID) :
				m_dScene(dScene),
				m_dwID(dwID)
			{ }

		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static const CString &StaticGetTypename (void);

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONTrail2D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static TArray<IDatatype::SMemberDesc> GetMembers ();

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CDatum m_dScene;
		DWORD m_dwID = 0;

		static TDatumPropertyHandler<CAEONTrail2D> m_Properties;
		static TDatumMethodHandler<CAEONTrail2D> m_Methods;
	};
class CAEONReanimator : public TExternalDatum<CAEONReanimator>, public IAEONReanimator
	{
	public:

		CAEONReanimator () { }

		static CDatum Create ();
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONReanimator(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual IAEONReanimator* GetReanimatorInterface () override { return this; }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return OpIsEqual(iValueType, dValue); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		//	IAEONReanimator

		virtual int GetFrameCount () const override { return m_Model.GetFrameCount(); }
		virtual int GetObjCount () const override { return m_Model.GetObjCount(); }
		virtual SequenceNumber GetSeq () const override { return m_Model.GetSeq(); }
		virtual bool IsStreamMode () const override { return m_Model.IsStreamMode(); }
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const override;
		virtual void SetSeq (SequenceNumber Seq) override { m_Model.SetSeq(Seq); }
		virtual void TrimKeyframes (int iFrame) override { m_Model.TrimKeyframes(iFrame); }

		//	Methods used by CAEONRect2D to access objects in the scene.

		bool AnimateObjProperty (DWORD dwID, Obj2DProp iProp, int iFrame, CDatum dDesc) { return AnimateProperty(dwID, iProp, iFrame, dDesc); }
		CDatum CreateCircleObj (CDatum dSelf, CDatum dDesc = CDatum());
		CDatum CreateRectangleObj (CDatum dSelf, CDatum dDesc = CDatum());
		CDatum CreateLineObj (CDatum dSelf, CDatum dDesc = CDatum());
		CDatum CreateTrailObj (CDatum dSelf, CDatum dDesc = CDatum());
		ILuminousObj2D *FindObj (DWORD dwID) { return m_Model.FindObj(dwID); }
		const ILuminousObj2D *FindObj (DWORD dwID) const { return m_Model.FindObj(dwID); }
		CDatum GetObjData (DWORD dwID) const { auto *pData = m_ObjData.GetAt(dwID); return pData ? *pData : CDatum(); }
		void SetObjData (DWORD dwID, CDatum dData) { if (dData.IsNil()) m_ObjData.DeleteAt(dwID); else m_ObjData.SetAt(dwID, dData); }
		bool RemoveObj (DWORD dwID);
		bool SetObjProperty (DWORD dwID, Obj2DProp iProp, CDatum dValue);
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static DWORD ParseID (CDatum dValue);

	private:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		bool AnimateProperty (DWORD dwID, Obj2DProp iProp, int iFrame, CDatum dDesc);
		bool AnimateProperty (ILuminousObj2D& Obj, Obj2DProp iProp, int iFrame, CDatum dDesc);
		static CDatum CompactValue (ObjPropType iType, CDatum dValue);
		static CDatum RenderAnimatedProperty (const ILuminousObj2D& Obj, const IAnimator2D& Animator);
		CDatum RenderConstProperty (const ILuminousObj2D& Obj, Obj2DProp iProp, ObjPropType iPropType) const;
		CDatum RenderObj (const ILuminousObj2D& Obj) const;
		void SetCircleDefaults (ILuminousObj2D& Obj);
		void SetRectangleDefaults (ILuminousObj2D& Obj);
		void SetLineDefaults (ILuminousObj2D& Obj);
		void SetTrailDefaults (ILuminousObj2D& Obj);
		bool SetObjProperty (ILuminousObj2D& Obj, Obj2DProp iProp, CDatum dValue);
		bool SetObjProperties (ILuminousObj2D& Obj, CDatum dData);

		CLuminousScene2D m_Model;
		TSortMap<DWORD, CDatum> m_ObjData;

		static TDatumPropertyHandler<CAEONReanimator> m_Properties;
		static TDatumMethodHandler<CAEONReanimator> m_Methods;
	};

class CAEONLuminousSprite : public TExternalDatum<CAEONLuminousSprite>
	{
	public:

		CAEONLuminousSprite () { }
		CAEONLuminousSprite (CDatum dCanvas, DWORD dwID) :
				m_dCanvas(dCanvas),
				m_dwID(dwID)
			{ }

		static CDatum Create (CDatum dCanvas, DWORD dwID);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONLuminousSprite(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override;
		virtual DWORD OnGetSerializeFlags (void) const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct *pStruct) const override;

	private:

		const ILuminousGraphic& GetGraphic () const;

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override { }
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override { }

		CDatum m_dCanvas;
		DWORD m_dwID = 0;

		static TDatumPropertyHandler<CAEONLuminousSprite> m_Properties;
		static TDatumMethodHandler<CAEONLuminousSprite> m_Methods;
	};

class CAEONLuminous
	{
	public:

		static bool Boot ();

		static CDatum AsDatum (const CLuminousColor& Color, const CLuminousColor& Default = CLuminousColor());
		static CDatum AsDatum (const CLuminousPath2D& Path);
		static CDatum AsDatum (const CLuminousFillStyle& Style);
		static CDatum AsDatum (const CLuminousFontStyle& Style);
		static CDatum AsDatum (const CLuminousLineStyle& Style);
		static CDatum AsDatum (const CLuminousShadowStyle& Style);
		static CDatum AsDatum (const CLuminousTextAlign& Style);

		static CLuminousColor AsColor (CDatum dValue, const CLuminousColor& Default = CLuminousColor());
		static CLuminousFillStyle AsFillStyle (CDatum dValue);
		static CLuminousFontStyle AsFontStyle (CDatum dValue);
		static CLuminousLineStyle AsLineStyle (CDatum dValue);
		static CLuminousPath2D AsPath2D (CDatum dValue);
		static CLuminousShadowStyle AsShadowStyle (CDatum dValue);
		static CLuminousTextAlign AsTextAlign (CDatum dValue);

		static DWORD BITMAP_TYPE;
		static DWORD BITMAP_MONO_TYPE;
		static DWORD BITMAP_GRAY8_TYPE;
		static DWORD BITMAP_RGBA8_TYPE;
		static DWORD PIXEL_FORMAT_ENUM;
		static DWORD CIRCLE2D_TYPE;
		static DWORD RECT2D_TYPE;
		static DWORD LINE2D_TYPE;
		static DWORD TRAIL2D_TYPE;
		static DWORD SCENE2D_TYPE;
		static DWORD SCENE3D_TYPE;

	private:

		static bool m_bInitialized;
	};

#include "LuminousAEONReanimator3D.h"
