#include <stdio.h>
#include "./SaveSTates.h"
#include <float.h>

int main() {
    float x, y, rot;

    puts("clear states");
    SST_SetMode(SST_MODE_TEXT);
    SST_ClearStates();

    SST_SetMode(SST_MODE_BINARY);
    SST_ClearStates();

    if (0) {
        // Write state
        puts("write states");
        SST_AddState(FLT_MAX, FLT_MAX, 1);
        SST_AddState(FLT_MAX, FLT_MAX, 2);
        SST_AddState(FLT_MAX, FLT_MAX, 3);

        // Remove state
        puts("remove state");
        SST_RemoveState(1);

        // Load state
        puts("load state");
        SST_LoadState(1, &x, &y, &rot);
        printf("%f %f %f\n", x, y, rot);

        // Sync states
        printf("states synced: %d\n", SST_SyncStates());
    } else {
        SST_AddState(1, 1, 1);
        SST_AddState(2, 2, 2);
    }

    SST_SetMode(SST_MODE_BINARY);
    //SST_ClearStates();
    puts("done");

    return 1;
}