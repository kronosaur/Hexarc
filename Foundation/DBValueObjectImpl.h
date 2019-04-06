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
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeArray; }

	private:
		TArray<CDBValue> m_Array;
	};

class CDBValueDateTime : public IDBValueObject
	{
	public:
		CDBValueDateTime (const CDateTime &Value = NULL_DATETIME) :
				m_Value(Value)
			{ }

		virtual CDateTime AsDateTime (void) const { return m_Value; }
		virtual CString AsString (void) const { return m_Value.FormatIMF(); }
		virtual const CDateTime &CastDateTime (void) const { return m_Value; }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueDateTime(m_Value); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeDateTime; }

	private:
		CDateTime m_Value;
	};

class CDBValueDouble : public IDBValueObject
	{
	public:
		CDBValueDouble (double rValue = 0.0) :
				m_rValue(rValue)
			{ }

		virtual CString AsString (void) const { return strFromDouble(m_rValue); }
		virtual double CastDouble (void) const override { return m_rValue; }
		virtual LONGLONG CastLONGLONG (void) const { return (LONGLONG)m_rValue; }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueDouble(m_rValue); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeDouble; }

	private:
		double m_rValue;
	};

class CDBValueStruct : public IDBValueObject
	{
	public:
		CDBValueStruct (void) { }
		CDBValueStruct (const TSortMap<CString, CDBValue> &Src) : m_Table(Src) { }

		virtual IDBValueObject *Clone (void) const override { return new CDBValueStruct(m_Table); }
		virtual const CDBValue &GetElement (const CString &sKey) const override { CDBValue *pValue = m_Table.GetAt(sKey); return (pValue ? *pValue : CDBValue::Null); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeStruct; }
		virtual void SetElement (const CString &sKey, const CDBValue &Value) override { m_Table.SetAt(sKey, Value); }

	private:
		TSortMap<CString, CDBValue> m_Table;
	};

class CDBValueTimeSpan : public IDBValueObject
	{
	public:
		CDBValueTimeSpan (const CTimeSpan &Value = CTimeSpan()) :
				m_Value(Value)
			{ }

		virtual CTimeSpan AsTimeSpan (void) const { return m_Value; }
		virtual CString AsString (void) const { return m_Value.Format(CString("hh:mm:ss")); }
		virtual LONGLONG CastLONGLONG (void) const { return (LONGLONG)m_Value.Milliseconds64(); }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueTimeSpan(m_Value); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeTimeSpan; }

	private:
		CTimeSpan m_Value;
	};

