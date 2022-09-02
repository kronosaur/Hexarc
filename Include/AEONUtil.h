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

