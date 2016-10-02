#pragma once
#include <cstdio>

inline char* cbItoA(int i, char b[], int size) {
	int count = sprintf_s(b, size,"%d", i);
	return b + count;
}

inline char* cbFtoA(float i, char b[], int size) {
	int count = sprintf_s(b, size, "%f", i);

	char * end = b + count;
	*end = 0;
	return ++end;
}

inline char* cbConcatStr(char *first, int firstLength, char *second, int secondLength)
{
	char * newString = (char *)malloc(firstLength + secondLength - 1);

	int index = 0;
	while(*first)
		newString[index++] = *first++;
	while(*second)
		newString[index++] = *second++;

	newString[index] = 0;
	return newString;
}
