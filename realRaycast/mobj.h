#include "../linkedList.h"
#include <SDL3/SDL.h>

enum {
    MOBJ_STATIC
};
typedef Uint8 mobj_type;

#define MOBJ_FLAG_SOLID 1
#define MOBJ_FLAG_SHOOTABLE (1 << 1)

__linked_list_all_add__(
    mobj,
        mobj_type type;
        Uint8 flags;
        Uint16 sprite_index;
        float x;
        float y;
        float z;
        float angle;
        void *extra,
    (mobj_type type, float x, float y, float z, float angle, Uint16 sprite_index, Uint8 flags),
        item->type = type;
        item->x = x;
        item->y = y;
        item->z = z;
        item->angle = angle;
        item->sprite_index = sprite_index;
        item->flags = flags;
)