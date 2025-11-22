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
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::BITMAP_RGBA8); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CRGBA32Image *GetImageInterface () override { return &m_Image; }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsNil () const override { return m_Image.IsEmpty(); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

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
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::CANVAS); }
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

		virtual int GetObjCount () const override { return m_Model.GetObjCount(); }
		virtual SequenceNumber GetSeq () const override { return m_Model.GetSeq(); }
		virtual void Play (int iStartFrame = 0) override { m_Model.Play(iStartFrame); }
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const override;
		virtual void SetSeq (SequenceNumber Seq) override { m_Model.SetSeq(Seq); }
		virtual void Stop () override { m_Model.Stop(); }

	private:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnMarked (void) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		bool AnimateProperty (DWORD dwID, Obj2DProp iProp, int iFrame, CDatum dDesc);
		bool AnimateProperty (ILuminousObj2D& Obj, Obj2DProp iProp, int iFrame, CDatum dDesc);
		DWORD CreateRectangle (CDatum dDesc);
		DWORD CreateRectangle (DWORD dwParentID, double rX, double rY, double rWidth, double rHeight);
		static CDatum GetAnimation (const ILuminousObj2D& Obj, const IAnimator2D& Animator);
		CDatum GetObjProperty (const ILuminousObj2D& Obj, Obj2DProp iProp, ObjPropType iPropType) const;
		CDatum RenderObj (const ILuminousObj2D& Obj) const;
		bool SetObjProperty (DWORD dwID, Obj2DProp iProp, CDatum dValue);
		bool SetObjProperty (ILuminousObj2D& Obj, Obj2DProp iProp, CDatum dValue);
		bool SetObjProperties (ILuminousObj2D& Obj, CDatum dData);

		CLuminousScene2D m_Model;

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

		static DWORD SCENE2D_TYPE;
		static DWORD SCENE3D_TYPE;

	private:

		static bool m_bInitialized;
	};

#include "LuminousAEONReanimator3D.h"
