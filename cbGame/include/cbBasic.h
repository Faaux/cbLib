#pragma once
#include <cstdio>
#include <cbMemory.h>

inline char* cbItoA(int i, char b[], size_t size) {
	int count = sprintf_s(b, size,"%d", i);
	return b + count;
}

inline char* cbFtoA(float i, char b[], size_t size) {
	int count = sprintf_s(b, size, "%f", i);

	char * end = b + count;
	*end = 0;
	return ++end;
}

inline char* cbConcatStr(cbArena* arena,char *first, size_t firstLength, char *second, size_t secondLength)
{
	char * newString = (char *)PushSize(arena, firstLength + secondLength - 1);

	int index = 0;
	while(*first)
		newString[index++] = *first++;
	while(*second)
		newString[index++] = *second++;

	newString[index] = 0;
	return newString;
}
