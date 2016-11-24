#pragma once
#include <stdio.h>
#include "cbInclude.h"

inline char *cbGetLastPosOf(const char search, const char* toSearch)
{
	char *current = nullptr;
	while (*toSearch)
	{
		if (*toSearch++ == search)
		{
			current = (char *)toSearch - 1;
		}
	}
	return current;
}

inline void cbStrCopy(char* destination, const char* source)
{
	while(*source)
	{
		*destination++ = *source++;
	}
}

inline int cbStrCmp(const char* lhs, const char* rhs)
{
	while (*lhs && *rhs && *lhs == *rhs){ lhs++; rhs++; }
	return *lhs - *rhs;
}

inline char* cbItoA(int i, char b[], mem_size size) {
	int count = sprintf_s(b, size,"%d", i);
	return b + count;
}

inline char* cbFtoA(float i, char b[], mem_size size) {
	int count = sprintf_s(b, size, "%f", i);

	char * end = b + count;
	*end = 0;
	return ++end;
}

inline char* cbConcatStr(char* target, mem_size targetSize, char *first, mem_size firstLength, char *second, mem_size secondLength)
{
	Assert(targetSize >= firstLength + secondLength - 1);

	int index = 0;
	while(*first)
		target[index++] = *first++;
	while(*second)
		target[index++] = *second++;

	target[index] = 0;
	return target;
}

#define cbSwap(name) void name(void* lhs, void *rhs)
#define cbQuicksortCompare(name) int name(const void *lhs, const void *rhs)

inline void cbQuicksort(void* l, void *r, uint32 stride, int(*cmp)(const void*, const void*), void(*swap)(void*, void*))
{
	if(l<r)
	{
		void *i = l;
		void *j = (uint8 *)r - stride;
		void *pivot = r;
		
		while((uint8 *)i < (uint8 *)j)
		{
			while(cmp(i,pivot) <= 0 && i < r)
			{
				i = (uint8 *)i + stride;
			}
			while (cmp(j, pivot) >= 0 && j > l)
			{
				j = (uint8 *)j - stride;
			}
			if (i < j)
				swap(i, j);
		}
		
		if (cmp(i, pivot) > 0)
			swap(i, r);
		cbQuicksort(l, (uint8 *)i - stride, stride, cmp, swap);
		cbQuicksort((uint8 *)i + stride, r, stride, cmp, swap);
	}
}

inline void cbQuicksort(void *arr, uint32 size, uint32 stride, int(*cmp)(const void*, const void*), void(*swap)(void*, void*))
{
	cbQuicksort(arr, (uint8 *)arr + (stride * (size-1)), stride, cmp, swap);
}
