//	HexeUtil.h
//
//	Hexe EXecution Environment
//	Copyright (c) 2022 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

template <class OBJ>
class TDatumMethodHandler
	{
	public:
		struct SDef
			{
			LPCSTR pMethod = NULL;
			LPCSTR pArgList = NULL;
			LPCSTR pHelp = NULL;
			DWORD dwExecutionRights = 0;

			std::function<bool(OBJ &, IInvokeCtx &Ctx, const CString &, CDatum, CDatum, CDatum &)> fnInvoke;
			};

		TDatumMethodHandler (const std::initializer_list<SDef> &Table)
			{
			m_Methods.GrowToFit(Table.size());
			for (auto &entry : Table)
				{
				int iIndex = m_Methods.GetCount();
				auto pEntry = m_Methods.Insert();
				pEntry->sName = CString(entry.pMethod);

				pEntry->Def.pName = entry.pMethod;
				pEntry->Def.pfFunc = InvokeThunk;
				pEntry->Def.dwData = iIndex;
				pEntry->Def.pArgList = entry.pArgList;
				pEntry->Def.pHelp = entry.pHelp;
				pEntry->Def.dwExecutionRights = entry.dwExecutionRights;

				pEntry->fnInvoke = entry.fnInvoke;

				m_Table.SetAt(pEntry->sName, iIndex);
				}
			}

		void AccumulateDocumentation (CDatum dTable, const CString& sLibrary, const CString& sComponent) const
			{
			DECLARE_CONST_STRING(FIELD_COMPONENT,					"component");
			DECLARE_CONST_STRING(FIELD_LIBRARY,						"library");
			DECLARE_CONST_STRING(FIELD_TEXT,						"text");
			DECLARE_CONST_STRING(FIELD_TITLE,						"title");
			DECLARE_CONST_STRING(FIELD_TYPE,						"type");
			DECLARE_CONST_STRING(FIELD_USAGE,						"usage");

			DECLARE_CONST_STRING(TYPE_METHOD,						"method");

			IAEONTable *pTable = dTable.GetTableInterface();
			if (!pTable)
				return;

			const IDatatype& Schema = dTable.GetDatatype();
			const int LIBRARY_INDEX = Schema.FindMember(FIELD_LIBRARY);
			const int COMPONENT_INDEX = Schema.FindMember(FIELD_COMPONENT);
			const int TITLE_INDEX = Schema.FindMember(FIELD_TITLE);
			const int TYPE_INDEX = Schema.FindMember(FIELD_TYPE);
			const int USAGE_INDEX = Schema.FindMember(FIELD_USAGE);
			const int TEXT_INDEX = Schema.FindMember(FIELD_TEXT);

			int iStart = pTable->GetRowCount();
			pTable->AppendEmptyRow(m_Methods.GetCount());
			for (int i = 0; i < m_Methods.GetCount(); i++)
				{
				auto& Entry = m_Methods[i];

				if (LIBRARY_INDEX != -1)	pTable->SetFieldValue(iStart + i, LIBRARY_INDEX, sLibrary);
				if (COMPONENT_INDEX != -1)	pTable->SetFieldValue(iStart + i, COMPONENT_INDEX, sComponent);
				if (TITLE_INDEX != -1)		pTable->SetFieldValue(iStart + i, TITLE_INDEX, Entry.sName);
				if (TYPE_INDEX != -1)		pTable->SetFieldValue(iStart + i, TYPE_INDEX, TYPE_METHOD);
				if (TEXT_INDEX != -1)		pTable->SetFieldValue(iStart + i, TEXT_INDEX, CString(Entry.Def.pHelp));
				}
			}

		CDatum GetMethod (const CString &sMethod)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return CDatum();

			if (m_Methods[*pEntry].dFunc.IsNil())
				{
				m_Methods[*pEntry].dFunc = CHexeLibrarian::CreateFunction(m_Methods[*pEntry].Def);

				if (!m_bMarkProcRegistered)
					{
					CDatum::RegisterMarkProc(MarkProc);
					m_bMarkProcRegistered = true;
					}
				}

			return m_Methods[*pEntry].dFunc;
			}

		bool InvokeMethod (CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum dContinueCtx, CDatum &retdResult)
			{
			IComplexDatum *pObj = dObj.GetComplex();
			if (!pObj)
				{
				retdResult = strPattern("Invalid object: %s.", dObj.AsString());
				return false;
				}

			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				{
				retdResult = strPattern("Undefined method: %s.", sMethod);
				return false;
				}

			return m_Methods[*pEntry].fnInvoke(*(OBJ *)pObj, Ctx, sMethod, dLocalEnv, dContinueCtx, retdResult);
			}

	private:
		struct SEntry
			{
			CString sName;
			CHexeLibrarian::SFunctionDef Def;
			CDatum dFunc;

			std::function<bool(OBJ &, IInvokeCtx &Ctx, const CString &, CDatum, CDatum, CDatum &)> fnInvoke;
			};

		static bool InvokeThunk (IInvokeCtx *pCtx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum *retdResult)
			{
			//	First argument must be the object.

			CDatum dObj = dLocalEnv.GetElement(0);
			IComplexDatum *pObj = dObj.GetComplex();
			if (!pObj)
				{
				*retdResult = strPattern("Invalid object: %s.", dObj.AsString());
				return false;
				}

			if (dwData >= (DWORD)m_Methods.GetCount())
				{
				*retdResult = strPattern("Invalid library function index: %x.", dwData);
				return false;
				}

			return m_Methods[dwData].fnInvoke(*(OBJ *)pObj, *pCtx, m_Methods[dwData].sName, dLocalEnv, dContinueCtx, *retdResult);
			}

		static void MarkProc ()
			{
			for (int i = 0; i < m_Methods.GetCount(); i++)
				m_Methods[i].dFunc.Mark();
			}

		TSortMap<LPCSTR, int> m_Table;

		static TArray<SEntry> m_Methods;
		static bool m_bMarkProcRegistered;
	};

template <class OBJ> TArray<struct TDatumMethodHandler<OBJ>::SEntry> TDatumMethodHandler<OBJ>::m_Methods;
template <class OBJ> bool TDatumMethodHandler<OBJ>::m_bMarkProcRegistered = false;
