//	DatatypeImpl.h
//
//	Defines subclasses of IDatatype
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

#include "AEON.h"

class CDatatypeAny : public IDatatype
	{
	public:
		CDatatypeAny (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

	private:

		//	IDatatype virtuals

		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual DWORD OnGetCoreType () const override { return IDatatype::ANY; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (&Type == this || Type.IsAny()); }
		virtual bool OnIsAbstract () const override { return true; }
		virtual bool OnIsAny () const override { return true; }
	};

class CDatatypeSimple : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatatypeList Implements;
			bool bAbstract = false;
			};

		CDatatypeSimple (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_Implements(Create.Implements),
				m_bAbstract(Create.bAbstract)
			{ }

	private:

		//	IDatatype virtuals

		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (&Type == this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual bool OnIsAbstract () const override { return m_bAbstract; }
		virtual void OnMark () override { m_Implements.Mark(); }

		DWORD m_dwCoreType = 0;
		CDatatypeList m_Implements;
		bool m_bAbstract = false;
	};

class CDatatypeNumber : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatatypeList Implements;
			int iBits = 0;
			bool bFloat = false;
			bool bUnsigned = false;
			};

		CDatatypeNumber (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_Implements(Create.Implements),
				m_iBits(Create.iBits),
				m_bFloat(Create.bFloat),
				m_bUnsigned(Create.bUnsigned)
			{ }

	private:

		//	IDatatype virtuals

		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Number; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (&Type == this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual void OnMark () override { m_Implements.Mark(); }

		DWORD m_dwCoreType = 0;
		CDatatypeList m_Implements;
		int m_iBits = 0;						//	0 = Infinite precision
		bool m_bFloat = false;
		bool m_bUnsigned = false;
	};

class CDatatypeArray : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatum dElementType;
			};

		CDatatypeArray (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_dElementType(Create.dElementType)
			{ }

	private:

		//	IDatatype virtuals

		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Array; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual SMemberDesc OnGetMember (int iIndex) const { if (iIndex != 0) throw CException(errFail); return SMemberDesc({ EMemberType::ArrayElement, NULL_STR, m_dElementType }); }
		virtual int OnGetMemberCount () const { return 1; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (&Type == this || Type.IsAny() || ((const IDatatype &)m_dElementType).IsA(Type)); }
		virtual void OnMark () override { m_dElementType.Mark(); }

		DWORD m_dwCoreType = 0;
		CDatum m_dElementType;
	};

class CDatatypeClass : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			CDatatypeList Implements;
			};

		CDatatypeClass (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_Implements(Create.Implements)
			{ }

	private:

		struct SMember
			{
			CString sName;
			EMemberType iType = EMemberType::None;
			CDatum dType;
			};

		//	IDatatype virtuals

		virtual bool OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError = NULL) override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::ClassDef; }
		virtual EMemberType OnHasMember (const CString &sName) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return (&Type == this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual void OnMark () override;

		TSortMap<CString, SMember> m_Members;
		CDatatypeList m_Implements;
	};

class CDatatypeSchema : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			CDatatypeList Implements;
			};

		CDatatypeSchema (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_Implements(Create.Implements)
			{ }

	private:

		struct SColumn
			{
			CString sName;
			CDatum dType;
			};

		//	IDatatype virtuals

		virtual bool OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError = NULL) override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Schema; }
		virtual SMemberDesc OnGetMember (int iIndex) const { if (iIndex < 0 || iIndex >= m_Columns.GetCount()) throw CException(errFail); return SMemberDesc({ EMemberType::InstanceVar, m_Columns[iIndex].sName, m_Columns[iIndex].dType }); }
		virtual int OnGetMemberCount () const { return m_Columns.GetCount(); }
		virtual EMemberType OnHasMember (const CString &sName) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return (&Type == this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual void OnMark () override;

		TSortMap<CString, SColumn> m_Columns;
		CDatatypeList m_Implements;
	};

class CDatatypeFunction : public IDatatype
	{
	};

//	CComplexDatatype -----------------------------------------------------------

class CComplexDatatype : public IComplexDatum
	{
	public:
		CComplexDatatype (IDatatype *pType) :
				m_pType(pType)
			{ }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { }
		virtual CString AsString (void) const override { return StructAsString(); }
		virtual size_t CalcMemorySize (void) const override { return 0; }
		virtual const IDatatype &CastIDatatype (void) const override { return *m_pType; }
		virtual IComplexDatum *Clone (void) const override { throw CException(errFail); }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) override { return false; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeDatatype; }
		virtual int GetCount (void) const override { return m_Properties.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::DATATYPE); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElement (int iIndex) const override { return m_Properties.GetProperty(*this, iIndex); }
		virtual CString GetKey (int iIndex) const override { return m_Properties.GetPropertyName(iIndex); }
		virtual const CString &GetTypename (void) const override;
		virtual void GrowToFit (int iCount) override { throw CException(errFail); }
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsNil (void) const override { return false; }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { throw CException(errFail); }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override { throw CException(errFail); }
		virtual void OnMarked (void) override { m_pType->Mark(); }

		TUniquePtr<IDatatype> m_pType;

		static TDatumPropertyHandler<CComplexDatatype> m_Properties;
	};
