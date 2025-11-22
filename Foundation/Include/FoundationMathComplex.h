//	FoundationMathComplex.h
//
//	Foundation header file
//	Copyright (c) 2020 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

template <class TYPE> class TComplex
	{
	public:
		TComplex () { }
		TComplex (TYPE A, TYPE B) :
				m_A(A),
				m_B(B)
			{ }

		TYPE A () const { return m_A; }
		TYPE B () const { return m_B; }

		TComplex<TYPE> operator+ (const TComplex<TYPE> &X) const { return TComplex<TYPE>(m_A + X.m_A, m_B + X.m_B); }
		TComplex<TYPE> operator- (const TComplex<TYPE> &X) const { return TComplex<TYPE>(m_A - X.m_A, m_B - X.m_B); }
		TComplex<TYPE> operator* (const TComplex<TYPE> &X) const { return TComplex<TYPE>(m_A * X.m_A - m_B * X.m_B, m_A * X.m_B + m_B * X.m_A); }

		TYPE Abs () const {	return sqrt(m_A * m_A + m_B * m_B);	}

	private:
		TYPE m_A = 0;			//	Real part
		TYPE m_B = 0;			//	Imaginary part
	};

template <class TYPE> TComplex<TYPE> operator+ (const TComplex<TYPE> &X, const TComplex<TYPE> &Y) { return TComplex<TYPE>(X.A() + Y.A(), X.B() + Y.B()); }
template <class TYPE> TComplex<TYPE> operator- (const TComplex<TYPE> &X, const TComplex<TYPE> &Y) { return TComplex<TYPE>(X.A() - Y.A(), X.B() - Y.B()); }
template <class TYPE> TComplex<TYPE> operator* (const TComplex<TYPE> &X, const TComplex<TYPE> &Y) { return TComplex<TYPE>(X.A() * Y.A() - X.B() * Y.B(), X.A() * Y.B() + X.B() * Y.A()); }

using CComplexDouble = TComplex<double>;
