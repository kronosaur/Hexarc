//	LuminousAEONReanimator3D.h
//
//	Luminous AEON Integration
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

class CAEONReanimator3D : public TExternalDatum<CAEONReanimator3D>, public IAEONReanimator
	{
	public:

		CAEONReanimator3D () { }

		static CDatum Create ();
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

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

		bool AnimateProperty (DWORD dwID, Obj3DProp iProp, int iFrame, CDatum dDesc);
		bool AnimateProperty (ILuminousObj3D& Obj, Obj3DProp iProp, int iFrame, CDatum dDesc);
		static CDatum GetAnimation (const ILuminousObj3D& Obj, const IAnimator3D& Animator);
		CDatum GetObjProperty (const ILuminousObj3D& Obj, Obj3DProp iProp, Obj3DPropType iPropType) const;
		CDatum RenderObj (const ILuminousObj3D& Obj) const;
		bool SetObjProperty (DWORD dwID, Obj3DProp iProp, CDatum dValue);
		bool SetObjProperty (ILuminousObj3D& Obj, Obj3DProp iProp, CDatum dValue);
		bool SetObjProperties (ILuminousObj3D& Obj, CDatum dData);

		CLuminousScene3D m_Model;

		static TDatumPropertyHandler<CAEONReanimator3D> m_Properties;
		static TDatumMethodHandler<CAEONReanimator3D> m_Methods;
	};

