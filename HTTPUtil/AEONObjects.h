//	AEONObjects.h
//
//	AEON Definition
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

class CAEONURL : public TExternalDatum<CAEONURL>
	{
	public:

		CAEONURL () { }

		static CDatum Create (const CString& sURL, CDatum dComponents);
		static const CString &StaticGetTypename (void);

		static CString ComposeURL (const CString& sProtocol, const CString& sHostname, DWORD dwPort = 0, const CString& sPath = NULL_STR, CDatum dQuery = CDatum(), const CString& sFragment = NULL_STR);
		static bool ParseURL (const CString& sURL, CString* retsProtocol = NULL, CString* retsHostname = NULL, DWORD* retdwPort = NULL, CString *retsPath = NULL, CDatum* retdQuery = NULL, CString *retsFragment = NULL);

		//	IComplexDatum

		virtual CString AsString () const override { return m_sURL; }
		virtual size_t CalcMemorySize () const override;
		virtual CStringView CastCString () const override { return m_sURL; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsNil () const override { return m_sURL.IsEmpty(); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override { return OpCompare(iValueType, dValue); }
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcMemorySize() + sizeof(DWORD); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		CAEONURL (const CString& sURL) : m_sURL(sURL)
			{ }

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		static bool EncodeFragment (CStringBuffer& Output, const CString& sFragment);
		static bool EncodePath (CStringBuffer& Output, const CString& sPath);
		static bool ParseFragment (const char*& pPos, CString* retsFragment = NULL);
		static bool ParseHostname (const char*& pPos, CString* retsHostname = NULL);
		static bool ParsePath (const char*& pPos, CString* retsPath = NULL);
		static bool ParseProtocol (const char*& pPos, CString* retsProtocol = NULL);
		static bool ParseQuery (const char*& pPos, CDatum* retdQuery = NULL);
		static bool ValidateHostname (const CString& sHostname);
		static bool ValidateProtocol (const CString& sProtocol);

		CString m_sURL;
		static TDatumPropertyHandler<CAEONURL> m_Properties;
		static TDatumMethodHandler<CAEONURL> m_Methods;
	};

class CAEONXMLElement : public TExternalDatum<CAEONXMLElement>
	{
	public:

		CAEONXMLElement () { }

		static CDatum Create (const IMemoryBlock& FileData);
		static CDatum Create (const CXMLElement& Element);
		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONXMLElement(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual int GetCount () const override { return m_Children.GetCount() + m_Text.GetCount(); }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsEqual (CDatum::Types iValueType, CDatum dValue) const override;
		virtual bool OpIsIdentical (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void OnMarked () override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static TArray<IDatatype::SMemberDesc> GetMembers (void);

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { return CalcMemorySize() + sizeof(DWORD); }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		static CDatum CreateError (CStringView sError);
		void WriteAsXML (CStringBuffer& Output) const;

		CString m_sTag;
		TSortMap<CString, CString> m_Attributes;
		TArray<CDatum> m_Children;
		TArray<CString> m_Text;

		static TDatumPropertyHandler<CAEONXMLElement> m_Properties;
		static TDatumMethodHandler<CAEONXMLElement> m_Methods;
	};
