#pragma once
#include <cstdio>
#include <cbInclude.h>

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
