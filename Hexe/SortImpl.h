//	SortImpl.h
//
//	Hexe header file
//	Copyright (c) 2014 by George Moromisato. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Hexe.h

#pragma once

class CSortFunctionProcessor : public TExternalDatum<CSortFunctionProcessor>
	{
	public:
		CSortFunctionProcessor (CDatum dList, CDatum dParams);
		static const CString &StaticGetTypename (void);

		bool Process (CDatum dSelf, CDatum *retResult);
		bool ProcessContinues (CDatum dSelf, CDatum dResult, CDatum *retResult);

	protected:
		virtual void OnMarked (void) override;

	private:
		enum EProcessType
			{
			processNone,

			processGetKeyValue,
			};

		struct SEntry
			{
			SEntry (void) : iIndex(-1)
				{ }

			CDatum dKey;
			int iIndex;
			CDatum dValue;
			};

		bool ProcessValueList (CDatum dSelf, CDatum *retResult);

		static int CompareEntry (void *pCtx, const SEntry &Key1, const SEntry &Key2) { return CDatum::Compare(Key1.dKey, Key2.dKey); }

		CDatum m_dList;
		CDatum m_dParams;

		EProcessType m_iProcess;			//	Current process
		ESortOptions m_iSort;				//	Sort (initialized in Process)
		CDatum m_dGetKeyValue;
		TArray<SEntry> m_ValueList;
		int m_iNext;						//	Next entry to fill
	};
