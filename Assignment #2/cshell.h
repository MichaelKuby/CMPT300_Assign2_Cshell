#define ANSI_COLOUR_RED     "\e[0;31m"
#define ANSI_COLOUR_GREEN   "\e[0;32m"
#define ANSI_COLOUR_BLUE    "\e[0;34m"
#define EXIT_FAILURE 1
#define MAXIMUM 100

char **parseTokens(char *buf, int numArg, char *args[]);    // check for mem leaks
int numArgs(char* str);                                     // check for mem leaks
int makeEnvVar(char **args);                                // check for mem leaks
int checkEnvArgs(int num, char **args);                     // check for mem leaks
int changeTheme(char **args);                               // check for mem leaks
void makeLog(char **args, int retVal);                      // check for mem leaks
int nonBuInComm (char **args);                              // check for mem leaks
char **runscript(char **argv, char **argmnts);              // check for mem leaks