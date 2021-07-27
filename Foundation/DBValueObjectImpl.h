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

		virtual CDateTime AsDateTime (void) const override { return m_Value; }
		virtual CString AsString (void) const override
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

		virtual const CDateTime &CastDateTime (void) const override { return m_Value; }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueDateTime(m_Value); }
		virtual CDBValue GetProperty (const CString &sProperty) const override;
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
		virtual int AsInt32 (bool *retbValid = NULL) const override { if (retbValid) *retbValid = true; return (int)m_rValue; }
		virtual CString AsString (void) const override { return strFromDouble(m_rValue, SIGNIFICANT_DIGITS); }
		virtual double CastDouble (void) const override { return m_rValue; }
		virtual int CastInt32 () const override { return (int)m_rValue; }
		virtual LONGLONG CastLONGLONG (void) const override { return (LONGLONG)m_rValue; }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueDouble(m_rValue); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeDouble; }

	private:
		double m_rValue;
	};

class CDBValueFormatted : public IDBValueObject
	{
	public:
		CDBValueFormatted (void) { }
		CDBValueFormatted (const CDBValue &Value, const CDBFormatDesc &Format);

		virtual CDateTime AsDateTime (void) const override { return m_Value.AsDateTime(); }
		virtual double AsDouble (bool *retbValid = NULL) const override { return m_Value.AsDouble(retbValid); }
		virtual int AsInt32 (bool *retbValid = NULL) const override { return m_Value.AsInt32(retbValid); }
		virtual CString AsString (void) const override { return m_Value.AsString(); }
		virtual const CDateTime &CastDateTime (void) const override { return (const CDateTime &)m_Value; }
		virtual double CastDouble (void) const override { return (double)m_Value; }
		virtual int CastInt32 () const override { return (int)m_Value; }
		virtual LONGLONG CastLONGLONG (void) const override { return (LONGLONG)m_Value; }
		virtual const CString &CastString (void) const override { return (const CString &)m_Value; }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueFormatted(m_Value, m_Format); }
		virtual const CDBFormatDesc &GetFormat () const override { return m_Format; }
		virtual CDBValue GetProperty (const CString &sProperty) const override { return m_Value.GetProperty(sProperty); }
		virtual CDBValue::ETypes GetType (void) const override { return m_Value.GetType(); }
		virtual bool GetValueWithoutFormat (CDBValue *retValue = NULL) const override { if (retValue) *retValue = m_Value; return true; }

	private:
		CDBValue m_Value;
		CDBFormatDesc m_Format;
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

		virtual CTimeSpan AsTimeSpan (void) const override { return m_Value; }
		virtual CString AsString (void) const override { return m_Value.Format(CString("hh:mm:ss")); }
		virtual LONGLONG CastLONGLONG (void) const override { return (LONGLONG)m_Value.Milliseconds64(); }
		virtual IDBValueObject *Clone (void) const override { return new CDBValueTimeSpan(m_Value); }
		virtual CDBValue::ETypes GetType (void) const override { return CDBValue::typeTimeSpan; }

	private:
		CTimeSpan m_Value;
	};
