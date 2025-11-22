//	AEONAlgorithms.h
//
//	AEON Algorithms
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#pragma once

class CAEONArrayAlgorithm
	{
	public:

		static CDatum Except (CDatum dArray, CDatum dExclude, CDatum dOptions);
		static CDatum Intersect (CDatum dArray, CDatum dIntersect, CDatum dOptions);
		static CDatum Union (CDatum dArray, CDatum dUnion, CDatum dOptions);
	};