//	TAEONVector.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 Kronosaur Productions, LLC. All Rights Reserved.

#pragma once

template <class VALUE, class IMPL> class TAEONVector : public IComplexDatum
	{
	public:
		TAEONVector () { }
		TAEONVector (const TArray<VALUE> &Src) : 
				m_Array(Src)
			{ }
		TAEONVector (const TArray<CDatum> &Src)
			{
			m_Array.GrowToFit(Src.GetCount());
			for (int i = 0; i < m_Array.GetCount(); i++)
				m_Array[i] = IMPL::FromDatum(Src[i]);
			}

		void Delete (int iIndex) { m_Array.Delete(iIndex); }

		bool FindElement (CDatum dValue, int *retiIndex = NULL) const
			{
			VALUE ValueToFind = IMPL::FromDatum(dValue);
			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (ValueToFind == m_Array[i])
					{
					if (retiIndex)
						*retiIndex = i;
					return true;
					}
				}
			return false;
			}

		void Insert (const VALUE &Element, int iIndex = -1) { m_Array.Insert(Element, iIndex); }
		void Insert (CDatum Element, int iIndex = -1) { Insert(IMPL::FromDatum(Element), iIndex); }
		void InsertEmpty (int iCount = 1, int iIndex = -1) { m_Array.InsertEmpty(iCount, iIndex); }

		static VALUE FromDatum (CDatum dValue) { return (VALUE)dValue; }
		static CDatum ToDatum (VALUE Value) { return CDatum(Value); }

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { m_Array.Insert(IMPL::FromDatum(dDatum)); }
		virtual CString AsString (void) const override
			{
			CStringBuffer Output;

			Output.Write("(", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Output.Write(" ", 1);

				CString sResult = IMPL::ToDatum(m_Array[i]).AsString();
				Output.Write(sResult);
				}

			Output.Write(")", 1);

			return CString(Output);
			}

		virtual size_t CalcMemorySize (void) const override
			{
			size_t dwSize = 0;

			for (int i = 0; i < m_Array.GetCount(); i++)
				dwSize += sizeof(m_Array[i]);

			return dwSize;
			}

		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override { return FindElement(dValue, retiIndex); }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeArray; }
		virtual int GetCount (void) const override { return m_Array.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? IMPL::ToDatum(m_Array[iIndex]) : CDatum()); }
		virtual void GrowToFit (int iCount) override { m_Array.GrowToFit(iCount); }
		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }
		virtual void Serialize (CDatum::EFormat iFormat, IByteStream &Stream) const override
			{
			switch (iFormat)
				{
				case CDatum::EFormat::AEONScript:
				case CDatum::EFormat::AEONLocal:
					{
					Stream.Write("(", 1);

					for (int i = 0; i < m_Array.GetCount(); i++)
						{
						if (i != 0)
							Stream.Write(" ", 1);

						IMPL::ToDatum(m_Array[i]).Serialize(iFormat, Stream);
						}

					Stream.Write(")", 1);
					break;
					}

				case CDatum::EFormat::JSON:
					{
					Stream.Write("[", 1);

					for (int i = 0; i < m_Array.GetCount(); i++)
						{
						if (i != 0)
							Stream.Write(", ", 2);

						IMPL::ToDatum(m_Array[i]).Serialize(iFormat, Stream);
						}

					Stream.Write("]", 1);
					break;
					}

				default:
					IComplexDatum::Serialize(iFormat, Stream);
					break;
				}
			}
			
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { throw CException(errFail); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { m_Array[iIndex] = IMPL::FromDatum(dDatum); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override
			{
			size_t TotalSize = 2 + m_Array.GetCount();

			for (int i = 0; i < m_Array.GetCount(); i++)
				TotalSize += IMPL::ToDatum(m_Array[i]).CalcSerializeSize(iFormat);

			return TotalSize;
			}

		virtual void OnMarked (void) override { }

		TArray<VALUE> m_Array;
	};

