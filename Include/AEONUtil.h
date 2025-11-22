//	AEONUtil.h
//
//	AEON Utilities
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

template <class OBJ>
class TDatumPropertyHandler
	{
	public:

		struct SDef
			{
			LPCSTR pProperty = NULL;
			LPCSTR pReturnType = NULL;
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

			const IDatatype& Schema = pTable->GetSchema();
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

		void AccumulateMembers (TArray<IDatatype::SMemberDesc>& retMembers) const
			{
			for (int i = 0; i < m_Table.GetCount(); i++)
				{
				IDatatype::EMemberType iType = (m_Table[i].fnSet ? IDatatype::EMemberType::InstanceProperty : IDatatype::EMemberType::InstanceReadOnlyProperty);
				retMembers.Insert(IDatatype::SMemberDesc({ iType, GetPropertyName(i), GetPropertyType(i) }));
				}
			}

		void AccumulateMembersMerge (TArray<IDatatype::SMemberDesc>& retMembers) const
			{
			for (int i = 0; i < m_Table.GetCount(); i++)
				{
				IDatatype::EMemberType iType = (m_Table[i].fnSet ? IDatatype::EMemberType::InstanceProperty : IDatatype::EMemberType::InstanceReadOnlyProperty);
				int iPos;
				if (IDatatype::FindMember(retMembers, GetPropertyName(i), iType, &iPos))
					retMembers[iPos] = IDatatype::SMemberDesc({ iType, GetPropertyName(i), GetPropertyType(i) });
				else
					retMembers.Insert(IDatatype::SMemberDesc({ iType, GetPropertyName(i), GetPropertyType(i) }));
				}
			}

		int FindProperty (const CString& sName) const
			{
			int iPos;
			if (!m_Table.FindPos(sName, &iPos))
				return -1;

			return iPos;
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

		bool GetProperty (const OBJ &Obj, const CString &sProperty, CDatum& retdValue) const
			{
			auto *pEntry = m_Table.GetAt(sProperty);
			if (!pEntry)
				return false;

			retdValue = pEntry->fnGet(Obj, sProperty);
			return true;
			}

		CString GetPropertyName (int iIndex) const
			{
			if (iIndex < 0 || iIndex >= m_Table.GetCount())
				throw CException(errFail);

			return CString(m_Table.GetKey(iIndex));
			}

		CDatum GetPropertyType (int iIndex) const
			{
			if (iIndex < 0 || iIndex >= m_Table.GetCount())
				throw CException(errFail);

			return CAEONTypes::CreatePropertyType(m_Table[iIndex].pReturnType);
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

		CDatum::EPropertyResult SetProperty (OBJ& Obj, const CString& sProperty, CDatum dValue, CString& retsError) const
			{
			auto *pEntry = m_Table.GetAt(sProperty);
			if (!pEntry)
				return CDatum::EPropertyResult::NotFound;

			if (!pEntry->fnSet)
				{
				retsError = strPattern("Property cannot be changed: %s.", sProperty);
				return CDatum::EPropertyResult::Error;
				}

			if (!pEntry->fnSet(Obj, sProperty, dValue, &retsError))
				return CDatum::EPropertyResult::Error;

			return CDatum::EPropertyResult::OK;
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
			DWORD dwExecFlags = 0;

			std::function<bool(OBJ &, IInvokeCtx &Ctx, const CString &, CHexeStackEnv&, CDatum, CDatum, SAEONInvokeResult &)> fnInvoke;
			std::function<bool(CDatum, CDatum, IInvokeCtx &Ctx, const CString &, SAEONInvokeResult &)> fnContinue;
			};

		TDatumMethodHandler (const std::initializer_list<SDef> &Table)
			{
			m_Methods.GrowToFit(Table.size());
			for (auto &entry : Table)
				{
				int iIndex = m_Methods.GetCount();
				auto pEntry = m_Methods.Insert();

				pEntry->sName = CString(entry.pMethod);
				pEntry->sArgs = CString(entry.pArgList);
				pEntry->sHelp = CString(entry.pHelp);
				pEntry->dwExecFlags = entry.dwExecFlags;
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

			const IDatatype& Schema = pTable->GetSchema();
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

		void AccumulateMembers (TArray<IDatatype::SMemberDesc>& retMembers) const
			{
			for (int i = 0; i < m_Table.GetCount(); i++)
				retMembers.Insert(IDatatype::SMemberDesc({ IDatatype::EMemberType::InstanceMethod, GetMethodName(i), GetMethodType(i) }));
			}

		void AccumulateMembersMerge (TArray<IDatatype::SMemberDesc>& retMembers) const
			{
			for (int i = 0; i < m_Table.GetCount(); i++)
				{
				int iPos;
				if (IDatatype::FindMember(retMembers, GetMethodName(i), IDatatype::EMemberType::InstanceMethod, &iPos))
					retMembers[iPos] = IDatatype::SMemberDesc({ IDatatype::EMemberType::InstanceMethod, GetMethodName(i), GetMethodType(i) });
				else
					retMembers.Insert(IDatatype::SMemberDesc({ IDatatype::EMemberType::InstanceMethod, GetMethodName(i), GetMethodType(i) }));
				}
			}

		int FindMethod (const CString& sMethod) const
			{
			int* pEntry = m_Table.GetAt(sMethod);
			if (pEntry)
				return *pEntry;
			else
				return -1;
			}

		int GetCount () const { return m_Methods.GetCount(); }

		CDatum GetMethod (int iIndex) const
			{
			ASSERT(iIndex >= 0 && iIndex < m_Table.GetCount());

			auto& Method = m_Methods[iIndex];
			if (Method.dFunc.IsNil())
				{
				SAEONLibraryFunctionCreate Create;
				Create.sName = Method.sName;
				Create.dwData = iIndex;
				Create.dwExecFlags = Method.dwExecFlags;

				Create.dType = CAEONTypes::CreateFunctionType(Method.sArgs);
				if (Create.dType.IsNil())
					Create.dType = CAEONTypes::Get(IDatatype::FUNCTION);

				Create.fnInvoke = [&Method](IInvokeCtx& Ctx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
					{
					if (dContinueCtx.IsNil())
						{
						//	First argument must be the object.

						CDatum dObj = LocalEnv.GetArgument(0);
						void* pObj = dObj.GetMethodThis();
						if (!pObj)
							{
							//	LATER: Invoke NULL methods.
							retResult.dResult = strPattern("Invalid object: %s.", dObj.AsString());
							return false;
							}

						return Method.fnInvoke(*(OBJ *)pObj, Ctx, Method.sName, LocalEnv, dContinueCtx, dContinueResult, retResult);
						}
					else if (Method.fnContinue)
						{
						return Method.fnContinue(dContinueCtx, dContinueResult, Ctx, Method.sName, retResult);
						}
					else
						{
						retResult.dResult = strPattern("Continue handler expected: %s", Method.sName);
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

		bool GetMethod (const CString& sMethod, CDatum& retdMethod) const
			{
			int* pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return false;

			retdMethod = GetMethod(*pEntry);
			return true;
			}

		CDatum GetMethod (const CString &sMethod) const
			{
			auto* pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return CDatum();

			return GetMethod(*pEntry);
			}

		CString GetMethodName (int iIndex) const
			{
			ASSERT(iIndex >= 0 && iIndex < m_Table.GetCount());
			return m_Methods[iIndex].sName;
			}

		CDatum GetMethodType (int iIndex) const
			{
			ASSERT(iIndex >= 0 && iIndex < m_Table.GetCount());
			return GetMethod(iIndex).GetDatatype();
			}

		CDatum GetStaticMethod (const CString &sMethod) const
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
				Create.dwExecFlags = Method.dwExecFlags;

				//	LATER: This should be a fully defined type.
				Create.dType = CAEONTypes::Get(IDatatype::FUNCTION);

				Create.fnInvoke = [&Method](IInvokeCtx& Ctx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
					{
					OBJ Dummy;
					return Method.fnInvoke(Dummy, Ctx, Method.sName, LocalEnv, dContinueCtx, dContinueResult, retResult);
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

		bool InvokeMethod (CDatum dObj, int iIndex, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			ASSERT(iIndex >= 0 && iIndex < m_Table.GetCount());

			void* pObj = dObj.GetMethodThis();
			if (!pObj)
				{
				//	LATER: Invoke NULL methods.
				retResult.dResult = strPattern("Invalid object: %s.", dObj.AsString());
				return false;
				}

			return m_Methods[iIndex].fnInvoke(*(OBJ *)pObj, Ctx, m_Methods[iIndex].sName, LocalEnv, dContinueCtx, dContinueResult, retResult);
			}

		bool InvokeMethod (CDatum dObj, const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				{
				retResult.dResult = strPattern("Undefined method: %s.", sMethod);
				return false;
				}

			return InvokeMethod(dObj, *pEntry, Ctx, LocalEnv, dContinueCtx, dContinueResult, retResult);
			}

		bool InvokeMethod (CDatum dObj, const CString& sMethod, IInvokeCtx& Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, bool& retbResult, SAEONInvokeResult& retResult)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				return false;

			retbResult = InvokeMethod(dObj, *pEntry, Ctx, LocalEnv, dContinueCtx, dContinueResult, retResult);
			return true;
			}

		bool InvokeStaticMethod (const CString &sMethod, IInvokeCtx &Ctx, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
			{
			auto *pEntry = m_Table.GetAt(sMethod);
			if (!pEntry)
				{
				retResult.dResult = strPattern("Undefined method: %s.", sMethod);
				return false;
				}

			OBJ Dummy;
			return m_Methods[*pEntry].fnInvoke(Dummy, Ctx, sMethod, LocalEnv, dContinueCtx, dContinueResult, retResult);
			}

	private:

		struct SEntry
			{
			CString sName;
			CString sArgs;
			DWORD dwExecFlags = 0;
			CString sHelp;
			std::function<bool(OBJ &, IInvokeCtx &Ctx, const CString &, CHexeStackEnv&, CDatum, CDatum, SAEONInvokeResult &)> fnInvoke;
			std::function<bool(CDatum, CDatum, IInvokeCtx &Ctx, const CString &, SAEONInvokeResult &)> fnContinue;

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

class CDatumFormat
	{
	public:

		static CDatum AsDatum (const CStringFormat& Format);
		static CString FormatDateTime (const CDateTime& Value, const CString& sFormat) { return CStringFormat(sFormat).FormatDateTime(Value); }
		static CString FormatDouble (double rValue, const CString& sFormat) { return CStringFormat(sFormat).FormatDouble(rValue); }
		static CString FormatInteger (int iValue, const CString& sFormat) { return CStringFormat(sFormat).FormatInteger(iValue); }
		static CString FormatIPInteger (const CIPInteger& Value, const CString& sFormat) { return CStringFormat(sFormat).FormatIPInteger(Value); }
		static CString FormatNull (CStringView sFormat) { return CStringFormat(sFormat).FormatNull(); }
		static CString FormatParams (CStringView sValue, CDatum dParams);
		static CString FormatParamsByOrdinal (CStringView sValue, CHexeStackEnv& LocalEnv);
		static CString FormatTimeSpan (const CTimeSpan& Value, CStringView sFormat) { return CStringFormat(sFormat).FormatTimeSpan(Value); }

	private:
	};

class CDatumInterpret
	{
	public:

		struct SOptions
			{
			bool bBigEndian = false;
			};

		static int CalcSizeOf (const IDatatype& Type);
		static bool EncodeAs (BYTE* pPos, BYTE* pEndPos, CDatum dType, CDatum dValue, const SOptions& Options, BYTE** retpNewPos = NULL, CString* retsError = NULL);
		static bool InterpretAs (const BYTE* pPos, const BYTE* pEndPos, CDatum dType, const SOptions& Options, CDatum& retdValue, const BYTE** retpNewPos = NULL, CString* retsError = NULL);
		static bool ParseOptions (CDatum dOptions, SOptions& retOptions);

	private:

		static bool CheckLength (const BYTE* pPos, const BYTE* pEndPos, size_t iLength, CString* retsError = NULL);
	};

struct CDatumHasher {
	std::size_t operator()(const CDatum& d) const noexcept { return d.Hash(); }
};

struct CDatumEqual {
	bool operator()(const CDatum& a, const CDatum& b) const noexcept { return a.OpIsIdentical(b); }
};

class CDatumStringTable
	{
	public:

		CDatumStringTable (int iCount = 0);

		DWORD AddString (CDatum dString);
		int GetCount () const { return m_Strings.GetCount(); }
		DWORD GetIDByIndex (int iIndex) const;
		CDatum GetStringByIndex (int iIndex) const;
		CDatum GetString (DWORD dwID) const;
		const TArray<CDatum> GetStringTable () const { return m_Strings; }
		void GrowToFit (int iCount);

	private:

		TArray<CDatum> m_Strings;
		std::unordered_map<CDatum, DWORD, CDatumHasher, CDatumEqual> m_StringToID;
	};