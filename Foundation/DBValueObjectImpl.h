//	DBValueObjectImpl.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

class CDBValueArray : public IDBValueObject
	{
	public:
		CDBValueArray (void) { }
		CDBValueArray (const TArray<CDBValue> &Src) : m_Array(Src) { }

		virtual IDBValueObject *Clone (void) const override { return new CDBValueArray(m_Array); }

	private:
		TArray<CDBValue> m_Array;
	};

class CDBValueDouble : public IDBValueObject
	{
	public:
		CDBValueDouble (double rValue = 0.0) :
				m_rValue(rValue)
			{ }

		virtual IDBValueObject *Clone (void) const override { return new CDBValueDouble(m_rValue); }

	private:
		double m_rValue;
	};

class CDBValueStruct : public IDBValueObject
	{
	public:
		CDBValueStruct (void) { }
		CDBValueStruct (const TSortMap<CString, CDBValue> &Src) : m_Table(Src) { }

		virtual IDBValueObject *Clone (void) const override { return new CDBValueStruct(m_Table); }

	private:
		TSortMap<CString, CDBValue> m_Table;
	};
