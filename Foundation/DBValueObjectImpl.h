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

		virtual CString AsString (void) const override;
		virtual IDBValueObject *Clone (void) const override { return new CDBValueArray(m_Array); }
		virtual const CDBValue &GetElement (int iIndex) const override { return (iIndex >= 0 && iIndex < m_Array.GetCount() ? m_Array[iIndex] : CDBValue::Null); }
		virtual int GetElementCount (void) const override { return m_Array.GetCount(); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeArray; }
		virtual void Push (const CDBValue &Value) override { m_Array.Insert(Value); }

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
		virtual CString AsString (void) const
			{
			if (m_Value.HasTime())
				{
				if (m_Value.Millisecond() > 0)
					return strPattern("%04d-%02d-%02d %02d:%02d:%02d.%03d", m_Value.Year(), m_Value.Month(), m_Value.Day(), m_Value.Hour(), m_Value.Minute(), m_Value.Second(), m_Value.Millisecond());
				else
					return strPattern("%04d-%02d-%02d %02d:%02d:%02d", m_Value.Year(), m_Value.Month(), m_Value.Day(), m_Value.Hour(), m_Value.Minute(), m_Value.Second());
				}
			else
				return strPattern("%04d-%02d-%02d", m_Value.Year(), m_Value.Month(), m_Value.Day());
			}

		virtual const CDateTime &CastDateTime (void) const { return m_Value; }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueDateTime(m_Value); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeDateTime; }

	private:
		CDateTime m_Value;
	};

class CDBValueDouble : public IDBValueObject
	{
	public:
		static constexpr int SIGNIFICANT_DIGITS = 16;

		CDBValueDouble (double rValue = 0.0) :
				m_rValue(rValue)
			{ }

		virtual double AsDouble (bool *retbValid = NULL) const override { if (retbValid) *retbValid = true; return m_rValue; }
		virtual int AsInt32 (bool *retbValid = NULL) const { if (retbValid) *retbValid = true; return (int)m_rValue; }
		virtual CString AsString (void) const { return strFromDouble(m_rValue, SIGNIFICANT_DIGITS); }
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

		virtual CString AsString (void) const override;
		virtual IDBValueObject *Clone (void) const override { return new CDBValueStruct(m_Table); }
		virtual const CDBValue &GetElement (int iIndex) const override { return (iIndex >= 0 && iIndex < m_Table.GetCount() ? m_Table[iIndex] : CDBValue::Null); }
		virtual const CDBValue &GetElement (const CString &sKey) const override { CDBValue *pValue = m_Table.GetAt(sKey); return (pValue ? *pValue : CDBValue::Null); }
		virtual int GetElementCount (void) const override { return m_Table.GetCount(); }
		virtual const CString &GetElementKey (int iIndex) const override { return (iIndex >= 0 && iIndex < m_Table.GetCount() ? m_Table.GetKey(iIndex) : NULL_STR); }
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

