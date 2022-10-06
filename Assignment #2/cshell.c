#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cshell.h"

/*
Environmental Structs
*/

typedef struct envVar
{
    char *name;
    char *value;
} EnvVar;

EnvVar envVars[MAXIMUM];
int envCount = 0;

/*
Log Structs
*/

typedef struct
{
    char *name;
    struct tm time;
    int value;
} Command;

Command logs[MAXIMUM];
int logCount = 0;

/* Create a script flag to denote potential for script sequence */
int scriptFlag = 0;

FILE *input;

int main (int argc, char *argv[])
{
    while (1)
    {
        //Prepare to receive a variable list of arguments from the user or script
        char buf[255];
        
        if (argc != 1) // Script mode
        {
            if (scriptFlag == 0)
            {
                input = fopen(*(argv+1), "r");

                if (input ==  NULL)
                {
                    printf("Unable to read script file: %s\n", *(argv + 1));
                    exit (1);
                }

                scriptFlag = 1;
            }

            if (fgets(buf, sizeof(buf), input) == NULL)
            {
                puts("Bye!");
                exit (0);
            }
            char *token = "\r";
            strtok(buf, token);
        }

        /* Interactive mode */
        if (scriptFlag == 0)
        {
            // Print promt and wait for user input
            printf("cshell$ ");
            fgets(buf, sizeof(buf), stdin);
        }

        // Parse the input
        if (strcmp(buf, "\n") == 0)
            ;
        else
        {
            // Get the number of arguments in the string
            int numArg = numArgs(buf);  
            
            // Create an array of char pointers to hold the arguments
            char **args;
            int i;
            args = (char **) malloc(sizeof(char) * 255);
            for (i = 0; i < numArg; i++)
                args[i] = (char *) malloc(sizeof(char) * 255);
            
            args = parseTokens(buf, numArg, args);
            
            /*
            Update the value of arguments that have the same name as environmental variables
            */

            int err = checkEnvArgs(numArg, args); 
            if (err == 1)
            {
                makeLog(args, err);
                continue;
            }
            /*
            Check details of first user argument
            If $ --> EnvVar
            Else if exit or log or theme or print --> Built-in Command
            Else non built-in command
            */

            if (*(*(args+0)+0) == '$')
            {
                int i = makeEnvVar(args);
                makeLog(args, i);
            }
            else if (strcmp(*args, "exit") == 0)
            {
                puts("Bye!");
                exit (0);
            }
            else if(strcmp(*args, "log") == 0)
            {
                // print log
                for (int i = 0; i < logCount; i++)
                {
                    printf("%s", asctime(&logs[i].time));
                    printf(" %s %d", logs[i].name, logs[i].value);
                    printf("\n");
                }
                makeLog(args,0);
            }
            else if(strcmp(*args, "theme") == 0)
            {
                int i = changeTheme(args);
                makeLog(args, i);
            }
            else if(strcmp(*args, "print") == 0) //***************************
            {        
                int i;
                for (i = 1; i < numArg; i++)
                {
                    printf("%s ", *(args+i));
                }
                printf("\n");

                makeLog(args, 0);
            }
            else
            {
                /* non-built-in command process commence */
                nonBuInComm(args);
            }
            free(args);
        }
    }

    exit (0);
}

char **parseTokens(char *buf, int numArg, char **args)
{   
    // Put the arguments in the array
    int i = 0;
    char* currArg;
    char *token = " \n";
    currArg = strtok(buf, token);
    while (i < numArg)
    {   
        strcpy(*(args + i), currArg);
        i++;
        currArg = strtok ( NULL, token ) ;
    }
    args[numArg] = NULL;

    return args;
}

int numArgs(char* str)
{
    // Create a local copy of the string so we do not modify the original
    char *strcopy = NULL;
    strcopy = (char *) malloc(sizeof(char) * strlen(str));
    strcopy = strcpy(strcopy, str);

    // Count number of args in a str
    int numArgs = 0;
    char *sep = " ";
    char *current;
    current = (char *) malloc (sizeof(char) * strlen(str));
    
    current = strtok(strcopy, sep);
    while (current != NULL && (strcmp(current, "\n") != 0))
    {
        numArgs++;
        current = strtok(NULL, sep);
    }

    free(strcopy);
    free(current);

    return numArgs;
}

int makeEnvVar(char **args)
{
    // Separate $ <name> = <val>
    
    char *token;
    token = (char *) malloc (sizeof(char) * strlen(*args));
    strcpy(token, *args);

    int j = 1;

    char *name;
    int i = 0;
    int len = strlen(token);
    name = (char *) malloc (len*sizeof(char));

    while (*(token+j) != '=' && *(token+j) != '\0')
    {   
        *(name+i) = *(token+j);
        i++;
        j++;
    }
    
    *(name+i) = '\0';

    char *val;
    val = (char *) malloc (len *sizeof(char));
    int k = 0;

    if (*(token+j) != '=')
    {
        printf("Variable value expected\n");
        return 1;
    }
    else
    {
        j++;
        while (*(token+j) != '\0')
        {
            *(val+k) = *(token+j);
            k++;
            j++;
        }
        *(val+k) = '\0';
    }
    if (*val == '\0')
    {
        printf ("Variable value expected\n");
        return 1;
    }
    // Note that now name == <name> and val == <val>

    // Check to see if the variable name already exists

    int q = 0, m = -1;
     while ( q < envCount && ((m = strcmp(name, envVars[q].name)) != 0) )
        q++;

    if (m == 0)
    {   
        //** need to reallocate here***/
        strcpy(envVars[q].value, val);
    }
    else
    {
        envVars[envCount].name = (char *) malloc (sizeof(char)*strlen(name));
        envVars[envCount].value = (char *) malloc (sizeof(char)*strlen(val));
        strcpy(envVars[envCount].name, name);
        strcpy(envVars[envCount].value, val);
        envCount++;
    }

    free(name);
    free(val);
    free(token);

    return 0;
}

int checkEnvArgs(int num, char **args)
{   
    char *current;
    char *token = "$";

    int i = 1;
    for (i; i < num; i++) // for each arg
    {
        int j = 0;
        if (*(*(args+i)+0) == '$') // if arg starts with $
        {
            current = strtok(*(args + i), token);
            int update = 0;

            for (j; j < envCount; j++) // for each envVar
            {
                if (strcmp(current, envVars[j].name) == 0)
                {
                    strcpy(*(args+i), envVars[j].value);
                    update = 1;
                }
            }
            
            if (update == 0)
            {
                printf("Variable %s not found.\n", current);
                return 1;
            }
        }
    }
    return 0;
}

int changeTheme(char **args)
{
    if (strcmp(*(args + 1), "red") == 0)
        printf(ANSI_COLOUR_RED);
    else if (strcmp(*(args + 1), "blue") == 0)
        printf(ANSI_COLOUR_BLUE);
    else if (strcmp(*(args + 1), "green") == 0)
        printf(ANSI_COLOUR_GREEN);
    else
    {
        puts("Unsupported theme.");
        return 1;
    }
    return 0;
}

void makeLog(char **args, int retVal)
{
    // insert log name

    logs[logCount].name = (char *) malloc (sizeof(char)*strlen(*(args+0)));
    sprintf(logs[logCount].name, "%s", *args);
    logs[logCount].value = retVal;

    // insert log time

    time_t t;
    t = time(0);
    struct tm *timeInfo;
    timeInfo = localtime(&t);
    logs[logCount].time = *timeInfo;

    // insert log value

    logCount++;

    return;
}

int nonBuInComm (char **args)
{
    /* Ready new process */
    pid_t pid;
    
    /* Set up pipes */
    int fdsOut[2];  // pipe for stdout
    int fdsErr[2];  // pipe for stderr
    int fdsExecFlag[2]; // pipe for exec success or failure
    
    if (pipe(fdsOut) == -1)
    {
        perror ("fdsOut error: ");
        return 1;
    }
    
    if (pipe(fdsErr) == -1)
    {
        perror ("fdsErr error: ");
        return 1;
    }

    if (pipe(fdsExecFlag) == -1)
    {
        perror ("fdsExecFlag error: ");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        fprintf(stderr, "Fork failed");
        return 1;
    }
    else if (pid == 0) /* child process */
    {
        if (dup2(STDOUT_FILENO, fdsOut[1]) < 0) // child writes stdout to fdsOut[1]
            printf("Unable to duplicate file descriptor.");
        if (dup2(STDERR_FILENO, fdsErr[1]) < 0) // child writes stderr to fdsErr[1]
            printf("Unable to duplicate file descriptor");

        char *dir = "/usr/bin/";
        char *file = *args;
        char pname[30];

        strcpy(pname, dir);
        strcat(pname, file);

        execvp(pname, args);

        // Notify the parent that execvp failed

        int i = 1;
        close(fdsExecFlag[0]);
        write(fdsExecFlag[1], &i, 1);
        close(fdsExecFlag[1]);

        exit (0);
    }
    else /* parent process */
    {
        int status;
        
        wait(&status);

        char buff[255];

        /* Parent process closes the write end of out */
        close(fdsOut[1]);
        /* Parent process reads from the read end until failure or EOF */
        while (read(fdsOut[0], buff, 255) == 1)
            printf("%s", buff);
        /* Parent closes the read end when finished reading */
        close(fdsOut[0]);

        /* Parent process closes the write end of err */
        close(fdsErr[1]);
        /* Parent process reads from the read end until failure or EOF */
        while (read(fdsErr[0], buff, 255) == 1)
            printf("%s", buff);
        /* Parent closes the read end when finished reading */
        close(fdsErr[0]);

        int flag = 0;

        /* Parent process closes the write end of ExexFlag */
        close(fdsExecFlag[1]);
        /* Parent reads the value sent by the child (if any) */
        while (read(fdsExecFlag[0], &flag, 1) == 1);
        
        if (flag == 1) // execvp in child process did not recognize the command
        {
            printf ("Missing keyword or command, or permission problem\n");
            makeLog(args, 1);
        }
        else // execvp executed normally in child process
            makeLog(args, 0);
        close(fdsExecFlag[0]);
    }
    return 0;
}

char **runscript(char **argv, char **argmnts)
{
    char str[255];
    FILE *input;
    input = fopen(*(argv+1), "r");
    
    if (input ==  NULL)
    {
        perror ("Error opening file");
        exit (1);
    }

    int i = 0;
    while (fgets(str, sizeof(str), input) != NULL)
    {
        strcpy(*(argmnts+i), str);
        i++;
    }

    fclose(input);

    return argmnts;
}