#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SST_STATE_FNAME "saves"

#define SST_STATE_TEXT_FNAME SST_STATE_FNAME ".txt"
#define SST_STATE_BIN_FNAME SST_STATE_FNAME ".bin"

#define SST_MAX_STATES 25

// State text format: x y rotation
// max x and y are the maximum character length of float values: 46
// max rotation is M_PI * 2, or about 6.3 (only one digit left of decimal)
// the length of a floating point number with the highest digit in the ones place is 8
// therefore, the length of rot is 8
// 2 characters for the minus signs on negative numbers (rotation cannot be negative)
// finally, room for the seperating spaces (2)
#define SST_STATE_MAX_STRLEN ((46 * 2) + 8 + 2 + 2)

// Modes
enum SST_MODE {
    SST_MODE_TEXT,
    SST_MODE_BINARY
} SST_mode;

void SST_SetMode(enum SST_MODE mode) { SST_mode = mode; }

// State manipulation functions
int SST_AddState(float player_x, float player_y, float player_rot) {
    FILE* f;
    int success = 1;

    if (SST_mode == SST_MODE_TEXT) {
        f = fopen(SST_STATE_TEXT_FNAME, "a");

        // Write player x, y, and rotation to new line in file
        if (f) {
            char* state_str;
            asprintf(&state_str, "%f %f %f", player_x, player_y, player_rot);
            fprintf(f, "%s\n", state_str);
            free(state_str);
        }
        else {
            printf("SST_AddState: text state file could not be appended to\n");
            success = 0;
        }
    } else if (SST_mode == SST_MODE_BINARY) {
        f = fopen(SST_STATE_BIN_FNAME, "ab");

        // Write the three floats to the file
        if (f) {
            int objs_written = (
                fwrite(&player_x,   sizeof(float), 1, f) +
                fwrite(&player_y,   sizeof(float), 1, f) +
                fwrite(&player_rot, sizeof(float), 1, f)
            );

            if (objs_written != 3) {
                printf("SST_AddState: write to binary file failed\n");
                success = 0;
            }
        }
        else {
            printf("SST_AddState: binary state file could not be appended to\n");
            success = 0;
        }
    } else return 0;

    fclose(f);

    return success;
}

int SST_LoadState(int state_num, float* player_x, float* player_y, float* player_rot) {
    FILE* f;
    int success = 1;

    if (SST_mode == SST_MODE_TEXT) {
        f = fopen(SST_STATE_TEXT_FNAME, "r");

        // Get state string specified by state_num
        if (f) {
            char state[SST_STATE_MAX_STRLEN + 2];
            for (int i = 0; i < state_num + 1; i++) fgets(state, sizeof(state), f);

            // Get each state value as a float and set its cooresponding player attribute

            // Player x
            char* first_space = strchr(state, ' ');
            *first_space = '\0';
            *player_x = atof(state);

            // Player y
            char* second_space = strchr(first_space + 1, ' ');
            *second_space = '\0';
            *player_y = atof(first_space + 1);

            // Player rotation
            *player_rot = atof(second_space + 1);
        }

        else {
            printf("SST_LoadState: text state file could not be read\n");
            success = 0;
        }
    } else if (SST_mode == SST_MODE_BINARY) {
        f = fopen(SST_STATE_BIN_FNAME, "rb");

        // Interpret the bytes at the specified state's location as three floats
        if (f) {
            // Move the cursor to the locations of the state
            fseek(f, state_num * (sizeof(float) * 3), SEEK_SET);

            // Read and assign
            int objs_read = (
                fread(player_x,   sizeof(float), 1, f) +
                fread(player_y,   sizeof(float), 1, f) +
                fread(player_rot, sizeof(float), 1, f)
            );

            if (objs_read != 3) {
                printf("SST_LoadState: read from binary file failed\n");
                success = 0;
            }
        }
        else {
            printf("SST_LoadState: binary state file could not be read\n");
            success = 0;
        }
    }

    else return 0;

    fclose(f);

    return success;
}

int SST_RemoveState(int state_num) {
    FILE* f;
    int success = 1;

    if (SST_mode == SST_MODE_TEXT) {
        f = fopen(SST_STATE_TEXT_FNAME, "r");

        // Read the file into an array of lines
        if (f) {
            char file_lines[SST_MAX_STATES][SST_STATE_MAX_STRLEN + 2];
            int num_lines = 0;
            while (fgets(file_lines[num_lines], sizeof(file_lines[0]), f) && num_lines++ < SST_MAX_STATES);

            fclose(f);
            
            f = fopen(SST_STATE_TEXT_FNAME, "w");

            // Write all lines but the one to be delted back to the file
            if (f) {
                for (int i = 0; i < num_lines; i++) {
                    if (i != state_num) fprintf(f, "%s", file_lines[i]);
                }
            }
            else {
                printf("SST_RemoveState: text state file could not be rewritten to\n");
                success = 0;
            }
        }
        else {
            printf("SST_RemoveState: text state file could not be read\n");
            success = 0;
        }
    } else if (SST_mode == SST_MODE_BINARY) {
        f = fopen(SST_STATE_BIN_FNAME, "rb");

        if (f) {
            // Get all floats from file as an array of floats
            float state_vals[SST_MAX_STATES * (sizeof(float) * 3)];
            int num_states = 0;

            while (
                fread(state_vals + (num_states * 3), sizeof(float) * 3, 1, f) == 1 &&
                num_states++ < SST_MAX_STATES
            );

            fclose(f);

            // Rewrite all but the state to be deleted back to the file
            f = fopen(SST_STATE_BIN_FNAME, "wb");

            if (f) {
                for (int i = 0; i < num_states; i++) {
                    if (i != state_num) {
                        if (fwrite(state_vals + (i * 3), sizeof(float) * 3, 1, f) != 1) {
                            printf("SST_RemoveState: rewrite to binary file failed\n");
                            success = 0;
                        }
                    }
                }
            }
            else {
                printf("SST_RemoveState: binary file could not be rewritten\n");
            }
        }
        else {
            printf("SST_RemoveState: binary file could not be read\n");
        }
    }

    else return 0;

    fclose(f);

    return success;
}

int SST_ClearStates(void) {
    FILE* f;
    int success = 1;

    if (SST_mode == SST_MODE_TEXT) {
        f = fopen(SST_STATE_TEXT_FNAME, "w");

        if (!f) {
            printf("SST_ClearStates: text state file could not be cleared\n");
            success = 0;
        }
    } else if (SST_mode == SST_MODE_BINARY) {
        f = fopen(SST_STATE_BIN_FNAME, "wb");

        if (!f) {
            printf("SST_ClearStates: binary state file could not be cleared\n");
            success = 0;
        }
    }

    else return 0;

    fclose(f);

    return success;
}

#define SST_ToggleMode() (SST_mode = SST_mode == SST_MODE_BINARY ? SST_MODE_TEXT : SST_MODE_BINARY)

int SST_SyncStates(void) {
    float t1, t2, t3;

    // Clear other file
    SST_ToggleMode();
    SST_ClearStates();

    int states_written = 0;

    while (1) {
        // Read state from mode file
        SST_ToggleMode();
        if (!SST_LoadState(states_written, &t1, &t2, &t3)) break;

        // Write state to other mode's file
        SST_ToggleMode();
        if (!SST_AddState(t1, t2, t3)) break;

        states_written++;
    }

    return states_written;
}