//	FoundationArraysDiff.h
//
//	Foundation header file
//	Copyright (c) 2024 GridWhale Corporation. All Rights Reserved.
//
//	USAGE
//
//	Automatically included by Foundation.h

#pragma once

class CArrayDiff
	{
	public:

		enum class ChangeType 
			{
			Inserted,
			Deleted,
			Modified
			};

		// Structure to represent an individual change

		struct Change
			{
			ChangeType Type;
			int iOldIndex = -1;
			int iNewIndex = -1;
			int iCount = 0;
			};

		// Struct to represent the results of the diff

		struct Results
			{
			TArray<Change> Changes;
			};
	};

template <class ARRAY, class VALUE> class TArrayDiff : public CArrayDiff
	{
	public:

		TArrayDiff (const ARRAY& OriginalArg, const ARRAY& NewArg) :
				Original(OriginalArg),
				New(NewArg),
				N(OriginalArg.GetCount()),
				M(NewArg.GetCount()),
				MAX(OriginalArg.GetCount() + NewArg.GetCount())
			{ }

		const ARRAY& Original;
		const ARRAY& New;

		const int N;
		const int M;
		const int MAX;

		//	In the original algorithm, the V array is indexed from -D to D. But 
		//	since Javascript doesn't have negative array indices, we offset it by
		//	MAX.

		int VI (int k) const { return k + MAX; }

		int VofI (const TArray<int>& V, int k) const { return (V[VI(k)] == -1 ? 0 : V[VI(k)]); }

		Results Diff ()

		//	Diff
		//
		//	This is an implementation of the simple Myers Difference Algorithm.
		//	From: An O(ND) Difference Algorithm and Its Variations, Eugene W. Myers.
		//	http://www.xmailserver.org/diff2.pdf
		//
		//	It doesn't implement the additional refinements in Section 4, so it 
		//	requires O(D^2) space, but for now, that's probably OK.

			{
			//	Short-circuit some degenerate cases

			if (N == 0 && M == 0)
				return Results();
			else if (N == 0)
				return Results({ Change{ ChangeType::Inserted, 0, 0, M } });
			else if (M == 0)
				return Results({ Change{ ChangeType::Deleted, 0, 0, N } });

			//	This is the algorithm from Figure 2 of Myers's paper.

			TArray<TArray<int>> VPath;
			VPath.InsertEmpty(MAX + 1);

			TArray<int> V;
			V.InsertEmpty(2 * MAX + 1);
			for (int i = 0; i < V.GetCount(); i++) V[i] = -1;

			int x = 0;	//	Index into oldLines
			int y = 0;	//	Index into newLines

			V[VI(1)] = 0;

			for (int D = 0; D <= MAX; D++)
				{
				for (int k = -D; k <= D; k += 2)
					{
					if (k == -D || (k != D && VofI(V, k - 1) < VofI(V, k + 1)))
						{
						x = VofI(V, k+1);
						}
					else
						{
						x = VofI(V, k-1) + 1;
						}

					y = x - k;
					if (y < 0)
						continue;

					while (x < N && y < M && Original.IsElementEqual(x, New, y))
						{
						x += 1;
						y += 1;
						}

					V[VI(k)] = x;

					if (x >= N && y >= M)
						{
						//	DONE!

						VPath[D] = V;
						auto diffPath = Path(VPath, D, N - M);
						return AsDiff(diffPath);
						}
					}

				//	Remember V because we need it to reconstruct the path when we're done.

				VPath[D] = V;
				}

			return Results();
			}

		TArray<TArray<int>> Path(TArray<TArray<int>>& VPath, int D, int k)

		//	Path
		// 
		//	This is a helper function that computes the final diff path
		//	through the edit graph. VPath is an array of V arrays indexed by D 
		//	value.

			{
			TArray<TArray<int>> result;

			while (D >= 0)
				{
				auto& V = VPath[D];
				int x0 = (D == 0) ? VofI(V, k) : V[VI(k)];

				if (x0 == -1)
					{
					return TArray<TArray<int>>();
					}

				int y0 = x0 - k;

				if (D == 0)
					{
					if (x0 == 0 && y0 == 0)
						{
						result.Insert(TArray<int>({0, 0}));
						}
					else
						{
						result.Insert(TArray<int>({x0, y0}));
						result.Insert(TArray<int>({0, 0}));
						}
					break;
					}
				else
					{
					//	NOTE: The original paper is not entirely clear on how to figure out 
					//	whether to take the k+1 or the k-1 paths. We just take the one with the
					//	smallest number of straight (non-diagonal) segments.

					auto& VDm1 = VPath[D - 1];
					bool bKMinus1Valid = VDm1[VI(k - 1)] != -1;
					bool bKPlus1Valid = VDm1[VI(k + 1)] != -1;

					if (bKMinus1Valid && !bKPlus1Valid)
						{
						k = k - 1;
						}
					else if (!bKMinus1Valid && bKPlus1Valid)
						{
						k = k + 1;
						}
					else
						{
						int x1 = VofI(VDm1, k - 1);
						int y1 = x1 - (k - 1);

						int x2 = VofI(VDm1, k + 1);
						int y2 = x2 - (k + 1);

						int x0_1 = x0;
						int y0_1 = y0;
						int x0_2 = x0;
						int y0_2 = y0;

						int snake1 = 0;
						int snake2 = 0;
						int snakeSize = 0;

						while (Original.IsIndexValid(x0 - 1 - snakeSize) &&
								New.IsIndexValid(y0 - 1 - snakeSize) &&
								Original.IsElementEqual(x0 - 1 - snakeSize, New, y0 - 1 - snakeSize))
							{
							if (x0_1 > x1 && y0_1 > y1)
								{
								x0_1--;
								y0_1--;
								snake1++;
								}

							if (x0_2 > x2 && y0_2 > y2)
								{
								x0_2--;
								y0_2--;
								snake2++;
								}
							snakeSize++;
							}

						int max1 = (x0_1 - x1) + (y0_1 - y1);
						int max2 = (x0_2 - x2) + (y0_2 - y2);

						if (max1 == 1 && max2 == 1)
							{
							k = (snake1 >= snake2) ? k - 1 : k + 1;
							}
						else if (max1 == 1)
							{
							k = k - 1;
							}
						else if (max2 == 1)
							{
							k = k + 1;
							}
						else
							{
							return TArray<TArray<int>>();
							}
						}
					}

				result.Insert(TArray<int>({x0, y0}));
				D--;
				}

			result.Reverse();
			return result;
			}

		Results AsDiff (TArray<TArray<int>>& path)

		//	AsDiff
		// 
		//	path is an array of coordinates in the edit graph [x, y]. We convert 
		//	those into diff instructions.

			{
			Results result;
			int iOriginalPos = 0;
			int iNewPos = 0;

			for (int i = 1; i < path.GetCount(); i++)
				{
				auto& prevPos = path[i - 1];
				auto& pos = path[i];

				int dX = pos[0] - prevPos[0];
				int dY = pos[1] - prevPos[1];

				if (dX == dY)
					{
					//	Move dX lines from original to new

					iOriginalPos += dX;
					iNewPos += dX;
					}
				else if (dX > dY)
					{
					//	Delete dX - dY lines from original

					int iCount = dX - dY;

					//	If this is an adjacent deletion, then we merge them into a single deletion.

					bool bMerged = false;
					if (result.Changes.GetCount() > 0)
						{
						auto& lastChange = result.Changes[result.Changes.GetCount() - 1];
						if (lastChange.Type == ChangeType::Deleted
								&& lastChange.iOldIndex == iOriginalPos - lastChange.iCount
								&& lastChange.iNewIndex == iNewPos)
							{
							lastChange.iCount += iCount;
							bMerged = true;
							}
						}

					if (!bMerged)
						result.Changes.Insert(Change{ ChangeType::Deleted, iOriginalPos, iNewPos, iCount });

					iOriginalPos += iCount;

					if (dY > 0)
						{
						iOriginalPos += dY;
						iNewPos += dY;
						}
					}
				else
					{
					//	Insert dY - dX lines from new

					int iCount = dY - dX;

					//	If this is an adjacent insertion, then we merge them into a single insertion.

					bool bMerged = false;
					if (result.Changes.GetCount() > 0)
						{
						auto& lastChange = result.Changes[result.Changes.GetCount() - 1];
						if (lastChange.Type == ChangeType::Inserted
								&& lastChange.iOldIndex == iOriginalPos
								&& lastChange.iNewIndex == iNewPos - lastChange.iCount)
							{
							lastChange.iCount += iCount;
							bMerged = true;
							}
						}

					if (!bMerged)
						result.Changes.Insert(Change{ ChangeType::Inserted, iOriginalPos, iNewPos, iCount });

					iNewPos += iCount;

					if (dX > 0)
						{
						iOriginalPos += dX;
						iNewPos += dX;
						}
					}
				}

			//	Do a post-processing pass where we convert adjacent insertions and
			//	deletions into modifications.

			for (int i = 0; i < result.Changes.GetCount() - 1; i++)
				{
				auto& change1 = result.Changes[i];
				auto& change2 = result.Changes[i + 1];

				if (change1.Type == ChangeType::Inserted && change2.Type == ChangeType::Deleted)
					{
					if (change1.iOldIndex == change2.iOldIndex
							&& change1.iCount == change2.iCount
							&& change1.iNewIndex == change2.iNewIndex - change1.iCount)
						{
						change1.Type = ChangeType::Modified;
						result.Changes.Delete(i + 1);
						i--;
						}
					}
				else if (change1.Type == ChangeType::Deleted && change2.Type == ChangeType::Inserted)
					{
					if (change1.iOldIndex + change1.iCount == change2.iOldIndex
							&& change1.iNewIndex + change1.iCount == change2.iNewIndex)
						{
						change1.Type = ChangeType::Modified;
						change1.iCount += change2.iCount;
						result.Changes.Delete(i + 1);
						i--;
						}
					}
				}

			return result;
			}
	};

