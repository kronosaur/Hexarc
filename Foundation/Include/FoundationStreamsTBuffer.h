//	FoundationStreams.h
//
//	Foundation header file
//	Copyright (c) 2019 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

template <typename ALLOCATOR>
class TBuffer : public CMemoryBlockImpl
	{
	public:
		TBuffer (void) { }
		TBuffer (size_t dwSize) :
				m_Block(dwSize)
			{ }

		TBuffer (const CString &sString, int iPos = 0, int iLength = -1) :
				m_Block(sString, iPos, iLength)
			{
			m_dwLength = m_Block.GetAllocSize();
			}

		TBuffer (void *pBuffer, int iLength) :
				m_Block(pBuffer, iLength),
				m_dwLength(iLength)
			{
			}

		void TakeHandoff (void *pBuffer, size_t dwLength)
			{
			m_Block.TakeHandoff(pBuffer, dwLength);
			m_dwLength = dwLength;
			}

		//	IMemoryBlock virtuals

		virtual int GetLength (void) const override { return (int)m_dwLength; }
		virtual char *GetPointer (void) const override { return m_Block.GetPointer(); }
		virtual void SetLength (int iLength) override
			{
			m_Block.GrowToFit(iLength, m_dwLength);
			m_dwLength = iLength;

			//	Make sure the seek position is correct

			if (GetPos() > (int)m_dwLength)
				Seek((int)m_dwLength);
			}

	private:
		size_t m_dwLength = 0;
		ALLOCATOR m_Block;
	};

class CMallocAllocator
	{
	public:
		CMallocAllocator (void) { }

		CMallocAllocator (const CMallocAllocator &Src)
			{
			Copy(Src);
			}

		CMallocAllocator (CMallocAllocator &&Src) noexcept
			{
			Move(Src);
			}

		CMallocAllocator (size_t dwSize)
			{
			m_pBuffer = (char *)malloc(dwSize);
			m_dwAllocSize = dwSize;
			}

		CMallocAllocator (void *pBuffer, int iLength)
			{
			if (iLength > 0)
				{
				m_pBuffer = (char *)malloc(iLength);
				m_dwAllocSize = iLength;
				utlMemCopy(pBuffer, m_pBuffer, m_dwAllocSize);
				}
			else
				{
				m_pBuffer = NULL;
				m_dwAllocSize = 0;
				}
			}

		CMallocAllocator (const CString &sString, int iPos, int iLength)
			{
			if (iLength == -1)
				iLength = sString.GetLength();

			if (iLength > 0)
				{
				m_dwAllocSize = (size_t)iLength + 1;
				m_pBuffer = (char *)malloc(m_dwAllocSize);
				utlMemCopy(sString.GetParsePointer(), m_pBuffer, m_dwAllocSize);
				}
			else
				{
				m_pBuffer = NULL;
				m_dwAllocSize = 0;
				}
			}

		~CMallocAllocator (void)
			{
			CleanUp();
			}

		CMallocAllocator &operator= (const CMallocAllocator &Src)
			{
			CleanUp();
			Copy(Src);
			}

		CMallocAllocator &operator= (CMallocAllocator &&Src) noexcept
			{
			CleanUp();
			Move(Src);
			}

		size_t GetAllocSize (void) const { return m_dwAllocSize; }
		char *GetPointer (void) const { return m_pBuffer; }

		void GrowToFit (size_t dwNewSize, size_t dwOldSize)
			{
			//	Grow, if necessary

			if (dwNewSize > m_dwAllocSize)
				{
				size_t dwInc = Max(MIN_ALLOC_INCREMENT, Min(MAX_ALLOC_INCREMENT, m_dwAllocSize));
				size_t dwNewAlloc = Max(dwNewSize, m_dwAllocSize + dwInc);

				char *pNewBuffer = (char *)malloc(dwNewAlloc);

				if (m_pBuffer)
					{
					utlMemCopy(m_pBuffer, pNewBuffer, dwOldSize);
					free(m_pBuffer);
					}

				m_pBuffer = pNewBuffer;
				m_dwAllocSize = dwNewAlloc;
				}
			}

		void TakeHandoff (void *pBuffer, size_t dwLength)
			{
			CleanUp();

			if (dwLength > 0 && pBuffer)
				{
				m_pBuffer = (char *)pBuffer;
				m_dwAllocSize = dwLength;
				}
			}

	private:
		static constexpr size_t MIN_ALLOC_INCREMENT = 4096;
		static constexpr size_t MAX_ALLOC_INCREMENT = 65536;

		void CleanUp (void)
			{
			if (m_pBuffer)
				free(m_pBuffer);

			m_pBuffer = NULL;
			m_dwAllocSize = 0;
			}

		void Copy (const CMallocAllocator &Src)
			{
			if (Src.m_dwAllocSize)
				{
				m_pBuffer = (char *)malloc(Src.m_dwAllocSize);
				m_dwAllocSize = Src.m_dwAllocSize;
				utlMemCopy(Src.m_pBuffer, m_pBuffer, m_dwAllocSize);
				}
			else
				{
				m_pBuffer = NULL;
				m_dwAllocSize = 0;
				}
			}

		void Move (CMallocAllocator &Src)
			{
			m_pBuffer = Src.m_pBuffer;
			m_dwAllocSize = Src.m_dwAllocSize;
			Src.m_pBuffer = NULL;
			Src.m_dwAllocSize = 0;
			}

		char *m_pBuffer = NULL;
		size_t m_dwAllocSize = 0;
	};

class CNewAllocator
	{
	public:
		CNewAllocator (void) { }

		CNewAllocator (const CNewAllocator &Src)
			{
			Copy(Src);
			}

		CNewAllocator (CNewAllocator &&Src) noexcept
			{
			Move(Src);
			}

		CNewAllocator (size_t dwSize)
			{
			m_pBuffer = new char [dwSize];
			m_dwAllocSize = dwSize;
			}

		CNewAllocator (void *pBuffer, int iLength)
			{
			if (iLength > 0)
				{
				m_pBuffer = new char [iLength];
				m_dwAllocSize = iLength;
				utlMemCopy(pBuffer, m_pBuffer, m_dwAllocSize);
				}
			else
				{
				m_pBuffer = NULL;
				m_dwAllocSize = 0;
				}
			}

		CNewAllocator (const CString &sString, int iPos, int iLength)
			{
			if (iLength == -1)
				iLength = sString.GetLength();

			if (iLength > 0)
				{
				m_dwAllocSize = (size_t)iLength + 1;
				m_pBuffer = new char [m_dwAllocSize];
				utlMemCopy(sString.GetParsePointer(), m_pBuffer, m_dwAllocSize);
				}
			else
				{
				m_pBuffer = NULL;
				m_dwAllocSize = 0;
				}
			}

		~CNewAllocator (void)
			{
			CleanUp();
			}

		CNewAllocator &operator= (const CNewAllocator &Src)
			{
			CleanUp();
			Copy(Src);
			}

		CNewAllocator &operator= (CNewAllocator &&Src) noexcept
			{
			CleanUp();
			Move(Src);
			}

		size_t GetAllocSize (void) const { return m_dwAllocSize; }
		char *GetPointer (void) const { return m_pBuffer; }

		void GrowToFit (size_t dwNewSize, size_t dwOldSize)
			{
			//	Grow, if necessary

			if (dwNewSize > m_dwAllocSize)
				{
				size_t dwInc = Max(MIN_ALLOC_INCREMENT, Min(MAX_ALLOC_INCREMENT, m_dwAllocSize));
				size_t dwNewAlloc = Max(dwNewSize, m_dwAllocSize + dwInc);

				char *pNewBuffer = new char [dwNewAlloc];

				if (m_pBuffer)
					{
					utlMemCopy(m_pBuffer, pNewBuffer, dwOldSize);
					delete [] m_pBuffer;
					}

				m_pBuffer = pNewBuffer;
				m_dwAllocSize = dwNewAlloc;
				}
			}

		void TakeHandoff (void *pBuffer, size_t dwLength)
			{
			CleanUp();

			if (dwLength > 0 && pBuffer)
				{
				m_pBuffer = (char *)pBuffer;
				m_dwAllocSize = dwLength;
				}
			}

	private:
		static constexpr size_t MIN_ALLOC_INCREMENT = 4096;
		static constexpr size_t MAX_ALLOC_INCREMENT = 65536;

		void CleanUp (void)
			{
			if (m_pBuffer)
				delete [] m_pBuffer;

			m_pBuffer = NULL;
			m_dwAllocSize = 0;
			}

		void Copy (const CNewAllocator &Src)
			{
			if (Src.m_dwAllocSize)
				{
				m_pBuffer = new char [Src.m_dwAllocSize];
				m_dwAllocSize = Src.m_dwAllocSize;
				utlMemCopy(Src.m_pBuffer, m_pBuffer, m_dwAllocSize);
				}
			else
				{
				m_pBuffer = NULL;
				m_dwAllocSize = 0;
				}
			}

		void Move (CNewAllocator &Src)
			{
			m_pBuffer = Src.m_pBuffer;
			m_dwAllocSize = Src.m_dwAllocSize;
			Src.m_pBuffer = NULL;
			Src.m_dwAllocSize = 0;
			}

		char *m_pBuffer = NULL;
		size_t m_dwAllocSize = 0;
	};

class CStaticAllocator
	{
	public:
		CStaticAllocator (void) { }

		CStaticAllocator (const CStaticAllocator &Src) = default;

		CStaticAllocator (size_t dwSize)
			{
			throw CException(errFail, CString("CStaticAllocator does not support dynamic allocation."));
			}

		CStaticAllocator (void *pBuffer, int iLength)
			{
			if (iLength > 0 && pBuffer)
				{
				m_pBuffer = (char *)pBuffer;
				m_dwAllocSize = iLength;
				}
			}

		CStaticAllocator (const CString &sString, int iPos, int iLength)
			{
			m_pBuffer = (LPSTR)sString + iPos;
			if (iLength == -1)
				m_dwAllocSize = (size_t)sString.GetLength() - iPos;
			else
				m_dwAllocSize = iLength;
			}

		~CStaticAllocator (void)
			{
			}

		CStaticAllocator &operator= (const CStaticAllocator &Src) = default;

		size_t GetAllocSize (void) const { return m_dwAllocSize; }
		char *GetPointer (void) const { return m_pBuffer; }

		void GrowToFit (size_t dwNewSize, size_t dwOldSize)
			{
			//	Grow, if necessary

			if (dwNewSize > m_dwAllocSize)
				{
				throw CException(errFail, CString("CStaticAllocator does not support dynamic allocation."));
				}
			}

		void TakeHandoff (void *pBuffer, size_t dwLength)
			{
			throw CException(errFail, CString("CStaticAllocator does not support dynamic allocation."));
			}

	private:
		char *m_pBuffer = NULL;
		size_t m_dwAllocSize = 0;
	};
