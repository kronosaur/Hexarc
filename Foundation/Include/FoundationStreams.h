//	FoundationStreams.h
//
//	Foundation header file
//	Copyright (c) 2010 by GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IMemoryBlock;
class IProgressEvents;

class IByteStream
	{
	public:

		enum class EBOM
			{
			None = 0,

			UTF16LE,				//	Little-endian
			UTF16BE,				//	Big-endian
			};

		virtual ~IByteStream (void) { }

		virtual int GetPos (void) = 0;
		virtual int GetStreamLength (void) = 0;
		virtual void GrowToFit (int iLength) { }
		virtual int Read (void *pData, int iLength) = 0;
		virtual void Seek (int iPos, bool bFromEnd = false) = 0;
		virtual int Write (const void *pData, int iLength) = 0;
		virtual int WriteStream (IByteStream& Stream, int iLength) { return WriteStreamDefault(Stream, iLength); }

		//	Helpers

		char ReadChar (void);
		void ReadChecked (void *pData, int iLength) { if (Read(pData, iLength) != iLength) throw CException(errNotFound); }
		int ReadInt () { int iValue; ReadChecked(&iValue, sizeof(int)); return iValue; }
		double ReadDouble () { double rValue; ReadChecked(&rValue, sizeof(double)); return rValue; }
		DWORD ReadDWORD () { DWORD dwValue; ReadChecked(&dwValue, sizeof(DWORD)); return dwValue; }
		DWORDLONG ReadDWORDLONG () { DWORDLONG dwValue; ReadChecked(&dwValue, sizeof(DWORDLONG)); return dwValue; }
		void SeekBackward (int iLength = 1) { Seek(GetPos() - iLength); }
		bool HasMore (void) { return (GetPos() < GetStreamLength()); }
		void WriteChecked (const void* pData, int iLength) { if (Write(pData, iLength) != iLength) throw CException(errFail); }
		void Write (int iData) { this->WriteChecked(&iData, sizeof(int)); }
		void Write (DWORD dwData) { this->WriteChecked(&dwData, sizeof(DWORD)); }
		void Write (DWORDLONG dwData) { this->WriteChecked(&dwData, sizeof(DWORDLONG)); }
		void Write (double rData) { this->WriteChecked(&rData, sizeof(double)); }
		int Write (const CString &sString) { return this->Write((LPSTR)sString, sString.GetLength()); }
		inline int Write (const IMemoryBlock &Block);
		int Write (const void *pData, DWORD iLength) { ASSERT(iLength < MAXINT); return this->Write(pData, (int)iLength); }
		int Write (const void *pData, size_t iLength) { ASSERT(iLength < MAXINT); return this->Write(pData, (int)iLength); }
		int Write (const void *pData, std::ptrdiff_t iLength) { ASSERT(iLength < MAXINT); return this->Write(pData, (int)iLength); }
		int Write (IByteStream &Stream, int iLength) { return WriteStream(Stream, iLength); }
		int WriteChar (char chChar, int iCount = 1);
		int WriteStreamDefault (IByteStream &Stream, int iLength);
		int WriteWithProgress (IByteStream &Stream, int iLength, IProgressEvents *pProgress);
	};

class IMemoryBlock : public IByteStream
	{
	public:

		IMemoryBlock (void) { }

		virtual int GetLength (void) const = 0;
		virtual char *GetPointer (void) const = 0;
		virtual void SetLength (int iLength) { }

		//	Helpers

		EBOM ReadBOM (int iPos = 0) const;
		bool ReadCSVRow (DWORD_PTR &iCurPos, TArray<CString> &Row);
	};

class CMemoryBlockImpl : public IMemoryBlock
	{
	public:
		CMemoryBlockImpl () { }

		//	IByteStream
		virtual int GetPos (void) override { return m_iPos; }
		virtual int GetStreamLength (void) override { return GetLength(); }
		virtual void GrowToFit (int iLength) override;
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override;
		virtual int Write (const void *pData, int iLength) override;
		virtual int WriteStream (IByteStream& Stream, int iLength) override;

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:
		int m_iPos = 0;
	};

//	Implementations ------------------------------------------------------------

class CNullStream : public CMemoryBlockImpl
	{
	public:
		//	IMemoryBlock virtuals
		virtual int GetLength (void) const override { return 0; }
		virtual char *GetPointer (void) const override { return (char *)""; }
	};

extern CNullStream NULL_STREAM;

class CBuffer : public CMemoryBlockImpl
	{
	public:
		CBuffer (void) { }
		CBuffer (const CBuffer &Src);
		CBuffer (CBuffer &&Src) noexcept;

		explicit CBuffer (int iSize);
		CBuffer (const CString &sString, int iPos = 0, int iLength = -1);
		CBuffer (const char *pBuffer, int iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const void *pBuffer, int iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const char *pBuffer, DWORD iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const void *pBuffer, DWORD iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const char *pBuffer, size_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const void *pBuffer, size_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const char *pBuffer, std::ptrdiff_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }
		CBuffer (const void *pBuffer, std::ptrdiff_t iLength, bool bCopy = true) { Init(pBuffer, iLength, bCopy); }

		virtual ~CBuffer (void);

		CBuffer &operator= (const CBuffer &Src);
		CBuffer &operator= (CBuffer &&Src) noexcept;

		static CBuffer AsUTF8 (const IMemoryBlock& Stream);
		LPVOID GetHandoff (int *retiAllocation = NULL);
		int GetAllocSize (void) const { return max(m_iAllocation, m_iLength); }
		void TakeHandoff (CBuffer &Src);
		void TakeHandoff (void *pBuffer, int iAllocLength);

		//	IMemoryBlock virtuals
		virtual int GetLength (void) const override { return m_iLength; }
		virtual char *GetPointer (void) const override { return m_pBuffer; }
		virtual void SetLength (int iLength) override;

	private:
		void Copy (const CBuffer &Src);
		void Init (const void *pBuffer, size_t dwLength, bool bCopy);

		int m_iLength = 0;
		char *m_pBuffer = NULL;

		bool m_bAllocated = false;
		int m_iAllocation = 0;
	};

class CCircularBuffer : public IByteStream
	{
	public:
		CCircularBuffer (int iSize = 16 * 1024);
		CCircularBuffer (const CCircularBuffer &Src) = delete;
		CCircularBuffer (CCircularBuffer &&Src) noexcept { Move(Src); }

		virtual ~CCircularBuffer () { CleanUp(); }

		CCircularBuffer &operator= (const CCircularBuffer &Src) = delete;
		CCircularBuffer &operator= (CCircularBuffer &&Src) noexcept { CleanUp(); Move(Src); return *this; }

		int Scan (const CString &sString) const;

		//	IByteStream
		virtual int GetPos (void) override { return 0; }
		virtual int GetStreamLength (void) override;
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override { }
		virtual int Write (const void *pData, int iLength) override;
	
		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:
		void CleanUp ();
		void Move (CCircularBuffer &Src);

		char *m_pBuffer = NULL;
		int m_iAlloc = 0;

		int m_iReadPos = 0;
		int m_iWritePos = 0;
	};

class CMemoryBuffer : public CMemoryBlockImpl
	{
	public:
		CMemoryBuffer (int iMaxSize = 0);
		CMemoryBuffer (const CMemoryBuffer &Src) = delete;
		CMemoryBuffer (CMemoryBuffer &&Src) noexcept { Move(Src); }
		CMemoryBuffer (void *pSource, int iLength);

		virtual ~CMemoryBuffer (void) { CleanUp(); }

		CMemoryBuffer &operator= (const CMemoryBuffer &Src) = delete;
		CMemoryBuffer &operator= (CMemoryBuffer &&Src) noexcept { CleanUp(); Move(Src); return *this; }

		//	IMemoryBlock virtuals
		virtual int GetLength (void) const override { return m_iCurrentSize; }
		virtual char *GetPointer (void) const override { return m_pBlock; }
		virtual void SetLength (int iLength);

	private:
		void CleanUp ();
		bool IsConstant (void) { return (m_iMaxSize == -1); }
		void Move (CMemoryBuffer &Src);

		int m_iMaxSize = 0;
		int m_iCommittedSize = 0;
		int m_iCurrentSize = 0;

		char *m_pBlock = NULL;
	};

class CSharedMemoryBuffer : public CMemoryBlockImpl
	{
	public:
		CSharedMemoryBuffer (void) { }
		CSharedMemoryBuffer (const CSharedMemoryBuffer &Src) = delete;
		CSharedMemoryBuffer (CSharedMemoryBuffer &&Src) noexcept;

		virtual ~CSharedMemoryBuffer (void);

		CSharedMemoryBuffer &operator= (const CSharedMemoryBuffer &Src) = delete;
		CSharedMemoryBuffer &operator= (CSharedMemoryBuffer &&Src) noexcept;

		void Close (void);
		void Create (LPSTR sName, int iMaxSize, bool *retbExists = NULL);
		bool IsOpen (void) const { return (m_pBlock != NULL); }
		void SetMaxSize (int iMaxSize);

		//	IMemoryBlock virtuals
		virtual int GetLength (void) const override { return m_iMaxSize; }
		virtual char *GetPointer (void) const override { return m_pBlock; }

	private:
		HANDLE m_hMapFile = INVALID_HANDLE_VALUE;
		int m_iMaxSize = 0;
		char *m_pBlock = NULL;
	};

class CStringBuffer : public CMemoryBlockImpl
	{
	public:
		CStringBuffer (void) { }
		CStringBuffer (const CStringBuffer &Src) = delete;
		CStringBuffer (CStringBuffer &&Src) noexcept { Move(Src); }
		CStringBuffer (LPSTR pString) : m_pString(pString) { }
		CStringBuffer (const CString &sString) : m_pString((LPSTR)sString) { }
		CStringBuffer (CString &&Src) noexcept;
		explicit CStringBuffer (int iSize);
		~CStringBuffer (void) { CleanUp(); }

		CStringBuffer &operator= (const CStringBuffer &Src) = delete;
		CStringBuffer &operator= (CStringBuffer &&Src) noexcept	{ CleanUp(); Move(Src); return *this; }

		operator LPSTR () const { return m_pString; }
		operator const CString & () const { return *(CString *)&m_pString; }

		LPSTR Handoff (void);

		//	IMemoryBlock virtuals
		virtual int GetLength (void) const override { return (m_pString ? ((CString *)&m_pString)->GetLength() : 0); }
		virtual char *GetPointer (void) const override { return m_pString; }
		virtual void SetLength (int iLength) override;

	private:
		void CleanUp ();
		void Move (CStringBuffer &Src);
		char *GetBuffer (void) const { return (char *)(m_pString - sizeof(int)); }
		int GetLengthParameter (void) const { return *((int *)GetBuffer()); }
		void SetLengthParameter (int iLen) { *((int *)GetBuffer()) = iLen; }

		char *m_pString = NULL;
		int m_iAlloc = 0;
	};

//	Filters --------------------------------------------------------------------

class CBufferedIO : public IByteStream
	{
	public:
		CBufferedIO (IByteStream &Stream, int iBufferSize = 64 * 1024);
		CBufferedIO (const CBufferedIO &Src) = delete;
		CBufferedIO (CBufferedIO &&Src) = delete;

		~CBufferedIO (void);

		CBufferedIO &operator= (const CBufferedIO &Src) = delete;
		CBufferedIO &operator= (CBufferedIO &&Src) = delete;

		void Flush (void);

		//	IByteStream
		virtual int GetPos (void) override { return (m_bReadBuffer ? m_iBufferStart + m_iBufferPos : m_Stream.GetPos()); }
		virtual int GetStreamLength (void) override { return m_Stream.GetStreamLength(); }
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override;
		virtual int Write (const void *pData, int iLength) override;

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:
		IByteStream &m_Stream;
		int m_iBufferAlloc;					//	Total size of buffer
		int m_iBufferStart;					//	Start of buffer in stream
		int m_iBufferPos;					//	Current position in buffer
		int m_iBufferLen;					//	Valid portion of buffer
		char *m_pBuffer;
		bool m_bReadBuffer;                 //  Buffer contains read data
		bool m_bWriteBuffer;                //  Writes to buffer need to be flushed
	};

class CBase64Decoder : public IByteStream
	{
	public:
		CBase64Decoder (IByteStream *pInput, DWORD dwFlags = 0);

		//	IByteStream
		virtual int GetPos (void) override { return 0; }
		virtual int GetStreamLength (void) override;
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override { }
		virtual int Write (const void *pData, int iLength) override { return 0; }

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:
		BYTE CharToByte (char chChar);

		IByteStream *m_pStream;

		int m_iBufferPos;
		BYTE m_chBuffer[3];
	};

class CBase64Encoder : public IByteStream
	{
	public:
		static constexpr DWORD FLAG_BASE64_URL = 0x00000001;

		CBase64Encoder (IByteStream *pOutput, DWORD dwFlags = 0);
		CBase64Encoder (const CBase64Encoder &Src) = delete;
		CBase64Encoder (CBase64Encoder &&Src) = delete;

		~CBase64Encoder (void) { Close(); }

		CBase64Encoder &operator= (const CBase64Encoder &Src) = delete;
		CBase64Encoder &operator= (CBase64Encoder &&Src) = delete;

		void Close (void);

		//	IByteStream
		virtual int GetPos (void) override { return 0; }
		virtual int GetStreamLength (void) override { return 0; }
		virtual int Read (void *pData, int iLength) override { return 0; }
		virtual void Seek (int iPos, bool bFromEnd = false) override { }
		virtual int Write (const void *pData, int iLength) override;

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:
		int WriteTriplet (void *pData);

		IByteStream *m_pStream = NULL;

		int m_iBufferLen = 0;
		BYTE m_chBuffer[3] = { 0 };

		DWORD m_dwFlags = 0;
		const BYTE* m_pTable64 = NULL;
	};

class CHexDecoder : public IByteStream
	{
	public:
		CHexDecoder (const IByteStream& Input, DWORD dwFlags = 0);

		//	IByteStream

		virtual int GetPos (void) override { return 0; }
		virtual int GetStreamLength (void) override;
		virtual int Read (void *pData, int iLength) override;
		virtual void Seek (int iPos, bool bFromEnd = false) override { }
		virtual int Write (const void *pData, int iLength) override { return 0; }

		//	We want to inherit all the overloaded versions of Write.

		using IByteStream::Write;

	private:

		int CharToValue (char chChar) { return (chChar >= '0' && chChar <= '9' ? chChar - '0' : (chChar >= 'a' && chChar <= 'f' ? chChar - 'a' + 10 : (chChar >= 'A' && chChar <= 'F' ? chChar - 'A' + 10 : -1))); }

		IByteStream &m_Stream;
	};

//	Other ----------------------------------------------------------------------

class CCharStream
	{
	public:
		CCharStream (void);

		CString ComposeError (const CString &sError);
		IByteStream *GetByteStream (void) const { return m_pStream; }
		char GetChar (void) const { return m_chChar; }
		int GetLine (void) const { return m_iLine; }
		bool Init (IByteStream &Stream, int iStartingLine = 1);
		bool ParseQuotedString (DWORD dwFlags, CString *retsString = NULL);
		void ParseToEndOfLine (CString *retsLine = NULL);
		void ParseWhitespace (void);
		char ReadChar (void);
		void RefreshStream (void);
		void UnreadChar (int iChars = 1);

	private:
		IByteStream *m_pStream;
		int m_iStreamLeft;

		char m_chChar;
		int m_iLine;
	};

//	Inlines --------------------------------------------------------------------

inline int IByteStream::Write (const IMemoryBlock &Block) { return this->Write(Block.GetPointer(), Block.GetLength()); }
