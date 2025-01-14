/*
DT_Command

The prefix should be the prefix of the terminal command, with no spaces. Prefixes are case sensitive.

Interpreter function format:

The interpreter function should perform the intended actions of the terminal command, using the arguments passed
to the terminal command (passed to the DT_Command interpreter function through the char** argument).

args:
    int: the number of arguments passed to the terminal command and, by proxy, the DT_Command interpreter function.

    char**: the arguments passed to the ternimal function. The arguments should be seperated by spaces in the
    terminal command. Quotation marks "" CANNOT be used to pass arguments containing spaces.

Example:
    Terminal input: [command] arg1 arg2
    Arguments are passed to the DT_Command with the prefix [command]:
    (int) 2, (char **) {"arg1", "arg2"}
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// Structures
typedef struct {
    char* prefix;
    void (*interpreter)(int, char**);
} DT_Command;

typedef struct {
    const DT_Command* commands;
    size_t num_commands;
} DT_Interpreter;

// Interpreter management
DT_Interpreter* DT_CreateInterpreter(const DT_Command* commands, size_t num_commands) {
    DT_Interpreter* interp = (DT_Interpreter *) malloc(sizeof(DT_Interpreter));
    interp->commands = commands;
    interp->num_commands = num_commands;
    return interp;
}

void DT_DestroyInterpreter(DT_Interpreter* interpreter) {
    free(interpreter);
}

// Interpreter processes
#define DT_CONSOLE_SIZE 1024

char DT_console_text[DT_CONSOLE_SIZE] = "";

void DT_ConsolePrint(char* text) {
    strlcat(DT_console_text, text, sizeof(DT_console_text));
}

void DT_ConsolePrintln(char* text) {
    DT_ConsolePrint(text); DT_ConsolePrint("\n");
}

void DT_ConsolePrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* final = NULL;

    vasprintf(&final, format, args);

    if (final) {
        DT_ConsolePrint(final);
        free(final);
    }

    va_end(args);
}

const DT_Command* DT_GetCommandByPrefix(DT_Interpreter* interpreter, char* prefix) {
    for (size_t i = 0; i < interpreter->num_commands; i++) {
        if (strcmp(interpreter->commands[i].prefix, prefix) == 0) {
            return interpreter->commands + i;
        }
    }
    return (DT_Command *) NULL;
}

int DT_InterpretCommand(DT_Interpreter* interpreter, char* text) {
    // Make a copy of the text for editing
    char command_text[strlen(text) + 1];
    strlcpy(command_text, text, sizeof(command_text));

    // Find first occurrance of a space character
    char* first_space_loc = strchr(command_text, ' ');

    // If a space was found, set the found space to a terminating character
    // so that the text before the space can be checked as a string
    if (first_space_loc) {
        *first_space_loc = '\0';
    }

    // Get the command with the prefix
    const DT_Command* command = DT_GetCommandByPrefix(interpreter, command_text);

    // If a command was matched to the prefix and there was no space,
    // run the command's interpreter function with no arguments
    if (command) {
        if (!first_space_loc) command->interpreter(0, NULL);

        // If there was a space, get the arguments to the command and run
        // its interpreter
        else {
            // Get arguments as an array of strings

            char* new_space_loc;
            int num_args = 0;
            char** args = NULL;

            while (1) {
                // If this is the first iteration, set the new space location to the absolute second space
                // (the space after the space following the prefix)
                if (num_args == 0) new_space_loc = strchr(first_space_loc + 1, ' ');

                num_args++;

                // Allocate extra space for new argument
                args = (char **) realloc(args, sizeof(char *) * num_args);

                // If this is the first argument, set its beginning character to the
                // first character of the first argument
                if (num_args == 1) args[0] = first_space_loc + 1;

                // Otherwise, set the argument's beginning character to the first character after
                // the terminating character of the last argument
                else args[num_args - 1] = *(args + (num_args - 2)) + strlen(args[num_args - 2]) + 1;

                // If this is not the last argument (a space was found),
                // set the found space character to a terminator and look for the next space
                if (new_space_loc) {
                    *new_space_loc = '\0';
                    new_space_loc = strchr(new_space_loc + 1, ' ');
                }

                // Otherwise, stop as we have found all arguments
                else break;
            }

            // Interpret the command using the arguments
            command->interpreter(num_args, args);

            free(args);
        }

        return 1;

    // If no command was found, return unsuccessful
    } else return 0;
}

void DT_CombineArgs(char** cmd_args, int start_arg, int num_args, char sep) {
    for (int i = 0; i < num_args - 1; i++) {
        cmd_args[start_arg + i][strlen(cmd_args[start_arg + i])] = sep;
    }
}