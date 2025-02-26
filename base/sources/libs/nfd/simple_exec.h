// copied from: https://github.com/wheybags/simple_exec/blob/5a74c507c4ce1b2bb166177ead4cca7cfa23cb35/simple_exec.h

// simple_exec.h, single header library to run external programs + retrieve their status code and output (unix only for now)
//
// do this:
// #define SIMPLE_EXEC_IMPLEMENTATION
//   before you include this file in *one* C or C++ file to create the implementation.
// i.e. it should look like this:
// #define SIMPLE_EXEC_IMPLEMENTATION
// #include "simple_exec.h"

#ifndef SIMPLE_EXEC_H
#define SIMPLE_EXEC_H

int runCommand(char** stdOut, int* stdOutByteCount, int* returnCode, int includeStdErr, char* command, ...);
int runCommandArray(char** stdOut, int* stdOutByteCount, int* returnCode, int includeStdErr, char* const* allArgs);

#endif // SIMPLE_EXEC_H

#ifdef SIMPLE_EXEC_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdarg.h>
#include <fcntl.h>

#define release_assert(exp) { if (!(exp)) { abort(); } }

enum PIPE_FILE_DESCRIPTORS
{
  READ_FD  = 0,
  WRITE_FD = 1
};

enum RUN_COMMAND_ERROR
{
    COMMAND_RAN_OK = 0,
    COMMAND_NOT_FOUND = 1
};

int runCommandArray(char** stdOut, int* stdOutByteCount, int* returnCode, int includeStdErr, char* const* allArgs)
{
    // adapted from: https://stackoverflow.com/a/479103

    int bufferSize = 256;
    char buffer[bufferSize + 1];

    int dataReadFromChildDefaultSize = bufferSize * 5;
    int dataReadFromChildSize = dataReadFromChildDefaultSize;
    int dataReadFromChildUsed = 0;
    char* dataReadFromChild = (char*)malloc(dataReadFromChildSize);


    int parentToChild[2];
    release_assert(pipe(parentToChild) == 0);

    int childToParent[2];
    release_assert(pipe(childToParent) == 0);

    int errPipe[2];
    release_assert(pipe(errPipe) == 0);

    pid_t pid;
    switch( pid = fork() )
    {
        case -1:
        {
            release_assert(0 && "Fork failed");
            break;
        }

        case 0: // child
        {
            release_assert(dup2(parentToChild[READ_FD ], STDIN_FILENO ) != -1);
            release_assert(dup2(childToParent[WRITE_FD], STDOUT_FILENO) != -1);
            
            if(includeStdErr)
            {
                release_assert(dup2(childToParent[WRITE_FD], STDERR_FILENO) != -1);
            }
            else
            {
                int devNull = open("/dev/null", O_WRONLY);
                release_assert(dup2(devNull, STDERR_FILENO) != -1);
            }

            // unused
            release_assert(close(parentToChild[WRITE_FD]) == 0);
            release_assert(close(childToParent[READ_FD ]) == 0);
            release_assert(close(errPipe[READ_FD]) == 0);
            
            const char* command = allArgs[0];
            execvp(command, allArgs);

            char err = 1;
            ssize_t result = write(errPipe[WRITE_FD], &err, 1);
            release_assert(result != -1);
            
            close(errPipe[WRITE_FD]);
            close(parentToChild[READ_FD]);
            close(childToParent[WRITE_FD]);

            exit(0);
        }


        default: // parent
        {
            // unused
            release_assert(close(parentToChild[READ_FD]) == 0);
            release_assert(close(childToParent[WRITE_FD]) == 0);
            release_assert(close(errPipe[WRITE_FD]) == 0);

            while(1)
            {
                ssize_t bytesRead = 0;
                switch(bytesRead = read(childToParent[READ_FD], buffer, bufferSize))
                {
                    case 0: // End-of-File, or non-blocking read.
                    {
                        int status = 0;
                        release_assert(waitpid(pid, &status, 0) == pid);

                        // done with these now
                        release_assert(close(parentToChild[WRITE_FD]) == 0);
                        release_assert(close(childToParent[READ_FD]) == 0);

                        char errChar = 0;
                        ssize_t result = read(errPipe[READ_FD], &errChar, 1);
                        release_assert(result != -1);
                        close(errPipe[READ_FD]);

                        if(errChar)
                        {
                            free(dataReadFromChild); 
                            return COMMAND_NOT_FOUND;
                        }
                        
                        // free any un-needed memory with realloc + add a null terminator for convenience
                        dataReadFromChild = (char*)realloc(dataReadFromChild, dataReadFromChildUsed + 1);
                        dataReadFromChild[dataReadFromChildUsed] = '\0';
                        
                        if(stdOut != NULL)
                            *stdOut = dataReadFromChild;
                        else
                            free(dataReadFromChild);

                        if(stdOutByteCount != NULL)
                            *stdOutByteCount = dataReadFromChildUsed;
                        if(returnCode != NULL)
                            *returnCode = WEXITSTATUS(status);

                        return COMMAND_RAN_OK;
                    }
                    case -1:
                    {
                        release_assert(0 && "read() failed");
                        break;
                    }

                    default:
                    {
                        if(dataReadFromChildUsed + bytesRead + 1 >= dataReadFromChildSize)
                        {
                            dataReadFromChildSize += dataReadFromChildDefaultSize;
                            dataReadFromChild = (char*)realloc(dataReadFromChild, dataReadFromChildSize);
                        }

                        memcpy(dataReadFromChild + dataReadFromChildUsed, buffer, bytesRead);
                        dataReadFromChildUsed += bytesRead;
                        break;
                    }
                }
            }
        }
    }
}

int runCommand(char** stdOut, int* stdOutByteCount, int* returnCode, int includeStdErr, char* command, ...)
{
    va_list vl;
    va_start(vl, command);
      
    char* currArg = NULL;
      
    int allArgsInitialSize = 16;
    int allArgsSize = allArgsInitialSize;
    char** allArgs = (char**)malloc(sizeof(char*) * allArgsSize);
    allArgs[0] = command;
        
    int i = 1;
    do
    {
        currArg = va_arg(vl, char*);
        allArgs[i] = currArg;

        i++;

        if(i >= allArgsSize)
        {
            allArgsSize += allArgsInitialSize;
            allArgs = (char**)realloc(allArgs, sizeof(char*) * allArgsSize);
        }

    } while(currArg != NULL);

    va_end(vl);

    int retval = runCommandArray(stdOut, stdOutByteCount, returnCode, includeStdErr, allArgs);
    free(allArgs);
    return retval;
}

#endif //SIMPLE_EXEC_IMPLEMENTATION
