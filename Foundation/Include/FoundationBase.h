//	FoundationBase.h
//
//	Foundation header file
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

//	Undefine some definitions used by Windows

#undef AddPort
#undef DeleteForm
#undef InitiateShutdown
#undef LoadLibrary
#undef SendMessage

//	Forward declarations

class IByteStream;
class CCriticalSection;
class CManualEvent;
class CSmartLock;

//	Miscellaneous utilities

typedef DWORDLONG SequenceNumber;

template <class VALUE> constexpr VALUE Abs (VALUE x) { return (x < 0 ? -x : x); }
template <class VALUE> constexpr VALUE AlignUp (VALUE iValue, VALUE iGranularity) { return ((iValue + iGranularity - 1) / iGranularity) * iGranularity; }
template <class VALUE> constexpr VALUE Clamp (VALUE x, VALUE a, VALUE b) { return (x < a ? a : (x > b ? b : x)); }
template <class VALUE> constexpr VALUE Min (VALUE a, VALUE b) { return (a < b ? a : b); }
template <class VALUE> constexpr VALUE Max (VALUE a, VALUE b) { return (a > b ? a : b); }
template <class VALUE> constexpr VALUE Sign (VALUE iValue) { return (iValue < 0 ? -1 : (iValue == 0 ? 0 : 1)); }
template <class VALUE> constexpr void Swap (VALUE &a, VALUE &b) { VALUE temp = a;	a = b;	b = temp; }

inline int RectHeight (RECT *pRect) { return pRect->bottom - pRect->top; }
inline int RectHeight (const RECT &Rect) { return Rect.bottom - Rect.top; }
inline int RectWidth (RECT *pRect) { return pRect->right - pRect->left; }
inline int RectWidth (const RECT &Rect) { return Rect.right - Rect.left; }

class CRandomModule
	{
	public:

		CRandomModule (DWORD dwSeed = 0);

		DWORD GetSeed (void) const { return m_dwSeed; }
		DWORD Random ();
		double RandomDouble ();
		int RandomInRange (int iFrom, int iTo);
		double RandomInRange (double rFrom, double rTo);
		void SetSeed (DWORD dwSeed) { m_dwSeed = (dwSeed ? dwSeed : InitSeed()); }

	private:

		static DWORD InitSeed ();

		DWORD m_dwSeed = 0;

		static constexpr DWORD	M = 2147483647;
		static constexpr DWORD	Q = 44488;

		static constexpr DWORD	A = 48271;
		static constexpr DWORD	R = 3399;
	};

