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
		virtual void Read (void *pData, DWORDLONG dwLength) = 0;
		virtual void Seek (DWORDLONG dwPos, bool bFromEnd = false) = 0;
		virtual void Write (void *pData, DWORDLONG dwLength) = 0;

		//	Helpers

		inline bool HasMore (void) { return (GetPos() < GetStreamLength()); }
		char ReadChar (void);

		inline void Write (void *pData, int iLength) { if (iLength > 0) Write(pData, (DWORDLONG)iLength); }
		inline void Write (void *pData, DWORD dwLength) { Write(pData, (DWORDLONG)dwLength); }
		inline void Write (void *pData, std::ptrdiff_t iLength) { if (iLength > 0) Write(pData, (DWORDLONG)iLength); }
		inline void Write (const CString &sString) { Write((LPSTR)sString, (DWORDLONG)sString.GetLength()); }
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
		virtual void Read (void *pData, DWORDLONG dwLength) override;
		virtual void Seek (DWORDLONG dwPos, bool bFromEnd = false) override;
		virtual void Write (void *pData, DWORDLONG dwLength) override;

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream64::Write;

	private:
		DWORDLONG m_dwPos;
	};

