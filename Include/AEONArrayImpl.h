//	AEONArrayImpl.h
//
//	AEON Utilities
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.

#pragma once

template <class OBJ, class ELEMENT>
class TArrayImpl
	{
	public:

		static CDatum GetElementAt (const OBJ* pObj, const TArray<ELEMENT>& Array, CDatum dIndex)
			{
			if (dIndex.IsIdenticalToNil())
				return pObj->MakeNullElement();

			else if (dIndex.IsIdenticalToTrue())
				return CDatum::raw_AsComplex(pObj).GetElementsAtRange(CDatum::CreateRange(0, Array.GetCount() - 1, 1));

			else if (dIndex.GetBasicType() == CDatum::typeRange)
				return CDatum::raw_AsComplex(pObj).GetElementsAtRange(dIndex);

			else if (dIndex.IsContainer())
				return CDatum::raw_AsComplex(pObj).GetElementsAtArray(dIndex);

			else
				{
				int iIndex = dIndex.AsArrayIndex(Array.GetCount());
				if (iIndex >= 0 && iIndex < Array.GetCount())
					return pObj->ToDatum(Array[iIndex]);
				else
					return pObj->MakeNullElement();
				}
			}

		static void SetElementAt (OBJ* pObj, TArray<ELEMENT>& Array, CDatum dIndex, CDatum dValue)
			{
			if (dIndex.IsIdenticalToNil())
				{ }

			else if (dIndex.IsIdenticalToTrue())
				CDatum::raw_AsComplex(pObj).SetElementsAtRange(CDatum::CreateRange(0, Array.GetCount() - 1, 1), dValue);

			else if (dIndex.GetBasicType() == CDatum::typeRange)
				CDatum::raw_AsComplex(pObj).SetElementsAtRange(dIndex, dValue);

			else if (dIndex.IsContainer())
				CDatum::raw_AsComplex(pObj).SetElementsAtArray(dIndex, dValue);

			else
				{
				int iIndex = dIndex.AsArrayIndex(Array.GetCount());
				if (iIndex >= 0 && iIndex < Array.GetCount())
					Array[iIndex] = pObj->FromDatum(dValue);

				else if (iIndex >= 0)
					{
					int iNew = iIndex - Array.GetCount();
					Array.GrowToFit(iNew + 1);
					for (int i = 0; i < iNew; i++)
						Array.Insert(pObj->MakeNullElement());

					Array.Insert(pObj->FromDatum(dValue));
					}
				}
			}
	};

