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
