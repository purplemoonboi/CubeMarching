#pragma once
#include <intsafe.h>


namespace Foundation
{
	class MemoryManager
	{
	public:
		virtual ~MemoryManager() {};

		virtual void Allocate(UINT64 ptr, UINT64 memSize) = 0;

		virtual void Deallocate(UINT64 ptr, UINT64 memSize) = 0;
	};
}
