#include <stdlib.h>
#include "../../SDL/SDLStart.h"
#include "../DebugTerminal/DebugTerminal.h"
#include "./SaveSTates.h"
#include <string.h>

typedef unsigned char byte;

struct temp_dgp {
    int x;
    int y;
    byte color_index;
};

struct fill_dgp {
    int x;
    int y;
    byte color_index;
    struct fill_dgp *next;
};

typedef struct {
    int x1, y1;
    int x2, y2;
    byte color_index;
} dgl;

void add_temp_dgp(int x, int y, byte color_index, size_t *num_dgps, struct temp_dgp **list);
void add_fill_dgp(int x, int y, byte color_index, int *num_dgps, int max_dgps, struct fill_dgp **head, struct fill_dgp **tail);
void add_dgl(int x1, int y1, int x2, int y2, byte color_index);
#define temp_dgp(x, y, color_index) add_temp_dgp(x, y, color_index, &num_temp_dgps, &temp_dgp_list)
#define fill_dgp(x, y, color_index) add_fill_dgp(x, y, color_index, &num_fill_dgps, max_fill_dgps, &fill_dgp_head, &fill_dgp_tail)
void underscore_space(char *str);
char str_num(char *str, float *dest);
#define debug_print(var, format) printf(#var ": " format "\n", var)

// Varlabels
typedef struct {
    char *label;
    Uint8 *value;
} char_varlabel;

typedef struct {
    char *label;
    float *value;
} flt_varlabel;

Uint8 show_grid_crosshairs = FALSE;

#define CHAR_VLS_LEN 10
char_varlabel char_vls[CHAR_VLS_LEN] = {
    {"grid show crosshairs", &show_grid_crosshairs},
    {"grid cam follows player", &grid_follow_player},
    {"show player vision",  &show_player_vision},
    {"show player trail",  &show_player_trail},
    {"show grid lines",  &show_grid_lines},
    {"render walls",  &fp_show_walls},
    {"show mouse coords",  &show_mouse_coords},
    {"grid casting",  &grid_casting},
    {"debug prr",  &debug_prr},
    {"show fps", &show_fps}
};

#define FLT_VLS_LEN 4
flt_varlabel flt_vls[FLT_VLS_LEN] = {
    {"player x", &player_x},
    {"player y", &player_y},
    {"player angle", &player_angle},
    {"zoom", &fp_scale}
};

// Terminal
rgb terminal_bg = {0, 0, 0};
rgb terminal_font_color = {84, 255, 84};
int terminal_font_size = WINDOW_HEIGHT / (BF_CHAR_HEIGHT * 30);
char* TERMINAL_PROMPT = " > ";
alstring *terminal_input;
DT_Interpreter *terminal;

void CMD_echo(int num_args, char **args) {
    DT_CombineArgs(args, 0, num_args, ' ');
    DT_ConsolePrintln(args[0]);
}

void CMD_set(int num_args, char **args) {
    // If there are zero arguments, print all the variables
    if (num_args == 0) {
        for (int i = 0; i < CHAR_VLS_LEN; i++) {
            DT_ConsolePrintf("%s: %d\n", char_vls[i].label, (int) *char_vls[i].value);
        }
        for (int i = 0; i < FLT_VLS_LEN; i++) {
            DT_ConsolePrintf("%s: %f\n", flt_vls[i].label, *flt_vls[i].value);
        }
    } else {
        // Convert underscores in the first argument to spaces
        underscore_space(args[0]);

        // Find varlabel with a label corresponding to the first argument
        void *varlabel = NULL;
        char is_char = FALSE;

        // Look for int varlabel
        for (int i = 0; i < CHAR_VLS_LEN; i++) {
            if (strcmp(args[0], char_vls[i].label) == 0) {
                varlabel = char_vls + i;
                is_char = TRUE;
                break;
            }
        }

        // Look for float varlabel
        if (!is_char) {
            for (int i = 0; i < FLT_VLS_LEN; i++) {
                if (strcmp(args[0], flt_vls[i].label) == 0) {
                    varlabel = flt_vls + i;
                    break;
                }
            }
        }

        if (varlabel) {
            // If there is one argument, look for a varlabel with that label and
            // print if found
            if (num_args == 1) {
                // Print char
                if (is_char) DT_ConsolePrintf("%s: %d\n", *((Uint8 **) varlabel), (int) *((char_varlabel *) varlabel)->value);

                // Print float
                else         DT_ConsolePrintf("%s: %f\n", *((char **) varlabel), *((flt_varlabel *) varlabel)->value);
            }

            // If there are two arguments, try to set the variable using the second argument
            else if (num_args == 2) {
                // Get second argument as a number
                // If second argument is valid, set it to the varlabel value
                float new_num;
                char result = str_num(args[1], &new_num);
                
                if (result) {
                    if (is_char) *((char_varlabel *) varlabel)->value = (Uint8) new_num;
                    else         *((flt_varlabel *)varlabel)->value = new_num;
                }
                
                else
                    DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[1]);
            }
        } else {
            if (num_args > 2)
                DT_ConsolePrintln("Err expected from 0 to 2 arguments");
            else
                DT_ConsolePrintf("Err no such variable '%s'\n", args[0]);
        }
    }
}

void CMD_state(int num_args, char **args) {
    float x, y, rot;

    // If there are zero arguments, print all states
    if (num_args == 0) {
        int i = 0;
        while (SST_LoadState(i, &x, &y, &rot))
            DT_ConsolePrintf("%*d %f %f %f\n", 2, i++, x, y, rad_deg(rot));
    }

    else {
        // Saving a state
        if (strcmp(args[0], "add") == 0) {
            if (num_args == 1)
                SST_AddState(player_x, player_y, player_angle);
            
            else if (num_args == 4) {
                float nums[3];
                int i = -1;
                while (++i < 3) {
                    if (!str_num(args[i + 1], nums + i)) {
                        DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[i + 1]);
                        return;
                    }
                }
                SST_AddState(nums[0], nums[1], deg_rad(nums[2]));
            } else
                DT_ConsolePrintln("Err expected 'add' [x y rot]");
        }

        // Loading a state
        else if (strcmp(args[0], "load") == 0) {
            if (num_args == 2) {
                float num;
                if (str_num(args[1], &num))
                    SST_LoadState(num, &player_x, &player_y, &player_angle);
                else
                    DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[1]);
            } else {
                DT_ConsolePrintln("Err expected 'load' and the number of the state to load");
            }
        }

        // Removing a state
        else if (strcmp(args[0], "remove") == 0) {
            if (num_args == 2) {
                float num;
                if (str_num(args[1], &num))
                    SST_RemoveState(num);
                else
                    DT_ConsolePrintf("Err cannot interpret '%s' as a number\n", args[1]);
            }
            else
                DT_ConsolePrintln("Err expected 'remove' and the number of the state to remove");
        }

        // Clearing all states
        else if (strcmp(args[0], "clear") == 0) {
            if (num_args == 1)
                SST_ClearStates();
            else
                DT_ConsolePrintln("Err expected 'clear' and no other arguments");
        }
        
        else
            DT_ConsolePrintln("Err expected 'add', 'load', 'remove', or 'clear'");
    }
}

#define CMD_DIR_NAME "./cmd/"
DIR *cmd_dir;
long cmd_dir_start;

void CMD_run(int num_args, char **args) {
    if (num_args != 1) {
        DT_ConsolePrintln("Err expected only name of program to run");
        return;
    }

    // Search the directory for the file
    rewinddir(cmd_dir);

    struct dirent *entry;
    while ((entry = readdir(cmd_dir))) {
        // Skip . and ..
        if (entry->d_name[0] == '.')
            continue;

        // If name matches, execute each line of the file
        if (strcmp(entry->d_name, args[0]) == 0) {
            char *path;
            asprintf(&path, CMD_DIR_NAME "%s", args[0]);
            FILE *f = fopen(path, "r");
            free(path);

            char line[51];
            char *newline;
            while (fgets(line, 51, f)) {
                // Remove newline
                if ((newline = strchr(line, '\n')))
                    *newline = '\0';

                // Execute line
                DT_InterpretCommand(terminal, line);
            }

            return;
        }
    }

    DT_ConsolePrintf("Err program '%s' does not exist\n", args[0]);
}

const DT_Command terminal_commands[] = {
    {"echo", *CMD_echo},
    {"set", *CMD_set},
    {"state", *CMD_state},
    {"run", *CMD_run}
};

// DGs
const rgb DG_COLORS[] = {
    C_WHITE,
    C_YELLOW,
    C_RED,
    C_BLUE,
    C_PURPLE,
    C_GREEN,
    C_BLACK
};

enum dg_color_index {
    DG_WHITE = 0,
    DG_YELLOW = 1,
    DG_RED = 2,
    DG_BLUE = 3,
    DG_PURPLE = 4,
    DG_GREEN = 5,
    DG_BLACK = 6
};

int dgp_radius = 5;
int num_fill_dgps = 0;
int max_fill_dgps = FPS / 2;
size_t num_temp_dgps = 0;
struct fill_dgp *fill_dgp_head;
struct fill_dgp *fill_dgp_tail;
struct temp_dgp *temp_dgp_list = NULL;

dgl *dgl_list = NULL;
size_t num_dgls = 0;

void add_fill_dgp(int x, int y, byte color_index, int *num_dgps, int max_dgps, struct fill_dgp **head, struct fill_dgp **tail) {
    struct fill_dgp *p = malloc(sizeof(*p));
    p->x = x;
    p->y = y;
    p->color_index = color_index;
    p->next = NULL;
    if (*head) {
        (*tail)->next = p;
        *tail = p;
    } else {
        *head = p;
        *tail = p;
    }

    (*num_dgps)++;
    if (*num_dgps > max_dgps) {
        struct fill_dgp *old_head = *head;
        *head = (*head)->next;
        free(old_head);
        (*num_dgps)--;
    }
}

void add_temp_dgp(int x, int y, byte color_index, size_t *num_dgps, struct temp_dgp **list) {
    (*num_dgps)++;
    *list = realloc(*list, sizeof(struct temp_dgp) * *num_dgps);
    struct temp_dgp *p = *list + (*num_dgps - 1);
    p->x = x;
    p->y = y;
    p->color_index = color_index;
    // printf("num_temp_dgps: %zu\n", num_temp_dgps);
}

void add_dgl(int x1, int y1, int x2, int y2, byte color_index) {
    num_dgls++;
    dgl_list = realloc(dgl_list, sizeof(dgl) * num_dgls);
    dgl *p = dgl_list + (num_dgls - 1);
    p->x1 = x1;
    p->x2 = x2;
    p->y1 = y1;
    p->y2 = y2;
    p->color_index = color_index;
}

void underscore_space(char *str) {
    char *index;
    while ((index = strchr(str, '_')))
        *index = ' ';
}

// Convert from string to number
char str_num(char *str, float *dest) {
    // Check for true or false
    if (strcmp(str, "true") == 0) {
        *dest = 1;
        return TRUE;
    }
    if (strcmp(str, "false") == 0) {
        *dest = 0;
        return TRUE;
    }

    // Convert to float
    float temp = atof(str);

    // If zero was returned but the text is not zero,
    // return failure
    if (temp == 0 && str[0] != '0') {
        return FALSE;
    }

    *dest = temp;
    return TRUE;
}

int cond(char* msg) {
    puts(msg);
    return TRUE;
}

void toggle(Uint8 *var) {
    if (*var) *var = 0;
    else *var = 1;
}

void debugging_start(void) {
    terminal = DT_CreateInterpreter(terminal_commands, sizeof(terminal_commands) / sizeof(DT_Command));

    // Terminal input init
    terminal_input = alstring_init(5);

    // Set save state mode
    SST_SetMode(SST_MODE_BINARY);

    // Load command files directory
    cmd_dir = opendir(CMD_DIR_NAME);

    char *start_file = "start.dt";
    if (cmd_dir)
        CMD_run(1, &start_file);
    else
        printf("Could not open command directory\n");
}

void debugging_end(void) {
    closedir(cmd_dir);
}