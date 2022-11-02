//	FoundationSmartPtr.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

template <class OBJ> class TSharedPtr
	{
	public:
		constexpr TSharedPtr (void) : m_pPtr(NULL) { }
		constexpr TSharedPtr (std::nullptr_t) : m_pPtr(NULL) { }

		explicit TSharedPtr (OBJ *pPtr) : m_pPtr(pPtr) { }

		TSharedPtr (const TSharedPtr<OBJ> &Src)
			{
			if (Src.m_pPtr)
				{
				Src.m_pPtr->AddRef();
				m_pPtr = Src.m_pPtr;
				}
			else
				m_pPtr = NULL;
			}

		TSharedPtr (TSharedPtr<OBJ> &&Src) noexcept : m_pPtr(Src.m_pPtr)
			{
			Src.m_pPtr = NULL;
			}

		~TSharedPtr (void)
			{
			if (m_pPtr)
				m_pPtr->Delete();
			}

		TSharedPtr<OBJ> &operator= (const TSharedPtr<OBJ> &Src)
			{
			OBJ *pOld = m_pPtr;

			if (Src.m_pPtr)
				{
				Src.m_pPtr->AddRef();
				m_pPtr = Src.m_pPtr;
				}
			else
				m_pPtr = NULL;

			if (pOld)
				pOld->Delete();

			return *this;
			}

		operator OBJ *() const { return m_pPtr; }
		OBJ * operator->() const { return m_pPtr; }

		explicit operator bool() const { return (m_pPtr != NULL); }

		void Delete (void) { Set(NULL);	}

		void Set (OBJ *pPtr)
			{
			OBJ *pOld = m_pPtr;

			m_pPtr = pPtr;

			if (pOld)
				pOld->Delete();
			}

		void Set (const TSharedPtr<OBJ> &Src)
			{
			*this = Src;
			}

	private:
		OBJ *m_pPtr;
	};

template <class OBJ> class TUniquePtr
	{
	public:
		constexpr TUniquePtr (void) : m_pPtr(NULL) { }
		constexpr TUniquePtr (std::nullptr_t) : m_pPtr(NULL) { }

		explicit TUniquePtr (OBJ *pPtr) : m_pPtr(pPtr) { }

		TUniquePtr (const TUniquePtr<OBJ> &Src)
			{
			if (Src.m_pPtr)
				m_pPtr = new OBJ(*Src.m_pPtr);
			else
				m_pPtr = NULL;
			}

		TUniquePtr (TUniquePtr<OBJ> &&Src) noexcept : m_pPtr(Src.m_pPtr)
			{
			Src.m_pPtr = NULL;
			}

		~TUniquePtr (void)
			{
			if (m_pPtr)
				delete m_pPtr;
			}

		TUniquePtr<OBJ> &operator= (const TUniquePtr<OBJ> &Src)
			{
			OBJ *pOld = m_pPtr;

			if (Src.m_pPtr)
				m_pPtr = new OBJ(*Src.m_pPtr);
			else
				m_pPtr = NULL;

			if (pOld)
				delete pOld;

			return *this;
			}

		TUniquePtr<OBJ> &operator= (TUniquePtr<OBJ> &&Src) noexcept
			{
			OBJ *pOld = m_pPtr;
			m_pPtr = Src.m_pPtr;
			Src.m_pPtr = NULL;

			if (pOld && pOld != m_pPtr)
				delete pOld;

			return *this;
			}

		operator OBJ *() const { return m_pPtr; }
		OBJ * operator->() const { return m_pPtr; }

		explicit operator bool() const { return (m_pPtr != NULL); }

		void Delete (void) { Set(NULL); }

		OBJ *Release (void)
			{
			OBJ *pOld = m_pPtr;
			m_pPtr = NULL;
			return pOld;
			}

		void Set (OBJ *pPtr)
			{
			OBJ *pOld = m_pPtr;

			m_pPtr = pPtr;

			if (pOld && pOld != m_pPtr)
				delete pOld;
			}

		void Set (const TUniquePtr<OBJ> &Src)
			{
			*this = Src;
			}

		void Set (TUniquePtr<OBJ> &&Src)
			{
			TakeHandoff(Src);
			}

		void TakeHandoff (TUniquePtr<OBJ> &Src)
			{
			OBJ *pOld = m_pPtr;

			m_pPtr = Src.m_pPtr;
			Src.m_pPtr = NULL;

			if (pOld && pOld != m_pPtr)
				delete pOld;
			}

	private:
		OBJ *m_pPtr;
	};

template <class OBJ> class TUniquePtr<OBJ[]>
	{
	public:
		constexpr TUniquePtr (void) : m_pPtr(NULL) { }
		constexpr TUniquePtr (std::nullptr_t) : m_pPtr(NULL) { }

		explicit TUniquePtr (OBJ *pPtr) : m_pPtr(pPtr) { }

		TUniquePtr (const TUniquePtr<OBJ> &Src) =delete;

		TUniquePtr (TUniquePtr<OBJ> &&Src) : m_pPtr(Src.m_pPtr)
			{
			Src.m_pPtr = NULL;
			}

		~TUniquePtr (void)
			{
			if (m_pPtr)
				delete [] m_pPtr;
			}

		TUniquePtr<OBJ> &operator= (const TUniquePtr<OBJ> &Src) =delete;
		TUniquePtr<OBJ[]> &operator= (const TUniquePtr<OBJ[]> &Src) =delete;

		OBJ & operator[](size_t i) const { return m_pPtr[i]; }

		explicit operator bool() const { return (m_pPtr != NULL); }

		void Delete (void) { Set(NULL); }

		void Set (OBJ *pPtr)
			{
			if (m_pPtr)
				delete [] m_pPtr;

			m_pPtr = pPtr;
			}

	private:
		OBJ *m_pPtr;
	};

template <class OBJ> class TRefCounted
	{
	public:
		TRefCounted (void) : m_dwRefCount(1) { }

		OBJ *AddRef (void) { m_dwRefCount++; return static_cast<OBJ *>(this); }
		void Delete (void) { if (--m_dwRefCount == 0) delete static_cast<OBJ *>(this); }

	private:
		int m_dwRefCount;
	};
