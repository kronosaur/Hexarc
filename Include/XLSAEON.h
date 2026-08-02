//	XLSAEON.h
//
//	XLS AEON Integration
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "XLSUtil.h"
#include "AEON.h"
#include "Hexe.h"

class CAEONXLSWorkbook : public TExternalDatum<CAEONXLSWorkbook>, public IAEONCanvas
	{
	public:

		CAEONXLSWorkbook () { }

		CAEONXLSWorkbook (const CXLSWorkbook& Src) = delete;
		CAEONXLSWorkbook (CXLSWorkbook&& Src) : m_Workbook(std::move(Src)) { }

		CAEONXLSWorkbook& operator= (const CXLSWorkbook& Src) = delete;
		CAEONXLSWorkbook& operator= (CXLSWorkbook&& Src) noexcept { m_Workbook = std::move(Src); return *this; }

		static bool Boot ();
		static CDatum Create (CDatum dValue);
		static CDatum Create (CXLSWorkbook&& Src);
		static const CString& StaticGetTypename (void);

		//	IComplexDatum

		virtual CString AsString () const override { return strPattern("XLS Workbook"); }
		virtual size_t CalcMemorySize () const override { return 0; }
		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override;
		virtual DWORD GetBasicDatatype () const override { return IDatatype::OBJECT; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeAEONObject; }
		virtual CDatum GetDatatype () const override;
		virtual CDatum GetElement (const CString& sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetMethod (const CString& sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual bool InvokeMethodImpl (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, SAEONInvokeResult& retResult) override 
			{ return m_Methods.InvokeMethod(dObj, sMethod, Ctx, LocalEnv, CDatum(), CDatum(), retResult); }
		virtual bool IsNil () const override { return m_Workbook.GetSheetCount() == 0; }
		virtual int OpCompare (CDatum::Types iValueType, CDatum dValue) const override;
		virtual int OpCompareExact (CDatum::Types iValueType, CDatum dValue) const override;
		virtual void SetElement (const CString& sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static DWORD CORE_TYPE;

	protected:

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString& sTypename, IByteStream& Stream) override;
		virtual void OnMarked (void) override { }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream& Stream) const override;

	private:

		virtual void DeserializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) override;
		virtual void SerializeAEONExternal (IByteStream& Stream, CAEONSerializedMap &Serialized) const override;

		CXLSWorkbook m_Workbook;

		static TDatumPropertyHandler<CAEONXLSWorkbook> m_Properties;
		static TDatumMethodHandler<CAEONXLSWorkbook> m_Methods;
		static bool m_bRegistered;
	};

