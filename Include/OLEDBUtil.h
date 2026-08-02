//	OLEDBUtil.h
//
//	OLEDB Utilities
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

#include "Foundation.h"
#include "AEON.h"
#include <atlbase.h>
#include <oledb.h>

DECLARE_CONST_STRING(STR_OLEDBUTIL_INVALID_PROVIDER_OR_SOURCE,	"Invalid provider or source.");
DECLARE_CONST_STRING(STR_OLEDBUTIL_UNABLE_TO_CREATE_SESSION,	"Unable to create session.");

enum class EOLEDBProvider
	{
	None =				-1,

	MSSQL =				0,
	SAS =				1,
	};

static constexpr int OLEDB_PROVIDER_COUNT = 2;

class COLEDBDataSource
	{
	public:

		struct SProviderStatus
			{
			EOLEDBProvider iProvider = EOLEDBProvider::None;
			bool bInstalled = false;
			CString sName;
			CString sStatus;
			};

		void Close ();
		EOLEDBProvider GetProvider () const { return m_iProvider; }
		bool GetTableColumns (CStringView sName, TArray<IDatatype::SMemberDesc>& retColumnInfo, CString *retsError = NULL);
		bool GetTableData (CStringView sName, const TArray<IDatatype::SMemberDesc>& Columns, CDatum& retdTable, CString *retsError = NULL);
		bool Matches (EOLEDBProvider iProvider, CStringView sDataSource) const;
		bool Open (EOLEDBProvider iProvider, CStringView sDataSource, CDatum dOptions, CString *retsError = NULL);

		static CDatum ConvertSASDate (double rValue);
		static CDatum ConvertSASDateTime (double rValue);
		static CDatum ConvertSASTime (double rValue);
		static CDatum MapSASFormat (CStringView sFormat);

	private:

		bool GetTableColumnsOLEDB (CStringView sName, TArray<IDatatype::SMemberDesc>& retColumnInfo, CString *retsError = NULL);
		bool GetTableColumnsSAS (CStringView sName, TArray<IDatatype::SMemberDesc>& retColumnInfo, CString *retsError = NULL);
		bool GetTableColumnSchema (CStringView sName, const TArray<IDatatype::SMemberDesc>& retColumnInfo, CDatum& retdData, CString *retsError = NULL);
		bool GetTableData (CStringView sName, IRowset& Rowset, const TArray<IDatatype::SMemberDesc>& Columns, CDatum& retdData, CString *retsError = NULL);
		static TArray<IDatatype::SMemberDesc> MapSASTypes (const TArray<IDatatype::SMemberDesc>& Columns);
		bool OpenSAS (EOLEDBProvider iProvider, CStringView sDataSource, CString *retsError = NULL);

		EOLEDBProvider m_iProvider = EOLEDBProvider::None;
		CString m_sDataSource;

		CComPtr<IDBInitialize> m_pIDBInitialize;

		IProgressEvents *m_pIO = NULL;
	};

class COLEDB
	{
	public:

		using CDataSourceID = void *;

		static constexpr int ORDCOL_TABLE_CATALOG = 1;
		static constexpr int ORDCOL_TABLE_SCHEMA = 2;
		static constexpr int ORDCOL_TABLE_NAME = 3;
		static constexpr int ORDCOL_COLUMN_NAME = 4;
		static constexpr int ORDCOL_COLUMN_GUID = 5;
		static constexpr int ORDCOL_COLUMN_PROPID = 6;
		static constexpr int ORDCOL_ORDINAL_POSITION = 7;
		static constexpr int ORDCOL_COLUMN_HAS_DEFAULT = 8;
		static constexpr int ORDCOL_COLUMN_DEFAULT = 9;
		static constexpr int ORDCOL_COLUMN_FLAGS = 10;
		static constexpr int ORDCOL_IS_NULLABLE = 11;
		static constexpr int ORDCOL_DATA_TYPE = 12;
		static constexpr int ORDCOL_TYPE_GUID = 13;
		static constexpr int ORDCOL_CHARACTER_MAXIMUM_LENGTH = 14;
		static constexpr int ORDCOL_CHARACTER_OCTET_LENGTH = 15;
		static constexpr int ORDCOL_NUMERIC_PRECISION = 16;
		static constexpr int ORDCOL_NUMERIC_SCALE = 17;
		static constexpr int ORDCOL_DATETIME_PRECISION = 18;
		static constexpr int ORDCOL_CHARACTER_SET_CATALOG = 19;
		static constexpr int ORDCOL_CHARACTER_SET_SCHEMA = 20;
		static constexpr int ORDCOL_CHARACTER_SET_NAME = 21;
		static constexpr int ORDCOL_COLLATION_CATALOG = 22;
		static constexpr int ORDCOL_COLLATION_SCHEMA = 23;
		static constexpr int ORDCOL_COLLATION_NAME = 24;
		static constexpr int ORDCOL_DOMAIN_CATALOG = 25;
		static constexpr int ORDCOL_DOMAIN_SCHEMA = 26;
		static constexpr int ORDCOL_DOMAIN_NAME = 27;
		static constexpr int ORDCOL_DESCRIPTION = 28;

		//	These are extended schema columns for SAS tables.
		//	See COLEDB for standard columns.

		static constexpr int ORDCOL_FORMAT_NAME =		29;
		static constexpr int ORDCOL_FORMAT_LENGTH =		30;
		static constexpr int ORDCOL_FORMAT_DECIMAL =	31;
		static constexpr int ORDCOL_INFORMAT_NAME =		32;
		static constexpr int ORDCOL_INFORMAT_LENGTH =	33;
		static constexpr int ORDCOL_INFORMAT_DECIMAL =	34;
		static constexpr int ORDCOL_SORT_ORDER =		35;
		static constexpr int ORDCOL_DBTYPE_INDEXED =	36;

		bool GetDataSource (EOLEDBProvider iProvider, CStringView sSource, CDatum dOptions, CDataSourceID& retID, CString *retsError = NULL);
		bool GetTableColumns (CDataSourceID ID, CStringView sTable, TArray<IDatatype::SMemberDesc>& retColumns, CString *retsError = NULL);
		bool GetTableData (CDataSourceID ID, CStringView sTable, const TArray<CString>& Columns, CDatum& retdTable, CString *retsError = NULL);

		static CString GetProviderName (EOLEDBProvider iType);
		static COLEDBDataSource::SProviderStatus GetProviderStatus (EOLEDBProvider iProvider);
		static bool GetProviderStatus (TArray<COLEDBDataSource::SProviderStatus> &retStatus);
		static DBTYPE MapToDBType (CDatum dType);
		static CDatum MapToType (DBTYPE iDBType);
		static EOLEDBProvider ParseProvider (CStringView sValue);

	private:

		struct SSession
			{
			SSession (EOLEDBProvider iProviderArg, CStringView sSourceArg) :
					ID(CDataSourceID(this)),
					iProvider(iProviderArg),
					sSource(sSourceArg)
				{
				if (iProviderArg == EOLEDBProvider::None)
					{
					bError = true;
					sErrorMsg = STR_OLEDBUTIL_INVALID_PROVIDER_OR_SOURCE;
					}
				else
					sIDKey = CreateSessionKey(iProviderArg, sSourceArg);
				}

			bool Busy (void) const { return (dwInUseBy != 0 && dwInUseBy != ::sysGetCurrentThreadID()); }
			void ClearBusy (void) { dwInUseBy = 0; }
			void SetBusy (void) { dwInUseBy = ::sysGetCurrentThreadID(); }

			CDataSourceID ID;				//	Unique ID of session (pointer to this structure)
			CString sIDKey;					//	Unique combination of provider, source, and options

			EOLEDBProvider iProvider = EOLEDBProvider::None;
			CString sSource;

			COLEDBDataSource DataSource;

			DWORD dwInUseBy = 0;			//	If non-zero, then session is being used by given thread
			bool bError = false;
			CString sErrorMsg;
			};

		class CSessionAccess
			{
			public:
				CSessionAccess (COLEDB& OLEDB, EOLEDBProvider iProvider, CStringView sSource, CDatum dOptions) :
						m_OLEDB(OLEDB),
						m_Session(OLEDB.GetSession(iProvider, sSource, dOptions))
					{ }

				CSessionAccess (COLEDB& OLEDB, CDataSourceID ID) :
						m_OLEDB(OLEDB),
						m_Session(OLEDB.GetSession(ID))
					{ }

				~CSessionAccess (void)
					{
					m_OLEDB.ReleaseSession(m_Session);
					}

				operator SSession& () { return m_Session; }
				operator const SSession& () const { return m_Session; }

				COLEDBDataSource& GetDataSource () { return m_Session.DataSource; }
				CDataSourceID& GetID () const { return m_Session.ID; }
				EOLEDBProvider GetProvider () const { return m_Session.iProvider; }

				bool IsError (CString *retsError = NULL) const
					{
					if (m_Session.bError)
						{
						if (retsError)
							{
							if (m_Session.sErrorMsg.IsEmpty())
								*retsError = STR_OLEDBUTIL_UNABLE_TO_CREATE_SESSION;
							else
								*retsError = m_Session.sErrorMsg;
							}

						return true;
						}
					else
						return false;
					}

			private:
				COLEDB& m_OLEDB;
				SSession& m_Session;
			};

		SSession* CreateSession (EOLEDBProvider iProvider, CStringView sSource, CDatum dOptions);
		static CString CreateSessionKey (EOLEDBProvider iType, CStringView sSource);
		SSession& GetSession (EOLEDBProvider iType, CStringView sSource, CDatum dOptions);
		SSession& GetSession (CDataSourceID ID);
		void ReleaseSession (SSession& Session);

		CCriticalSection m_cs;
		TSortMap<CString, TUniquePtr<SSession>> m_Sessions;

		static SSession m_ErrorSession;
	};

