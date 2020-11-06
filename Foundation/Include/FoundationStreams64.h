//	FoundationStreams64.h
//
//	Foundation header file
//	Copyright (c) 2017 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IByteStream64
	{
	public:
		virtual ~IByteStream64 (void) { }

		virtual DWORDLONG GetPos (void) = 0;
		virtual DWORDLONG GetStreamLength (void) = 0;
		virtual char ReadChar (void);
		virtual DWORDLONG ReadTry (void *pData, DWORDLONG dwLength) = 0;
		virtual void Seek (DWORDLONG dwPos, bool bFromEnd = false) = 0;
		virtual void Write (const void *pData, DWORDLONG dwLength) = 0;

		//	Helpers

		bool HasMore (void) { return (GetPos() < GetStreamLength()); }

		void Read (void *pData, DWORDLONG dwLength) { DWORDLONG dwRead = ReadTry(pData, dwLength); if (dwRead != dwLength) throw CException(errEoS); }

		void Write (const void *pData, int iLength) { if (iLength > 0) Write(pData, (DWORDLONG)iLength); }
		void Write (const void *pData, DWORD dwLength) { Write(pData, (DWORDLONG)dwLength); }
		void Write (const void *pData, std::ptrdiff_t iLength) { if (iLength > 0) Write(pData, (DWORDLONG)iLength); }
		void Write (const CString &sString) { Write((LPSTR)sString, (DWORDLONG)sString.GetLength()); }
		void Write (IByteStream &Stream, int iLength);
		void Write (IByteStream64 &Stream, DWORDLONG dwLength);
		void Write (IMemoryBlock &Block);

		void WriteChar (char chChar, int iCount = 1);
		void WriteWithProgress (IByteStream &Stream, int iLength, IProgressEvents *pProgress);
	};

class IMemoryBlock64 : public IByteStream64
	{
	public:
		IMemoryBlock64 (void) { }

		virtual DWORDLONG GetLength (void) const = 0;
		virtual char *GetPointer (void) const = 0;
		virtual void SetLength (DWORDLONG dwLength) { }
	};

class CMemoryBlockImpl64 : public IMemoryBlock64
	{
	public:
		CMemoryBlockImpl64 (void) : m_dwPos(0) { }

		//	IByteStream

		virtual DWORDLONG GetPos (void) override { return m_dwPos; }
		virtual DWORDLONG GetStreamLength (void) override { return GetLength(); }
		virtual char ReadChar (void) override;
		virtual DWORDLONG ReadTry (void *pData, DWORDLONG dwLength) override;
		virtual void Seek (DWORDLONG dwPos, bool bFromEnd = false) override;
		virtual void Write (const void *pData, DWORDLONG dwLength) override;

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream64::Write;

	private:
		DWORDLONG m_dwPos;
	};

//	Implementations ------------------------------------------------------------

class CBuffer64 : public CMemoryBlockImpl64
	{
	public:
		CBuffer64 (void) { }
		CBuffer64 (const CBuffer64 &Src);
		CBuffer64 (CBuffer64 &&Src) noexcept;

		explicit CBuffer64 (int iSize);
		CBuffer64 (const CString &sString, int iPos = 0, int iLength = -1);
		CBuffer64 (const char *pBuffer, int iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const void *pBuffer, int iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const char *pBuffer, DWORD iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const void *pBuffer, DWORD iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const char *pBuffer, size_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const void *pBuffer, size_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const char *pBuffer, std::ptrdiff_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer64 (const void *pBuffer, std::ptrdiff_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }

		virtual ~CBuffer64 (void);

		CBuffer64 &operator= (const CBuffer64 &Src);
		CBuffer64 &operator= (CBuffer64 &&Src) noexcept { TakeHandoff(Src); return *this; }

		LPVOID GetHandoff (DWORDLONG *retdwAllocation = NULL);
		DWORDLONG GetAllocSize (void) const { return Max(m_dwAllocation, m_dwLength); }
		void TakeHandoff (CBuffer64 &Src);
		void TakeHandoff (void *pBuffer, DWORDLONG dwAllocLength);

		//	IMemoryBlock virtuals
		virtual DWORDLONG GetLength (void) const override { return m_dwLength; }
		virtual char *GetPointer (void) const override { return m_pBuffer; }
		virtual void SetLength (DWORDLONG dwLength) override;

	private:
		static constexpr DWORDLONG MIN_ALLOC_INCREMENT =	4096;
		static constexpr DWORDLONG MAX_ALLOC_INCREMENT = 	65536;

		void Copy (const CBuffer64 &Src);
		void Init (const void *pBuffer, size_t dwLength, bool bCopy);

		DWORDLONG m_dwLength = 0;
		char *m_pBuffer = NULL;

		bool m_bAllocated = false;
		DWORDLONG m_dwAllocation = 0;
	};

