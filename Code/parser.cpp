#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "header.h"

void replaceOperation(char ip[]) {
	string str;
	int num, flag;
	char inst[100], *temp;
	switch(ip[1]) {
	flag = 0;
	case '!':
		if (history.size() == 0) {
			return;
		}
		str = history.at(history.size()-1);
		bzero(ip, sizeof(ip));
		strcpy(ip, str.c_str());
		break;
	case '-' :
		sscanf(ip+2, "%d", &num);
		num = history.size() -num;
		if (num<=0 || history.size()<num) {
			return;
		}
		str = history.at(num);
		bzero(ip, sizeof(ip));
		strcpy(ip, str.c_str());
		cout << ip << "\n";
		break;
	case ' ':
		addToHistory(ip, 0);
		strcpy(inst, ip+2);
		bzero(ip, sizeof(ip));
		strcpy(ip, inst);
		return;
	case '\0':
		bzero(ip, sizeof(ip));
		return;	
	default:
		if (ip[1]>='0' && ip[1]<='9') {
			sscanf(ip+1, "%d", &num);
			if (history.size()>num) {
				str = history.at(num-1);
				bzero(ip, sizeof(ip));
				strcpy(ip, str.c_str());
				addToHistory(ip, 0);
			}
		} else {
			temp = strtok(ip+1, " ");
			for (vector<string>::reverse_iterator it=history.rbegin(); it!=history.rend(); ++it) {
				str = *it;
				if (!strstr(temp, str.c_str())) {
					strcpy(inst, str.c_str());
					bzero(ip, sizeof(ip));
					strcpy(ip, inst);
					addToHistory(ip+1, 0);
					flag = 1;
					break;
				}
			}
			if (!flag) {
				bzero(ip, sizeof(ip));
				strcpy(ip, "Error");
			}
		}
		cout << ip << "\n";
		return;
	}
	return;
}


char **parser (char *input) {
	char **tokens, quote;
	int i = 0, position = 0, index;
	tokens = (char **)calloc(10, sizeof(char *));
	for (i=0;i<10;i++) {
		tokens[i] = (char *)calloc(MAX_LENGTH, sizeof(char));
	}
	i = 0;
	index = 0;
	input = removeSpace(input);
	while(input[i] != '\0') {
		switch(input[i]) {
		case '\\': i++;
			tokens[position][index++] = input[i];
			i++;
			break;
		case '\'':	
		case '"': quote = input[i];
			i++;
			while(input[i] != '\0') {
				if (input[i] == quote) {
					i++;
					break;
				} else if (input[i] == '\\' && input[i+1] == quote) {
					i++;
					tokens[position][index++] = input[i];
				} else if (input[i] == '\\') {
					i++;
					tokens[position][index++] = input[i];
				} else {
					tokens[position][index++] = input[i];
				}
				i++;
			}	
			break;
		case '<': 
		case '>': tokens[position][index] = input[i];
			i++;
			while(isSpace(input[i])) i++;
			index = 0;
			if (input[i]!= '\0') {
				position++;
			}
			break;
		case ' ': 
			while(isSpace(input[i])) i++;
			position++;				
			index = 0; 	
			break;
		default: tokens[position][index++] = input[i];
			i++;
			break;
		}
	}	
	if (!strcmp(tokens[0], "grep")){
		strcpy(tokens[++position], "--color=always");
	}
	tokens[++position] = NULL;
	return tokens;
}



vector<char**> parseInput(char input[]) {
	char ip[MAX_LENGTH], quote, **tokens;
	int i = 0, j;
	vector<char **> v;
	j = 0;
	quote = '\0';
	while(isSpace(input[i])) i++;
	while(input[i]!= '\0') {
		switch(input[i]) {
		case '\\': i++;
			ip[j++] = input[i];
			if (input[i]!='\0')
				i++;
			break;
		case '\'':	
		case '"': quote = input[i];
			ip[j++] = input[i];
			i++;
			while(input[i] != '\0') {
				if (input[i] == quote && input[i-1]!= '\\') {
					ip[j++] = input[i];
					break;
				} else {
					ip[j++] = input[i];
				}
				i++;
			}	
			if (input[i]!= '\0')
				i++;
			break;
		case '|': ip[j] = '\0';
			if (ip[0] == '!') {
				replaceOperation(ip);	
			}
			tokens = parser(ip);
			v.push_back(tokens);
			bzero(ip, sizeof(ip));
			j = 0;
			while(isSpace(input[i])) i++;
			if (input[i]!= '\0')
				i++;
			break;
		default : 	ip[j++] = input[i];
			i++;
		}
	}
	if (ip[0] != '\0') {
		ip[j] = '\0';
		if (ip[0] == '!') {
			replaceOperation(ip);	
		}
		tokens = parser(ip);
		v.push_back(tokens);
		bzero(ip, sizeof(ip));
		j = 0;
		while(isSpace(input[i])) i++;
		if (input[i]!= '\0')
			i++;
	}
	return v;
}