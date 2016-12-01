#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "header.h"
FILE *historyFp;
char **enVar;

void initHistory(char **env) {
	char buffer[100];
	enVar = env;
	historyFp = fopen("history.txt","a+");
	if (!historyFp) {
		cout << "Error creating history file\n";
		return;
	}
	while(fgets(buffer, sizeof(buffer),historyFp)) {
		buffer[strlen(buffer)-1] = '\0';
		string str(buffer);
		history.push_back(str);
	}
}
void closeHistory() {
	fclose(historyFp);
}
void writeFile(string str) {
	char buffer[100];
	bzero(buffer, sizeof(buffer));
	strcpy(buffer, str.c_str());
	strcat(buffer, "\n");
	fwrite(buffer, sizeof(char), strlen(buffer), historyFp);
}

void addToHistory(char ip[], int forceAdd) {
	string str(ip);
	if (forceAdd) {
		if (history.size() <= 0) {
			history.push_back(str);
			writeFile(str);
		} else if (history.at(history.size()-1) != str) {
			history.push_back(str);
			writeFile(str);
		}
		return;
	}
	if (ip[0] == '!') {
		return;
	}
	if (history.size() <= 0) {
		history.push_back(str);
		writeFile(str);
	} else if (history.at(history.size()-1) != str) {
		history.push_back(str);
		writeFile(str);
	}
}

int isShellBuiltIn(char cmd[]) {
	int i;
	for (i=0;i<SHELL_BUILT_IN; i++) {
		if (!strcmp(cmd, shellBuiltIns[i])) {
			return i;
		}
	}
	if (cmd[0] == '!') {
		return i;
	}
	return -1;
}

int getPwd(char value[]) {
	getcwd(value, MAX_LENGTH-2);
	value[strlen(value)] = '\n';
	return 1;
}

int changeDirectory(char **tokens, char value[]) {
	char dir[100];
	if (!tokens[1]) {
		if(chdir("../")<0) {
			strcpy(value, "Error while changing directory\n");
			return -1;
		}
		strcpy(oldDir, "../");
		return 0;
	} else if (!strcmp(tokens[1], "~")) {
		bzero(oldDir, sizeof(oldDir));
		getPwd(oldDir);
		if(chdir(getenv("HOME"))<0) {
			strcpy(value, "Error while changing directory\n");
			return -1;
		}
		strcpy(value, getenv("HOME"));
		strcat(value, "\n");
		return 0;
	} else if (!strcmp(tokens[1], "-")) {
		strcpy(dir, oldDir);
		bzero(oldDir, sizeof(oldDir));
		getPwd(oldDir);
		if(chdir(dir)<0) {
			strcpy(value, "Error while changing directory\n");
			return -1;
		}
		getPwd(value);
		return 0;
	} else {
		if(chdir(tokens[1])<0) {
			strcpy(value, "Error while changing directory\n");
			return -1;
		}
		bzero(oldDir, sizeof(oldDir));
		strcpy(oldDir, tokens[1]);
		return 0;
	}
}

int exportVar(char **tokens, char value[]) {
	int i =1, j, t;
	char **e;
	char var[50], val[50], *enVal;
	bzero(val, sizeof(val));
	if (tokens[1]) {
		while(tokens[i]!=NULL) {
			j=0;
			while(tokens[i][j]!='\0' && tokens[i][j]!='=') {
				var[j] = tokens[i][j];
				j++;
			}
			j++;
			t=0;
			while(tokens[i][j]!='\0' && tokens[i][j]!=' ') {
				val[t] = tokens[i][j];
				t++;
				j++;
			}
			if (var[0]!='_' && !(var[0]>='A' && var[0]<='Z') && !(var[0]>='a' && var[0]<='z')) {
				strcpy(value, "Error identifier\n");
				return -1;
			}
			int len = strlen(var);
			j=0;
			while(len--) {
				if (var[j]!='_' && !(var[j]>='A' && var[j]<='Z') && !(var[j]>='a' && var[j]<='z') && !(var[j]>='0' && var[j]<='9')) {
					strcpy(value, "Error identifier\n");
					return -1;
				}
				j++;
			}
			enVal = getenv(var);
			if(enVal) {
				setenv(var,val,1);
			} else {
				if(exportVars.find(var)!=exportVars.end()) {
					exportVars[var] = val;
				} else
					exportVars[var] = val;
			}
			i++;
		}
	} else {
		for (e = enVar; *e != 0; e++) {
			strcat(value, *e);
			strcat(value, "\n");
		}
		for (map<string, string>::iterator it=exportVars.begin(); it!=exportVars.end(); ++it) {
			strcat(value, it->first.c_str());
			strcat(value, "=");
			strcat(value, "\"");
			strcat(value, it->second.c_str());
			strcat(value, "\"");
			strcat(value, "\n");
		}
	}
	return 0;
}

void handleEcho(char **tokens, char value[],int isParent) {
	int num;
	char *data;
	if (!tokens[1]) {
		bzero(value, sizeof(value));
		cout << "\n";
		return;
	}
	if (!strcmp(tokens[1], "$$")) {
		if (isParent) {
			num = getpid();
		} else {
			num = getppid();
		}
		sprintf(value, "%d", num);
		strcat(value, "\n");
		return;
	}
	int i = 1, len;
	bzero(value, sizeof(value));
	while(tokens[i]) {
		if(tokens[i][0] == '$') {
			len = strlen(tokens[i]);
			if (len == 1) {
				strcat(value, tokens[i]);
				strcat(value, " ");
			} else {
				char str[len];
				strncpy(str, tokens[i]+1, len-1);
				str[len-1]='\0';
				char *temp = getenv(str);	
				if (temp) {
					strcat(value, temp);
					strcat(value, " ");
				} else {
					if(exportVars.find(str) != exportVars.end()) {
						strcat(value, exportVars[str].c_str());
						strcat(value, " ");
					}
				}
			}	
		}
		else {
			if (!strcmp(tokens[i], "<") || !strcmp(tokens[i], ">")) {
				strcat(value, "\n");
				return ;
			}
			strcat(value, tokens[i]);
			strcat(value, " ");
		}
		i++;
	}
	strcat(value, "\n");
}

//handle whole history data
void handleHistoryCommand(char value[]) {
	int i=0;
	string str;
	char num[10];
	while(strlen(value) < MAX_LENGTH && i<history.size()) {
		str = history.at(i);
		bzero(num, sizeof(num));
		sprintf(num, "%d", i+1);
		strcat(value, num);
		strcat(value, " ");
		strcat(value, str.c_str());
		strcat(value, "\n");
		i++;
	}
}

int handleShellBuiltIns(char **tokens, int cmdNum, char value[], int isParent) {
	int status = 0;
	switch(cmdNum) {
		case 0: status = changeDirectory(tokens, value);
		break;
		case 1: status = getPwd(value);
			break;
		case 2: status = exportVar(tokens, value);
		break;
		case 3: handleEcho(tokens, value, isParent);
		break;
		case 4: handleHistoryCommand(value);	
		break;
	}
	return status;
}

