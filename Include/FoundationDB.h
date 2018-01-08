//	FoundationDB.h
//
//	Foundation header file
//	Copyright (c) 2018 Kronosaur Productions, LLC. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class IDBValueObject;

class CDBValue
	{
	public:
		enum ETypes
			{
			typeUnknown =			-1,

			typeNil =				0,
			typeTrue =				1,
			typeInt32 =				2,
			typeInt64 =				3,
			typeDouble =			4,
			typeString =			5,
			typeArray =				6,
			typeBinary =			7,
			typeDateTime =			8,
			typeIntIP =				9,
			typeStruct =			10,
			typeObject =			11,
			};

		CDBValue (void) : m_dwData(0) { }
		CDBValue (const CDBValue &Src) { Copy(Src); }
		explicit CDBValue (ETypes iType);

		~CDBValue (void) { CleanUp(); }

		CDBValue &operator= (const CDBValue &Src) { CleanUp(); Copy(Src); return *this; }

	private:
		static constexpr DWORD DISCRIMINATOR_1_MASK =	0x00000003;
		static constexpr DWORD DISCRIMINATOR_2_MASK =	0x0000000f;

		static constexpr DWORD TYPE_STRING =			0x00;
		static constexpr DWORD TYPE_OBJECT =			0x01;
		static constexpr DWORD TYPE_INT_32 =			0x03;
		static constexpr DWORD TYPE_INT_60 =			0x07;
		static constexpr DWORD TYPE_DOUBLE =			0x0b;
		static constexpr DWORD TYPE_SPECIAL =			0x0f;

		static constexpr DWORDLONG SPECIAL_TRUE =		0x000000000000010f;

		void CleanUp (void);
		void Copy (const CDBValue &Src);

		static DWORD DecodeDiscriminator1 (DWORDLONG dwData) { return (DWORD)dwData & DISCRIMINATOR_1_MASK; }
		static DWORD DecodeDiscriminator2 (DWORDLONG dwData) { return (DWORD)dwData & DISCRIMINATOR_2_MASK; }
		static IDBValueObject *DecodeObject (DWORDLONG dwData) { return (IDBValueObject *)(dwData & ~(DWORDLONG)DISCRIMINATOR_1_MASK); }
		static LPSTR DecodeString (DWORDLONG dwData) { return (LPSTR)(dwData & ~(DWORDLONG)DISCRIMINATOR_1_MASK); }
		static DWORDLONG EncodeDouble (double rValue);
		inline static DWORDLONG EncodeInt32 (int iValue) { return ((((DWORDLONG)(DWORD)iValue) << 32) | TYPE_INT_32); }
		inline static DWORDLONG EncodeObjectPtr (IDBValueObject *pValue) { return (((DWORDLONG)pValue) | TYPE_OBJECT); }
		static DWORDLONG EncodeString (const CString &sValue);

		DWORDLONG m_dwData;
	};

class IDBValueObject
	{
	public:
		virtual ~IDBValueObject (void) { }

		virtual IDBValueObject *Clone (void) const = 0;
	};
