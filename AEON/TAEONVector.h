//	TAEONVector.h
//
//	AEON Class Implementations
//	Copyright (c) 2021 GridWhale Corporation. All Rights Reserved.

#pragma once

#include <type_traits>

template <class VALUE, class IMPL> class TAEONVector : public IComplexDatum
	{
	public:

		TAEONVector () { }

		//	This construct omits the constructor if VALUE = CDatum. Otherwise we 
		//	get duplicate definitions.

		template <typename T = VALUE, typename std::enable_if<!std::is_same<T, CDatum>::value>::type* = nullptr>
		TAEONVector (const TArray<VALUE> &Src) : 
				m_Array(Src)
			{ }
		TAEONVector (const TArray<CDatum> &Src)
			{
			m_Array.InsertEmpty(Src.GetCount());
			for (int i = 0; i < m_Array.GetCount(); i++)
				m_Array[i] = IMPL::FromDatum(Src[i]);
			}

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

		//	IComplexDatum

		virtual void Append (CDatum dDatum) override { m_Array.Insert(IMPL::FromDatum(dDatum)); }
		virtual CString AsString (void) const override { return Format(CStringFormat()); }

		virtual size_t CalcMemorySize (void) const override
			{
			size_t dwSize = 0;

			for (int i = 0; i < m_Array.GetCount(); i++)
				dwSize += sizeof(m_Array[i]);

			return dwSize;
			}

		virtual IComplexDatum *Clone (CDatum::EClone iMode) const override
			{
			switch (iMode)
				{
				case CDatum::EClone::CopyOnWrite:
					//	Default handler
					return NULL;

				default:
					return new IMPL(m_Array);
				}
			}

		virtual void DeleteElement (int iIndex) override { if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array.Delete(iIndex); }

		virtual bool Find (CDatum dValue, int *retiIndex = NULL) const override
			{
			VALUE ValueToFind = IMPL::FromDatum(dValue);
			for (int i = 0; i < GetCount(); i++)
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

		virtual CDatum FindAll (CDatum dValue) const override
			{
			VALUE ValueToFind = IMPL::FromDatum(dValue);
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < GetCount(); i++)
				{
				if (ValueToFind == m_Array[i])
					dResult.Append(i);
				}

			return dResult;
			}

		virtual CDatum FindAllExact (CDatum dValue) const override
			{
			VALUE ValueToFind = IMPL::FromDatum(dValue);
			CDatum dResult(CDatum::typeArray);
			for (int i = 0; i < GetCount(); i++)
				{
				if (ValueToFind == m_Array[i])
					dResult.Append(i);
				}

			return dResult;
			}

		virtual bool FindExact (CDatum dValue, int *retiIndex = NULL) const override
			{
			VALUE ValueToFind = IMPL::FromDatum(dValue);
			for (int i = 0; i < GetCount(); i++)
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

		virtual CString Format (const CStringFormat& Format) const override
			{
			CRecursionGuard Guard(*this);
			if (Guard.InRecursion())
				return AsAddress();

			CStringBuffer Output;

			Output.Write("[", 1);

			for (int i = 0; i < m_Array.GetCount(); i++)
				{
				if (i != 0)
					Output.Write(", ", 2);

				Output.Write(IMPL::ToDatum(m_Array[i]).Format(Format));
				}

			Output.Write("]", 1);

			return CString(std::move(Output));
			}

		virtual CDatum GetArrayElementUnchecked (int iIndex) const override { return IMPL::ToDatum(m_Array[iIndex]); }
		virtual DWORD GetBasicDatatype () const override { return IDatatype::ARRAY; }
		virtual CDatum::Types GetBasicType (void) const override { return CDatum::typeArray; }
		virtual int GetCount (void) const override { return m_Array.GetCount(); }
		virtual CDatum GetElement (int iIndex) const override { return ((iIndex >= 0 && iIndex < m_Array.GetCount()) ? IMPL::ToDatum(m_Array[iIndex]) : IMPL::MakeNullElement()); }
		virtual CDatum GetElementAt (int iIndex) const override
			{
			if (iIndex >= 0 && iIndex < m_Array.GetCount())
				return IMPL::ToDatum(m_Array[iIndex]);
			else if (iIndex < 0)
				{
				int iNewIndex = iIndex + m_Array.GetCount();
				if (iNewIndex >= 0 && iNewIndex < m_Array.GetCount())
					return IMPL::ToDatum(m_Array[iNewIndex]);
				else
					return IMPL::MakeNullElement();
				}
			else
				return IMPL::MakeNullElement();
			}

		virtual void GrowToFit (int iCount) override { m_Array.GrowToFit(iCount); }

		virtual void InsertElementAt (CDatum dIndex, CDatum dDatum) override
			{
			bool bFromEnd;
			int iIndex = dIndex.AsArrayIndex(m_Array.GetCount(), &bFromEnd);

			//	If from the end, we insert AFTER the specified position (because
			//	we want -1 to mean AFTER the last entry).

			if (bFromEnd && iIndex >= 0)
				iIndex++;

			//	Handle different cases

			if (iIndex >= 0 && iIndex < m_Array.GetCount())
				{
				m_Array.Insert(IMPL::FromDatum(dDatum), iIndex);
				}
			else if (iIndex >= 0)
				{
				CDatum dNullDatum = IMPL::MakeNullElement();
				int iNew = iIndex - GetCount();
				GrowToFit(iNew + 1);
				for (int i = 0; i < iNew; i++)
					Append(dNullDatum);

				Append(dDatum);
				}
			else if (dIndex.IsContainer())
				{
				TSortMap<int, bool> Indices;
				for (int i = 0; i < dIndex.GetCount(); i++)
					Indices.SetAt(dIndex.GetElement(i).AsArrayIndex(m_Array.GetCount()), true);

				m_Array.Delete(Indices);
				}
			}

		virtual bool IsArray (void) const override { return true; }
		virtual bool IsContainer () const override { return true; }
		virtual bool IsNil (void) const override { return (GetCount() == 0); }

		virtual bool RemoveAll () override
			{
			m_Array.DeleteAll();
			return true;
			}

		virtual bool RemoveElementAt (CDatum dIndex) override
			{
			int iIndex = dIndex.AsArrayIndex(m_Array.GetCount());
			if (iIndex >= 0 && iIndex < m_Array.GetCount())
				{
				m_Array.Delete(iIndex);
				return true;
				}
			else if (dIndex.IsContainer())
				{
				int iOldSize = m_Array.GetCount();

				TSortMap<int, bool> Indices;
				for (int i = 0; i < dIndex.GetCount(); i++)
					Indices.SetAt(dIndex.GetElement(i).AsArrayIndex(m_Array.GetCount()), true);

				m_Array.Delete(Indices);

				return (iOldSize != m_Array.GetCount());
				}
			else
				return false;
			}

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

				case CDatum::EFormat::GridLang:
					{
					Stream.Write("[ ", 2);

					for (int i = 0; i < m_Array.GetCount(); i++)
						{
						if (i != 0)
							Stream.Write(", ", 2);

						IMPL::ToDatum(m_Array[i]).Serialize(iFormat, Stream);
						}

					Stream.Write(" ]", 2);
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
			
		virtual void Sort (ESortOptions Order = AscendingSort, TArray<CDatum>::COMPAREPROC pfCompare = NULL, void *pCtx = NULL) override { m_Array.Sort(Order); }
		virtual void SetArrayElementUnchecked (int iIndex, CDatum dValue) override { m_Array[iIndex] = IMPL::FromDatum(dValue); }
		virtual void SetElement (int iIndex, CDatum dDatum) override { if (iIndex >= 0 && iIndex < m_Array.GetCount()) m_Array[iIndex] = IMPL::FromDatum(dDatum); }

	protected:
		virtual size_t OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const override
			{
			size_t TotalSize = 2 + m_Array.GetCount();

			for (int i = 0; i < m_Array.GetCount(); i++)
				TotalSize += IMPL::ToDatum(m_Array[i]).CalcSerializeSize(iFormat);

			return TotalSize;
			}

		TArray<VALUE> m_Array;
	};

