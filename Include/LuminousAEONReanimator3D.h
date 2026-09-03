//	LuminousAEONReanimator3D.h
//
//	Luminous AEON Integration
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

class CAEONReanimator3D;

class CAEONTexture3D : public TExternalDatum<CAEONTexture3D>
	{
	public:
		CAEONTexture3D () { }
		CAEONTexture3D (CDatum dScene, DWORD dwID) : m_dScene(dScene), m_dwID(dwID) { }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static const CString& StaticGetTypename (void);
		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }

		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEONTexture3D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { throw CException(errFail); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override { return false; }
		virtual DWORD OnGetSerializeFlags () const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked () override { m_dScene.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct* pStruct) const override { throw CException(errFail); }

	private:
		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dScene;
		DWORD m_dwID = 0;
		static TDatumPropertyHandler<CAEONTexture3D> m_Properties;
		static TDatumMethodHandler<CAEONTexture3D> m_Methods;
	};

class CAEONMaterial3D : public TExternalDatum<CAEONMaterial3D>
	{
	public:
		CAEONMaterial3D () { }
		CAEONMaterial3D (CDatum dScene, DWORD dwID) : m_dScene(dScene), m_dwID(dwID) { }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static const CString& StaticGetTypename (void);
		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }
		bool SetProperty (const CString& sKey, CDatum dValue, CString* retsError = NULL) { return m_Properties.SetProperty(*this, sKey, dValue, retsError); }

		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEONMaterial3D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { throw CException(errFail); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override { return false; }
		virtual DWORD OnGetSerializeFlags () const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked () override { m_dScene.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct* pStruct) const override { throw CException(errFail); }

	private:
		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dScene;
		DWORD m_dwID = 0;
		static TDatumPropertyHandler<CAEONMaterial3D> m_Properties;
		static TDatumMethodHandler<CAEONMaterial3D> m_Methods;
	};

class CAEONObj3D : public TExternalDatum<CAEONObj3D>
	{
	public:
		CAEONObj3D () { }
		CAEONObj3D (CDatum dScene, DWORD dwID) : m_dScene(dScene), m_dwID(dwID) { }

		static CDatum Create (CDatum dScene, DWORD dwID);
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static const CString& StaticGetTypename (void);
		DWORD GetID () const { return m_dwID; }
		CDatum GetScene () const { return m_dScene; }

		virtual IComplexDatum* Clone (CDatum::EClone iMode) const override { return new CAEONObj3D(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { throw CException(errFail); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, CDatum dStruct) override { return false; }
		virtual DWORD OnGetSerializeFlags () const override { return FLAG_SERIALIZE_AS_STRUCT; }
		virtual void OnMarked () override { m_dScene.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, CComplexStruct* pStruct) const override { throw CException(errFail); }

	private:
		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		CDatum m_dScene;
		DWORD m_dwID = 0;
		static TDatumPropertyHandler<CAEONObj3D> m_Properties;
		static TDatumMethodHandler<CAEONObj3D> m_Methods;
	};

class CAEONReanimator3D : public TExternalDatum<CAEONReanimator3D>, public IAEONReanimator
	{
	public:
		CAEONReanimator3D () { }

		static CDatum Create ();
		static TArray<IDatatype::SMemberDesc> GetMembers ();
		static const CString& StaticGetTypename (void);

		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType () const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual IAEONReanimator* GetReanimatorInterface () override { return this; }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override { return OpIsEqual(iValueType, dValue); }
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		virtual int GetFrameCount () const override { return m_Model.GetFrameCount(); }
		virtual int GetObjCount () const override { return m_Model.GetObjCount(); }
		virtual SequenceNumber GetSeq () const override { return m_Model.GetSeq(); }
		virtual bool IsStreamMode () const override { return m_Model.IsStreamMode(); }
		virtual CDatum RenderAsHTMLCanvasCommands (SequenceNumber Seq = 0) const override;
		virtual void SetSeq (SequenceNumber Seq) override { m_Model.SetSeq(Seq); }
		virtual void TrimKeyframes (int iFrame) override { m_Model.TrimKeyframes(iFrame); }

		bool AnimateObjProperty (DWORD dwID, Obj3DProp iProp, int iFrame, CDatum dDesc) { return AnimateProperty(dwID, iProp, iFrame, dDesc); }
		CDatum Create3DSObj (CDatum dSelf, const CString& sGridID, CDatum dDesc = CDatum());
		CDatum CreateCameraObj (CDatum dSelf, CDatum dDesc = CDatum());
		CDatum CreateCubeObj (CDatum dSelf, CDatum dDesc = CDatum(), CString* retsError = NULL);
		CDatum CreateGLTFObj (CDatum dSelf, const CString& sGridID, CDatum dDesc = CDatum());
		CDatum CreateImageTexture (CDatum dSelf, const CString& sGridID, CDatum dOptions = CDatum());
		CDatum CreatePhysicalMaterial (CDatum dSelf, CDatum dDesc = CDatum(), CString* retsError = NULL);
		CDatum CreatePointLightObj (CDatum dSelf, CDatum dDesc = CDatum());
		ILuminousMaterial3D* FindMaterial (DWORD dwID) { return m_Model.FindMaterial(dwID); }
		const ILuminousMaterial3D* FindMaterial (DWORD dwID) const { return m_Model.FindMaterial(dwID); }
		ILuminousObj3D* FindObj (DWORD dwID) { return m_Model.FindObj(dwID); }
		const ILuminousObj3D* FindObj (DWORD dwID) const { return m_Model.FindObj(dwID); }
		ILuminousTexture3D* FindTexture (DWORD dwID) { return m_Model.FindTexture(dwID); }
		const ILuminousTexture3D* FindTexture (DWORD dwID) const { return m_Model.FindTexture(dwID); }
		bool RemoveObj (DWORD dwID) { return m_Model.RemoveObj(dwID); }
		bool SetObjColor (DWORD dwID, CDatum dValue);
		bool SetObjMaterial (DWORD dwID, CDatum dValue);
		bool SetObjProperty (DWORD dwID, Obj3DProp iProp, CDatum dValue);
		static DWORD ParseID (CDatum dValue);

	private:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return 0; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream) override;
		virtual void OnMarked () override { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const override { m_Model.Write(Stream); }
		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) override { m_Model = CLuminousScene3D::CreateFromStream(Stream); }
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { m_Model.Write(Stream); }

		bool AnimateProperty (DWORD dwID, Obj3DProp iProp, int iFrame, CDatum dDesc);
		bool AnimateProperty (ILuminousObj3D& Obj, Obj3DProp iProp, int iFrame, CDatum dDesc);
		static CDatum CompactValue (Obj3DPropType iType, CDatum dValue);
		static CDatum RenderAnimatedProperty (const IAnimator3D& Animator);
		CDatum RenderConstProperty (const ILuminousObj3D& Obj, Obj3DProp iProp, Obj3DPropType iType) const;
		CDatum RenderObj (const ILuminousObj3D& Obj) const;
		bool SetObjProperty (ILuminousObj3D& Obj, Obj3DProp iProp, CDatum dValue);
		bool SetObjProperties (ILuminousObj3D& Obj, CDatum dData);

		CLuminousScene3D m_Model;
		static TDatumPropertyHandler<CAEONReanimator3D> m_Properties;
		static TDatumMethodHandler<CAEONReanimator3D> m_Methods;
	};
