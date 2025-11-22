//	FoundationZipFile.h
//
//	Foundation header file
//	Copyright (c) 2023 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CZipFormat
	{
	public:

		static constexpr DWORD SIG_END_OF_CENTRAL_DIRECTORY = 0x06054b50;		//	NOTE: All are little-endian
		static constexpr DWORD SIG_CENTRAL_DIRECTORY_FILE_HEADER = 0x02014b50;
		static constexpr DWORD SIG_LOCAL_FILE_HEADER = 0x04034b50;

		static constexpr DWORD FLAG_ENCRYPTED = (1 << 0);
		static constexpr DWORD FLAG_UTF8 = (1 << 11);

		static constexpr DWORD VERSION = 20;
		static constexpr DWORD VERSION_MADE_BY_WINDOWS = 0x0A00;

		static constexpr DWORD COMP_NONE = 0;
		static constexpr DWORD COMP_DEFLATED = 8;

#pragma pack(push, 1)

		struct SEndOfCentralDirectory
			{
			DWORD dwSignature = 0;
			WORD wDiskNumber = 0;
			WORD wDiskStart = 0;
			WORD wDiskEntries = 0;
			WORD wTotalEntries = 0;
			DWORD dwCentralDirectorySize = 0;
			DWORD dwCentralDirectoryOffset = 0;
			WORD wCommentLen = 0;
			};

		struct SCentralDirectoryFileHeader
			{
			DWORD dwSignature = 0;
			WORD wVersion = 0;
			WORD wVersionNeeded = 0;
			WORD wFlags = 0;
			WORD wCompression = 0;
			WORD wModTime = 0;
			WORD wModDate = 0;
			DWORD dwCRC = 0;
			DWORD dwCompressedSize = 0;
			DWORD dwUncompressedSize = 0;
			WORD wFilenameLen = 0;
			WORD wExtraLen = 0;
			WORD wCommentLen = 0;
			WORD wDiskStart = 0;
			WORD wInternal = 0;
			DWORD dwExternal = 0;
			DWORD dwLocalHeaderOffset = 0;
			};

		struct SLocalFileHeader
			{
			DWORD dwSignature = 0;
			WORD wVersion = 0;
			WORD wFlags = 0;
			WORD wCompression = 0;
			WORD wModTime = 0;
			WORD wModDate = 0;
			DWORD dwCRC = 0;
			DWORD dwCompressedSize = 0;
			DWORD dwUncompressedSize = 0;
			WORD wFilenameLen = 0;
			WORD wExtraLen = 0;
			};

#pragma pack(pop)
	};

