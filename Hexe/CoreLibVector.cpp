//	CoreLibVector.cpp
//
//	Core Library
//	Copyright (c) 2014 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

bool corePolygon (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD POLY_ADD =									0;
DECLARE_CONST_STRING(POLY_ADD_NAME,						"polyAdd")
DECLARE_CONST_STRING(POLY_ADD_ARGS,						"*")
DECLARE_CONST_STRING(POLY_ADD_HELP,						"(polyAdd poly1 poly2) -> polygon-array")

const DWORD POLY_INTERSECT =							1;
DECLARE_CONST_STRING(POLY_INTERSECT_NAME,				"polyIntersect")
DECLARE_CONST_STRING(POLY_INTERSECT_ARGS,				"*")
DECLARE_CONST_STRING(POLY_INTERSECT_HELP,				"(polyIntersect srcPoly clipPoly) -> polygon-array")

const DWORD POLY_SUBTRACT =								2;
DECLARE_CONST_STRING(POLY_SUBTRACT_NAME,				"polySubtract")
DECLARE_CONST_STRING(POLY_SUBTRACT_ARGS,				"*")
DECLARE_CONST_STRING(POLY_SUBTRACT_HELP,				"(polySubtract srcPoly clipPoly) -> polygon-array")

bool coreVector (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult);

const DWORD VECTOR_ADD =								0;
DECLARE_CONST_STRING(VECTOR_ADD_NAME,					"vec+")
DECLARE_CONST_STRING(VECTOR_ADD_ARGS,					"*")
DECLARE_CONST_STRING(VECTOR_ADD_HELP,					"(vec+ vector ...) -> vector")

const DWORD VECTOR_SUBTRACT =							1;
DECLARE_CONST_STRING(VECTOR_SUBTRACT_NAME,				"vec-")
DECLARE_CONST_STRING(VECTOR_SUBTRACT_ARGS,				"*")
DECLARE_CONST_STRING(VECTOR_SUBTRACT_HELP,				"(vec- vector ...) -> vector")

const DWORD VECTOR_SCALE_MULT =							2;
DECLARE_CONST_STRING(VECTOR_SCALE_MULT_NAME,			"vec*")
DECLARE_CONST_STRING(VECTOR_SCALE_MULT_ARGS,			"*")
DECLARE_CONST_STRING(VECTOR_SCALE_MULT_HELP,			"(vec* vector scalar) -> vector")

const DWORD VECTOR_SCALE_DIV =							3;
DECLARE_CONST_STRING(VECTOR_SCALE_DIV_NAME,				"vec/")
DECLARE_CONST_STRING(VECTOR_SCALE_DIV_ARGS,				"*")
DECLARE_CONST_STRING(VECTOR_SCALE_DIV_HELP,				"(vec/ vector scalar) -> vector")

const DWORD VECTOR_ROTATE =								4;
DECLARE_CONST_STRING(VECTOR_ROTATE_NAME,				"vecRotate")
DECLARE_CONST_STRING(VECTOR_ROTATE_ARGS,				"*")
DECLARE_CONST_STRING(VECTOR_ROTATE_HELP,				"(vecRotate vector angle) -> vector")

const DWORD VECTOR_TO_POLAR =							5;
DECLARE_CONST_STRING(VECTOR_TO_POLAR_NAME,				"vecToPolar")
DECLARE_CONST_STRING(VECTOR_TO_POLAR_ARGS,				"*")
DECLARE_CONST_STRING(VECTOR_TO_POLAR_HELP,				"(vecToPolar vector) -> polar-vector")

const DWORD VECTOR_TO_UNIT =							6;
DECLARE_CONST_STRING(VECTOR_TO_UNIT_NAME,				"vecToUnit")
DECLARE_CONST_STRING(VECTOR_TO_UNIT_ARGS,				"*")
DECLARE_CONST_STRING(VECTOR_TO_UNIT_HELP,				"(vecToUnit vector) -> unit-vector")

const DWORD VECTOR_TO_XY =								7;
DECLARE_CONST_STRING(VECTOR_TO_XY_NAME,					"vecToXY")
DECLARE_CONST_STRING(VECTOR_TO_XY_ARGS,					"*")
DECLARE_CONST_STRING(VECTOR_TO_XY_HELP,					"(vecToXY polar-vector) -> vector")

DECLARE_CONST_STRING(FIELD_HOLES,						"holes")
DECLARE_CONST_STRING(FIELD_OUTLINE,						"outline")

DECLARE_CONST_STRING(ERR_DIVIDE_BY_ZERO,				"Divide by zero.")
DECLARE_CONST_STRING(ERR_BAD_POLYGON,					"Not a polygon.")

//	Library --------------------------------------------------------------------

SLibraryFuncDef g_CoreVectorLibraryDef[] =
	{
	DECLARE_DEF_LIBRARY_FUNC(POLY_ADD, corePolygon, 0),
	DECLARE_DEF_LIBRARY_FUNC(POLY_INTERSECT, corePolygon, 0),
	DECLARE_DEF_LIBRARY_FUNC(POLY_SUBTRACT, corePolygon, 0),

	DECLARE_DEF_LIBRARY_FUNC(VECTOR_ADD, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_SUBTRACT, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_SCALE_MULT, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_SCALE_DIV, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_ROTATE, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_TO_POLAR, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_TO_UNIT, coreVector, 0),
	DECLARE_DEF_LIBRARY_FUNC(VECTOR_TO_XY, coreVector, 0),
	};

const int g_iCoreVectorLibraryDefCount = SIZEOF_STATIC_ARRAY(g_CoreVectorLibraryDef);

bool corePolygon (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	int i;

	switch (dwData)
		{
		case POLY_ADD:
			{
			CPolygon2D TempSrc;
			const CPolygon2D *pSrc;
			if (!HexeGetPolygon2DArg(LocalEnv.GetArgument(0), &pSrc, &TempSrc, &retResult.dResult))
				return false;

			CPolygon2D TempClip;
			const CPolygon2D *pClip;
			if (!HexeGetPolygon2DArg(LocalEnv.GetArgument(1), &pClip, &TempClip, &retResult.dResult))
				return false;

			CPolygon2D Result;
			CPolygon2D::EOpResult iResult = pSrc->UnionPolygon(*pClip, &Result);

			switch (iResult)
				{
				//	If disjoint, then we return both polygons

				case CPolygon2D::resultEmpty:
					{
					CComplexArray *pArray = new CComplexArray;
					pArray->GrowToFit(2);
					pArray->Append(LocalEnv.GetArgument(0));
					pArray->Append(LocalEnv.GetArgument(1));
					retResult.dResult = CDatum(pArray);
					break;
					}

				//	Otherwise, we return the result

				default:
					{
					CComplexArray *pArray = new CComplexArray;
					pArray->GrowToFit(1);
					pArray->Append(CAEONPolygon2D::CreateFromHandoff(Result));
					retResult.dResult = CDatum(pArray);
					}
				}

			return true;
			}

		case POLY_INTERSECT:
			{
			CPolygon2D TempSrc;
			const CPolygon2D *pSrc;
			if (!HexeGetPolygon2DArg(LocalEnv.GetArgument(0), &pSrc, &TempSrc, &retResult.dResult))
				return false;

			CPolygon2D TempClip;
			const CPolygon2D *pClip;
			if (!HexeGetPolygon2DArg(LocalEnv.GetArgument(1), &pClip, &TempClip, &retResult.dResult))
				return false;

			TArray<CPolygon2D> Result;
			CPolygon2D::EOpResult iResult = pSrc->IntersectPolygon(*pClip, &Result);

			switch (iResult)
				{
				case CPolygon2D::resultEmpty:
					retResult.dResult = CDatum();
					break;

				default:
					{
					CComplexArray *pArray = new CComplexArray;
					pArray->GrowToFit(Result.GetCount());
					for (i = 0; i < Result.GetCount(); i++)
						pArray->Append(CAEONPolygon2D::CreateFromHandoff(Result[i]));
					retResult.dResult = CDatum(pArray);
					}
				}

			return true;
			}

		case POLY_SUBTRACT:
			{
			CPolygon2D TempSrc;
			const CPolygon2D *pSrc;
			if (!HexeGetPolygon2DArg(LocalEnv.GetArgument(0), &pSrc, &TempSrc, &retResult.dResult))
				return false;

			CPolygon2D TempClip;
			const CPolygon2D *pClip;
			if (!HexeGetPolygon2DArg(LocalEnv.GetArgument(1), &pClip, &TempClip, &retResult.dResult))
				return false;

			TArray<CPolygon2D> Result;
			CPolygon2D::EOpResult iResult = pSrc->SubtractPolygon(*pClip, &Result);

			switch (iResult)
				{
				case CPolygon2D::resultEmpty:
					retResult.dResult = CDatum();
					break;

				default:
					{
					CComplexArray *pArray = new CComplexArray;
					pArray->GrowToFit(Result.GetCount());
					for (i = 0; i < Result.GetCount(); i++)
						pArray->Append(CAEONPolygon2D::CreateFromHandoff(Result[i]));
					retResult.dResult = CDatum(pArray);
					}
				}

			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

bool coreVector (IInvokeCtx *pCtx, DWORD dwData, CHexeStackEnv& LocalEnv, CDatum dContinueCtx, CDatum dContinueResult, SAEONInvokeResult& retResult)
	{
	switch (dwData)
		{
		case VECTOR_ADD:
			{
			int iArg = 0;
			CVector2D vVector;
			if (!HexeGetVector2DArg(LocalEnv, &iArg, &vVector, &retResult.dResult))
				return false;

			//	Keep adding vectors

			while (iArg < LocalEnv.GetCount())
				{
				CVector2D vArg;
				if (!HexeGetVector2DArg(LocalEnv, &iArg, &vArg, &retResult.dResult))
					return false;

				vVector = vVector + vArg;
				}

			//	Done

			retResult.dResult = CDatum(vVector);
			return true;
			}

		case VECTOR_SUBTRACT:
			{
			int iArg = 0;
			CVector2D vVector;
			if (!HexeGetVector2DArg(LocalEnv, &iArg, &vVector, &retResult.dResult))
				return false;

			//	Keep adding vectors

			while (iArg < LocalEnv.GetCount())
				{
				CVector2D vArg;
				if (!HexeGetVector2DArg(LocalEnv, &iArg, &vArg, &retResult.dResult))
					return false;

				vVector = vVector - vArg;
				}

			//	Done

			retResult.dResult = CDatum(vVector);
			return true;
			}

		case VECTOR_SCALE_MULT:
			{
			CDatum dArg1 = LocalEnv.GetArgument(0);
			CDatum dArg2 = LocalEnv.GetArgument(1);

			//	If the first argument is a vector, then we expect the second to 
			//	be a scalar. Otherwise, it's the reverse.

			CVector2D vVector;
			if (dArg1.GetCount() >= 2)
				vVector = CVector2D((double)dArg1.GetElement(0), (double)dArg1.GetElement(1)) * (double)dArg2;
			else
				vVector = CVector2D((double)dArg2.GetElement(0), (double)dArg2.GetElement(1)) * (double)dArg1;

			//	Done

			retResult.dResult = CDatum(vVector);
			return true;
			}

		case VECTOR_SCALE_DIV:
			{
			CDatum dArg1 = LocalEnv.GetArgument(0);
			CDatum dArg2 = LocalEnv.GetArgument(1);

			//	If the first argument is a vector, then we expect the second to 
			//	be a scalar. Otherwise, it's the reverse.

			CVector2D vVector;
			double rScalar;
			if (dArg1.GetCount() >= 2)
				{
				vVector = CVector2D((double)dArg1.GetElement(0), (double)dArg1.GetElement(1));
				rScalar = (double)dArg2;
				}
			else
				{
				vVector = CVector2D((double)dArg2.GetElement(0), (double)dArg2.GetElement(1));
				rScalar = (double)dArg1;
				}

			//	Check for divide by 0

			if (rScalar == 0.0)
				{
				CHexeError::Create(NULL_STR, ERR_DIVIDE_BY_ZERO, &retResult.dResult);
				return false;
				}

			//	Done

			retResult.dResult = CDatum(vVector / rScalar);
			return true;
			}

		case VECTOR_ROTATE:
			{
			int iArg = 0;
			CVector2D vVector;
			if (!HexeGetVector2DArg(LocalEnv, &iArg, &vVector, &retResult.dResult))
				return false;

			double rAngle = LocalEnv.GetArgument(iArg++);

			retResult.dResult = CDatum(vVector.Rotation(rAngle));
			return true;
			}

		case VECTOR_TO_POLAR:
			{
			int iArg = 0;
			CVector2D vVector;
			if (!HexeGetVector2DArg(LocalEnv, &iArg, &vVector, &retResult.dResult))
				return false;

			retResult.dResult = CDatum(vVector.ToPolar());
			return true;
			}

		case VECTOR_TO_UNIT:
			{
			int iArg = 0;
			CVector2D vVector;
			if (!HexeGetVector2DArg(LocalEnv, &iArg, &vVector, &retResult.dResult))
				return false;

			retResult.dResult = CDatum(vVector.Unit());
			return true;
			}

		case VECTOR_TO_XY:
			{
			int iArg = 0;
			CVector2D vVector;
			if (!HexeGetVector2DArg(LocalEnv, &iArg, &vVector, &retResult.dResult))
				return false;

			retResult.dResult = CDatum(vVector.ToXY());
			return true;
			}

		default:
			ASSERT(false);
			return false;
		}
	}

//	Utilities -------------------------------------------------------------------

bool HexeGetPolygon2DArg (CDatum dArg, const CPolygon2D **retpPolygon, CPolygon2D *retTempStore, CDatum* retdResult)
	{
	int i;

	//	Try to cast the argument as an AEONPolygon

	CAEONPolygon2D *pAEONPoly = CAEONPolygon2D::Upconvert(dArg);
	if (pAEONPoly)
		{
		if (retpPolygon)
			*retpPolygon = &pAEONPoly->GetPolygon();
		return true;
		}

	//	If this is a structure, we expect an outline and holes

	else if (dArg.GetBasicType() == CDatum::typeStruct)
		{
		CPolygon2D *pNewPoly;

		CDatum dOutline = dArg.GetElement(FIELD_OUTLINE);
		if (dOutline.GetBasicType() != CDatum::typeArray)
			{
			CHexeError::Create(NULL_STR, ERR_BAD_POLYGON, retdResult);
			return false;
			}

		//	Get the outline as a polygon (we know that we're going to create
		//	the polygon out of retTempStore because we've checked to see that
		//	this is an array).

		if (!HexeGetPolygon2DArg(dOutline, (const CPolygon2D **)&pNewPoly, retTempStore, retdResult))
			return false;

		//	Add any holes

		CDatum dHoles = dArg.GetElement(FIELD_HOLES);
		for (i = 0; i < dHoles.GetCount(); i++)
			{
			const CPolygon2D *pResult;
			CPolygon2D Hole;

			if (!HexeGetPolygon2DArg(dHoles.GetElement(i), &pResult, &Hole, retdResult))
				return false;

			if (pResult == &Hole)
				pNewPoly->AddHoleHandoff(Hole);
			else
				pNewPoly->AddHole(*pResult);
			}

		//	Done

		if (retpPolygon)
			*retpPolygon = pNewPoly;

		return true;
		}

	//	Otherwise see if this is an array of vectors. If so, then we can 
	//	construct a polygon from the array.

	else if (dArg.GetBasicType() == CDatum::typeArray)
		{
		if (dArg.GetCount() < 3)
			{
			CHexeError::Create(NULL_STR, ERR_BAD_POLYGON, retdResult);
			return false;
			}

		TArray<CVector2D> Points;
		Points.InsertEmpty(dArg.GetCount());
		for (i = 0; i < dArg.GetCount(); i++)
			{
			CDatum dVector = dArg.GetElement(i);
			Points[i] = CVector2D((double)dVector.GetElement(0), (double)dVector.GetElement(1));
			}

		*retTempStore = CPolygon2D();
		retTempStore->TakeHandoff(Points);

		if (retpPolygon)
			*retpPolygon = retTempStore;

		return true;
		}

	//	Otherwise, we don't know

	else
		{
		CHexeError::Create(NULL_STR, ERR_BAD_POLYGON, retdResult);
		return false;
		}
	}

bool HexeGetVector2DArg (CHexeStackEnv& LocalEnv, int *ioArg, CVector2D *retVector, CDatum* retdResult)
	{
	//	Get the first argument. If this is an array then we expect it to be a
	//	vector and we're done.

	CDatum dArg = LocalEnv.GetArgument((*ioArg)++);
	if (dArg.IsNil())
		{
		*retVector = CVector2D(0.0, 0.0);
		return true;
		}
	else if (dArg.GetCount() > 1)
		{
		*retVector = CVector2D((double)dArg.GetElement(0), (double)dArg.GetElement(1));
		return true;
		}

	//	Otherwise we expect two numbers.

	CDatum dArg2 = LocalEnv.GetArgument((*ioArg)++);
	*retVector = CVector2D((double)dArg, (double)dArg2);
	return true;
	}
