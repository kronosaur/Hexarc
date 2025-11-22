//	CComplexBinaryFileFile.cpp
//
//	CComplexBinaryFileFile class
//	Copyright (c) 2015 by GridWhale Corporation. All Rights Reserved.

#include "stdafx.h"

#ifdef DEBUG
//#define DEBUG_BINARY_OBJECT
#endif

DECLARE_CONST_STRING(PATH_DATUM,					"AEON")

DECLARE_CONST_STRING(TYPENAME_BINARY_FILE,			"binaryFile")
const CString &CComplexBinaryFile::GetTypename (void) const { return TYPENAME_BINARY_FILE; }

const DWORD TIMEOUT_LOCK =							2000;
const DWORD AEON_FILE_SIGNATURE =					'AEBF';

CComplexBinaryFile::CComplexBinaryFile (IByteStream &Stream, int iLength)

//	CComplexBinaryFile constructor (we throw if we have any problems).

	{
	ASSERT(iLength >= 0);

	CreateBinaryFile(&m_sFilespec, &m_File);
	m_File.Write(Stream, iLength);
	m_dwLength = iLength;

#ifdef DEBUG_BINARY_OBJECT
	printf("Created %s\n", (LPSTR)m_sFilespec);
#endif
	}

CComplexBinaryFile::~CComplexBinaryFile (void)

//	CComplexBinaryFile destructor

	{
	DecrementRefCount();
	}

void CComplexBinaryFile::Append (IMemoryBlock &Data)

//	Append
//
//	Appends a memory block to the file.

	{
	int iAppendSize = Data.GetLength();
	if (iAppendSize <= 0)
		return;

	//	If we don't yet have a file, create one.

	if (!m_File.IsOpen())
		{
		CreateBinaryFile(&m_sFilespec, &m_File);
		m_dwLength = 0;
		}

	//	Otherwise, seek to the end

	else
		m_File.Seek(0, true);

	//	Write the data

	m_File.Write(Data);
	m_dwLength += iAppendSize;
	}

void CComplexBinaryFile::Append (CDatum dDatum)

//	Append
//
//	Appends binary data to the file.

	{
	int iAppendSize = dDatum.GetBinarySize();
	if (iAppendSize <= 0)
		return;

	//	If we don't yet have a file, create one with this data.

	if (!m_File.IsOpen())
		{
		CreateBinaryFile(&m_sFilespec, &m_File);
		dDatum.WriteBinaryToStream(m_File);
		m_dwLength = iAppendSize;
		}

	//	Otherwise, we append to the existing file.

	else

		{
		m_File.Seek(0, true);
		dDatum.WriteBinaryToStream(m_File);
		m_dwLength += iAppendSize;
		}
	}

CStringView CComplexBinaryFile::CastCString (void) const

//	CastCString
//
//	Casts to a CString

	{
#ifdef DEBUG_BINARY_OBJECT
	printf("Attempting to cast CComplexBinaryFile to CString.\n");
#endif

	return CStringView();
	}

IComplexDatum *CComplexBinaryFile::Clone (CDatum::EClone iMode) const

//	Clone
//
//	Makes a copy

	{
	CStringBuffer Src(CastCString());

#ifdef DEBUG_BINARY_OBJECT
	printf("Cloning CComplexBinaryFile\n");
#endif

	return new CComplexBinaryFile(Src, Src.GetLength());
	}

void CComplexBinaryFile::CreateBinaryFile (CString *retsFilespec, CFile *retFile)

//	CreateBinaryFile
//
//	Creates a unique temporary file and returns the filename. Throws an error 
//	if we have a problem.

	{
	static CString sTempPath;

	//	First make sure we have a path in the temporary directory

	if (sTempPath.IsEmpty())
		{
		CString sTempPathRoot = fileGetTempPath();
		if (sTempPathRoot.IsEmpty())
			throw CException(errFail);

		sTempPath = fileAppend(sTempPathRoot, PATH_DATUM);
		if (!filePathCreate(sTempPath))
			throw CException(errFail);
		}

	//	Create a unique temp file

	CString sFilespec;

	bool bSuccess = false;
	int iTries = 0;
	do
		{
		iTries++;

		//	Pick a random ID for the file

		CDateTime Now(CDateTime::Now);
		DWORD dwID1 = (DWORD)(Now.DaysSince1AD() - (2000 * 365));
		DWORD dwID2 = mathRandom(1, 65535);
		DWORD dwID3 = (DWORD)Now.MillisecondsSinceMidnight();

		//	Attempt to create a file of that name

		sFilespec = fileAppend(sTempPath, strPattern("%04x%04x_%08x.aeon", dwID1, dwID2, dwID3));
		bSuccess = retFile->Create(sFilespec, CFile::FLAG_CREATE_NEW | CFile::FLAG_ALLOW_WRITE);
		}
	while (!bSuccess && iTries < 20);

	//	Success?

	if (!bSuccess)
		throw CException(errOutOfDiskspace);

	if (retsFilespec)
		*retsFilespec = sFilespec;

	//	Write out the header

	SHeader Header;
	Header.dwSignature = AEON_FILE_SIGNATURE;
	Header.dwRefCount = 1;
	retFile->Write(&Header, sizeof(Header));
	}

bool CComplexBinaryFile::DecrementRefCount (void)

//	DecrementRefCount
//
//	Decrements the ref count on the file and return TRUE if we've decremented
//	to 0 (meaning we should delete the file).

	{
	if (!m_File.Lock(0, sizeof(DWORD), TIMEOUT_LOCK))
		throw CException(errFail);

	SHeader Header;
	m_File.Seek(0);
	m_File.Read(&Header, sizeof(Header));
	if (Header.dwRefCount > 1)
		{
		Header.dwRefCount--;

		m_File.Seek(0);
		m_File.Write(&Header, sizeof(Header));

		m_File.Unlock(0, sizeof(DWORD));
		m_File.Close();
		m_dwLength = 0;

		//	File is still held by someone, so we don't delete it.

		return false;
		}

	//	Otherwise, we delete the file

	else
		{
		m_File.Unlock(0, sizeof(DWORD));
		m_File.Close();
		m_dwLength = 0;

		fileDelete(m_sFilespec);

#ifdef DEBUG_BINARY_OBJECT
		printf("Deleted %s\n", (LPSTR)m_sFilespec);
#endif
		return true;
		}
	}

CDatum CComplexBinaryFile::GetElement (const CString &sKey) const
	{
#ifdef DEBUG_BINARY_OBJECT
	printf("Attempting to get element %s from CComplexBinaryFile\n", (LPSTR)sKey);
#endif

	return CDatum();
	}

void CComplexBinaryFile::IncrementRefCount (void) const

//	IncrementRefCount
//
//	Increments the ref count on the file.

	{
	ASSERT(m_File.IsOpen());

	if (!m_File.Lock(0, sizeof(DWORD), TIMEOUT_LOCK))
		throw CException(errFail);

	SHeader Header;
	m_File.Seek(0);
	m_File.Read(&Header, sizeof(Header));
	Header.dwRefCount++;

	m_File.Seek(0);
	m_File.Write(&Header, sizeof(Header));
	m_File.Unlock(0, sizeof(DWORD));

#ifdef DEBUG_BINARY_OBJECT
	printf("Increment ref on %s\n", (LPSTR)m_sFilespec);
#endif
	}

size_t CComplexBinaryFile::OnCalcSerializeSizeAEONScript (CDatum::EFormat iFormat) const

//	OnCalcSerializeSizeAEONScript
//
//	Returns an approximation of serialization size.

	{
	switch (iFormat)
		{
		case CDatum::EFormat::AEONLocal:
			return (3 * sizeof(DWORD)) + m_sFilespec.GetLength();

		default:
			return sizeof(DWORD) + GetLength();
		}
	}

bool CComplexBinaryFile::OnDeserialize (CDatum::EFormat iFormat, const CString &sTypename, IByteStream &Stream)

//	OnDeserialize
//
//	Deserialize

	{
	//	Release any file we have open

	if (m_File.IsOpen())
		DecrementRefCount();

	//	Read the length from the stream

	DWORD dwLength;
	Stream.Read(&dwLength, sizeof(DWORD));

	//	If this is 0xffffffff then it means we have a reference to a file.

	if (dwLength == 0xffffffff)
		{
		//	Read the actual length

		Stream.Read(&m_dwLength, sizeof(DWORD));

		//	Read the filespec

		m_sFilespec = CString::Deserialize(Stream);

		//	Open the file

		if (!m_File.Create(m_sFilespec, CFile::FLAG_ALLOW_WRITE))
			throw CException(errFail);

		//	We take ownership of the file, so we don't need to increment the
		//	ref count.

		//	LATER: We're not passing iFormat correctly, but in the future we 
		//	should pass formatAEONLocal or else throw an error.

#ifdef DEBUG_BINARY_OBJECT
		printf("Deserialized reference to %s\n", (LPSTR)m_sFilespec);
#endif
		}

	//	Otherwise, read the stream into a new file.

	else
		{
		CreateBinaryFile(&m_sFilespec, &m_File);
		m_File.Write(Stream, dwLength);
		m_dwLength = dwLength;

#ifdef DEBUG_BINARY_OBJECT
		printf("Deserialized into %s\n", (LPSTR)m_sFilespec);
#endif
		}

	return true;
	}

void CComplexBinaryFile::OnMarked (void)

//	OnMarked
//
//	Mark data in use

	{
#ifdef DEBUG_BINARY_OBJECT
	printf("Mark!\n");
#endif
	}

void CComplexBinaryFile::OnSerialize (CDatum::EFormat iFormat, IByteStream &Stream) const

//	OnSerialize
//
//	Serialize

	{
	switch (iFormat)
		{
		//	NOTE: We make two major assumptions here:
		//
		//	1.	The serialization is single-use. That is, only a single client will
		//		deserialize what we've serialized.
		//
		//	2.	The serialization is tied to the machine. That is, we cannot deserialize
		//		this stream on a different machine. (But it does work cross-process).

		case CDatum::EFormat::AEONLocal:
			{
			//	Write a length of 0xffffffff to indicate that this is a file reference.

			DWORD dwLength = 0xffffffff;
			Stream.Write(&dwLength, sizeof(DWORD));

			//	Write the actual length

			dwLength = GetLength();
			Stream.Write(&dwLength, sizeof(DWORD));

			//	Write the filespec

			m_sFilespec.Serialize(Stream);

			//	Now increment the reference to indicate that we should not 
			//	delete the file until it gets deserialized.

			IncrementRefCount();

#ifdef DEBUG_BINARY_OBJECT
			printf("Serialized reference to %s\n", (LPSTR)m_sFilespec);
#endif
			break;
			}

		//	Otherwise, we save the full file

		default:
			{
			DWORD dwLength = GetLength();

			Stream.Write(&dwLength, sizeof(DWORD));
			if (dwLength)
				{
				m_File.Seek(sizeof(SHeader));
				Stream.Write(m_File, dwLength);
				}
			}
		}
	}

CDatum CComplexBinaryFile::DeserializeAEON (IByteStream& Stream, DWORD dwID, CAEONSerializedMap &Serialized)
	{
#ifdef DEBUG_BINARY_OBJECT
	printf("Deserializing binary file\n");
#endif

	CComplexBinaryFile* pValue = new CComplexBinaryFile;
	CDatum dValue(pValue);

	DWORD dwLength = Stream.ReadDWORD();

	//	If this is 0xffffffff then it means we have a reference to a file.

	if (dwLength == 0xffffffff)
		{
		//	Read the actual length

		pValue->m_dwLength = Stream.ReadDWORD();

		//	Read the filespec

		pValue->m_sFilespec = CString::Deserialize(Stream);

		//	Open the file

		if (!pValue->m_File.Create(pValue->m_sFilespec, CFile::FLAG_ALLOW_WRITE))
			throw CException(errFail);

		//	We take ownership of the file, so we don't need to increment the
		//	ref count.

		//	LATER: We're not passing iFormat correctly, but in the future we 
		//	should pass formatAEONLocal or else throw an error.
		}

	//	Otherwise, create a new file.

	else
		{
		CreateBinaryFile(&pValue->m_sFilespec, &pValue->m_File);
		pValue->m_File.Write(Stream, dwLength);
		pValue->m_dwLength = dwLength;

		Stream.Read(NULL, AlignUp((int)dwLength, (int)sizeof(DWORD)) - (int)dwLength);
		}

	return dValue;
	}

void CComplexBinaryFile::SerializeAEON (IByteStream& Stream, CAEONSerializedMap& Serialized) const
	{
	Stream.Write(CDatum::SERIALIZE_TYPE_BINARY_FILE);

	if (Serialized.IsLocal())
		{
#ifdef DEBUG_BINARY_OBJECT
		printf("Serializing %s reference\n", (LPSTR)m_sFilespec);
#endif

		//	Write a length of 0xffffffff to indicate that this is a file reference.

		DWORD dwLength = 0xffffffff;
		Stream.Write(dwLength);

		//	Write the actual length

		Stream.Write(GetLength());

		//	Write the filespec

		m_sFilespec.Serialize(Stream);

		//	Now increment the reference to indicate that we should not 
		//	delete the file until it gets deserialized.

		IncrementRefCount();
		}
	else
		{
#ifdef DEBUG_BINARY_OBJECT
		printf("Serializing %s to AEON\n", (LPSTR)m_sFilespec);
#endif

		Stream.Write(GetLength());
		m_File.Seek(sizeof(SHeader));
		Stream.Write(m_File, GetLength());
		Stream.Write("    ", AlignUp(GetLength(), (int)sizeof(DWORD)) - GetLength());
		}
	}

void CComplexBinaryFile::WriteBinaryToStream (IByteStream &Stream, int iPos, int iLength, IProgressEvents *pProgress) const

//	WriteBinaryToStream
//
//	Writes the file to the stream

	{
#ifdef DEBUG_BINARY_OBJECT
	printf("Writing %s to stream\n", (LPSTR)m_sFilespec);
#endif

	if (!m_File.IsOpen() 
			|| (DWORD)iPos >= m_dwLength)
		return;

	if (iLength == -1)
		iLength = Max(0, (int)m_dwLength - iPos);
	else
		iLength = Min(iLength, (int)m_dwLength - iPos);

	m_File.Seek(sizeof(SHeader) + iPos);
	Stream.WriteStream(m_File, iLength);
	}

void CComplexBinaryFile::WriteBinaryToStream64 (IByteStream64 &Stream, DWORDLONG dwPos, DWORDLONG dwLength, IProgressEvents *pProgress) const
	{
	//	LATER: Handle 64-bit lengths
#ifdef DEBUG_BINARY_OBJECT
	printf("Writing %s to stream\n", (LPSTR)m_sFilespec);
#endif

	if (!m_File.IsOpen() 
			|| (DWORD)dwPos >= m_dwLength)
		return;

	if (dwLength == 0xffffffffffffffff)
		dwLength = Max((DWORD)0, m_dwLength - (DWORD)dwPos);
	else
		dwLength = Min((DWORD)dwLength, m_dwLength - (DWORD)dwPos);

	m_File.Seek(sizeof(SHeader) + (DWORD)dwPos);
	Stream.WriteStream(m_File, (DWORD)dwLength);
	}
