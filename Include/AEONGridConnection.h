//	AEONGridConnection.h
//
//	AEONGridConnection Definition
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

class CAEONGridConnection : public TExternalDatum<CAEONGridConnection>
	{
	public:

		CAEONGridConnection () { }
		static CDatum Create (const IExternalAccount::SConnectionCreate& CreateCtx);

		CStringView GetID () const { return m_sID; }
		const CGridName& GetUsername () const { return m_User; }

		static const CString &StaticGetTypename (void);

		//	IComplexDatum

		virtual CString AsString () const override;
		virtual size_t CalcMemorySize () const override;
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override { return new CAEONGridConnection(*this); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl(CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsNil () const override { return false; }
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

		CDatum GetAPIListAsTable () const;
		CString GetCredentialsID () const;

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CGridName m_User;					//	User
		CString m_sID;						//	ID of connection
		CString m_sName;					//	Name of connection
		CString m_sSourceName;				//	Name of source
		CString m_sIconID;					//	Icon ID
		EAccountProtocol m_iProtocol = EAccountProtocol::None;

		bool m_bAuthService = false;		//	TRUE if we have an auth service
		bool m_bAPIKey = false;				//	TRUE if we have an API key service
		bool m_bEmail = false;				//	TRUE if we have an email service

		TArray<IExternalAccount::SAPIDesc> m_APIList;
		CString m_sDomain;					//	Domain for connection. Depending on the type,
											//	this has different functions. E.g., for SendGrid,
											//	this is the email domain of sender.
		
		static TDatumPropertyHandler<CAEONGridConnection> m_Properties;
		static TDatumMethodHandler<CAEONGridConnection> m_Methods;
	};
