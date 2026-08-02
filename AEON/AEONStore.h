//	AEONStore.h
//
//	AEON Class Implementations
//	Copyright (c) 2026 GridWhale Corporation. All Rights Reserved.

#pragma once

class CAEONRecord;

class CAEONStore
	{
	public:

		static DWORD Alloc (IComplexDatum* pValue) { return m_ComplexAlloc.New(pValue); }
		static DWORD Alloc (LPSTR pValue) { return m_StringAlloc.New(pValue); }
		static CDatum CreateRecord (CDatum dType);
		static CDatum CreateRecord (CDatum dType, const CDatum* pValues, int iCount);

		static DWORD AllocTableID (CDatum dTable) { return m_TableTable.AddTable(dTable); }
		static void FreeTableID (DWORD dwID) { m_TableTable.DeleteTable(dwID); }
		static CDatum GetTableByID (DWORD dwID) { return m_TableTable.GetTable(dwID); }

		static void MarkComplex (IComplexDatum* pValue) { m_ComplexAlloc.Mark(pValue); }
		static void MarkString (LPSTR Value) { m_StringAlloc.Mark(Value); }

		static void RegisterMarkProc (MARKPROC fnProc) { m_MarkList.Insert(fnProc); }

		static void Sweep ();

	private:

		static CGCStringAllocator m_StringAlloc;
		static CGCComplexAllocator m_ComplexAlloc;
		static TGCSlabAllocator<CAEONRecord> m_RecordAlloc;

		static TArray<MARKPROC> m_MarkList;
		static CAEONTableTable m_TableTable;
	};


