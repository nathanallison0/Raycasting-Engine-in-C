#include <math.h>
#include <string.h>
#include "./constants.h"
#include <dirent.h>

// Custom SDL graphics functions
// draw_rect(), etc.
#include "../../SDL/SDL3Start.h"

// Font
#include "../BasicFont/BasicFont.h"

// Linked list generation
#include "../../linkedList.h"

// Textures
#include "textures.h"

#include "arraySprites.h"

#define WALL_TEXTURES TRUE
#define FLOOR_TEXTURES TRUE
#define SPRITES TRUE

void calc_grid_cam_center(void);
void reset_grid_cam(void);
void reset_player(void);
void zoom_grid_cam(float);
void zoom_grid_cam_center(float);
void rotate_player(float);
void push_player_forward(float);
void push_player_right(float);

#define rad_deg(radians) radians * (180 / M_PI)
#define deg_rad(degrees) degrees * (M_PI / 180)

#define fix_angle(angle) { if (angle < 0) angle += M_PI * 2; else if (angle >= M_PI * 2) angle -= M_PI * 2; }

// View modes
typedef enum {
    VIEW_GRID,
    VIEW_FPS,
    VIEW_TERMINAL
} view_mode;

view_mode view = VIEW_FPS;
view_mode prev_view;

void set_view(view_mode new) {
    prev_view = view;
    view = new;
}


typedef struct {
    float x;
    float y;
} xy;

#include "map.h"

Uint8 show_fps = TRUE;
// Grid visual
rgb grid_bg = {255, 0, 255};
rgb grid_fill_nonsolid = {235, 235, 235};
rgb grid_fill_solid = {100, 110, 100};
int grid_line_width = 1;
rgb grid_line_fill = C_BLACK;
#define GRID_DOOR_THICKNESS 4
Uint8 show_grid_lines = FALSE;
// Grid Visual Camera
float grid_cam_x;
float grid_cam_y;
float grid_cam_zoom;
float grid_cam_zoom_incr = 0.025f;
float grid_cam_zoom_min = 0.1f;
float grid_cam_zoom_max = 1.25f;
float grid_cam_center_x;
float grid_cam_center_y;
Uint8 show_mouse_coords = FALSE;
float grid_mouse_x, grid_mouse_y;
Uint8 grid_casting = FALSE;
Uint8 debug_prr = FALSE;

// Player
float player_x;
float player_y;
int player_radius = 10;
float player_x_velocity;
float player_y_velocity;
float player_velocity_angle;
int player_max_velocity = 200;
float player_angle;
float fov = M_PI / 3;
int player_movement_accel = 600;
int player_movement_decel = 600;
float player_angle_increment = M_PI / 18;
int player_rotation_speed = 8;

// Player interactions
#define KEY_OPEN_DOOR SDL_SCANCODE_SPACE
#define OPEN_DOOR_DIST (GRID_SPACING * 2.5f)
#define DOOR_SPEED 100

// Player grid visual
int grid_player_pointer_dist = 15;
rgb grid_player_fill = {255, 50, 50};
Uint8 grid_follow_player = TRUE;
Uint8 show_player_vision = FALSE;
Uint8 show_player_trail = FALSE;

// Grid mobjs
int grid_mobj_radius = 7;

// First person rendering
float fp_scale = 1 / 0.009417f;
#define FP_RENDER_DISTANCE 2000
#define FP_BRIGHTNESS 0.9f
float player_height = GRID_SPACING / 2;

float fp_brightness_appl;
Uint8 fp_show_walls = TRUE;
int pixel_fov_circumfrence;
float radians_per_pixel;
//#define FLOOR_RES 1

// User Input Variables
int shift = FALSE;
int left_mouse_down = FALSE;
int mouse_x;
int mouse_y;
int prev_mouse_x = -1;
int prev_mouse_y = -1;
char horizontal_input;
char vertical_input;
char rotation_input = 0;

rgb grid_mobj_color = C_BLUE;
int num_mobjs = 0;
__linked_list_all_add__(
    mobj,
    float x; float y; int sprite_num,
    (float x, float y, int sprite_num),
        item->x = x;
        item->y = y;
        item->sprite_num = sprite_num;
        num_mobjs++
)

#define NUM_ROT_SPRITE_FRAMES 8
int num_rot_mobjs = 0;
float rot_sprite_incr = (M_PI * 2) / NUM_ROT_SPRITE_FRAMES;
__linked_list_all_add__(
    rot_mobj, float x; float y; float angle; int sprite_num,
    (float x, float y, float angle, int sprite_num),
        item->x = x;
        item->y = y;
        item->angle = angle;
        item->sprite_num = sprite_num;
        num_rot_mobjs++
)

__linked_list_all__(
    sprite_proj, float dist; float angle; int sprite_num,
    (float dist, float angle, int sprite_num),
        item->dist = dist;
        item->angle = angle;
        item->sprite_num = sprite_num
)

// Debugging
#include "./debugging.h"

// Grid Graphics
void g_draw_rect(float x, float y, float length, float width, Uint8 r, Uint8 g, Uint8 b) {
    draw_rect_f(
        (x - grid_cam_x) * grid_cam_zoom,
        (y - grid_cam_y) * grid_cam_zoom,
        length * grid_cam_zoom,
        width * grid_cam_zoom,
        r, g, b
    );
}

void g_draw_rect_rgb(float x, float y, float length, float width, rgb color) {
    g_draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void g_draw_point(float x, float y, float radius, Uint8 r, Uint8 g, Uint8 b) {
    draw_point_f(
        (x - grid_cam_x) * grid_cam_zoom,
        (y - grid_cam_y) * grid_cam_zoom,
        radius,
        r, g, b
    );
}

void g_draw_point_rgb(float x, float y, float radius, rgb color) {
    g_draw_point(x, y, radius, color.r, color.g, color.b);
}

void g_draw_scale_point(float x, float y, float radius, Uint8 r, Uint8 g, Uint8 b) {
    g_draw_point(x, y, radius * grid_cam_zoom, r, g, b);
}

void g_draw_scale_point_rgb(float x, float y, float radius, rgb color) {
    g_draw_scale_point(x, y, radius, color.r, color.g, color.b);
}

// First person rendering
float point_dist(float x1, float y1, float x2, float y2) {
    return sqrtf(powf(x1 - x2, 2) + powf(y1 - y2, 2));
}

float fp_dist(float x, float y, float angle_to) {
    float offset = fmodf((player_angle - angle_to) + (M_PI * 2), M_PI * 2);
    if (offset > M_PI) {
        offset -= M_PI * 2;
    }
    offset = fabsf(offset);

    if (M_PI_4 < offset && offset < M_PI - M_PI_4) {
        offset -= M_PI_2;
    }

    return point_dist(x, y, player_x, player_y) * cosf(offset);
}

float angle_to_screen_x(float angle_to) {
    float relative_angle = angle_to - (player_angle - (fov / 2));
    fix_angle(relative_angle);
    return (WINDOW_WIDTH / fov) * relative_angle;
}

float player_height_y_offset(float height) {
    return -((height / 2) - ((player_height / GRID_SPACING) * height));
}

//             0     1     2     3     4     5     6     7     8            9            10           11
// line_vars: {c_hx, c_hy, d_hx, d_hy, c_vx, c_vy, d_vx, d_vy, c_epsilon_h, d_epsilon_h, c_epsilon_v, d_epsilon_v}
#define c_hx        vars[0]
#define c_hy        vars[1]
#define d_hx        vars[2]
#define d_hy        vars[3]
#define c_vx        vars[4]
#define c_vy        vars[5]
#define d_vx        vars[6]
#define d_vy        vars[7]
#define c_epsilon_h vars[8]
#define d_epsilon_h vars[9]
#define c_epsilon_v vars[10]
#define d_epsilon_v vars[11]
char raycast_vars(float x, float y, float angle, float *vars) {
    char quadrant = (int) (angle / M_PI_2) + 1;
    float alpha;
    if (quadrant == 1) {
        alpha = angle;
        c_hy = ((ceilf(y / GRID_SPACING)) * GRID_SPACING);// - 1;
        c_hx = ((c_hy - y) / tanf(alpha)) + x;
        d_hx = GRID_SPACING / tanf(alpha);
        d_hy = GRID_SPACING;
        c_vx = (floorf(x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = (tanf(alpha) * (c_vx - x)) + y;
        d_vx = GRID_SPACING;
        d_vy = tanf(alpha) * (d_vx);
    } else if (quadrant == 2) {
        alpha = angle - (M_PI_2);
        c_hy = (ceilf(y / GRID_SPACING) * GRID_SPACING);// - 1;
        c_hx = tanf(alpha) * (y - c_hy) + x;
        d_hy = GRID_SPACING;
        d_hx = -GRID_SPACING * tan(alpha);
        c_vx = (floorf(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y + ((x - c_vx) / tanf(alpha));
        d_vx = -GRID_SPACING;
        d_vy = GRID_SPACING / tanf(alpha);
    } else if (quadrant == 3) {
        alpha = angle - M_PI;
        c_hy = (floorf(y / GRID_SPACING) * GRID_SPACING);
        c_hx = x + (c_hy - y) / tanf(alpha);
        d_hy = -GRID_SPACING;
        d_hx = -GRID_SPACING * tanf((M_PI_2) - alpha);
        c_vx = (floorf(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y - (tanf(alpha) * (x - c_vx));
        d_vx = -GRID_SPACING;
        d_vy = -GRID_SPACING * tanf(alpha);
    } else {
        alpha = angle - (M_PI + (M_PI_2));
        c_hy = ((floor(y / GRID_SPACING) - 1) * GRID_SPACING) + GRID_SPACING;
        c_hx = x - tanf(alpha) * (c_hy - y);
        d_hy = -GRID_SPACING;
        d_hx = GRID_SPACING * tan(alpha);
        c_vx = (floorf(x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = y - (c_vx - x) / tanf(alpha);
        d_vx = GRID_SPACING;
        d_vy = -GRID_SPACING / tanf(alpha);
    }

    float diagonal_dist = fabs((M_PI_4) - alpha) / (M_PI * 200);
    c_epsilon_h = (sqrtf(powf(c_hx - x, 2) + powf(c_hy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_h = (sqrtf(powf(d_hx, 2) +     powf(d_hy, 2))     / GRID_SPACING) * diagonal_dist;
    c_epsilon_v = (sqrtf(powf(c_vx - x, 2) + powf(c_vy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_v = (sqrtf(powf(d_vx, 2) +     powf(d_vy, 2))     / GRID_SPACING) * diagonal_dist;

    return quadrant;
}

#define get_horiz_texture(x, y) horiz_textures   [(int) (y) / GRID_SPACING][(int) (x) / GRID_SPACING]
#define get_vert_texture(x, y)  vertical_textures[(int) (y) / GRID_SPACING][(int) (x) / GRID_SPACING]

typedef struct {
    door *door;
    float x;
    float y;
} door_hit_info;

xy raycast(float x, float y, float angle, int *texture_index, int *texture_col, door_hit_info* door_hit) {
    float vars[12];
    char quadrant = raycast_vars(x, y, angle, vars);

    if (door_hit) {
        door_hit->door = NULL;
    }

    float texture_offset_h = 0;
    float rel_wall_hit_h;
    door *horiz_door_hit = NULL;
    while (TRUE) {
        if (!(
            ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < GRID_WIDTH ) &&
            ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < GRID_HEIGHT )
        )) {
            break;
        }

        rel_wall_hit_h = fmodf(c_hx, GRID_SPACING);
        
        // Select the grid space to take as the space we hit using an epsilon
        Uint8 left_down =  get_map((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1);
        Uint8 right_down = get_map((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1);
        Uint8 left_up =    get_map((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING);
        Uint8 right_up =   get_map((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING);

        Uint8 hit;
        if ((left_down || left_up) && (right_down || right_up)) {
            if (rel_wall_hit_h < GRID_SPACING / 2) {
                goto choose_from_left;
            } else {
                goto choose_from_right;
            }
        } else if (left_down || left_up) {
            goto choose_from_left;
        } else {
            goto choose_from_right;
        }

        choose_from_left:
        if (left_down && left_up) {
            if (quadrant == 1 || quadrant == 2) {
                hit = left_down;
            } else {
                hit = left_up;
            }
        } else if (left_down) {
            hit = left_down;
        } else {
            hit = left_up;
        }
        goto process_hit;

        choose_from_right:
        if (right_down && right_up) {
            if (quadrant == 1 || quadrant == 2) {
                hit = right_down;
            } else {
                hit = right_up;
            }
        } else if (right_down) {
            hit = right_down;
        } else {
            hit = right_up;
        }

        process_hit:
        if (hit == MAP_SOLID) {
            break;
        } else if (hit == MAP_HORIZ_DOOR) {
            door *door;

            if (quadrant == 1 || quadrant == 2) {
                door = get_door_coords(c_hx, c_hy + (GRID_SPACING / 2));
            } else {
                door = get_door_coords(c_hx, c_hy - (GRID_SPACING / 2));
            }

            // Record this door as the one passed if we haven't passed one already
            // and if 
            if (door_hit && door && !door_hit->door) {
                door_hit->door = door;
                door_hit->x = c_hx + (d_hx / 2);
                door_hit->y = c_hy + (d_hy / 2);
            }

            // If we are at a door and either the door doesn't open or it does and we have hit it,
            // register a hit halfway through the space
            rel_wall_hit_h += d_hx / 2;
            if (0 <= rel_wall_hit_h && rel_wall_hit_h < GRID_SPACING && (!door || rel_wall_hit_h <= door->progress)) {
                if (door) {
                    texture_offset_h = GRID_SPACING - door->progress;
                }
                horiz_door_hit = door;
                break;

            // If we didn't hit it but will again on the next iteration, increment extra to not hit the back side of the door space
            } else {
                float next_rel_hit_x = rel_wall_hit_h + (d_hx / 2);
                if (0 <= next_rel_hit_x && next_rel_hit_x < GRID_SPACING)
                    c_hx += d_hx; c_hy += d_hy;
            }
        }

        c_hx += d_hx; c_hy += d_hy;
        c_epsilon_h += d_epsilon_h;
        if (show_player_vision)
            temp_dgp(c_hx, c_hy, DG_BLUE);
    }

    while (!(
        !(
            ( 0 < (c_vx / GRID_SPACING) && (c_vx / GRID_SPACING) < GRID_WIDTH ) &&
            ( 0 < (c_vy / GRID_SPACING) && (c_vy / GRID_SPACING) < GRID_HEIGHT )
        ) || (
            ( get_map((c_vx / GRID_SPACING) - 1, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_map((c_vx / GRID_SPACING) - 1, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING)) ||
            ( get_map(c_vx / GRID_SPACING, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_map(c_vx / GRID_SPACING, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) )
        ))
    ) {
        c_vx += d_vx; c_vy += d_vy;
        c_epsilon_v += d_epsilon_v;
        if (show_player_vision)
            temp_dgp(c_vx, c_vy, DG_BLUE);
    }

    if (point_dist(c_hx, c_hy, x, y) < point_dist(c_vx, c_vy, x, y)) {
        if (texture_index) {
            *texture_index = get_horiz_texture(c_hx, c_hy);
        }

        if (texture_col) {
            *texture_col = ((rel_wall_hit_h + texture_offset_h) / GRID_SPACING) * TEXTURE_WIDTH;

            // Account for drawing reverse for backward-facing walls
            if (quadrant == 1 || quadrant == 2) {
                *texture_col = TEXTURE_WIDTH - *texture_col - 1;
            }
        }

        // If hit door, return coordinates in the middle of the space
        if (horiz_door_hit) {
            c_hx += d_hx / 2;
            c_hy += d_hy / 2;
        }
        return (xy) {c_hx, c_hy};
    } else {
        if (texture_index) {
            *texture_index = get_vert_texture(c_vx, c_vy);
        }
        if (texture_col) {
            *texture_col = (fmodf(c_vy, GRID_SPACING) / GRID_SPACING) * TEXTURE_WIDTH;

            // Account for drawing reverse for backward-facing walls
            if (quadrant == 2 || quadrant == 3) {
                *texture_col = TEXTURE_WIDTH - *texture_col - 1;
            }
        }

        return (xy) {c_vx, c_vy};
    }
}

float project(float distance) {
    return WINDOW_HEIGHT / (distance / fp_scale);
}

#define SHADE_DIST 1024
Uint8 shading_table[SHADE_DIST][256];
void precompute_shading_table(void) {
    for (int i = 0; i < SHADE_DIST; i++) {
        float dist = ((float) i / SHADE_DIST) * FP_RENDER_DISTANCE;
        for (int c = 0; c < 256; c++) {
            float shaded = (c - ((float) c / FP_RENDER_DISTANCE) * dist) * FP_BRIGHTNESS;
            if (shaded < 0) shaded = 0;
            else if (shaded > 255) shaded = 255;
            shading_table[i][c] = (Uint8) shaded;
        }
    }
}

Uint8 shade(Uint8 color, float distance) {
    int index = (distance / FP_RENDER_DISTANCE) * SHADE_DIST;
    return shading_table[index][color];
}

rgb shade_rgb(rgb color, float distance) {
    return (rgb) {
        shade(color.r, distance),
        shade(color.g, distance),
        shade(color.b, distance)
    };
}

rgba shade_rgba(rgba color, float distance) {
    return (rgba) {
        shade(color.r, distance),
        shade(color.g, distance),
        shade(color.b, distance),
        color.a
    };
}

float get_angle_to(float x, float y) {
    float angle = atan2f(y - player_y, x - player_x);
    if (angle < 0) angle += (M_PI * 2);
    return angle;
}

void add_sprite_proj(float x, float y, float angle, int sprite_num) {
    float distance = fp_dist(x, y, angle);
    if (distance > FP_RENDER_DISTANCE) {
        return;
    }

    // Store, sorted farthest to closest, for rendering
    sprite_proj *new_proj = sprite_proj_create(distance, angle, sprite_num);
    
    if (sprite_proj_head) {
        if (sprite_proj_head->dist < distance) {
            new_proj->next = sprite_proj_head;
            sprite_proj_head = new_proj;
        } else {
            // Find closest greater sprite
            sprite_proj *greater = sprite_proj_head;
            for (;greater->next && greater->next->dist > distance; greater = greater->next);

            // Insert this sprite after the lesser sprite
            sprite_proj *lesser = greater->next;
            greater->next = new_proj;
            new_proj->next = lesser;
        }
    } else {
        sprite_proj_head = new_proj;
    }
}

void calc_grid_cam_center(void) {
    grid_cam_center_x = grid_cam_x + ((WINDOW_WIDTH / 2) / grid_cam_zoom);
    grid_cam_center_y = grid_cam_y + ((WINDOW_HEIGHT / 2) / grid_cam_zoom);
}

// Resets
void reset_grid_cam(void) {
    grid_cam_x = 0;
    grid_cam_y = 0;
    grid_cam_zoom = 1;
}

void reset_player(void) {
    player_x = GRID_SPACING * 4;
    player_y = GRID_SPACING * 4;
    player_height = GRID_SPACING / 2;
    player_x_velocity = 0;
    player_y_velocity = 0;
    player_angle = 0;
}

// Zoom grid
void zoom_grid_cam(float zoom) {
    grid_cam_zoom += zoom;
    if (grid_cam_zoom < grid_cam_zoom_min) {
        grid_cam_zoom = grid_cam_zoom_min;
    } else if (grid_cam_zoom > grid_cam_zoom_max) {
        grid_cam_zoom = grid_cam_zoom_max;
    }
}

void zoom_grid_cam_center(float zoom) {
    calc_grid_cam_center();
    float old_grid_cam_center_x = grid_cam_center_x;
    float old_grid_cam_center_y = grid_cam_center_y;
    zoom_grid_cam(zoom);
    grid_cam_x = old_grid_cam_center_x - ((WINDOW_WIDTH / 2) / grid_cam_zoom);
    grid_cam_y = old_grid_cam_center_y - ((WINDOW_HEIGHT / 2) / grid_cam_zoom);
}

// Player control
void rotate_player(float angle) {
    player_angle += angle;
    while (player_angle < 0) {
        player_angle += M_PI * 2;
    }
    while (player_angle >= M_PI * 2) {
        player_angle -= (M_PI * 2);
    }
}

void push_player_forward(float force) {
    if (force != 0) {
        player_x_velocity += cos(player_angle) * force;
        player_y_velocity += sin(player_angle) * force;
    }
}

void push_player_right(float force) {
    if (force != 0) {
        player_angle += M_PI_2;
        push_player_forward(force);
        player_angle -= M_PI_2;
    }
}

const bool *state;
void setup(void) {
    // Initialize keyboard state
    state = SDL_GetKeyboardState(NULL);

    pixel_fov_circumfrence = (WINDOW_WIDTH / fov) * (M_PI * 2);
    radians_per_pixel = fov / WINDOW_WIDTH;
    sky_scale_x = (float) array_sprites[0].width / pixel_fov_circumfrence;
    sky_scale_y = (float) array_sprites[0].height / (WINDOW_HEIGHT / 2);

    precompute_shading_table();

    fp_brightness_appl = FP_BRIGHTNESS;

    // Set player start pos
    reset_player();
    reset_grid_cam();
    calc_grid_cam_center();

    #define mobj_flat(x, y, name) mobj_create(x * GRID_SPACING, y * GRID_SPACING, 0)
    #define mobj_rot(x, y, angle, name) rot_mobj_create(x * GRID_SPACING, y * GRID_SPACING, angle, 0)
    
    // Army of MJs
    for (float x = 14.9; x < 24; x += 2) {
        for (int y = 11; y < 21; y += 2) {
            mobj_flat(x, y, "mj1");
        }
    }

    mobj_flat(3.5, 8.5, "plant");
    mobj_flat(3.5, 6.5, "plant");
    mobj_flat(10.5, 8.5, "keepOut");
    mobj_rot(6.5, 8.5, M_PI + M_PI_2, "couch");
    rot_mobj_create(714, 423, 0.1066f, 0);

    mobj_create(732 + 8, 1104 + 32, 0);
}

bool prev_state[SDL_SCANCODE_COUNT];
int key_just_pressed(int scancode) {
    return state[scancode] && !prev_state[scancode];
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_EVENT_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = FALSE;
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = TRUE;
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            zoom_grid_cam_center(-event.wheel.y * grid_cam_zoom_incr);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            if (show_mouse_coords) {
                grid_mouse_x = grid_cam_x + (event.motion.x / grid_cam_zoom);
                grid_mouse_y = grid_cam_y + (event.motion.y / grid_cam_zoom);
            }

            if (prev_mouse_x != -1 && prev_mouse_y != -1 && left_mouse_down) {
                grid_cam_x -= (mouse_x - prev_mouse_x) / grid_cam_zoom;
                grid_cam_y -= (mouse_y - prev_mouse_y) / grid_cam_zoom;
            }
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
    }

    shift = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];

    if (state[SDL_SCANCODE_ESCAPE]) game_is_running = FALSE;

    if (key_just_pressed(SDL_SCANCODE_SLASH)) {
        if (view == VIEW_TERMINAL) set_view(prev_view);
        else set_view(VIEW_TERMINAL);
    }

    if (view == VIEW_FPS || view == VIEW_GRID) {
    if (key_just_pressed(SDL_SCANCODE_2)) set_view(VIEW_GRID);
    if (key_just_pressed(SDL_SCANCODE_1)) set_view(VIEW_FPS);
    if (key_just_pressed(SDL_SCANCODE_C)) toggle(&grid_follow_player);

        if (state[SDL_SCANCODE_RIGHT]) rotation_input++;
        if (state[SDL_SCANCODE_LEFT]) rotation_input--;

        if (state[SDL_SCANCODE_R]) {
            reset_grid_cam();
            reset_player();
        }

        if (state[SDL_SCANCODE_W]) vertical_input++;
        if (state[SDL_SCANCODE_S]) vertical_input--;
        if (state[SDL_SCANCODE_A]) horizontal_input--;
        if (state[SDL_SCANCODE_D]) horizontal_input++;

        if (state[SDL_SCANCODE_UP]) player_height++;
        if (state[SDL_SCANCODE_DOWN]) player_height--;

        if (player_height < 0) player_height = 0;
        else if (player_height > GRID_SPACING) player_height = GRID_SPACING;
        
        // Door opening and closing
        if (key_just_pressed(KEY_OPEN_DOOR)) {
            door_hit_info door_hit;
            raycast(player_x, player_y, player_angle, NULL, NULL, &door_hit);
            if (door_hit.door && point_dist(player_x, player_y, door_hit.x, door_hit.y) <= OPEN_DOOR_DIST) {
                door *door = door_hit.door;
                // If moving, reverse direction
                if (door->flags & DFLAG_MOVING) {
                    door->flags ^= DFLAG_OPENING;
                } else {
                    // If not moving, switch to other open/closed
                    door->flags |= DFLAG_MOVING;
                    if (door->progress == 0) {
                        door->flags |= DFLAG_OPENING;
                    } else {
                        door->flags &= ~DFLAG_OPENING;
                    }
                }
            }
        }

    } else if (view == VIEW_TERMINAL && event.type == SDL_EVENT_KEY_DOWN) {
        char key = event.key.key;

        // If the shift key is pressed, look for a shifted character corresponding
        // with the key pressed and use if found
        // (only characters used in the font are included in BF_CHAR_KEYS)
        if (shift) {
            char new_key = BF_GetUpperCharKey(key);
            if (new_key != -1) {
                alstring_append(terminal_input, new_key);
            }
        }

        // If the key is space or in the character set, add it to the input string
        else if (key == ' ' || BF_GetCharIndex(key) != -1) {
            alstring_append(terminal_input, key);
        }

        // If backspace is pressed, delete
        else if (state[SDL_SCANCODE_BACKSPACE]) {
            alstring_pop(terminal_input);
        }

        // If return key pressed and there is text, interpret input text as command
        if (state[SDL_SCANCODE_RETURN]) {
            DT_console_text[0] = '\0';
            // If command was not found, print error message
            if (!DT_InterpretCommand(terminal, terminal_input->text)) {
                DT_ConsolePrintln("Err no such command");
            }
            alstring_clear(terminal_input);
        }
    }

    memcpy(prev_state, state, sizeof(prev_state));
}

float delta_time;
void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    // Only delay if we are too fast to update this frame
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    // Get a delta time factor converted to seconds to be used to update my objects
    delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks();


    // Update doors
    for (int i = 0; i < NUM_DOORS; i++) {
        door *d = doors + i;
        if (d->flags & DFLAG_MOVING) {
            d->progress += (d->flags & DFLAG_OPENING ? DOOR_SPEED : -DOOR_SPEED) * delta_time;
            
            // Bounds check
            if (d->flags & DFLAG_OPENING && d->progress >= GRID_SPACING) {
                d->progress = GRID_SPACING;
                d->flags &= ~DFLAG_MOVING;
            } else if (d->flags | ~DFLAG_MOVING && d->progress <= 0) {
                d->progress = 0;
                d->flags &= ~DFLAG_MOVING;
            }
        }
    }

    push_player_forward(player_movement_accel * vertical_input * delta_time);
    push_player_right(player_movement_accel * horizontal_input * delta_time);
    rotate_player(rotation_input * player_angle_increment * player_rotation_speed * delta_time);

    // Calculate velocity angle
    player_velocity_angle = atan(player_y_velocity / player_x_velocity);
    if (player_x_velocity < 0) player_velocity_angle += M_PI;

    //printf("player_velocity_angle: %f\n", rad_deg(player_velocity_angle));

    // Cap player velocity within range of maximum
    if (player_x_velocity != 0 || player_y_velocity != 0) {
        if (player_x_velocity == 0) {
            if (player_y_velocity > player_max_velocity)
                player_y_velocity = player_max_velocity;
            else if (player_y_velocity < -player_max_velocity)
                player_y_velocity = -player_max_velocity;

        } else if (player_y_velocity == 0) {
            if (player_x_velocity > player_max_velocity)
                player_x_velocity = player_max_velocity;
            else if (player_x_velocity < -player_max_velocity)
                player_x_velocity = -player_max_velocity;

        } else {
            if (sqrt( pow(player_x_velocity, 2) + pow(player_y_velocity, 2) ) > player_max_velocity) {
                player_x_velocity = cos(player_velocity_angle) * player_max_velocity;
                player_y_velocity = sin(player_velocity_angle) * player_max_velocity;
            }
        }
    }

    // Decrease player speed if not moving
    if (!horizontal_input && !vertical_input) {
        if (player_x_velocity != 0) {
            int vel_is_pos = player_x_velocity > 0;
            player_x_velocity -= cos(player_velocity_angle) * player_movement_decel * delta_time;
            if (vel_is_pos && player_x_velocity < 0)
                player_x_velocity = 0;
            else if (!vel_is_pos && player_x_velocity > 0)
                player_x_velocity = 0;
        }
        if (player_y_velocity != 0) {
            int vel_is_pos = player_y_velocity > 0;
            player_y_velocity -= sin(player_velocity_angle) * player_movement_decel * delta_time;
            if (vel_is_pos && player_y_velocity < 0)
                player_y_velocity = 0;
            else if (!vel_is_pos && player_y_velocity > 0)
                player_y_velocity = 0;
        }
    }

    // Wall collisions
    int northeast_collision = get_map_coords(player_x + player_radius, player_y - player_radius);
    int southeast_collision = get_map_coords(player_x + player_radius, player_y + player_radius);
    int southwest_collision = get_map_coords(player_x - player_radius, player_y + player_radius);
    int northwest_collision = get_map_coords(player_x - player_radius, player_y - player_radius);

    //
    // Consider moving macro's into a .h file
    //
    #define push_left() /*prev_player_x = player_x;*/ player_x = ((((int) player_x + player_radius) / GRID_SPACING) * GRID_SPACING) - player_radius/*; printf("prev_player_x - player_x = %f\n", prev_player_x - player_x); player_y_velocity -= (prev_player_x - player_x) / tanf(player_velocity_angle)*/
    #define push_down() player_y = (((((int) player_y - player_radius) / GRID_SPACING) + 1) * GRID_SPACING) + player_radius
    #define push_right() player_x = (((((int) player_x - player_radius) / GRID_SPACING) + 1) * GRID_SPACING) + player_radius
    #define push_up() player_y = ((((int) player_y + player_radius) / GRID_SPACING) * GRID_SPACING) - player_radius

    #define push_left_wall() push_left(); player_x_velocity = 0
    #define push_down_wall() push_down(); player_y_velocity = 0
    #define push_right_wall() push_right(); player_x_velocity = 0
    #define push_up_wall() push_up(); player_y_velocity = 0

    if (northeast_collision) {
        if (southeast_collision || northwest_collision) {
            if (southeast_collision && player_x_velocity >= 0) {
                push_left_wall();
            }
            if (northwest_collision) {
                push_down_wall();
            }
        } else {
            int overlapX = GRID_SPACING - (((int) player_x + player_radius) % GRID_SPACING);
            int overlapY = ((int) player_y - player_radius) % GRID_SPACING;
            if (overlapX > overlapY && player_x_velocity >= 0) {
                push_left_wall();
            } else {
                push_down_wall();
            }
        }
    } else if (southwest_collision) {
        if (northwest_collision || southeast_collision) {
            if (northwest_collision) {
                push_right_wall();
            }
            if (southeast_collision && player_y_velocity >= 0) {
                push_up_wall();
            }
        } else {
            int overlapX = GRID_SPACING - (((int) player_x - player_radius) % GRID_SPACING);
            int overlapY = ((int) player_y + player_radius) % GRID_SPACING;
            if (overlapX < overlapY) {
                push_right_wall();
            } else if (player_y_velocity >= 0) {
                push_up_wall();
            }
        }
    } else if (northwest_collision) {
        if (northeast_collision || southwest_collision) {
            if (northeast_collision) {
                push_down_wall();
            }
            if (southwest_collision) {
                push_right_wall();
            }
        } else {
            int overlapX = GRID_SPACING - (((int) player_x - player_radius) % GRID_SPACING);
            int overlapY = GRID_SPACING - (((int) player_y - player_radius) % GRID_SPACING);
            if (overlapX < overlapY) {
                push_right_wall();
            } else {
                push_down_wall();
            }
        }
    }
    if (southeast_collision) {
        if (southwest_collision || northeast_collision) {
            if (southwest_collision && player_y_velocity >= 0) {
                push_up_wall();
            }
            if (northeast_collision && player_x_velocity >= 0) {
                push_left_wall();
            }
        } else { 
            int overlapX = ((int) player_x + player_radius) % GRID_SPACING;
            int overlapY = ((int) player_y + player_radius) % GRID_SPACING;
            if (overlapX < overlapY && player_x_velocity >= 0) {
                push_left_wall();
            } else if (player_y_velocity >= 0) {
                push_up_wall();
            }
        }
    }

    //doors[0].progress = (-labs((((ssize_t) (SDL_GetTicks() / 20)) % GRID_SPACING) - (GRID_SPACING / 2)) * 2) + GRID_SPACING;

    // Move player by velocity
    player_x += player_x_velocity * delta_time;
    player_y += player_y_velocity * delta_time;

    if (show_player_trail) fill_dgp(player_x, player_y, DG_YELLOW);

    if (grid_follow_player) {
        grid_cam_x = player_x - ((WINDOW_WIDTH / 2) / grid_cam_zoom);
        grid_cam_y = player_y - ((WINDOW_HEIGHT / 2) / grid_cam_zoom);
    }

    
    //printf("player pos: (%f, %f) facing %f (%f deg)\n", player_x, player_y, player_angle, rad_deg(player_angle));
    //printf("player velocity: (%f, %f)\n", player_x_velocity, player_y_velocity);
    

    vertical_input = 0;
    horizontal_input = 0;
    rotation_input = 0;

    //rot_mobj_head->angle += M_PI / 20;
    //if (rot_mobj_head->angle > M_PI * 2) rot_mobj_head->angle -= M_PI * 2;
}

void render(void) {
    // Set background
    set_draw_color_rgb(C_BLACK);
    SDL_RenderClear(renderer);
    clear_array_window(0);

    if ((view == VIEW_FPS && fp_show_walls) || show_player_vision || grid_casting) { // Raycasting
        float ray_dists[WINDOW_WIDTH];
        float *floor_side_dists = NULL;
        int floor_side_dists_len = 0;

        float relative_ray_angle = -fov / 2;
        int sky_start = -1;
        // Raycast, draw walls, draw floors
        for (int ray_i = 0; ray_i < WINDOW_WIDTH; ray_i++) {
            // Calculate ray angle
            relative_ray_angle += radians_per_pixel;
            float ray_angle = player_angle + relative_ray_angle;
            fix_angle(ray_angle);

            if (sky_start == -1) {
                sky_start = ray_angle * (pixel_fov_circumfrence / (M_PI * 2));
            }

            #if FLOOR_TEXTURES
            float rel_cos = cosf(relative_ray_angle);
            float angle_cos = cosf(ray_angle);
            float angle_sin = sinf(ray_angle);
            #endif

            int wall_texture_index, wall_texture_x;
            xy hit = raycast(player_x, player_y, ray_angle, &wall_texture_index, &wall_texture_x, NULL);

            if ((view == VIEW_FPS && fp_show_walls) || grid_casting) {
                // Get hit distance
                ray_dists[ray_i] = fp_dist(hit.x, hit.y, ray_angle);

                if (view == VIEW_FPS && fp_show_walls) {
                    float wall_height = project(ray_dists[ray_i]);
                    float offset_y = player_height_y_offset(wall_height);

                    // Draw texture column offset based on player height
                    char is_wall_visible = ray_dists[ray_i] <= FP_RENDER_DISTANCE;
                    float start_wall = ((WINDOW_HEIGHT - wall_height) / 2.0f) + offset_y;

                    if (is_wall_visible) {
                        #if WALL_TEXTURES
                        float texture_row_height = wall_height / TEXTURE_WIDTH;
                        
                        float row_y = start_wall;
                        for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                            rgb original_color = textures[wall_texture_index][texture_y][wall_texture_x];
                            draw_col_frgb(ray_i, row_y, texture_row_height, shade_rgb(original_color, ray_dists[ray_i]));
                            row_y += texture_row_height;
                        }
                        #else
                        draw_col_rgb(ray_i, start_wall, wall_height, shade_rgb(C_WHITE, ray_dists[ray_i]));
                        #endif
                    }
                    
                    // Draw ceiling and floor
                    // Iterate enough to draw the entire ceiling or floor side, whichever is taller
                    #if FLOOR_TEXTURES
                    int end_ceiling = roundf(start_wall);
                    int end_floor = roundf(WINDOW_HEIGHT - (start_wall + wall_height));
                    int end_draw = end_floor > end_ceiling ? end_floor : end_ceiling;

                    float ceil_dist_factor = (GRID_SPACING - player_height) / player_height;
                    float pixel_y_worldspace = 0;
                    for (int pixel_y = 0; pixel_y < end_draw; pixel_y++) {
                        // Optimization possible:
                        // Calculate for floor or height distance depending on which will be calculated the most
                        // number of times so that conversion isn't necessary when only the side that needs conversion
                        // is being calculated.
                        // Performance impact would be minimal.

                        if (pixel_y == floor_side_dists_len) {
                            // If we have not calculated this distance yet, do so and store
                            floor_side_dists = realloc(floor_side_dists, ++floor_side_dists_len * sizeof(float));
                            floor_side_dists[pixel_y] = (player_height / ((GRID_SPACING / 2) - pixel_y_worldspace)) * fp_scale;
                        }

                        float floor_side_dist = floor_side_dists[pixel_y];
                        
                        if (pixel_y < end_floor) {
                            float straight_dist = floor_side_dist / rel_cos;
                            
                            float point_x = (angle_cos * straight_dist) + player_x;
                            float point_y = (angle_sin * straight_dist) + player_y;

                            Uint8 texture_num = floor_textures[(int) point_y / GRID_SPACING][(int) point_x / GRID_SPACING];
                            
                            // If we are drawing the sky texture
                            if (texture_num == 0) {
                                int sky_x = sky_start + ray_i;
                                if (sky_x >= pixel_fov_circumfrence) {
                                    sky_x -= pixel_fov_circumfrence;
                                }

                                rgba color = get_array_sprite(array_sprites[0], (int) (sky_x * sky_scale_x), (int) (pixel_y * sky_scale_y));
                                set_pixel_rgba(ray_i, WINDOW_HEIGHT - pixel_y - 1, color);
                            
                            // If we are drawing a ceiling texture
                            } else {
                                int texture_x = fmodf(point_x, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);
                                int texture_y = fmodf(point_y, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);

                                // Offset texture num by one because texture zero is the sky texture num
                                rgb color = shade_rgb(textures[texture_num - 1][texture_y][texture_x], floor_side_dist);
                                set_pixel_rgb(ray_i, WINDOW_HEIGHT - pixel_y - 1, color);
                            }
                        }

                        if (pixel_y < end_ceiling) {
                            float ceil_side_dist = floor_side_dist * ceil_dist_factor;
                            float straight_dist = ceil_side_dist / rel_cos;

                            float point_x = (angle_cos * straight_dist) + player_x;
                            float point_y = (angle_sin * straight_dist) + player_y;

                            Uint8 texture_num = ceiling_textures[(int) point_y / GRID_SPACING][(int) point_x / GRID_SPACING];
                            
                            // If we are drawing the sky texture
                            if (texture_num == 0) {
                                int sky_x = sky_start + ray_i;
                                if (sky_x >= pixel_fov_circumfrence) {
                                    sky_x -= pixel_fov_circumfrence;
                                }

                                rgba color = get_array_sprite(array_sprites[0], (int) (sky_x * sky_scale_x), (int) (pixel_y * sky_scale_y));
                                set_pixel_rgba(ray_i, pixel_y, color);
                            
                            // If we are drawing a ceiling texture
                            } else {
                                int texture_x = fmodf(point_x, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);
                                int texture_y = fmodf(point_y, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);

                                // Offset texture num by one because texture zero is the sky texture num
                                rgb color = shade_rgb(textures[texture_num - 1][texture_y][texture_x], ceil_side_dist);
                                set_pixel_rgb(ray_i, pixel_y, color);
                            }
                        }

                        pixel_y_worldspace += (float) GRID_SPACING / WINDOW_HEIGHT;
                    }
                    #endif
                }
            }
        }

        if (floor_side_dists_len != 0) {
            free(floor_side_dists);
            floor_side_dists_len = 0;
        }

        if (!grid_casting) {
            present_array_window();
            clear_array_window(0);
        }

        #if SPRITES
        if (view == VIEW_FPS || grid_casting) {
            // Store positions and distances of sprites on screen sorted by distance
            for (mobj *o = mobj_head; o; o = o->next) {
                float angle_to = get_angle_to(o->x, o->y);
                add_sprite_proj(o->x, o->y, angle_to, o->sprite_num);           
            }

            for (rot_mobj *o = rot_mobj_head; o; o = o->next) {
                float angle_to = get_angle_to(o->x, o->y);
                float relative_angle = player_angle + o->angle + (rot_sprite_incr / 2);
                if (relative_angle > M_PI * 2) relative_angle -= M_PI * 2;
                add_sprite_proj(o->x, o->y, angle_to, o->sprite_num + (int) (relative_angle / rot_sprite_incr));
            }

            // Render sprites
            for (sprite_proj *sprite = sprite_proj_head; sprite; sprite = sprite->next) {
                // Calculate screen x pos
                float center_x = angle_to_screen_x(sprite->angle);

                array_sprite *image = array_sprites + sprite->sprite_num;

                float proj_height = project(sprite->dist);
                float sprite_height = proj_height * image->world_height_percent;
                float sprite_width = (proj_height / image->height) * image->world_height_percent * image->width;

                // Don't draw if the sprite is completely offscreen
                if (center_x > WINDOW_WIDTH + (sprite_width / 2) && center_x < pixel_fov_circumfrence - (sprite_width / 2)) {
                    continue;
                }

                // Calculate bounds and increments for rendering
                float start_x = center_x - (sprite_width / 2);
                if (start_x > pixel_fov_circumfrence - sprite_width) {
                    start_x -= pixel_fov_circumfrence;
                }

                // Limit x to inside of screen
                int end_x = min(roundf(start_x + sprite_width), WINDOW_WIDTH);
                float skipped_x = 0;
                if (start_x < 0) {
                    skipped_x = -start_x;
                    start_x = 0;
                }

                float start_y_f = (WINDOW_HEIGHT - sprite_height) / 2;
                start_y_f += player_height_y_offset(proj_height);
                
                int start_y = roundf(start_y_f);
                int end_y = roundf(start_y_f + sprite_height);

                // Increments for source image position
                float image_y_incr = (float) image->height / (end_y - start_y);
                float image_x_incr = image->width / sprite_width;

                // Limit to inside of the screen
                float skipped_y = 0;
                if (start_y < 0) {
                    skipped_y = -start_y_f;
                    start_y = 0;
                }
                end_y = min(end_y, WINDOW_HEIGHT);

                // Render by column
                float image_x = skipped_x * image_x_incr;
                float start_image_y = skipped_y * image_y_incr;
                for (int render_x = roundf(start_x); render_x < end_x; render_x++) {
                    // Hide behind walls
                    if (sprite->dist < ray_dists[render_x]) {
                        float image_y = start_image_y;
                        for (int render_y = start_y; render_y < end_y; render_y++) {
                            rgba c = image->pixels[(image->width * (int) image_y) + (int) image_x];
                            set_pixel_rgba(render_x, render_y, shade_rgba(c, sprite->dist));
                            image_y += image_y_incr;
                        }
                    }
                    image_x += image_x_incr;
                }
            }

            sprite_proj_destroy_all();
        }
        #endif
    }

    if (view == VIEW_GRID) { // Grid rendering
        // Grid box fill
        for (int row = 0; row < GRID_HEIGHT; row++) {
            for (int col = 0; col < GRID_WIDTH; col++) {
                Uint8 space = get_map(col, row);
                switch (space) {
                    case MAP_EMPTY:
                    case MAP_SOLID:
                        g_draw_rect_rgb(
                            col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING,
                            space == MAP_EMPTY ? grid_fill_nonsolid : grid_fill_solid
                        );
                        break;
                    case MAP_HORIZ_DOOR:
                        //g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GID)
                        break;
                }
                switch (space) {
                    case MAP_EMPTY:
                    case MAP_HORIZ_DOOR:
                        g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING, grid_fill_nonsolid);
                        if (space == MAP_HORIZ_DOOR) {
                            door *door = get_door(col, row);
                            float progress;
                            if (door) {
                                progress = door->progress;
                            } else {
                                progress = GRID_SPACING;
                            }

                            g_draw_rect_rgb(
                                col * GRID_SPACING, (row * GRID_SPACING) + (GRID_SPACING / 2) - (GRID_DOOR_THICKNESS / 2),
                                progress, GRID_DOOR_THICKNESS, grid_fill_solid
                            );
                        }
                        break;
                    case MAP_SOLID:
                        g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING, grid_fill_solid);
                        break;
                }
            }
        }

        if (show_grid_lines) { // Grid lines
            // Vertical
            for (int i = 0; i < GRID_WIDTH + 1; i++) {
                g_draw_rect_rgb(
                    (i * GRID_SPACING) - (grid_line_width / 2.0f), 
                    0,
                    grid_line_width,
                    GRID_HEIGHT * GRID_SPACING,
                    grid_line_fill
                );
            }
            // Horizontal
            for (int i = 0; i < GRID_WIDTH + 1; i++) {
                g_draw_rect_rgb(
                    0,
                    (i * GRID_SPACING) - (grid_line_width / 2.0f),
                    GRID_WIDTH * GRID_SPACING,
                    grid_line_width,
                    grid_line_fill
                );
            }
        }

        // Map objects
        for (mobj *temp = mobj_head; temp; temp = temp->next) {
            g_draw_scale_point_rgb(temp->x, temp->y, grid_mobj_radius, grid_mobj_color);
        }

        for (rot_mobj *temp = rot_mobj_head; temp; temp = temp->next) {
            g_draw_scale_point_rgb(temp->x, temp->y, grid_mobj_radius, grid_mobj_color);
        }

        // Fill debug grid points
        for (struct fill_dgp *p_i = fill_dgp_head; p_i; p_i = p_i->next) {
            g_draw_point_rgb(p_i->x, p_i->y, dgp_radius, DG_COLORS[p_i->color_index]);
        }

        // Player direction pointer
        g_draw_scale_point_rgb(
            player_x + cosf(player_angle) * grid_player_pointer_dist,
            player_y + sinf(player_angle) * grid_player_pointer_dist,
            player_radius / 2.0f,
            grid_player_fill
        );
        // Player
        g_draw_scale_point_rgb(player_x, player_y, player_radius, grid_player_fill);

        // Temporary debug grid points
        for (size_t i = 0; i < num_temp_dgps; i++) {
            g_draw_point_rgb(temp_dgp_list[i].x, temp_dgp_list[i].y, 
                dgp_radius * (temp_dgp_list[i].color_index == DG_BLUE ? 1.5f : 1),
                DG_COLORS[temp_dgp_list[i].color_index]);
        }

        // Grid crosshairs
        if (show_grid_crosshairs) {
            draw_rect_frgb(WINDOW_WIDTH / 2, 0, 1, WINDOW_HEIGHT, C_WHITE);
            draw_rect_frgb(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, 1, C_WHITE);
        }

        // On-screen mouse coords
        if (show_mouse_coords) {
            char *coords_text;
            asprintf(&coords_text, "(%d, %d) %s", (int) grid_mouse_x, (int) grid_mouse_y, get_map_coords(grid_mouse_x, grid_mouse_y) ? "true" : "false");
            BF_DrawTextRgb(coords_text, mouse_x, mouse_y, 3, -1, C_RED, FALSE);
            free(coords_text);
        }
    } else if (view == VIEW_TERMINAL) { // Terminal view
        // Output from previous command
        BF_DrawTextRgb(DT_console_text, 0, 0, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);
    
        // Prompt string
        BF_FillTextRgb(TERMINAL_PROMPT, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);

        // Input text
        BF_FillTextRgb(terminal_input->text, terminal_font_size, WINDOW_WIDTH, terminal_font_color, TRUE);
    }

    // FPS readout
    if (show_fps && view != VIEW_TERMINAL) {
        #define FPS_READOUT_SIZE 5
        char *fps_text;
        asprintf(&fps_text, "fps %f", 1 / delta_time);
        BF_DrawText(fps_text, 0, 0, FPS_READOUT_SIZE, -1, 255, 255, 255, FALSE);
        free(fps_text);
    }

    // Clear dg things
    num_temp_dgps = 0;
    num_dgls = 0;

    present_array_window();
    SDL_RenderPresent(renderer);
}

void free_memory(void) {
    // Fill dgps
    struct fill_dgp *p_i = fill_dgp_head;
    while (p_i) {
        struct fill_dgp *next = p_i->next;
        free(p_i);
        p_i = next;
    }

    // Terminal alstring
    alstring_destroy(terminal_input);

    // Map objects
    mobj_destroy_all();
}

int main() {
    printf("Start\n");

    game_is_running = initialize_window("Raycasting");
    initialize_array_window();

    setup();
    debugging_start();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    debugging_end();
    destroy_window();
    destroy_array_window();
    free_memory();

    return 0;
}