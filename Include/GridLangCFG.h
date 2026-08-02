//	GridLangCFG.h
//
//	GridLang Internals
//	Copyright (c) 2025 GridWhale Corporation. All Rights Reserved.

#pragma once

class CGridLangCFG
	{
	public:

	private:

		enum EEdgeType
			{
			Unknown,

			Normal,
			True,
			False,
			Return,							//	To node is a return node.
			Throw,							//	To node is a throw node.
			};

		struct SConstraints
			{
			CDatum dType;

			bool bTruthy = false;			//	If TRUE, definitely TRUE (otherwise, may be FALSE).
			bool bFalsy = false;			//	If TRUE, definitely FALSE (otherwise, may be TRUE).
			bool bNotNull = false;			//	If TRUE, definitely not NULL (otherwise, may be NULL).
			bool bConstant = false;
			CDatum dConstantValue;
			};

		struct SEdge
			{
			int iID = 0;					//	Index into m_Edges array.
			EEdgeType iType = EEdgeType::Unknown;
			int iFrom = -1;					//	Index into m_Nodes array.
			int iTo = -1;					//	Index into m_Nodes array.

			const IASTNode* pCondition = NULL;
			TSortMap<CString, SConstraints> m_Constraints;
			};

		struct SNode
			{
			int iID = 0;					//	Index into m_Nodes array.
			const IASTNode* pAST = NULL;
			TArray<int> OutEdges;
			TArray<int> InEdges;
			};

		TArray<SEdge> m_Edges;
		TArray<SNode> m_Nodes;
		int m_iStartNode = -1;
		int m_iEndNode = -1;
	};