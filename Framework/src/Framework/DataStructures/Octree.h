#pragma once
// Implementation of Octree in c++
#include <intsafe.h>
#include <iostream>
#include <vector>

namespace Engine
{

#define TLF 0
#define TRF 1
#define BRF 2
#define BLF 3
#define TLB 4
#define TRB 5
#define BRB 6
#define BLB 7

	// Structure of a Point
	struct Voxel
	{
		INT32 X;
		INT32 Y;
		INT32 Z;
		float Density;

		Voxel()
			: X(-1), Y(-1), Z(-1)
		{
		}

		Voxel(INT32 a, INT32 b, INT32 c)
			: X(a), Y(b), Z(c)
		{
		}
	};

	// Octree class
	class Octree
	{

		// if Point == NULL, node is an internal node.
		// if Point == (-1, -1, -1), node is empty.
		Voxel* Voxel;

		// Represent the boundary of the cube
		struct Voxel* TopLeftFront;
		struct Voxel* BottomRightBack;
		std::vector<Octree*> Children;

	public:
		// Constructor
		Octree()
		{
			// To declare empty node
			Voxel = new struct Voxel();
		}

		// Constructor with three arguments
		Octree(INT32 x, INT32 y, INT32 z)
		{
			// To declare Point node
			Voxel = new struct Voxel(x, y, z);
		}

		// Constructor with six arguments
		Octree(INT32 x1, INT32 y1, INT32 z1, INT32 x2, INT32 y2, INT32 z2)
		{
			// This use to construct Octree
			// with boundaries defined
			if (x2 < x1 || y2 < y1 || z2 < z1)
			{
				return;
			}

			Voxel = nullptr;

			TopLeftFront = new struct Voxel(x1, y1, z1);
			BottomRightBack = new struct Voxel(x2, y2, z2);

			// Assigning null to the children
			Children.assign(8, nullptr);
			for (INT32 i = TLF; i <= BLB; ++i)
			{
				Children[i] = new Octree();
			}
		}

		// Function to insert a Point in the octree
		void Insert(INT32 x, INT32 y, INT32 z)
		{

			// If the Point already exists in the octree
			if (Find(x, y, z)) 
			{
				return;
			}

			// If the Point is out of bounds
			if (x < TopLeftFront->X
				|| x > BottomRightBack->X
				|| y < TopLeftFront->Y
				|| y > BottomRightBack->Y
				|| z < TopLeftFront->Z
				|| z > BottomRightBack->Z) 
			{
				return;
			}

			// Binary search to insert the Point
			INT32 midx = (TopLeftFront->X + BottomRightBack->X) / 2;
			INT32 midy = (TopLeftFront->Y + BottomRightBack->Y) / 2;
			INT32 midz = (TopLeftFront->Z + BottomRightBack->Z) / 2;

			INT32 pos = -1;

			// Checking the octant of
			// the Point
			if (x <= midx)
			{
				if (y <= midy) 
				{
					if (z <= midz)
					{
						pos = TLF;
					}
					else
					{
						pos = TLB;
					}
				}
				else 
				{
					if (z <= midz)
					{
						pos = BLF;
					}
					else
					{
						pos = BLB;
					}
				}
			}
			else 
			{
				if (y <= midy) 
				{
					if (z <= midz)
					{
						pos = TRF;
					}
					else
					{
						pos = TRB;
					}
				}
				else 
				{
					if (z <= midz)
					{
						pos = BRF;
					}
					else
					{
						pos = BRB;
					}
				}
			}

			// If an internal node is encountered
			if (Children[pos]->Voxel == nullptr)
			{
				Children[pos]->Insert(x, y, z);
				return;
			}

			// If an empty node is encountered
			if (Children[pos]->Voxel->X == -1)
			{
				delete Children[pos];
				Children[pos] = new Octree(x, y, z);
				return;
			}
			
			INT32 x_ = Children[pos]->Voxel->X, y_ = Children[pos]->Voxel->Y, z_ = Children[pos]->Voxel->Z;
			delete Children[pos];

			Children[pos] = nullptr;
			if (pos == TLF) 
			{
				Children[pos] = new Octree(
					TopLeftFront->X,
					TopLeftFront->Y,
					TopLeftFront->Z,
					midx,
					midy,
					midz);
			}

			else if (pos == TRF) 
			{
				Children[pos] = new Octree(midx + 1,
					TopLeftFront->Y,
					TopLeftFront->Z,
					BottomRightBack->X,
					midy,
					midz);
			}
			else if (pos == BRF) 
			{
				Children[pos] = new Octree(midx + 1,
					midy + 1,
					TopLeftFront->Z,
					BottomRightBack->X,
					BottomRightBack->Y,
					midz);
			}
			else if (pos == BLF) 
			{
				Children[pos] = new Octree(TopLeftFront->X,
					midy + 1,
					TopLeftFront->Z,
					midx,
					BottomRightBack->Y,
					midz);
			}
			else if (pos == TLB)
			{
				Children[pos] = new Octree(TopLeftFront->X,
					TopLeftFront->Y,
					midz + 1,
					midx,
					midy,
					BottomRightBack->Z);
			}
			else if (pos == TRB) 
			{
				Children[pos] = new Octree(midx + 1,
					TopLeftFront->Y,
					midz + 1,
					BottomRightBack->X,
					midy,
					BottomRightBack->Z);
			}
			else if (pos == BRB)
			{
				Children[pos] = new Octree(midx + 1,
					midy + 1,
					midz + 1,
					BottomRightBack->X,
					BottomRightBack->Y,
					BottomRightBack->Z);
			}
			else if (pos == BLB)
			{
				Children[pos] = new Octree(TopLeftFront->X,
					midy + 1,
					midz + 1,
					midx,
					BottomRightBack->Y,
					BottomRightBack->Z);
			}
			Children[pos]->Insert(x_, y_, z_);
			Children[pos]->Insert(x, y, z);
			
		}

		// Function that returns true if the Point
		// (x, y, z) exists in the octree
		bool Find(INT32 x, INT32 y, INT32 z)
		{
			// If Point is out of bound
			if (x < TopLeftFront->X
				|| x > BottomRightBack->X
				|| y < TopLeftFront->Y
				|| y > BottomRightBack->Y
				|| z < TopLeftFront->Z
				|| z > BottomRightBack->Z)
			{
				return 0;
			}

			// Otherwise perform binary search
			// for each ordinate
			INT32 midx = (TopLeftFront->X + BottomRightBack->X) / 2;
			INT32 midy = (TopLeftFront->Y + BottomRightBack->Y) / 2;
			INT32 midz = (TopLeftFront->Z + BottomRightBack->Z) / 2;

			INT32 pos = -1;

			// Deciding the position
			// where to move
			if (x <= midx) 
			{
				if (y <= midy) 
				{
					if (z <= midz)
					{
						pos = TLF;
					}
					else
					{
						pos = TLB;
					}
				}
				else
				{
					if (z <= midz)
					{
						pos = BLF;
					}
					else
					{
						pos = BLB;
					}
				}
			}
			else 
			{
				if (y <= midy) 
				{
					if (z <= midz)
					{
						pos = TRF;
					}
					else
					{
						pos = TRB;
					}
				}
				else 
				{
					if (z <= midz)
						pos = BRF;
					else
						pos = BRB;
				}
			}

			// If an INT32ernal node is encountered
			if (Children[pos]->Voxel == nullptr) 
			{
				return Children[pos]->Find(x, y, z);
			}

			// If an empty node is encountered
			if (Children[pos]->Voxel->X == -1) 
			{
				return 0;
			}
			 

			// If node is found with
			// the given value
			if (x == Children[pos]->Voxel->X && y == Children[pos]->Voxel->Y && z == Children[pos]->Voxel->Z)
			{
				return 1;
			}
			
			return 0;
		}
	};

	//// Driver code
	//INT32 main()
	//{
	//	Octree tree(1, 1, 1, 5, 5, 5);

	//	tree.Insert(1, 2, 3);
	//	tree.Insert(1, 2, 3);
	//	tree.Insert(6, 5, 5);

	//	cout << (tree.Find(1, 2, 3)
	//		? "Found\n"
	//		: "Not Found\n");

	//	cout << (tree.Find(3, 4, 4)
	//		? "Found\n"
	//		: "Not Found\n");
	//	tree.Insert(3, 4, 4);

	//	cout << (tree.Find(3, 4, 4)
	//		? "Found\n"
	//		: "Not Found\n");

	//	return 0;
	//}

}
