//	GridLangLibraries.h
//
//	GridLang Classes
//	Copyright (c) 2022 GridWhale Corporation. All Rights Reserved.

#pragma once

class CGridLangCoreLibrary
	{
	public:

		enum class Op
			{
			unknown			= -1,

			//	NOTE: These values must match the enum ordinals as defined in
			//	CGridLangCoreLibrary::Register

			arrayValues		= 0,
			average			= 1,
			count			= 2,
			first			= 3,
			max				= 4,
			median			= 5,
			min				= 6,
			sum				= 7,
			unique			= 8,
			uniqueCount		= 9,
			};

//		static void Define (const IGLType &IsA, CGLTypeNamespace &Namespace);
		static TSharedPtr<IASTNode> GetDefinitions ();
		static CDatum CreateDiffResult (const CArrayDiff::Results& Results);
		static void Register ();

		static bool Impl_ArrayCons (IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);
		static CDatum Print (CDatum dValue);

		static DWORD AGGREGATION_ENUM;
		static DWORD DIFF_RESULT_ENUM;
		static DWORD DIFF_RESULT_SCHEMA;
		static DWORD SYSTEM_INFO_SCHEMA;

		static Op GetOpFromDatum (CDatum dOp);

	private:

		//	These must match the ordinal for DIFF_RESULT_ENUM

		static constexpr int DIFF_RESULT_DELETED = 0;
		static constexpr int DIFF_RESULT_INSERTED = 1;
		static constexpr int DIFF_RESULT_MODIFIED = 2;

		static bool IsArrayOptionStruct (CDatum dValue);

		static bool m_bRegistered;

		static TDatumMethodHandler<IComplexDatum> m_ArrayMethodsExt;
		static TDatumMethodHandler<IComplexDatum> m_DateTimeMethodsExt;
		static TDatumMethodHandler<IComplexDatum> m_DictionaryMethodsExt;
		static TDatumMethodHandler<IComplexDatum> m_StructMethodsExt;
		static TDatumMethodHandler<IComplexDatum> m_TableMethodsExt;
		static TDatumMethodHandler<IComplexDatum> m_TensorMethodsExt;
	};

class CGridLangMathLibrary
	{
	public:
		static TSharedPtr<IASTNode> GetDefinitions ();
		static void Register ();

	private:
		static bool m_bRegistered;

		static std::initializer_list<CHexeLibrarian::SConstDoubleDef> m_ConstDefs;
	};

class CGLCoreMacroLibrary
	{
	public:
		static CGLMacroHandler m_Handlers;
	};

class CGridLangLibraries
	{
	public:
		struct SLibrary
			{
			CString sName;
			mutable TSharedPtr<IASTNode> pDefinitions;
			};

		CGridLangLibraries ();

		void AddLibrary (TSharedPtr<IASTNode> pDefinitions) { AddLibrary(pDefinitions->GetName(), pDefinitions->GetName(), pDefinitions); }
		void AddLibrary (const CString &sID, const CString& sName, TSharedPtr<IASTNode> pDefinitions);
		CDatum AsTable (const CString& sNodeID) const;
		const SLibrary *FindLibrary (const CString &sName) const;
		static CDatum FindOrCreateDocumentationSchema (CAEONTypeSystem &Types);
		CDatum GetDocumentation (CAEONTypeSystem &Types, const CString &sName) const;
		int GetCount () const { return m_Libraries.GetCount(); }
		TSharedPtr<IASTNode> GetLibrary (int iIndex) const { if (iIndex >= 0 && iIndex < m_Libraries.GetCount()) return m_Libraries[iIndex].pDefinitions; else throw CException(errFail); }
		TArray<CString> GetLibraries () const;
		bool IsEmpty () const { return m_Libraries.GetCount() == 0; }
		void Mark ();

	private:

		static void AddDefinitions (CDatum dTable, const SLibrary &Library);

		TSortMap<CString, SLibrary> m_Libraries;
	};

