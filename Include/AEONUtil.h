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

