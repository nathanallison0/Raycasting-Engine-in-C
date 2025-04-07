#include <math.h>
#include <string.h>
#include "./constants.h"
#include <dirent.h>

// Extra SDL functions
#include "../../SDL/SDL3Start.h"

// Font
#include "../BasicFont/BasicFont.h"

// Linked list generation
#include "../../linkedList.h"

// Textures
#include "textures.h"

float perc(int);
void calc_grid_cam_center(void);
void calc_grid_cam_zoom_p(void);
void reset_grid_cam(void);
void reset_player(void);
void zoom_grid_cam(int);
void zoom_grid_cam_center(int);
void rotate_player(float);
void push_player_forward(float);
void push_player_right(float);

#define rad_deg(radians) radians * (180 / M_PI)
#define deg_rad(degrees) degrees * (M_PI / 180)

enum VIEW_MODE {
    VIEW_GRID,
    VIEW_FPS,
    VIEW_TERMINAL
}
view_mode = VIEW_FPS,
prev_view_mode;

void set_view(enum VIEW_MODE mode) { prev_view_mode = view_mode; view_mode = mode; }

typedef struct {
    float x;
    float y;
} xy;

// Grid
#define GRID_WIDTH 25
#define GRID_HEIGHT 25

#define get_map(x, y) map[(int) (y)][(int) (x)]
#define get_map_coords(x, y) get_map((x) / GRID_SPACING, (y) / GRID_SPACING)

#define MAP_HORIZ_DOOR 2

Uint8 map[GRID_HEIGHT][GRID_WIDTH] = {
//   1 2 3 4 5 6 7 8 9 10111213141516171819202122232425
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 1
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 2
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 3
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1}, // 4
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 5
    {1,1,1,1,0,0,1,1,1,2,1,1,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 6
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 7
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 8
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 9
    {1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1}, // 10
    {1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 11
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 12
    {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 13
    {1,1,1,1,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 14
    {1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 15
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 16
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 17
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 18
    {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 19
    {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 20
    {1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 21
    {1,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1}, // 22
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1}, // 23
    {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1}, // 24
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}  // 25
};

Uint8 horiz_textures[GRID_HEIGHT + 1][GRID_WIDTH] = {
//   1 2 3 4 5 6 7 8 9 10111213141516171819202122232425
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 2
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 3
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 4
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 5
    {0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 6
    {0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 7
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 8
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 9
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 10
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 11
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 12
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 13
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 14
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 15
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 16
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 17
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 18
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 19
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 20
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 21
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 22
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 23
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 24
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 25
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}  // 26
};

Uint8 vertical_textures[GRID_HEIGHT][GRID_WIDTH + 1] = {
//   1 2 3 4 5 6 7 8 9 1011121314151617181920212223242526
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 1
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 2
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 3
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 4
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 5
    {0,0,0,0,0,0,0,0,0,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 6
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 7
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 8
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 9
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 10
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 11
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 12
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 13
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 14
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 15
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 16
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 17
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 18
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 19
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 20
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 21
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 22
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 23
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 24
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} // 25
};

// Grid visual
rgb grid_bg = {255, 0, 255};
rgb grid_fill_nonsolid = {235, 235, 235};
// blue {105, 165, 255}
rgb grid_fill_solid = {100, 110, 100};
int grid_line_width = 1;
rgb grid_line_fill = C_BLACK;
char show_grid_lines = FALSE;
// Grid Visual Camera
int grid_cam_x;
int grid_cam_y;
int grid_cam_zoom;
float grid_cam_zoom_p;
int grid_cam_zoom_min;
int grid_cam_zoom_max;
int grid_cam_center_x;
int grid_cam_center_y;
char show_mouse_coords = FALSE;
float grid_mouse_x, grid_mouse_y;
char grid_casting = FALSE;
char debug_prr = FALSE;

// Player
float player_x;
int i_player_x;
float player_y;
int i_player_y;
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

// Player interaction
#define TRANSITION_LEN 0.5
float transition_timer = -1;
char transition_dir = 0;

void assign_i_player_pos(void) {
    i_player_x = roundf(player_x);
    i_player_y = roundf(player_y);
}
// Player grid visual
int grid_player_pointer_dist = 15;
int grid_player_pointer_radius_offset = 50;
rgb grid_player_fill = {255, 50, 50};
char grid_follow_player = TRUE;
char show_player_vision = FALSE;
char show_player_trail = FALSE;

// Grid mobjs
int grid_mobj_radius = 7;

// First person rendering
float fp_scale = 1 / 0.009417f;
#define FP_RENDER_DISTANCE 2000
#define FP_BRIGHTNESS 0.9f

#define HAS_FLASH FALSE
#if HAS_FLASH
#define FLASH_DISTANCE 2000
#define FLASH_BRIGHTNESS 1.0f
#define FLASH_WIDTH (WINDOW_WIDTH / 6)
#define FLASH_RAD_SQUARED ((FLASH_WIDTH / 2) * (FLASH_WIDTH / 2))
float flash_heights[FLASH_WIDTH / 2];
#endif

float fp_brightness_appl;
int fp_floor_render_height;
char fp_show_walls = TRUE;
int pixel_fov_circumfrence;
float radians_per_pixel;
#define FLOOR_RES 2

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

// Mobjs and sprites
int sprite_not_found_num = -1;
int num_sprites = 0;
char **sprite_names = NULL;

#define __get_sprite_num__() \
item->sprite_num = sprite_not_found_num; \
for (int i = 0; i < num_sprites; i++) { \
    if (sprite_names[i] && strcmp(sprite_name, sprite_names[i]) == 0) { \
        item->sprite_num = i; \
        break; \
    } \
} \
//if (item->sprite_num == sprite_not_found_num) printf("Couldn't find sprite %s\n", sprite_name);

rgb grid_mobj_color = C_BLUE;
int num_mobjs = 0;
__linked_list_all_add__(
    mobj,
    float x; float y; int sprite_num,
    (float x, float y, char *sprite_name),
        item->x = x;
        item->y = y;
        __get_sprite_num__()
        num_mobjs++
)

#define NUM_ROT_SPRITE_FRAMES 8
int num_rot_mobjs = 0;
float rot_sprite_incr;
__linked_list_all_add__(
    rot_mobj, float x; float y; float angle; int sprite_num,
    (float x, float y, float angle, char *sprite_name),
        item->x = x;
        item->y = y;
        __get_sprite_num__()
        num_rot_mobjs++
)

#define SPRITE_PATH "./sprites/"
SDL_Texture **sprites = NULL;
#define SPRITE_NOT_FOUND_NAME "sprite_not_found"

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
    draw_rect(
        (x - grid_cam_x) * grid_cam_zoom_p,
        (y - grid_cam_y) * grid_cam_zoom_p,
        length * grid_cam_zoom_p,
        width * grid_cam_zoom_p,
        r, g, b
    );
}

void g_draw_rect_rgb(float x, float y, float length, float width, rgb color) {
    g_draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void g_draw_point(float x, float y, float radius, Uint8 r, Uint8 g, Uint8 b) {
    draw_point(
        (x - grid_cam_x) * grid_cam_zoom_p,
        (y - grid_cam_y) * grid_cam_zoom_p,
        radius,
        r, g, b
    );
}

void g_draw_point_rgb(float x, float y, float radius, rgb color) {
    g_draw_point(x, y, radius, color.r, color.g, color.b);
}

void g_draw_scale_point(float x, float y, float radius, Uint8 r, Uint8 g, Uint8 b) {
    g_draw_point(x, y, radius * grid_cam_zoom_p, r, g, b);
}

void g_draw_scale_point_rgb(float x, float y, float radius, rgb color) {
    g_draw_scale_point(x, y, radius, color.r, color.g, color.b);
}

void g_draw_line(float x1, float y1, float x2, float y2, Uint8 r, Uint8 g, Uint8 b) {
    draw_line(
        (x1 - grid_cam_x) * grid_cam_zoom_p,
        (y1 - grid_cam_y) * grid_cam_zoom_p,
        (x2 - grid_cam_x) * grid_cam_zoom_p,
        (y2 - grid_cam_y) * grid_cam_zoom_p,
        r, g, b
    );
}

void g_draw_line_rgb(float x1, float y1, float x2, float y2, rgb color) {
    g_draw_line(x1, y1, x2, y2, color.r, color.g, color.b);
}

// Raycasting
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
        c_hy = ((ceil(y / GRID_SPACING)) * GRID_SPACING);// - 1;
        c_hx = ((c_hy - y) / tan(alpha)) + x;
        d_hx = GRID_SPACING / tan(alpha);
        d_hy = GRID_SPACING;
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = (tan(alpha) * (c_vx - x)) + y;
        d_vx = GRID_SPACING;
        d_vy = tan(alpha) * (d_vx);
    } else if (quadrant == 2) {
        alpha = angle - (M_PI_2);
        c_hy = (ceil(y / GRID_SPACING) * GRID_SPACING);// - 1;
        c_hx = tan(alpha) * (y - c_hy) + x;
        d_hy = GRID_SPACING;
        d_hx = -GRID_SPACING * tan(alpha);
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y + ((x - c_vx) / tan(alpha));
        d_vx = -GRID_SPACING;
        d_vy = GRID_SPACING / tan(alpha);
    } else if (quadrant == 3) {
        alpha = angle - M_PI;
        c_hy = (floor(y / GRID_SPACING) * GRID_SPACING);
        c_hx = x + (c_hy - y) / tan(alpha);
        d_hy = -GRID_SPACING;
        d_hx = -GRID_SPACING * tan((M_PI_2) - alpha);
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y - (tan(alpha) * (x - c_vx));
        d_vx = -GRID_SPACING;
        d_vy = -GRID_SPACING * tan(alpha);
    } else {
        alpha = angle - (M_PI + (M_PI_2));
        c_hy = ((floor(y / GRID_SPACING) - 1) * GRID_SPACING) + GRID_SPACING;
        c_hx = x - tan(alpha) * (c_hy - y);
        d_hy = -GRID_SPACING;
        d_hx = GRID_SPACING * tan(alpha);
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = y - (c_vx - x) / tan(alpha);
        d_vx = GRID_SPACING;
        d_vy = -GRID_SPACING / tan(alpha);
    }

    float diagonal_dist = fabs((M_PI_4) - alpha) / (M_PI * 200);
    c_epsilon_h = (sqrt(pow(c_hx - x, 2) + pow(c_hy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_h = (sqrt(pow(d_hx, 2) + pow(d_hy, 2)) / GRID_SPACING) * diagonal_dist;
    c_epsilon_v = (sqrt(pow(c_vx - x, 2) + pow(c_vy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_v = (sqrt(pow(d_vx, 2) + pow(d_vy, 2)) / GRID_SPACING) * diagonal_dist;

    return quadrant;
}

#define get_horiz_texture(x, y) horiz_textures   [(int) (y) / GRID_SPACING][(int) (x) / GRID_SPACING]
#define get_vert_texture(x, y)  vertical_textures[(int) (y) / GRID_SPACING][(int) (x) / GRID_SPACING]
xy raycast(float x, float y, float angle, int *texture_index, char *horiz_hit) {
    float vars[12];
    char quadrant = raycast_vars(x, y, angle, vars);

    while (!(
        !(
            ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < GRID_WIDTH ) &&
            ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < GRID_HEIGHT )
            
        ) || (
            ( get_map((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) || get_map((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) ) ||
            ( get_map((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) || get_map((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) )
        ))
    ) {
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
        *horiz_hit = TRUE;
        *texture_index = get_horiz_texture(c_hx, c_hy);

        // Door handling
        if (
            (quadrant <= 2 && get_map_coords(c_hx, c_hy + (GRID_SPACING / 2)) == MAP_HORIZ_DOOR) ||
            (quadrant >= 3 && get_map_coords(c_hx, c_hy - (GRID_SPACING / 2)) == MAP_HORIZ_DOOR)
        ) {
            if (
                ((quadrant == 1 || quadrant == 2) && (int) c_vy >= c_hy + (GRID_SPACING / 2)) ||
                ((quadrant == 3 || quadrant == 4) && (int) c_vy <= c_hy - GRID_SPACING / 2)
            ) {
                return (xy) {c_hx + (d_hx / 2), c_hy + (d_hy / 2)};
            } else {
                *horiz_hit = FALSE;
                *texture_index = get_vert_texture(c_vx, c_vy);
                return (xy) {c_vx, c_vy};
            }
        }

        return (xy) {c_hx, c_hy};
    } else {
        *horiz_hit = FALSE;
        *texture_index = get_vert_texture(c_vx, c_vy);

        // Door handling
        

        return (xy) {c_vx, c_vy};
    }
}

int project(float distance) {
    return WINDOW_HEIGHT / (distance / fp_scale);
}

Uint8 shade(Uint8 color, float distance, float render_distance, float bright) {
    return (color - (((float) color / render_distance) * distance)) * bright * fp_brightness_appl;
}

rgb shade_rgb(rgb color, float distance, float render_distance, float bright) {
    return (rgb) {
        shade(color.r, distance, render_distance, bright),
        shade(color.g, distance, render_distance, bright),
        shade(color.b, distance, render_distance, bright)
    };
}

#define shade_reg(color, distance) shade_rgb(color, distance, FP_RENDER_DISTANCE, FP_BRIGHTNESS)
#define shade_flash(color, distance) shade_rgb(color, distance, FLASH_DISTANCE, FLASH_BRIGHTNESS)

void draw_floor_texture_bit(float ray_angle, float rel_ray_angle, int x, int y, int height
    #if HAS_FLASH
    , char is_flash
    #endif
) {
    // Get real and project distance to point
    // Player height reference
    float dist_to_point = ((float) (WINDOW_HEIGHT / 2) / ((WINDOW_HEIGHT / 2) - (y + ((float) height / 2)))) * fp_scale;
    float proj_dist_to_point = dist_to_point / cosf(rel_ray_angle);
    
    // Get coordinates of texture point in world space
    float point_x = (cosf(ray_angle) * proj_dist_to_point) + player_x;
    float point_y = (sinf(ray_angle) * proj_dist_to_point) + player_y;

    // Find texture coordinates
    int texture_x = fmodf(point_x, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);
    int texture_y = fmodf(point_y, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);

    // Shade color
    #if HAS_FLASH
    rgb shaded_color;
    if (is_flash) {
        shaded_color = shade_flash(textures[TEXTURE_FLOOR_INDEX][texture_y][texture_x], proj_dist_to_point);
    } else {
        shaded_color = shade_reg(textures[TEXTURE_FLOOR_INDEX][texture_y][texture_x], proj_dist_to_point);
    }
    #else
    rgb shaded_color = shade_reg(textures[TEXTURE_FLOOR_INDEX][texture_y][texture_x], proj_dist_to_point);
    #endif


    // Draw on floor and ceiling
    draw_rect_rgb(x, y, 1, height, shaded_color);
    draw_rect_rgb(x, WINDOW_HEIGHT - y - height, 1, height, shaded_color);
}

float get_mobj_angle(float x, float y) {
    float angle = atan2f(y - player_y, x - player_x);
    if (angle < 0) angle += (M_PI * 2);
    return angle;
}

void add_sprite_proj(float x, float y, float angle, int sprite_num) {
    float distance = fp_dist(x, y, angle);

    #if !HAS_FLASH
    if (distance > FP_RENDER_DISTANCE) {
        return;
    }
    #endif

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

float perc(int percent) {
    return (percent / 100.0f);
}

void calc_grid_cam_zoom_p(void) {
    grid_cam_zoom_p = perc(grid_cam_zoom);
}

void calc_grid_cam_center(void) {
    grid_cam_center_x = grid_cam_x + roundf( (WINDOW_WIDTH / 2) / grid_cam_zoom_p );
    grid_cam_center_y = grid_cam_y + roundf( (WINDOW_HEIGHT / 2) / grid_cam_zoom_p );
}

// Resets
void reset_grid_cam(void) {
    grid_cam_x = 0;
    grid_cam_y = 0;
    grid_cam_zoom = 100;
    calc_grid_cam_zoom_p();
    grid_cam_zoom_min = 40;
    grid_cam_zoom_max = 300;
}

void reset_player(void) {
    player_x = GRID_SPACING * 4;
    player_y = GRID_SPACING * 4;
    assign_i_player_pos();
    player_x_velocity = 0;
    player_y_velocity = 0;
    player_angle = 0;
}

// Zoom grid
void zoom_grid_cam(int zoom) {
    grid_cam_zoom += zoom;
    if (grid_cam_zoom < grid_cam_zoom_min) grid_cam_zoom = grid_cam_zoom_min;
    else if (grid_cam_zoom > grid_cam_zoom_max) grid_cam_zoom = grid_cam_zoom_max;
    calc_grid_cam_zoom_p();
}

void zoom_grid_cam_center(int zoom) {
    calc_grid_cam_center();
    int old_grid_cam_center_x = grid_cam_center_x;
    int old_grid_cam_center_y = grid_cam_center_y;
    zoom_grid_cam(zoom);
    grid_cam_x = old_grid_cam_center_x - round( (WINDOW_WIDTH / 2) / grid_cam_zoom_p );
    grid_cam_y = old_grid_cam_center_y - round( (WINDOW_HEIGHT / 2) / grid_cam_zoom_p );
}

// Player control
void rotate_player(float angle) {
    player_angle += angle;
    while (player_angle < 0) player_angle += M_PI * 2;
    while (player_angle >= M_PI * 2) player_angle -= (M_PI * 2);
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

void load_sprite(char *relative_path, char *name) {
    // Get full path to sprite
    char *full_path;
    asprintf(&full_path, SPRITE_PATH "%s", relative_path);

    // Load texture
    SDL_Texture *sprite = IMG_LoadTexture(renderer, full_path);
    free(full_path);

    if (sprite) {
        // Allocate space for texture and name
        sprites = realloc(sprites, ++num_sprites * sizeof(SDL_Texture *));
        sprite_names = realloc(sprite_names, num_sprites * sizeof(char *));

        // Add texture to list
        sprites[num_sprites - 1] = sprite;

        // Add name to list
        if (name) {
            int new_name_size = strlen(name) + 1;
            sprite_names[num_sprites - 1] = malloc(new_name_size);
            strlcpy(sprite_names[num_sprites - 1], name, new_name_size);

            // Set sprite not found index if not already set
            if (sprite_not_found_num == -1 && strcmp(sprite_names[num_sprites - 1], SPRITE_NOT_FOUND_NAME) == 0) {
                sprite_not_found_num = num_sprites - 1;
            }
        } else {
            sprite_names[num_sprites - 1] = NULL;
        }
    } else {
        printf("Could not load sprite %s%s\n", relative_path, name);
    }
}

const bool *state;
void setup(void) {
    // Initialize keyboard state
    state = SDL_GetKeyboardState(NULL);

    pixel_fov_circumfrence = (WINDOW_WIDTH / fov) * (M_PI * 2);
    radians_per_pixel = fov / WINDOW_WIDTH;
    fp_floor_render_height = ceilf((WINDOW_HEIGHT - project(FP_RENDER_DISTANCE)) / 2.0f);
    rot_sprite_incr = ((M_PI * 2) / NUM_ROT_SPRITE_FRAMES);

    // Calculate flashlight heights
    #if HAS_FLASH
    for (int i = 0; i < FLASH_WIDTH / 2; i++) {
        flash_heights[i] = sqrtf(FLASH_RAD_SQUARED - powf(i - (FLASH_WIDTH / 2), 2));
    }
    #endif

    fp_brightness_appl = FP_BRIGHTNESS;

    // Set player start pos
    reset_player();
    reset_grid_cam();
    grid_cam_zoom_p = perc(grid_cam_zoom);
    calc_grid_cam_center();

    // Load sprite images
    DIR *sprite_dir = opendir(SPRITE_PATH);
    if (sprite_dir) {
        struct dirent *entry;
        while ((entry = readdir(sprite_dir))) {
            if (entry->d_type == DT_REG) {
                int name_len = strlen(entry->d_name) - 3;
                char name[name_len + 1];
                strlcpy(name, entry->d_name, name_len);
                load_sprite(entry->d_name, name);
            } else if (entry->d_type == DT_DIR && entry->d_name[0] != '.') {
                // Load sprite 1 in with the name of the directory so it serves as the first frame of the rotation
                char *first;
                asprintf(&first, "%s/1.png", entry->d_name);
                load_sprite(first, entry->d_name);
                free(first);

                // Load the rest of the sprites with NULL names
                char *others;
                for (int i = 2; i <= NUM_ROT_SPRITE_FRAMES; i++) {
                    asprintf(&others, "%s/%d.png", entry->d_name, i);
                    load_sprite(others, NULL);
                    free(others);
                }
            }
        }
        closedir(sprite_dir);
    } else {
        printf("Could not open sprite directory\n");
    }

    // Debug mobjs
    #define mobj_flat(x, y, name) mobj_create(x * GRID_SPACING, y * GRID_SPACING, name)
    #define mobj_rot(x, y, angle, name) rot_mobj_create(x * GRID_SPACING, y * GRID_SPACING, angle, name)
    
    // Army of MJs
    for (float x = 14.9; x < 24; x += 2) {
        for (int y = 11; y < 21; y += 2) {
            mobj_flat(x, y, "mj1");
        }
    }

    mobj_rot(3, 3, 0, "couch");
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
            zoom_grid_cam_center(-event.wheel.y);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            mouse_x = event.motion.x;
            mouse_y = event.motion.y;
            if (show_mouse_coords) {
                grid_mouse_x = grid_cam_x + (event.motion.x / grid_cam_zoom_p);
                grid_mouse_y = grid_cam_y + (event.motion.y / grid_cam_zoom_p);
            }

            if (prev_mouse_x != -1 && prev_mouse_y != -1 && left_mouse_down) {
                grid_cam_x -= roundf( (mouse_x - prev_mouse_x) / grid_cam_zoom_p );
                grid_cam_y -= roundf( (mouse_y - prev_mouse_y) / grid_cam_zoom_p );
            }
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
    }

    shift = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];

    if (state[SDL_SCANCODE_ESCAPE]) game_is_running = FALSE;

    if (key_just_pressed(SDL_SCANCODE_SLASH)) {
        if (view_mode == VIEW_TERMINAL) set_view(prev_view_mode);
        else set_view(VIEW_TERMINAL);
    }

    if (view_mode == VIEW_FPS || view_mode == VIEW_GRID) {
        if (key_just_pressed(SDL_SCANCODE_2)) set_view(VIEW_GRID);
        if (key_just_pressed(SDL_SCANCODE_1)) set_view(VIEW_FPS);
        if (key_just_pressed(SDL_SCANCODE_C)) toggle(&grid_follow_player);

        if (transition_dir == 0) {
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
        }

    } else if (view_mode == VIEW_TERMINAL && event.type == SDL_EVENT_KEY_DOWN) {
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
    //
    // Has anything changed, ie is ther anything to be done?
    //

    // Rotate and apply force to player by input
    //
    // Consider only calling these if its required
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

    //
    // consider moving this into a function so you can abstract the logic
    // elsewhere and then adjust simply inside the function
    //
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

    // Decrement transition timer
    if (transition_dir != 0) {
        transition_timer += delta_time * transition_dir;
        if (transition_dir == 1 && transition_timer > TRANSITION_LEN) {
            transition_dir = 0;
            transition_timer = TRANSITION_LEN;
        } else if (transition_dir == -1 && transition_timer < 0) {
            transition_timer = 0;
            transition_dir = 1;
        }
        fp_brightness_appl = transition_timer / TRANSITION_LEN;
    }

    // Move player by velocity
    player_x += player_x_velocity * delta_time;
    player_y += player_y_velocity * delta_time;
    assign_i_player_pos();

    if (show_player_trail) fill_dgp(player_x, player_y, DG_YELLOW);

    if (grid_follow_player) {
        grid_cam_x = roundf( player_x - ((WINDOW_WIDTH / 2) / grid_cam_zoom_p) );
        grid_cam_y = roundf( player_y - ((WINDOW_HEIGHT / 2) / grid_cam_zoom_p) );
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

    if ((view_mode == VIEW_FPS && fp_show_walls) || show_player_vision || grid_casting) { // Raycasting
        float ray_dists[WINDOW_WIDTH];

        // Raycast, draw walls, draw floors
        for (int ray_i = 0; ray_i < WINDOW_WIDTH; ray_i++) {
            // Calculate ray angle
            float relative_ray_angle = (radians_per_pixel * ray_i) - (fov / 2);
            float ray_angle = player_angle + relative_ray_angle; 
            if (ray_angle < 0) ray_angle += M_PI * 2;
            else if (ray_angle >= M_PI * 2) ray_angle -= M_PI * 2;

            char horiz_hit;
            int texture_index;
            xy hit = raycast(player_x, player_y, ray_angle, &texture_index, &horiz_hit);
            
            char draw_reverse = (ray_angle < M_PI && horiz_hit) || (M_PI_2 < ray_angle && ray_angle < M_PI + M_PI_2 && !horiz_hit);

            if ((view_mode == VIEW_FPS && fp_show_walls) || grid_casting) {
                // Get hit distance
                ray_dists[ray_i] = fp_dist(hit.x, hit.y, ray_angle);

                if (view_mode == VIEW_FPS && fp_show_walls) {
                    int wall_height = project(ray_dists[ray_i]);

                    // Draw texture column
                    char is_wall_visible = ray_dists[ray_i] <= FP_RENDER_DISTANCE;
                    #if HAS_FLASH
                    char col_is_flash = ((WINDOW_WIDTH / 2) - (FLASH_WIDTH) / 2) < ray_i && ray_i < ((WINDOW_WIDTH / 2) + (FLASH_WIDTH / 2));
                    float half_flash_height;
                    #endif

                    if (
                        #if HAS_FLASH
                            is_wall_visible || col_is_flash
                        #else
                            is_wall_visible
                        #endif
                    ) {
                        // Constants for both types of wall column
                        float relative_wall_x = fmodf(horiz_hit ? hit.x : hit.y, GRID_SPACING);
                        int texture_x = (relative_wall_x / GRID_SPACING) * TEXTURE_WIDTH;

                        if (draw_reverse) {
                            texture_x = TEXTURE_WIDTH - texture_x - 1;
                        }

                        float texture_row_height = (float) wall_height / TEXTURE_WIDTH;

                        #if HAS_FLASH // Draw with flashlight
                        if (col_is_flash) {
                            half_flash_height = flash_heights[(FLASH_WIDTH / 2) - abs(ray_i - (WINDOW_WIDTH / 2)) - 1];
                        }

                        float row_y;
                        // If the wall would not be visible without the flashlight but is in the flashlight's path
                        if (col_is_flash && !is_wall_visible && half_flash_height * 2 < wall_height) {
                            row_y = (WINDOW_HEIGHT / 2) - half_flash_height;
                            float margin_height = fmodf(half_flash_height, texture_row_height);
                            int texture_y = ((float) TEXTURE_WIDTH / wall_height) * (row_y - ((WINDOW_HEIGHT / 2) - (wall_height / 2.0f)));

                            // Draw topmost texture pixel, partially if needed so the circle looks natural
                            if (margin_height != 0) {
                                draw_rect_rgb(ray_i, row_y, 1, margin_height, shade_flash(textures[texture_index][texture_y++][texture_x], ray_dists[ray_i]));
                            }

                            // Draw uniform texture pixels until bottom edge
                            for (;row_y + texture_row_height < (WINDOW_HEIGHT / 2) + half_flash_height; row_y += texture_row_height) {
                                draw_rect_rgb(ray_i, row_y, 1, texture_row_height, shade_flash(textures[texture_index][texture_y++][texture_x], ray_dists[ray_i]));
                            }

                            // Draw bottom texture pixel partially
                            if (margin_height != 0) {
                                draw_rect_rgb(ray_i, row_y, 1, margin_height, shade_flash(textures[texture_index][texture_y][texture_x], ray_dists[ray_i]));
                            }
                        } else {
                            row_y = (WINDOW_HEIGHT - wall_height) / 2.0f;
                            if (is_wall_visible) {
                                for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                                    rgb original_color = textures[texture_index][texture_y][texture_x];
                                    draw_rect_rgb(ray_i, row_y, 1, texture_row_height, col_is_flash && (WINDOW_HEIGHT / 2) - half_flash_height < row_y + (texture_row_height / 2) && row_y + (texture_row_height / 2) < (WINDOW_HEIGHT / 2) + half_flash_height ? shade_flash(original_color, ray_dists[ray_i]) : shade_reg(original_color, ray_dists[ray_i]));
                                    row_y += texture_row_height;
                                }
                            } else {
                                for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                                    rgb original_color = textures[texture_index][texture_y][texture_x];
                                    draw_rect_rgb(ray_i, row_y, 1, texture_row_height, shade_flash(original_color, ray_dists[ray_i]));
                                    row_y += texture_row_height;
                                }
                            }
                        }
                        #else // Draw normal walls, no flashlight
                        float row_y = (WINDOW_HEIGHT - wall_height) / 2.0f;
                        for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                            rgb original_color = textures[texture_index][texture_y][texture_x];
                            draw_rect_rgb(ray_i, row_y, 1, texture_row_height, shade_reg(original_color, ray_dists[ray_i]));
                            row_y += texture_row_height;
                        }
                        #endif
                    }
                    // Draw floor and ceiling tile pixels below wall
                    int end_floor;
                    int end_floor_wall = ceilf(((float) WINDOW_HEIGHT - wall_height) / 2);
                    if (is_wall_visible) {
                        end_floor = end_floor_wall;
                    } else {
                        end_floor = fp_floor_render_height;
                    }

                    #if HAS_FLASH
                    char flash_shows_floor = !is_wall_visible && (int) (half_flash_height * 2) > wall_height;
                    char row_is_flash = FALSE;
                    char drawing_flash = FALSE;
                    #endif
                    for (int pixel_y = 0; pixel_y < end_floor; pixel_y += FLOOR_RES) {
                        // Remove height if drawing up to the wall slice or end of visibility
                        int row_height = pixel_y + FLOOR_RES > end_floor ? end_floor - pixel_y : FLOOR_RES;

                        // Check to see if we have entered the flashlight's area
                        #if HAS_FLASH
                        if (!row_is_flash && pixel_y > (WINDOW_HEIGHT / 2) - half_flash_height) {
                            row_is_flash = TRUE;
                        }
                        #endif

                        draw_floor_texture_bit(ray_angle, relative_ray_angle, ray_i, pixel_y, row_height 
                            #if HAS_FLASH
                            , row_is_flash
                            #endif
                        );

                        // Jump ahead to flashlight area and draw if the flashlight is illuminating an area too far
                        // away to see normally
                        #if HAS_FLASH
                        if (!drawing_flash && flash_shows_floor && pixel_y + FLOOR_RES > fp_floor_render_height) {
                            drawing_flash = TRUE;
                            pixel_y = (WINDOW_HEIGHT / 2) - half_flash_height;
                            int margin_height = FLOOR_RES - (pixel_y % FLOOR_RES);

                            // Draw margin row so that the flash circle looks natural
                            draw_floor_texture_bit(ray_angle, relative_ray_angle, ray_i, pixel_y, margin_height, TRUE);

                            // Get back onto the set increment path for the rows so the texturing looks the same
                            pixel_y += margin_height - FLOOR_RES;

                            end_floor = end_floor_wall;
                        }
                        #endif
                    }
                }
            }
        }

        if (!grid_casting) {
            present_array_window();
            clear_array_window(0);
        }

        if (view_mode == VIEW_FPS || grid_casting) {

            // Store positions and distances of sprites on screen
            for (mobj *o = mobj_head; o; o = o->next) {
                float angle_to = get_mobj_angle(o->x, o->y);
                add_sprite_proj(o->x, o->y, angle_to, o->sprite_num);                
            }

            for (rot_mobj *o = rot_mobj_head; o; o = o->next) {
                float angle_to = get_mobj_angle(o->x, o->y);
                float relative_angle = player_angle + o->angle + (rot_sprite_incr / 2);
                if (relative_angle > M_PI * 2) relative_angle -= M_PI * 2;
                add_sprite_proj(o->x, o->y, angle_to, o->sprite_num + (int) (relative_angle / rot_sprite_incr));
            }

            // Render sprites
            for (sprite_proj *sprite = sprite_proj_head; sprite; sprite = sprite->next) {
                int sprite_height = project(sprite->dist);

                // Calculate screen x pos
                float relative_angle = sprite->angle - (player_angle - (fov / 2));
                if (relative_angle > M_PI * 2) relative_angle -= M_PI * 2;
                else if (relative_angle < 0) relative_angle += M_PI * 2;
                int screen_x = (WINDOW_WIDTH / fov) * relative_angle;

                SDL_Texture *sprite_to_draw = sprites[sprite->sprite_num];

                // Scale sprite width using height
                float image_width, image_height;
                SDL_GetTextureSize(sprite_to_draw, &image_width, &image_height);
                int sprite_width = ((float) sprite_height / image_height) * image_width;

                // End if sprite would draw completely off screen
                // OR the sprite is farther than the render distance and isn't in the flashlight
                #if HAS_FLASH
                char is_in_flash = (
                    screen_x - (sprite_width / 2) > (WINDOW_WIDTH / 2) + (FLASH_WIDTH / 2) ||
                    screen_x + (sprite_width / 2) < (WINDOW_WIDTH / 2) - (FLASH_WIDTH / 2)
                );
                #endif
                if (
                    (screen_x > WINDOW_WIDTH + (sprite_width / 2) && screen_x < pixel_fov_circumfrence - (sprite_width / 2))
                    #if HAS_FLASH
                    || (sprite->dist > FP_RENDER_DISTANCE && is_in_flash)
                    #endif
                ) {
                    continue;
                }

                // Shade sprite by distance
                Uint8 sprite_shading = shade(255, sprite->dist, FP_RENDER_DISTANCE, FP_BRIGHTNESS);
                SDL_SetTextureColorMod(sprite_to_draw, sprite_shading, sprite_shading, sprite_shading);

                // Get start and end of drawing and how much was skipped
                int start_x = roundf(screen_x - (sprite_width / 2));
                if (start_x > pixel_fov_circumfrence - sprite_width)
                    start_x -= pixel_fov_circumfrence;
                
                int end_x = start_x + sprite_width;
                int skipped = start_x;
                start_x = max(start_x, 0);
                skipped = start_x - skipped;
                
                // Draw columns to scale the sprite by distance
                SDL_FRect dest = {start_x, (WINDOW_HEIGHT / 2) - (sprite_height / 2), 1, sprite_height};
                float texture_incr = (float) image_width / sprite_width;
                float texture_col = skipped * texture_incr;
                SDL_FRect source = {0, 0, ceilf(texture_incr), image_height};

                while (dest.x < end_x && dest.x < WINDOW_WIDTH) {
                    // Only draw column if it is in front of its corresponding wall
                    if (ray_dists[(int) dest.x] > sprite->dist) {
                        // float angle_to_point = ((dest.x * radians_per_pixel) - (fov / 2)) + player_angle;
                        // float rel_angle_to = angle_to_point - sprite->angle;
                        // float dist_from_center = sprite->dist * tanf(rel_angle_to);

                        // float sprite_part_x = sprite->obj->x - (dist_from_center * sinf(player_angle));
                        // float sprite_part_y = sprite->obj->y + (dist_from_center * cosf(player_angle));

                        source.x = texture_col;
                        if (view_mode == VIEW_FPS) {
                            SDL_RenderTexture(renderer, sprite_to_draw, &source, &dest);
                        }
                    }
                    dest.x++;
                    texture_col += texture_incr;
                }
            }

            sprite_proj_destroy_all();
        }

        //draw_rect_a((WINDOW_WIDTH / 2) - (FLASH_WIDTH / 2), (WINDOW_HEIGHT / 2) - (FLASH_WIDTH / 2), FLASH_WIDTH, FLASH_WIDTH, 0, 0, 255, 25);
    }

    if (view_mode == VIEW_GRID) { // Grid rendering
        // Grid box fill
        for (int row = 0; row < GRID_HEIGHT; row++) {
            for (int col = 0; col < GRID_WIDTH; col++) {
                g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING,
                get_map(col, row) ? grid_fill_solid : grid_fill_nonsolid);
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
            player_radius * perc(grid_player_pointer_radius_offset),
            grid_player_fill
        );
        // Player
        g_draw_scale_point_rgb(i_player_x, i_player_y, player_radius, grid_player_fill);

        // Temporary debug grid points
        for (size_t i = 0; i < num_temp_dgps; i++) {
            g_draw_point_rgb(temp_dgp_list[i].x, temp_dgp_list[i].y, 
                dgp_radius * (temp_dgp_list[i].color_index == DG_BLUE ? 1.5f : 1),
                DG_COLORS[temp_dgp_list[i].color_index]);
        }

        // Debug grid lines
        for (size_t i = 0; i < num_dgls; i++) {
            g_draw_line_rgb(
                dgl_list[i].x1,
                dgl_list[i].y1,
                dgl_list[i].x2,
                dgl_list[i].y2,
                DG_COLORS[dgl_list[i].color_index]
            );
        }

        // Player fov quadrant
        rgb quad_color = {0, 255, 0};
        int quad_len = GRID_SPACING * 4;
        float cos_l = cos(player_angle - M_PI_4) * quad_len;
        float sin_l = sin(player_angle - M_PI_4) * quad_len;
        draw_line_rgb((WINDOW_WIDTH / 2) - cos_l, (WINDOW_HEIGHT / 2) - sin_l, (WINDOW_WIDTH / 2) + cos_l, (WINDOW_HEIGHT / 2) + sin_l, quad_color);
        draw_line_rgb((WINDOW_WIDTH / 2) + sin_l, (WINDOW_HEIGHT / 2) - cos_l, (WINDOW_WIDTH / 2) - sin_l, (WINDOW_HEIGHT / 2) + cos_l, quad_color);


        // Grid crosshairs
        if (show_grid_crosshairs) {
            draw_rect_rgb(WINDOW_WIDTH / 2, 0, 1, WINDOW_HEIGHT, C_WHITE);
            draw_rect_rgb(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, 1, C_WHITE);
        }

        // On-screen mouse coords
        if (show_mouse_coords) {
            char *coords_text;
            asprintf(&coords_text, "(%d, %d) %s", (int) grid_mouse_x, (int) grid_mouse_y, get_map_coords(grid_mouse_x, grid_mouse_y) ? "true" : "false");
            BF_DrawTextRgb(coords_text, mouse_x, mouse_y, 3, -1, C_RED, FALSE);
            free(coords_text);
        }
    } else if (view_mode == VIEW_TERMINAL) { // Terminal view

        // Output from previous command
        BF_DrawTextRgb(DT_console_text, 0, 0, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);
    
        // Prompt string
        BF_FillTextRgb(TERMINAL_PROMPT, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);

        // Input text
        BF_FillTextRgb(terminal_input->text, terminal_font_size, WINDOW_WIDTH, terminal_font_color, TRUE);
    }

    // FPS readout
    if (view_mode != VIEW_TERMINAL) {
        #define FPS_READOUT_SIZE 5
        draw_rect_a(0, 0, WINDOW_WIDTH / 2, FPS_READOUT_SIZE * BF_CHAR_WIDTH, 0, 0, 0, 127);
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

    // Sprites and sprite names
    for (int i = 0; i < num_sprites; i++) {
        SDL_DestroyTexture(sprites[i]);
        free(sprite_names[i]);
    }
    free(sprites);
    free(sprite_names);
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