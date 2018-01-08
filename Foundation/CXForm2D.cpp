//	CXForm2D2D.cpp
//
//	CXForm2D2D class
//	Copyright (c) 2013 Kronosaur Productions, LLC. All Rights Reserved.

#include "stdafx.h"

CXForm2D::CXForm2D (void)
	{
	}

CXForm2D::CXForm2D (XFormType type)
	{
	ASSERT(type == xformIdentity);

	m_Xform[0][0] = 1.0;
	m_Xform[0][1] = 0.0;
	m_Xform[0][2] = 0.0;

	m_Xform[1][0] = 0.0;
	m_Xform[1][1] = 1.0;
	m_Xform[1][2] = 0.0;

	m_Xform[2][0] = 0.0;
	m_Xform[2][1] = 0.0;
	m_Xform[2][2] = 1.0;
	}

CXForm2D::CXForm2D (XFormType type, double rX, double rY)
	{
	switch (type)
		{
		case xformTranslate:
			m_Xform[0][0] = 1.0;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = 1.0;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = rX;
			m_Xform[2][1] = rY;
			m_Xform[2][2] = 1.0;
			break;

		case xformScale:
			m_Xform[0][0] = rX;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = rY;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;

		default:
			ASSERT(false);
		}
	}

CXForm2D::CXForm2D (XFormType type, const CVector2D &vVector)
	{
	switch (type)
		{
		case xformTranslate:
			m_Xform[0][0] = 1.0;
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = 1.0;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = vVector.X();
			m_Xform[2][1] = vVector.Y();
			m_Xform[2][2] = 1.0;
			break;

		case xformScale:
			m_Xform[0][0] = vVector.X();
			m_Xform[0][1] = 0.0;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = 0.0;
			m_Xform[1][1] = vVector.Y();
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;

		default:
			ASSERT(false);
		}
	}

CXForm2D::CXForm2D (XFormType type, double rAngle)
	{
	switch (type)
		{
		case xformRotate:
			{
			double rCos = cos(rAngle);
			double rSin = sin(rAngle);

			m_Xform[0][0] = rCos;
			m_Xform[0][1] = rSin;
			m_Xform[0][2] = 0.0;

			m_Xform[1][0] = -rSin;
			m_Xform[1][1] = rCos;
			m_Xform[1][2] = 0.0;

			m_Xform[2][0] = 0.0;
			m_Xform[2][1] = 0.0;
			m_Xform[2][2] = 1.0;
			break;
			}

		default:
			ASSERT(false);
		}
	}

void CXForm2D::Transform (double x, double y, double *retx, double *rety) const
	{
	double xNew = x * m_Xform[0][0] + y * m_Xform[1][0] + m_Xform[2][0];
	double yNew = x * m_Xform[0][1] + y * m_Xform[1][1] + m_Xform[2][1];

	*retx = xNew;
	*rety = yNew;
	}

void CXForm2D::Transform (TArray<CVector2D> *Points) const

//	Transform
//
//	Transform a list of points

	{
	int i;

	for (i = 0; i < Points->GetCount(); i++)
		{
		CVector2D &Point = Points->GetAt(i);

		double x = Point.X() * m_Xform[0][0] + Point.Y() * m_Xform[1][0] + m_Xform[2][0];
		double y = Point.X() * m_Xform[0][1] + Point.Y() * m_Xform[1][1] + m_Xform[2][1];

		Point.SetX(x);
		Point.SetY(y);
		}
	}

CVector2D CXForm2D::Transform (const CVector2D &vVector) const
	{
	return CVector2D(
			vVector.X() * m_Xform[0][0] + vVector.Y() * m_Xform[1][0] + m_Xform[2][0],
			vVector.X() * m_Xform[0][1] + vVector.Y() * m_Xform[1][1] + m_Xform[2][1]
			);
	}

const CXForm2D operator* (const CXForm2D &op1, const CXForm2D &op2)
	{
	CXForm2D Result;

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			Result.m_Xform[i][j] = 
					  op1.m_Xform[i][0] * op2.m_Xform[0][j]
					+ op1.m_Xform[i][1] * op2.m_Xform[1][j]
					+ op1.m_Xform[i][2] * op2.m_Xform[2][j];

	return Result;
	}

