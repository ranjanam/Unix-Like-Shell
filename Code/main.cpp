//remove unnecesary headers

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <vector>
#include <map>
#include <signal.h>
#include <stdlib.h>
#include "header.h"
using namespace std;

char space[5] = {'\t', ' ', '\n'};
char shellBuiltIns[10][10] = {"cd", "pwd", "export", "echo", "history"};
char oldDir[100];
vector<string> history;
map<string, string> exportVars;

int hasOpRedirection(char **tokens, char fileName[]) {
	int i = 0;
	char name[100];
	while(tokens[i]!=NULL) {
		if (!strcmp(tokens[i], ">")) {
			if (tokens[i+1]!=NULL && strcmp(tokens[i+1], "--color=always")) {
				strcpy(fileName, tokens[i+1]);
				return 1;
			} else {
				cout << "No file specified " << name << "\n";
				return -1;
			}
		}
		i++;
	}
	return 0;
}

int hasIpRedirection(char **tokens, char fileName[]) {
	int i = 0, isExists, isAllowed;
	char name[100];
	while(tokens[i]!=NULL) {
		if (!strcmp(tokens[i], "<")) {
			if (tokens[i+1]!=NULL && strcmp(tokens[i+1], "--color=always")) {
				strcpy(name, tokens[i+1]);
				isExists = access(name, F_OK);
				isAllowed = access(name, R_OK);
				if (isExists == -1) {
					cout << "File does not exists " << name << "\n";
					return -1;
				} else if (isAllowed == -1) {
					cout << "Permission Denied " << name << "\n";
					return -1;
				} else {
					strcpy(fileName, name);
					return 1;
				}
			} else {
				cout << "No file specified " << name << "\n";
				return -1;
			}
		}
		i++;
	}
	return 0;
}

void removeTokens(char **tokens, int isIpRed) {
	int i = 0;
	char direction[2];
	if (isIpRed) {
		strcpy(direction, "<");
	} else {
		strcpy(direction, ">");
	}
	while (tokens[i]!='\0') {
		if (!strcmp(tokens[i],direction)) {
			while(tokens[i+2]) {
				strcpy(tokens[i],tokens[i+2]);
				i++;
			}
			tokens[i] = NULL;
		}
		i++;
	}
}


void signalHandler(int sigNum) {
	if (sigNum == SIGINT) {
		signal(SIGINT, SIG_IGN);
	}
	if (sigNum == SIGTSTP) {
		signal(SIGTSTP, SIG_IGN);
	}
}

void signalHandler1(int sigNum) {
	if (sigNum == SIGINT) {
		_exit(1);
	}
}
void executeShellBuiltIn(int fdIn[], int fdOut[], int isOpRedExists, char opFile[], char value[], int i) {
	int fp;
	if (isOpRedExists == -1) {
			return;
	}
	if (isOpRedExists == 1) {
		fp = open(opFile, O_CREAT | O_RDWR , 0777);
		if (!fp) {
			cout << "Error creating file " << opFile << "\n"; 
			return;
		}
		int saved_stdout = dup(1);
		dup2(fp, 1);
		if (write(fp, value, strlen(value))<=0) {
			cout << "Error while executing\n";
		}
		dup2(saved_stdout, 1);
		close(saved_stdout);
		return;
	} 
	if (fdOut[i] == -1) {
		if (value[0]!='\0') {
			if (write(1, value, strlen(value))<=0) {
				cout << "Error while executing\n";
			}
		}
	} else {
		if (value[0]!='\0') {
			if (write(fdOut[i], value, strlen(value))<=0) {
				cout << "Error while executing\n";
			}
			close(fdOut[i]);
		}
		
	}
}

void executeCommand(vector<char**> v) {
	if (!v.size()) {
		return;
	}
	int pid, retStatus, shellCmd, i, fp, size = v.size(), status;
	int fdIn[size], fdOut[size], fd[2], isIpRedExists, isOpRedExists;
	char **tokens, value[MAX_LENGTH2], ipFile[100], opFile[100];
	for (i=0;i<size;i++) {
		fdIn[i] = -1;
		fdOut[i] = -1;
	}
	for (i=1;i<size;i++) {
		pipe(fd);
		fdIn[i] = fd[0];
		fdOut[i-1] = fd[1]; 
	}
	for (i=0;i<size;i++) {
		tokens = v.at(i);
		shellCmd = isShellBuiltIn(tokens[0]);
		bzero(ipFile, sizeof(ipFile));
		bzero(opFile, sizeof(opFile));
		isOpRedExists = hasOpRedirection(tokens, opFile);
		isIpRedExists = hasIpRedirection(tokens, ipFile);
		if (shellCmd!=-1) {
			bzero(value, sizeof(value));
			if (size > 1) {
				pid = fork();
				if (pid == 0) {
					status = handleShellBuiltIns(tokens, shellCmd, value,0);
					if (status == -1 ) {
						cout << value;
						bzero(value, sizeof(value));
					} else {
						executeShellBuiltIn(fdIn, fdOut,isOpRedExists,opFile, value, i );
					}
					_exit(1);
				} else if (pid < 0) {
					cout << "error executing\n";
				} else {
					do {
				 		waitpid(pid, &retStatus, WUNTRACED);
		    		} while (!WIFEXITED(retStatus) && !WIFSIGNALED(retStatus));
			 		if (fdOut[i] != -1)
			    		close(fdOut[i]);
				}
			} else {
				status = handleShellBuiltIns(tokens, shellCmd, value, 1);
				if (status == -1 ) {
					cout << value;
					bzero(value, sizeof(value));
				} else {
					executeShellBuiltIn(fdIn, fdOut,isOpRedExists,opFile, value, i);
				}
			}
		} else {
			pid = fork(); 
			if (pid == 0) {
				signal(SIGINT, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				if (isIpRedExists == 1) {
					fp = open(ipFile, O_RDONLY);
					if (!fp) {
						cout << "cannot open file " << ipFile << "\n";  
					}
					removeTokens(tokens , 1);
					close(0);
					fdIn[i] = fp;
					dup2(fdIn[i], 0);
				} else if (isIpRedExists == -1) {
					return;
				} else if (fdIn[i] != -1) {
					close(0);
					dup2(fdIn[i], 0);
				}
				if (isOpRedExists == 1) {
					fp = open(opFile, O_CREAT | O_RDWR | O_TRUNC , 0777);
					if (!fp) {
						cout << "Error creating file " << opFile << "\n"; 
						return;
					}
					removeTokens(tokens , 0);
					close(1);
					fdOut[i] = fp;
					dup2(fdOut[i], 1);
				} else if (isOpRedExists == -1) {
					return;
				} else if (fdOut[i]!=-1) {
					close(1);
					dup2(fdOut[i], 1);
				}
				if (execvp(tokens[0], tokens)) {
		    		cout << "error executing\n";
				}
				_exit(1);
			}  else if (pid < 0) {
				cout << "error executing\n";
			} else {
				do {
			 	waitpid(pid, &retStatus, WUNTRACED);
	    		} while (!WIFEXITED(retStatus) && !WIFSIGNALED(retStatus));
		 		if (fdOut[i] != -1)
		    		close(fdOut[i]);
		    	if (i>0) {
		    		close(fdIn[i]);
		    	}
			}
		}
	}
	return;
}

void processInput(char input[]) {
	vector<char **> v;
	v = parseInput(input);
	executeCommand(v);
}

int main(int argc, char const *argv[], char **enVar)
{
	char *cwd, input[MAX_LENGTH];
	string shellName, inputCmd;
	initHistory(enVar);
	//env = enVar;
	signal(SIGINT, signalHandler);
	signal(SIGTSTP, signalHandler);
	while(1) {
		cwd = (char *)calloc(MAX_LENGTH, sizeof(char));
		getcwd(cwd, MAX_LENGTH);
		shellName = "MyShell:" + toStr(cwd) + ">";
		cout << shellName;
		getline(cin, inputCmd);
		bzero(input, sizeof(input));
		toChar(inputCmd, input);
		if (strcmp(input, "\n") && strlen(input)) {
			if (!strcmp(input, "exit")) {
				cout << "Bye..\n";
				closeHistory();
				exit(1);
			}
			if (strlen(input) == 1 && input[0] == '!') {
					addToHistory(input, 1);
			} else {
				addToHistory(input, 0);
				processInput(input);
				free(cwd);
			}
		}
	}
	return 0;
}