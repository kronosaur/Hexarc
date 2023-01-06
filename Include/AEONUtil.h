//	AEONUtil.h
//
//	AEON Utilities
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

template <class OBJ>
class TDatumPropertyHandler
	{
	public:
		struct SDef
			{
			LPCSTR pProperty = NULL;
			LPCSTR pShortDesc = NULL;
			std::function<CDatum(const OBJ &, const CString &)> fnGet;
			std::function<bool(OBJ &, const CString &, CDatum, CString *)> fnSet;
			};

		TDatumPropertyHandler (const std::initializer_list<SDef> &Table)
			{
			m_Table.GrowToFit(Table.size());
			for (auto &entry : Table)
				m_Table.SetAt(entry.pProperty, entry);
			}

		void AccumulateDocumentation (CDatum dTable, const CString& sLibrary, const CString& sComponent) const
			{
			DECLARE_CONST_STRING(FIELD_COMPONENT,					"component");
			DECLARE_CONST_STRING(FIELD_LIBRARY,						"library");
			DECLARE_CONST_STRING(FIELD_TEXT,						"text");
			DECLARE_CONST_STRING(FIELD_TITLE,						"title");
			DECLARE_CONST_STRING(FIELD_TYPE,						"type");
			DECLARE_CONST_STRING(FIELD_USAGE,						"usage");

			DECLARE_CONST_STRING(TYPE_PROPERTY,						"property");
			DECLARE_CONST_STRING(TYPE_READ_ONLY_PROPERTY,			"read-only property");
			DECLARE_CONST_STRING(TYPE_UNDEFINED,					"undefined");
			DECLARE_CONST_STRING(TYPE_WRITE_ONLY_PROPERTY,			"write-only property");

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
			pTable->AppendEmptyRow(GetCount());
			for (int i = 0; i < m_Table.GetCount(); i++)
				{
				auto& Entry = m_Table[i];

				CString sType;
				if (Entry.fnGet && Entry.fnSet)
					sType = TYPE_PROPERTY;
				else if (Entry.fnGet)
					sType = TYPE_READ_ONLY_PROPERTY;
				else if (Entry.fnSet)
					sType = TYPE_WRITE_ONLY_PROPERTY;
				else
					sType = TYPE_UNDEFINED;

				if (LIBRARY_INDEX != -1)	pTable->SetFieldValue(iStart + i, LIBRARY_INDEX, sLibrary);
				if (COMPONENT_INDEX != -1)	pTable->SetFieldValue(iStart + i, COMPONENT_INDEX, sComponent);
				if (TITLE_INDEX != -1)		pTable->SetFieldValue(iStart + i, TITLE_INDEX, CString(Entry.pProperty));
				if (TYPE_INDEX != -1)		pTable->SetFieldValue(iStart + i, TYPE_INDEX, sType);
				if (TEXT_INDEX != -1)		pTable->SetFieldValue(iStart + i, TEXT_INDEX, CString(Entry.pShortDesc));
				}
			}

		CDatum GetProperty (const OBJ &Obj, const CString &sProperty) const
			{
			auto *pEntry = m_Table.GetAt(sProperty);
			if (!pEntry)
				return CDatum();

			return pEntry->fnGet(Obj, sProperty);
			}

		CDatum GetProperty (const OBJ &Obj, int iIndex) const
			{
			if (iIndex < 0 || iIndex >= m_Table.GetCount())
				throw CException(errFail);

			return m_Table[iIndex].fnGet(Obj, m_Table.GetKey(iIndex));
			}

		CString GetPropertyName (int iIndex) const
			{
			if (iIndex < 0 || iIndex >= m_Table.GetCount())
				throw CException(errFail);

			return CString(m_Table.GetKey(iIndex));
			}

		bool SetProperty (OBJ &Obj, const CString &sProperty, CDatum dValue, CString *retsError) const
			{
			auto *pEntry = m_Table.GetAt(sProperty);
			if (!pEntry)
				{
				if (retsError) *retsError = strPattern("Unknown property: %s.", sProperty);
				return false;
				}

			if (!pEntry->fnSet)
				{
				if (retsError) *retsError = strPattern("Property cannot be changed: %s.", sProperty);
				return false;
				}

			return pEntry->fnSet(Obj, sProperty, dValue, retsError);
			}

		int GetCount () const { return m_Table.GetCount(); }

	private:
		TSortMap<LPCSTR, SDef> m_Table;
	};

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
			std::function<bool(CDatum, IInvokeCtx &Ctx, const CString &, CDatum, CDatum &)> fnContinue;
			};

		TDatumMethodHandler (const std::initializer_list<SDef> &Table)
			{
			m_Methods.GrowToFit(Table.size());
			for (auto &entry : Table)
				{
				int iIndex = m_Methods.GetCount();
				auto pEntry = m_Methods.Insert();

				pEntry->sName = CString(entry.pMethod);
				pEntry->sHelp = CString(entry.pHelp);
				pEntry->dwExecutionRights = entry.dwExecutionRights;
				pEntry->fnInvoke = entry.fnInvoke;
				pEntry->fnContinue = entry.fnContinue;

				bool bNew;
				m_Table.SetAt(pEntry->sName, iIndex, &bNew);
				if (!bNew)
					throw CException(errFail);	//	Duplicate method in table.
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
				if (TEXT_INDEX != -1)		pTable->SetFieldValue(iStart + i, TEXT_INDEX, Entry.sHelp);
				}
			}

		int FindMethod (const CString& sMethod)
			{
			int* pEntry = m_Table.GetAt(sMethod);
			if (pEntry)
				return *pEntry;
			else
				return -1;
			}

		CDatum GetMethod (int iIndex)
			{
			ASSERT(iIndex >= 0 && iIndex < m_Table.GetCount());

			auto& Method = m_Methods[iIndex];
			if (Method.dFunc.IsNil())
				{
				SAEONLibraryFunctionCreate Create;
				Create.sName = Method.sName;
				Create.dwData = iIndex;
				Create.dwExecutionRights = Method.dwExecutionRights;

				//	LATER: This should be a fully defined type.
				Create.dType = CAEONTypes::Get(IDatatype::FUNCTION);

				Create.fnInvoke = [&Method](IInvokeCtx& Ctx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
					{
					if (dContinueCtx.IsNil())
						{
						//	First argument must be the object.

						CDatum dObj = dLocalEnv.GetElement(0);
						void* pObj = dObj.GetMethodThis();
						if (!pObj)
							{
							//	LATER: Invoke NULL methods.
							retdResult = strPattern("Invalid object: %s.", dObj.AsString());
							return false;
							}

						return Method.fnInvoke(*(OBJ *)pObj, Ctx, Method.sName, dLocalEnv, dContinueCtx, retdResult);
						}
					else if (Method.fnContinue)
						{
						return Method.fnContinue(dContinueCtx, Ctx, Method.sName, dLocalEnv, retdResult);
						}
					else
						{
						retdResult = strPattern("Continue handler expected: %s", Method.sName);
						return false;
						}
					};

				Method.dFunc = CDatum::CreateLibraryFunction(Create);

				if (!m_bMarkProcRegistered)
					{
					CDatum::RegisterMarkProc(MarkProc);
					m_bMarkProcRegistered = true;
					}

				m_Mark.Insert(Method.dFunc);
				}

			return Method.dFunc;
			}

		bool GetMethod (const CString& sMethod, CDatum& retdMethod)
			{
			int* pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return false;

			retdMethod = GetMethod(*pEntry);
			return true;
			}

		CDatum GetMethod (const CString &sMethod)
			{
			auto* pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return CDatum();

			return GetMethod(*pEntry);
			}

		CDatum GetStaticMethod (const CString &sMethod)
			{
			auto* pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return CDatum();

			auto& Method = m_Methods[*pEntry];
			if (Method.dFunc.IsNil())
				{
				SAEONLibraryFunctionCreate Create;
				Create.sName = Method.sName;
				Create.dwData = *pEntry;
				Create.dwExecutionRights = Method.dwExecutionRights;

				//	LATER: This should be a fully defined type.
				Create.dType = CAEONTypes::Get(IDatatype::FUNCTION);

				Create.fnInvoke = [&Method](IInvokeCtx& Ctx, DWORD dwData, CDatum dLocalEnv, CDatum dContinueCtx, CDatum& retdResult)
					{
					OBJ Dummy;
					return Method.fnInvoke(Dummy, Ctx, Method.sName, dLocalEnv, dContinueCtx, retdResult);
					};

				Method.dFunc = CDatum::CreateLibraryFunction(Create);

				if (!m_bMarkProcRegistered)
					{
					CDatum::RegisterMarkProc(MarkProc);
					m_bMarkProcRegistered = true;
					}

				m_Mark.Insert(Method.dFunc);
				}

			return Method.dFunc;
			}

		bool InvokeMethod (CDatum dObj, int iIndex, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum dContinueCtx, CDatum &retdResult)
			{
			ASSERT(iIndex >= 0 && iIndex < m_Table.GetCount());

			void* pObj = dObj.GetMethodThis();
			if (!pObj)
				{
				//	LATER: Invoke NULL methods.
				retdResult = strPattern("Invalid object: %s.", dObj.AsString());
				return false;
				}

			return m_Methods[iIndex].fnInvoke(*(OBJ *)pObj, Ctx, m_Methods[iIndex].sName, dLocalEnv, dContinueCtx, retdResult);
			}

		bool InvokeMethod (CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum dContinueCtx, CDatum &retdResult)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				{
				retdResult = strPattern("Undefined method: %s.", sMethod);
				return false;
				}

			return InvokeMethod(dObj, *pEntry, Ctx, dLocalEnv, dContinueCtx, retdResult);
			}

		bool InvokeMethod (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CDatum dLocalEnv, CDatum dContinueCtx, bool& retbResult, CDatum& retdResult)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return false;

			retbResult = InvokeMethod(dObj, *pEntry, Ctx, dLocalEnv, dContinueCtx, retdResult);
			return true;
			}

		bool InvokeStaticMethod (const CString &sMethod, IInvokeCtx &Ctx, CDatum dLocalEnv, CDatum dContinueCtx, CDatum &retdResult)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				{
				retdResult = strPattern("Undefined method: %s.", sMethod);
				return false;
				}

			OBJ Dummy;
			return m_Methods[*pEntry].fnInvoke(Dummy, Ctx, sMethod, dLocalEnv, dContinueCtx, retdResult);
			}

	private:
		struct SEntry
			{
			CString sName;
			DWORD dwExecutionRights = 0;
			CString sHelp;
			std::function<bool(OBJ &, IInvokeCtx &Ctx, const CString &, CDatum, CDatum, CDatum &)> fnInvoke;
			std::function<bool(CDatum, IInvokeCtx &Ctx, const CString &, CDatum, CDatum &)> fnContinue;

			CDatum dFunc;
			};

		static void MarkProc ()
			{
			for (int i = 0; i < m_Mark.GetCount(); i++)
				m_Mark[i].Mark();
			}

		TSortMap<LPCSTR, int> m_Table;

		TArray<SEntry> m_Methods;
		static bool m_bMarkProcRegistered;
		static TArray<CDatum> m_Mark;
	};

template <class OBJ> bool TDatumMethodHandler<OBJ>::m_bMarkProcRegistered = false;
template <class OBJ> TArray<CDatum> TDatumMethodHandler<OBJ>::m_Mark;
