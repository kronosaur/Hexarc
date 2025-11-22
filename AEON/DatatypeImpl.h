//	DatatypeImpl.h
//
//	Defines subclasses of IDatatype
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "AEON.h"

class CDatatypeAny : public IDatatype
	{
	public:
		CDatatypeAny (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName, IDatatype::ANY)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeNull () const override { return true; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override { return true; }
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override { return true; }
		virtual bool OnEquals (const IDatatype &Src) const override { return true; }
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Any; }
		virtual bool OnIsAbstract () const override { return true; }
		virtual bool OnIsAny () const override { return true; }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { }
	};

class CDatatypeArray : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatum dElementType;
			bool bTable = false;

			bool bDictionary = false;
			CDatum dKeyType;					//	May be Nil (defaults to INTEGER)
			bool bAnonymous = false;
			};

		CDatatypeArray (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeArray (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType, Create.bAnonymous),
				m_dKeyType(Create.dKeyType),
				m_dElementType(Create.dElementType),
				m_bTable(Create.bTable),
				m_bDictionary(Create.bDictionary)
			{ 
			if (m_bTable && ((const IDatatype&)Create.dElementType).GetClass() != IDatatype::ECategory::Schema)
				throw CException(errFail);

			if (m_bDictionary && Create.dElementType.GetBasicType() != CDatum::typeDatatype)
				throw CException(errFail);

			if (m_bTable && m_bDictionary)
				throw CException(errFail);
			}

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType = NULL, CString* retsError = NULL) const override;
		virtual bool OnCanBeNull () const override { return false; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override 
			{
			if (m_bTable)
				return IDatatype::ECategory::Table;
			else if (m_bDictionary)
				return IDatatype::ECategory::Dictionary;
			else
				return IDatatype::ECategory::Array;
			}
		virtual TArray<CDatum> OnGetDimensionTypes () const override;
		virtual CDatum OnGetFieldsAsTable () const override;
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Array; }
		virtual CDatum OnGetKeyType () const override;
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override;
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual CString OnGetName () const override;
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual void OnMark () override { m_dKeyType.Mark(); m_dElementType.Mark(); }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dKeyType;						//	Nil means INTEGER
		CDatum m_dElementType;
		bool m_bTable = false;
		bool m_bDictionary = false;
		CRecursionState m_rs;
	};

class CDatatypeClass : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			CDatatypeList Implements;
			std::initializer_list<SMemberDesc> Members;
			};

		CDatatypeClass (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeClass (const SCreate &Create);

	private:

		struct SMember
			{
			EMemberType iType = EMemberType::None;
			CString sID;
			CDatum dType;
			CString sLabel;
			DWORD dwFlags = 0;
			};

		//	IDatatype virtuals

		virtual bool OnAddImplementation (CDatum dType) override;
		virtual bool OnAddMember (const SMemberDesc& Desc, CString *retsError = NULL) override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::ClassDef; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Class; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override { return m_Members.GetCount(); }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return m_Implements.IsA(Type); }
		virtual bool OnIsSupersetOf (const IDatatype& Type) const;
		virtual void OnMark () override;
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void OnSetMemberType (const CString& sName, CDatum dType, DWORD dwFlags) override;

		TSortMap<CString, SMember> m_Members;
		CDatatypeList m_Implements;
		CRecursionState m_rs;
	};

class CDatatypeEnum : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			};

		CDatatypeEnum (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType)
			{ }

		explicit CDatatypeEnum (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		bool IsEqual (const CDatatypeEnum& Other) const;

	private:

		struct SEntry
			{
			int iOrdinal = 0;
			CString sID;
			CString sLabel;		//	If blank, use sID
			};

		//	IDatatype virtuals

		virtual bool OnAddMember (const SMemberDesc& Desc, CString *retsError = NULL) override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual int OnFindMemberByOrdinal (int iOrdinal) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Enum; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Enum; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override { return m_Entries.GetCount(); }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return (Type.GetCoreType() == IDatatype::ENUM); }
		virtual bool OnIsEnum (const TArray<IDatatype::SMemberDesc>& Values) const override;
		virtual CDatum OnIteratorBegin () const override { if (m_Entries.GetCount() == 0) return CDatum(); return CDatum(0); }
		virtual CDatum OnIteratorGetKey (CDatum dThisType, CDatum dIterator) const override;
		virtual CDatum OnIteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dThisType, CDatum dIterator) const override;
		virtual CDatum OnIteratorNext (CDatum dIterator) const override { int iNext = (int)dIterator + 1; return (iNext < m_Entries.GetCount() ? CDatum(iNext) : CDatum()); }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CString GetLabel (const SEntry& Entry) { return (Entry.sLabel.IsEmpty() ? Entry.sID : Entry.sLabel); }

		TArray<SEntry> m_Entries;
		TSortMap<CString, int> m_EntriesByName;
	};

class CDatatypeFunction : public IDatatype
	{
	public:

		struct SCreate
			{
			CString sFullyQualifiedName;
			SReturnTypeDesc Return;
			TArray<SArgDesc> Args;
			};

		CDatatypeFunction (const SCreate& Create);

		explicit CDatatypeFunction (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

	private:

		struct SSigDesc
			{
			SReturnTypeDesc Return;
			TArray<SArgDesc> Args;
			};

		//	IDatatype virtuals

		virtual bool OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType = NULL, CString* retsError = NULL) const override;
		virtual bool OnCanBeCalledWithArgCount (CDatum dThisType, int iArgCount, CDatum* retdReturnType = NULL, CString* retsError = NULL) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Function; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Function; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override;
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual void OnMark () override;
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum CalcReturnType (const SSigDesc& Signature, CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes);
		bool CalcSignatureMatch (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, bool bCoerce = false, int* retiSignature = NULL) const;

		TArray<SSigDesc> m_Signatures;
		CRecursionState m_rs;
	};

class CDatatypeTensor : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatum dElementType;
			TArray<CDatum> Dimensions;
			};

		CDatatypeTensor (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeTensor (const SCreate &Create);
		CDatatypeTensor (CStringView sFullyQualifiedName, CDatum dElementType, int iRows, int iCols, DWORD dwCoreType = 0);

	private:

		struct SDimDesc
			{
			int iOrdinal = 0;
			CDatum dType;
			int iStart = 0;
			int iLength = 0;
			bool bEnum = false;
			};

		//	IDatatype virtuals

		virtual bool OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType = NULL, CString* retsError = NULL) const override;
		virtual bool OnCanBeConstructedFrom (CDatum dType) const override;
		virtual bool OnCanBeNull () const override { return true; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Tensor; }
		virtual TArray<CDatum> OnGetDimensionTypes () const override;
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Tensor; }
		virtual SMemberDesc OnGetMember (int iIndex) const override { if (iIndex != 0) throw CException(errFail); return SMemberDesc({ EMemberType::ArrayElement, NULL_STR, m_dElementType }); }
		virtual int OnGetMemberCount () const override { return 1; }
		virtual CString OnGetName () const override;
		virtual CDatum OnGetSliceType () const override { return m_dSliceType; }
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual void OnMark () override;
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		static CDatum CalcSliceType (CDatum dElementType, const TArray<SDimDesc>& Dims);
		void InitDims (int iRows, int iCols);
		void InitDimFromType (int iOrdinal, CDatum dType, SDimDesc& retDim);

		CDatum m_dElementType;
		CDatum m_dSliceType;	//	For 2D+ arrays, the slice type if indexing on first dimension
		TArray<SDimDesc> m_Dims;
	};

class CDatatypeNull : public IDatatype
	{
	public:
		CDatatypeNull (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName, IDatatype::NULL_T)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeNull () const override { return true; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override { return true; }
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override { return true; }
		virtual bool OnEquals (const IDatatype &Src) const override { return true; }
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Null; }
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual bool OnIsAbstract () const override { return false; }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { }
	};

class CDatatypeNullable : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatum dVariantType;
			};

		CDatatypeNullable (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeNullable (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType),
				m_dVariantType(Create.dVariantType)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeNull () const override { return true; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVersion, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Nullable; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Nullable; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override;
		virtual CString OnGetName () const override;
		virtual CDatum OnGetVariantType () const override { return m_dVariantType; }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual void OnMark () override { m_dVariantType.Mark(); }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dVariantType;
		CRecursionState m_rs;
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

			bool bSubRange = false;
			int iSubRangeMin = 0;
			int iSubRangeMax = 0;

			bool bAbstract = false;
			bool bCanBeNull = false;
			};

		CDatatypeNumber (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeNumber (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType),
				m_Implements(Create.Implements),
				m_iBits(Create.iBits),
				m_bFloat(Create.bFloat),
				m_bUnsigned(Create.bUnsigned),
				m_bSubRange(Create.bSubRange),
				m_iSubRangeMin(Create.iSubRangeMin),
				m_iSubRangeMax(Create.iSubRangeMax),
				m_bAbstract(Create.bAbstract),
				m_bCanBeNull(Create.bCanBeNull)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeConstructedFrom (CDatum dType) const override;
		virtual bool OnCanBeNull () const override { return m_bCanBeNull; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Number; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Number; }
		virtual CString OnGetName () const override;
		virtual SNumberDesc OnGetNumberDesc () const override;
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const { return EMemberType::None; }
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual bool OnIsAbstract () const override { return m_bAbstract; }
		virtual void OnMark () override { m_Implements.Mark(); }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatatypeList m_Implements;
		bool m_bAbstract = false;
		bool m_bCanBeNull = false;
		int m_iBits = 0;						//	0 = Infinite precision
		bool m_bFloat = false;
		bool m_bUnsigned = false;

		bool m_bSubRange = false;				//	Subrange of the full range
		int m_iSubRangeMin = 0;					//	For now, we only support subrange of Int32
		int m_iSubRangeMax = 0;
	};

class CDatatypeRange : public IDatatype
	{
	public:
		struct SCreate
			{
			CString sFullyQualifiedName;
			DWORD dwCoreType = 0;
			CDatum dBaseType;
			};

		CDatatypeRange (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType),
				m_dBaseType(Create.dBaseType)
			{ }

		explicit CDatatypeRange (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Range; }
		virtual EImplementation OnGetImplementation () const { return IDatatype::EImplementation::Range; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override { return 1; }
		virtual CDatum OnGetRangeType () const override;
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual void OnMark () override { m_dBaseType.Mark(); }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatum m_dBaseType;
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

		CDatatypeSchema (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType),
				m_Implements(Create.Implements)
			{ }

		explicit CDatatypeSchema (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

	private:

		struct SColumn
			{
			int iOrdinal = 0;
			CString sID;
			CDatum dType;
			EDisplay iDisplay = EDisplay::Default;
			CString sLabel;
			CString sFormat;

			bool bKey = false;
			};

		//	IDatatype virtuals

		virtual bool OnAddImplementation (CDatum dType) override;
		virtual bool OnAddMember (const SMemberDesc& Desc, CString *retsError = NULL) override;
		virtual bool OnCanBeConstructedFrom (CDatum dType) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Schema; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Schema; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override { return m_Columns.GetCount(); }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override { return m_Implements.IsA(Type); }
		virtual void OnMark () override;
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		TArray<SColumn> m_Columns;
		TSortMap<CString, int> m_ColumnsByName;
		CDatatypeList m_Implements;
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
			bool bCanBeNull = false;
			bool bNoMembers = false;
			};

		CDatatypeSimple (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

		CDatatypeSimple (const SCreate &Create) : IDatatype(Create.sFullyQualifiedName, Create.dwCoreType),
				m_Implements(Create.Implements),
				m_bAbstract(Create.bAbstract),
				m_bCanBeNull(Create.bCanBeNull),
				m_bNoMembers(Create.bNoMembers)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeCalledWith (CDatum dThisType, const TArray<CDatum>& ArgTypes, const TArray<CDatum>& ArgLiteralTypes, CDatum* retdReturnType = NULL, CString* retsError = NULL) const override;
		virtual bool OnCanBeNull () const override { return m_bCanBeNull; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override;
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override;
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Simple; }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override { return (m_bNoMembers ? EMemberType::None : EMemberType::DynamicMember); }
		virtual bool OnIsA (const IDatatype &Type) const override { return m_Implements.IsA(Type); }
		virtual bool OnIsAbstract () const override { return m_bAbstract; }
		virtual void OnMark () override { m_Implements.Mark(); }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;

		CDatatypeList m_Implements;
		bool m_bAbstract = false;
		bool m_bCanBeNull = false;
		bool m_bNoMembers = false;
	};

class CDatatypeString : public IDatatype
	{
	public:

		CDatatypeString (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName, IDatatype::STRING)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeNull () const override { return true; }
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override { return true; }
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override { return true; }
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Simple; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override;
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual bool OnIsAbstract () const override { return false; }
		virtual void OnMark () override { }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { }
	};

class CDatatypeAEON : public IDatatype
	{
	public:

		CDatatypeAEON (CStringView sFullyQualifiedName, DWORD dwCoreID, const CDatatypeList& Implements, CStringView sDatumTypename, TArray<SMemberDesc>&& Members = TArray<SMemberDesc>(), bool bCanBeNull = false);

		void SetMembers (TArray<SMemberDesc>&& Members) { m_Members = std::move(Members); }

	private:

		//	IDatatype virtuals

		virtual bool OnCanBeNull () const override { return m_bCanBeNull; }
		virtual CDatum OnCreateAsType (CDatum dValue) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override { return true; }
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override { return true; }
		virtual bool OnEquals (const IDatatype &Src) const override;
		virtual int OnFindMember (CStringView sName) const override;
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Simple; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Unknown; }
		virtual SMemberDesc OnGetMember (int iIndex) const override;
		virtual int OnGetMemberCount () const override { return m_Members.GetCount(); }
		virtual EMemberType OnHasMember (CStringView sName, CDatum* retdType = NULL, int* retiOrdinal = NULL) const override;
		virtual bool OnIsA (const IDatatype &Type) const override;
		virtual bool OnIsAbstract () const override { return false; }
		virtual void OnMark () override;
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { }

		CDatatypeList m_Implements;
		TArray<SMemberDesc> m_Members;
		bool m_bCanBeNull = false;

		CString m_sDatumTypename;
		IComplexFactory* m_pFactory = NULL;
	};

class CDatatypeUnknownCoreType : public IDatatype
	{
	public:
		CDatatypeUnknownCoreType (const CString &sFullyQualifiedName) : IDatatype(sFullyQualifiedName)
			{ }

	private:

		//	IDatatype virtuals

		virtual bool OnDeserialize (CDatum::EFormat iFormat, IByteStream &Stream, DWORD dwVersion) override { throw CException(errFail); }
		virtual bool OnDeserializeAEON (IByteStream& Stream, DWORD dwVerson, CAEONSerializedMap &Serialized) override { throw CException(errFail); }
		virtual bool OnEquals (const IDatatype &Src) const override { return strEquals(GetFullyQualifiedName(), Src.GetFullyQualifiedName()); }
		virtual ECategory OnGetClass () const override { return IDatatype::ECategory::Unknown; }
		virtual EImplementation OnGetImplementation () const override { return IDatatype::EImplementation::Unknown; }
		virtual void OnSerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override { throw CException(errFail); }
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

		static bool CreateFromStream (CCharStream& Stream, CDatum& retdDatum);
		static bool CreateFromStream (IByteStream& Stream, CDatum& retdDatum);

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { }
		virtual CString AsString (void) const override { return m_pType->GetName(); }
		virtual size_t CalcMemorySize (void) const override { return 0; }
		virtual const IDatatype &CastIDatatype (void) const override { return *m_pType; }
		virtual bool FindElement (const CString &sKey, CDatum *retpValue) const override { return m_Properties.FindProperty(sKey) != -1; }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::DATATYPE; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeDatatype; }
		virtual int GetCount (void) const override { return m_Properties.GetCount(); }
		virtual CDatum GetDatatype () const override { return CAEONTypeSystem::GetCoreType(IDatatype::DATATYPE); }
		virtual CDatum GetElement (const CString &sKey) const override { return m_Properties.GetProperty(*this, sKey); }
		virtual CDatum GetElement (int iIndex) const override { return m_Properties.GetProperty(*this, iIndex); }
		virtual CString GetKey (int iIndex) const override { return m_Properties.GetPropertyName(iIndex); }
		virtual CDatum GetMethod (const CString &sMethod) const override { return m_Methods.GetMethod(sMethod); }
		virtual const CString &GetTypename (void) const override;
		virtual bool IsArray (void) const override { return false; }
		virtual bool IsImmutable () const override { return true; }
		virtual bool IsNil (void) const override { return false; }
		virtual CDatum IteratorBegin () const override { return m_pType->IteratorBegin(); }
		virtual CDatum IteratorGetKey (CDatum dIterator) const override { return m_pType->IteratorGetKey(CDatum::raw_AsComplex(this), dIterator); }
		virtual CDatum IteratorGetValue (CAEONTypeSystem& TypeSystem, CDatum dIterator) const override { return m_pType->IteratorGetValue(TypeSystem, CDatum::raw_AsComplex(this), dIterator); }
		virtual CDatum IteratorNext (CDatum dIterator) const override { return m_pType->IteratorNext(dIterator); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const override;
		virtual void SetElement (const CString &sKey, CDatum dDatum) override { m_Properties.SetProperty(*this, sKey, dDatum, NULL); }

		static CDatum DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized);
		static int GetID (IDatatype::EImplementation iValue);
		static int GetImplCoreTypeID () { return IMPL_CORE_TYPE_ID; }
		static IDatatype::EImplementation GetImplementation (int iID, DWORD& retdwVersion);
		static TArray<IDatatype::SMemberDesc> GetMembers (void);
		static int FindPropertyByKey (const CString& sKey) { return m_Properties.FindProperty(sKey); }

	private:

		//	These values are used to encode the implementation class and save
		//	version when serializing. Do not change the values.

		static constexpr int IMPL_ANY_ID =				1;
		static constexpr int IMPL_ARRAY_ID =			2;
		static constexpr int IMPL_CLASS1_ID =			3;
		static constexpr int IMPL_NUMBER_ID =			4;
		static constexpr int IMPL_SCHEMA1_ID =			5;
		static constexpr int IMPL_SIMPLE_ID =			6;
		static constexpr int IMPL_ENUM1_ID =			7;	//	CDatatypeEnum v1
		static constexpr int IMPL_MATRIX_ID =			8;
		static constexpr int IMPL_ENUM2_ID =			9;	//	CDatatypeEnum v2
		static constexpr int IMPL_CORE_TYPE_ID =		10;
		static constexpr int IMPL_CLASS2_ID =			11;
		static constexpr int IMPL_SCHEMA2_ID =			12;
		static constexpr int IMPL_FUNCTION_ID =			13;
		static constexpr int IMPL_SERIALIZATION_V2 =	14;
		static constexpr int IMPL_ARRAY2_ID =			15;
		static constexpr int IMPL_ARRAY3_ID =			16;
		static constexpr int IMPL_NULLABLE_ID =			17;
		static constexpr int IMPL_TENSOR_ID =			18;
		static constexpr int IMPL_CLASS3_ID =			19;

		//	IComplexDatum

		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override;
		virtual bool OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream) override;
		virtual void OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const override;
		virtual void OnMarked (void) override { m_pType->Mark(); }

		TUniquePtr<IDatatype> m_pType;

		static TDatumPropertyHandler<CComplexDatatype> m_Properties;
		static TDatumMethodHandler<CComplexDatatype> m_Methods;
	};
