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

		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) override { return true; }
		virtual bool OnEquals (const IDatatype &Src) const override { return true; }
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual DWORD OnGetCoreType () const override { return IDatatype::ANY; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Any; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type == *this || Type.IsAny()); }
		virtual bool OnIsAbstract () const override { return true; }
		virtual bool OnIsAny () const override { return true; }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override { }
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

		CDatatypeSimple (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeSimple (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_Implements(Create.Implements),
				m_bAbstract(Create.bAbstract)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Simple; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type == *this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual bool OnIsAbstract () const override { return m_bAbstract; }
		virtual void OnMark () override { m_Implements.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

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

		CDatatypeNumber (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeNumber (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_Implements(Create.Implements),
				m_iBits(Create.iBits),
				m_bFloat(Create.bFloat),
				m_bUnsigned(Create.bUnsigned)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Number; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Number; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type == *this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual void OnMark () override { m_Implements.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

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

		CDatatypeArray (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeArray (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_dElementType(Create.dElementType)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Array; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Array; }
		virtual SMemberDesc OnGetMember (int iIndex) const { if (iIndex != 0) throw CException(errFail); return SMemberDesc({ EMemberType::ArrayElement, NULL_STR, m_dElementType }); }
		virtual int OnGetMemberCount () const { return 1; }
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type == *this || Type.IsAny() || ((const IDatatype &)m_dElementType).IsA(Type)); }
		virtual void OnMark () override { m_dElementType.Mark(); }
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

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

		CDatatypeClass (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

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
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (const CString &sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::ClassDef; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Class; }
		virtual EMemberType OnHasMember (const CString &sName, CDatum *retdType = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type == *this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual void OnMark () override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

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
			DWORD dwCoreType = 0;
			};

		CDatatypeSchema (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName),
				m_dwCoreType(Create.dwCoreType),
				m_Implements(Create.Implements)
			{ }

		explicit CDatatypeSchema (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

	private:

		struct SColumn
			{
			int iOrdinal = 0;
			CString sName;
			CDatum dType;
			};

		//	IDatatype virtuals

		virtual bool OnAddMember (const CString &sName, EMemberType iType, CDatum dType, CString *retsError = NULL) override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (const CString &sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Schema; }
		virtual DWORD OnGetCoreType () const override { return m_dwCoreType; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Schema; }
		virtual SMemberDesc OnGetMember (int iIndex) const { if (iIndex < 0 || iIndex >= m_Columns.GetCount()) throw CException(errFail); return SMemberDesc({ EMemberType::InstanceVar, m_Columns[iIndex].sName, m_Columns[iIndex].dType }); }
		virtual int OnGetMemberCount () const { return m_Columns.GetCount(); }
		virtual CDatum OnGetMembersAsTable () const override;
		virtual EMemberType OnHasMember (const CString &sName, CDatum *retdType = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type == *this || Type.IsAny() || m_Implements.IsA(Type)); }
		virtual void OnMark () override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;

		TArray<SColumn> m_Columns;
		TSortMap<CString, int> m_ColumnsByName;
		DWORD m_dwCoreType = 0;
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

		CComplexDatatype (TUniquePtr<IDatatype> &&Type) :
				m_pType(std::move(Type))
			{ }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { }
		virtual CString AsString (void) const override { return StructAsString(); }
		virtual size_t CalcMemorySize (void) const override { return 0; }
		virtual const IDatatype &CastIDatatype (void) const override { return *m_pType; }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) override { return false; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeDatatype; }
		virtual int GetCount (void) const override { return m_Properties.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::DATATYPE); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElement (int iIndex) const override { return m_Properties.GetProperty(*this, iIndex); }
		virtual CString GetKey (int iIndex) const override { return m_Properties.GetPropertyName(iIndex); }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsNil (void) const override { return false; }
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static int GetID (IDatatype::EImplementation iValue);
		static IDatatype::EImplementation GetImplementation (int iID);

	private:

		static constexpr int IMPL_ANY_ID =				1;
		static constexpr int IMPL_ARRAY_ID =			2;
		static constexpr int IMPL_CLASS_ID =			3;
		static constexpr int IMPL_NUMBER_ID =			4;
		static constexpr int IMPL_SCHEMA_ID =			5;
		static constexpr int IMPL_SIMPLE_ID =			6;

		//	IComplexDatum

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void OnMarked (void) override { m_pType->Mark(); }

		TUniquePtr<IDatatype> m_pType;

		static TDatumPropertyHandler<CComplexDatatype> m_Properties;
	};
