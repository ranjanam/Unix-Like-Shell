#include <iostream>
#include <vector>
#include <map>
using namespace std;

#define MAX_LENGTH 1000
#define MAX_LENGTH2 100000
#define SPACE_DELIM_NUM 3
#define SHELL_BUILT_IN 5

char **parser (char *);
vector<char**> parseInput(char input[]); 
string toStr(char array[]);
int isSpace(char );
char *removeSpace(char *);
void toChar(string , char ip[]);
void replaceOperation(char ip[]);
void addToHistory(char ip[], int );
int isShellBuiltIn(char cmd[]);
int getPwd(char value[]);
int changeDirectory(char **, char value[]);
int exportVar(char **, char value[]);
void handleEcho(char **, char value[],int );
void handleHistoryCommand(char value[]);
int handleShellBuiltIns(char **, int , char value[], int);
void initHistory(char **);
void closeHistory();

extern char space[5];
extern char shellBuiltIns[10][10];
extern char oldDir[100];
extern vector<string> history;
extern map<string, string> exportVars;