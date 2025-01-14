#include <stdio.h>
#include "./DebugTerminal.h"

void CMD_echo(int num_args, char** args) {
    if (num_args != 1) {
        DT_ConsolePrintln("Err expected one argument");
        return;
    }

    DT_ConsolePrintln(args[0]);
}

void CMD_math(int num_args, char** args) {
    if (num_args < 3) {
        DT_ConsolePrintln("Err expected at least three arguments");
    }

    char* operation = args[0];
    int answer;
    if (strcmp(operation, "add") == 0) {
        answer = 0;
        for (int i = 0; i < num_args - 1; i++) {
            answer += atoi(args[i + 1]);
        }
    }

    else if (strcmp(operation, "sub") == 0) {
        answer = atoi(args[1]);
        for (int i = 0; i < num_args - 2; i++) {
            answer -= atoi(args[i + 2]);
        }
    }

    else if (strcmp(operation, "mul") == 0) {
        answer = 1;
        for (int i = 0; i < num_args - 1; i++) {
            answer *= atoi(args[i + 1]);
        }
    }

    else if (strcmp(operation, "div") == 0) {
        answer = atoi(args[1]);
        for (int i = 0; i < num_args - 2; i++) {
            answer /= atoi(args[i + 2]);
        }
    }

    else {
        DT_ConsolePrintln("Err expected operation 'add', 'sub', 'mul', or 'div'");
        return;
    }

    char* output;
    asprintf(&output, "%d", answer);
    DT_ConsolePrintln(output);
    free(output);
}

int main() {
    const DT_Command commands[] = {
        {"echo", *CMD_echo},
        {"math", *CMD_math}
    };

    DT_Interpreter* terminal = DT_CreateInterpreter(commands, sizeof(commands) / sizeof(DT_Command));

    size_t command_len = 4;
    char* user_command;
    while (1) {
        printf(" >> ");

        command_len = getline(&user_command, &command_len, stdin);
        user_command[command_len - 1] = '\0';

        if (strcmp(user_command, "quit") == 0) break;

        if (!DT_InterpretCommand(terminal, user_command)) printf("Err command does not exist\n");
        else printf("%s", DT_console_text);
        DT_console_text[0] = '\0';
    }

    free(user_command);
    DT_DestroyInterpreter(terminal);
}