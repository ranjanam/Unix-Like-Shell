#include <string>
#include <string.h>
#include <stdlib.h>
#include "header.h"
using namespace std;

string toStr(char array[]) {
	string str(array);
	return str;
}

int isSpace(char ch) {
	int i;
	for (i=0;i<SPACE_DELIM_NUM;i++) {
		if (ch == space[i]) {
			return 1;
		}
	}
	return 0;
}

char *removeSpace(char *ip) {
	char *arr, *end;
	while(isSpace(*ip)) ip++;
	if (*ip == 0) 
		return ip;
	end = ip + strlen(ip) - 1;
	while(end > ip && isspace(*end)) end--;
	*(end+1) = 0;
	return ip;
}
//remove leading and trailing space in command
void toChar(string str, char ip[]) {
	char *arr, *end;
	arr = (char *)calloc(MAX_LENGTH, sizeof(char));
	strcpy(arr, str.c_str());
	while(isSpace(*arr)) arr++;
	if (*arr == 0)  {
		string str(arr);
		strcpy(ip, str.c_str());
		return;
	}
	end = arr + strlen(arr) - 1;
	while(end > arr && isspace(*end)) end--;
	*(end+1) = 0;
	string str1(arr);
	strcpy(ip, str1.c_str());
}