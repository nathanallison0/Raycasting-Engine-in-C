#include <math.h>
#include <string.h>
#include "./constants.h"
#include <dirent.h>

// Extra SDL functions
#include "../../SDL/SDLStart.h"

// Grid encoding
#include "./grid.h"

// Font
#include "../BasicFont/BasicFont.h"

// Linked list generation
#include "../../linkedList.h"

// Textures
#include "textures.h"

void setup(void);
void process_input(void);
void update(void);
void render(void);
void free_memory(void);

float perc(int);
void calc_grid_cam_center(void);
void calc_grid_cam_zoom_p(void);
void reset_grid_cam(void);
void reset_player(void);
void zoom_grid_cam(int);
void zoom_grid_cam_center(int);
void g_draw_rect(int, int, int, int, unsigned char, unsigned char, unsigned char);
void g_draw_rect_rgb(int, int, int, int, rgb);
void g_draw_point(int, int, float, unsigned char, unsigned char, unsigned char);
void g_draw_point_rgb(int, int, float, rgb);
void draw_rect_bordered(int, int, int, int, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char, unsigned char);
void draw_rect_bordered_rgb(int, int, int, int, rgb, rgb);
void rotate_player(float);
void push_player_forward(int);
void push_player_right(int);
void g_draw_scale_point(int, int, int, unsigned char, unsigned char, unsigned char);
void g_draw_scale_point_rgb(int, int, int, rgb);

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

// Grid
#define GRID_LENGTH 25
#define GRID_HEIGHT 25
bool_cont *grid_enc;
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
int player_max_velocity = 350;
float player_angle;
float fov = M_PI / 3;
int player_movement_accel = 20;
int player_movement_decel = 20;
float player_angle_increment = M_PI / 18;
int player_rotation_speed = 10;

void assign_i_player_pos(void) {
    i_player_x = round(player_x);
    i_player_y = round(player_y);
}
// Player grid visual
int grid_player_pointer_dist = 15;
int grid_player_pointer_radius_offset = 50;
rgb grid_player_fill = {255, 50, 50};
char grid_follow_player = TRUE;
char show_player_vision = FALSE;
char show_player_trail = FALSE;
#define PLAYER_VISION_LINES FALSE

// Grid mobjs
int grid_mobj_radius = 7;

// First person rendering
rgb fp_bg_top = /*{255, 255, 230};*/ {137, 125, 116};
rgb fp_bg_bottom;
rgb wall_color = {150, 150, 150};
int fp_brightness = 100;
float fp_scale = 1 / 0.009417f;
int fp_render_distance = 2000;
char fp_show_walls = TRUE;
int pixel_fov_circumfrence;
float radians_per_pixel;
#define FLOOR_RES 10

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

rgb mobj_color = C_BLUE;
int num_mobjs = 0;
__linked_list_all_add__(
    mobj, float x; float y; int sprite_num,
    (float x, float y, char *sprite_name),
        item->x = x;
        item->y = y;
        item->sprite_num = sprite_not_found_num;
        for (int i = 0; i < num_sprites; i++) {
            if (strcmp(sprite_name, sprite_names[i]) == 0) {
                item->sprite_num = i;
                break;
            }
        }
        num_mobjs++
)

#define SPRITE_PATH "./sprites/"
SDL_Texture **sprites = NULL;
#define SPRITE_NOT_FOUND_NAME "sprite_not_found"

__linked_list_all__(
    sprite_proj, float dist; float angle; mobj *obj,
    (float dist, float angle, mobj* obj),
        item->dist = dist;
        item->angle = angle;
        item->obj = obj
)

// Debugging
#include "./debugging.h"


// Grid Graphics
void g_draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b) {
    draw_rect(
        roundf( (x - grid_cam_x) * grid_cam_zoom_p ),
        roundf( (y - grid_cam_y) * grid_cam_zoom_p ),
        ceilf(length * grid_cam_zoom_p),
        ceilf(width * grid_cam_zoom_p),
        r, g, b
    );
}

void g_draw_rect_rgb(int x, int y, int length, int width, rgb color) {
    g_draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void g_draw_point(int x, int y, float radius, unsigned char r, unsigned char g, unsigned char b) {
    draw_point(
        roundf((x - grid_cam_x) * grid_cam_zoom_p),
        roundf((y - grid_cam_y) * grid_cam_zoom_p),
        radius,
        r, g, b
    );
}

void g_draw_point_rgb(int x, int y, float radius, rgb color) {
    g_draw_point(x, y, radius, color.r, color.g, color.b);
}

void g_draw_scale_point(int x, int y, int radius, unsigned char r, unsigned char g, unsigned char b) {
    g_draw_point(x, y, round(radius * grid_cam_zoom_p), r, g, b);
}

void g_draw_scale_point_rgb(int x, int y, int radius, rgb color) {
    g_draw_scale_point(x, y, radius, color.r, color.g, color.b);
}

void g_draw_line(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b) {
    draw_line(
        roundf((x1 - grid_cam_x) * grid_cam_zoom_p),
        roundf((y1 - grid_cam_y) * grid_cam_zoom_p),
        roundf((x2 - grid_cam_x) * grid_cam_zoom_p),
        roundf((y2 - grid_cam_y) * grid_cam_zoom_p),
        r, g, b
    );
}

void g_draw_line_rgb(int x1, int y1, int x2, int y2, rgb color) {
    g_draw_line(x1, y1, x2, y2, color.r, color.g, color.b);
}

// Grid 
int get_grid_bool(int x, int y) {
    return bit_bool(
        grid_enc[((GRID_LENGTH * y) + x) / (SIZE_BOOL_CONT * 8)],
        ((GRID_LENGTH * y) + x) % (SIZE_BOOL_CONT * 8)
    );
}

int get_grid_bool_coords(int x, int y) {
    return get_grid_bool(x / GRID_SPACING, y / GRID_SPACING);
}

// Raycasting
typedef struct {
    float x;
    float y;
} xy;

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
void raycast_vars(float x, float y, float angle, float* vars) {
    float alpha;
    if (angle < M_PI_2) {
        alpha = angle;
        c_hy = ((ceil(y / GRID_SPACING)) * GRID_SPACING);// - 1;
        c_hx = ((c_hy - y) / tan(alpha)) + x;
        d_hx = GRID_SPACING / tan(alpha);
        d_hy = GRID_SPACING;
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING) + GRID_SPACING;
        c_vy = (tan(alpha) * (c_vx - x)) + y;
        d_vx = GRID_SPACING;
        d_vy = tan(alpha) * (d_vx);
        
    } else if (angle < M_PI) {
        alpha = angle - (M_PI_2);
        c_hy = (ceil(y / GRID_SPACING) * GRID_SPACING);// - 1;
        c_hx = tan(alpha) * (y - c_hy) + x;
        d_hy = GRID_SPACING;
        d_hx = -GRID_SPACING * tan(alpha);
        c_vx = (floor(x / GRID_SPACING) * GRID_SPACING);// - 1;
        c_vy = y + ((x - c_vx) / tan(alpha));
        d_vx = -GRID_SPACING;
        d_vy = GRID_SPACING / tan(alpha);
    } else if (angle < M_PI + (M_PI_2)) {
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
}

xy raycast(float x, float y, float angle, char *horiz_hit) {
    float vars[12];
    raycast_vars(x, y, angle, vars);

    while (!(
        !(
            ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < GRID_LENGTH ) &&
            ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < GRID_HEIGHT )
            
        ) || (
            ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) ) ||
            ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) )
        ))
    ) {
        c_hx += d_hx; c_hy += d_hy;
        c_epsilon_h += d_epsilon_h;
        #if PLAYER_VISION_LINES
        if (show_player_vision)
            temp_dgp(c_hx, c_hy, DG_BLUE);
        #endif
    }

    while (!(
        !(
            ( 0 < (c_vx / GRID_SPACING) && (c_vx / GRID_SPACING) < GRID_LENGTH ) &&
            ( 0 < (c_vy / GRID_SPACING) && (c_vy / GRID_SPACING) < GRID_HEIGHT )
        ) || (
            ( get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING)) ||
            ( get_grid_bool(c_vx / GRID_SPACING, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool(c_vx / GRID_SPACING, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) )
        ))
    ) {
        c_vx += d_vx; c_vy += d_vy;
        c_epsilon_v += d_epsilon_v;
        #if PLAYER_VISION_LINES
        if (show_player_vision)
            temp_dgp(c_vx, c_vy, DG_BLUE);
        #endif
    }

    if (point_dist(c_hx, c_hy, x, y) < point_dist(c_vx, c_vy, x, y)) {
        if (show_player_vision)
            temp_dgp(c_hx, c_hy, DG_WHITE);
        *horiz_hit = TRUE;
        return (xy) {c_hx, c_hy};
    } else {
        if (show_player_vision)
            temp_dgp(c_vx, c_vy, DG_WHITE);
        *horiz_hit = FALSE;
        return (xy) {c_vx, c_vy};
    }
}

int pointfind_reverse_raycast(float hit_x, float hit_y, float point_x, float point_y, float angle) {
    float vars[12];
    raycast_vars(hit_x, hit_y, angle, vars);

    int stop_h = (((int) point_y / GRID_SPACING) + (hit_y < point_y)) * GRID_SPACING;
    int stop_v = (((int) point_x / GRID_SPACING) + (hit_x < point_x)) * GRID_SPACING;

    char successes = 0;
    
    c_hx += d_hx; c_hy += d_hy;
    c_epsilon_h += d_epsilon_h;

    while (
        ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) ) &&
        ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) )
    ) {
        if (!(
            (
                ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < GRID_LENGTH ) &&
                ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < GRID_HEIGHT )
            ) && (
                (hit_y < point_y && c_hy < stop_h) || 
                (hit_y >= point_y && c_hy > stop_h)
            )
        )) {
            successes++;
            break;
        }
        c_hx += d_hx; c_hy += d_hy;
        c_epsilon_h += d_epsilon_h;
    }
   
    /* do {
        c_hx += d_hx; c_hy += d_hy;
        c_epsilon_h += d_epsilon_h;
        if (!(
            (
                ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < GRID_LENGTH ) &&
                ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < GRID_HEIGHT )
            ) && (
                (hit_y < point_y && c_hy < stop_h) || 
                (hit_y >= point_y && c_hy > stop_h)
            )
        )) {
            successes++;
            break;
        }
    } while (
        ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) ) &&
        ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) )
    ); */

    char prev_success;
    if (debug_prr) {
        if (successes == 1) {
            temp_dgp(c_hx, c_hy, DG_BLUE); // Horizontal success
        } else {
            temp_dgp(c_hx, c_hy, DG_RED); // No horizontal success
            add_dgl(player_x, player_y, c_hx, c_hy, DG_RED);
        }
        prev_success = successes;
    }

    c_vx += d_vx; c_vy += d_vy;
    c_epsilon_v += d_epsilon_v;

    while (
        ( get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING)) &&
        ( get_grid_bool(c_vx / GRID_SPACING, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool(c_vx / GRID_SPACING, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) )
    ) {
        if (!(
            (
                ( 0 < (c_vx / GRID_SPACING) && (c_vx / GRID_SPACING) < GRID_LENGTH ) &&
                ( 0 < (c_vy / GRID_SPACING) && (c_vy / GRID_SPACING) < GRID_HEIGHT )
            ) && (
                (hit_x < point_x && c_vx < stop_v) ||
                (hit_x >= point_x && c_vx > stop_v)
            )
        )) {
            successes++;
            break;
        }
        c_vx += d_vx; c_vy += d_vy;
        c_epsilon_v += d_epsilon_v;
    }
   
    /* do {
        c_vx += d_vx; c_vy += d_vy;
        c_epsilon_v += d_epsilon_v;
        if (!(
            (
                ( 0 < (c_vx / GRID_SPACING) && (c_vx / GRID_SPACING) < GRID_LENGTH ) &&
                ( 0 < (c_vy / GRID_SPACING) && (c_vy / GRID_SPACING) < GRID_HEIGHT )
            ) && (
                (hit_x < point_x && c_vx < stop_v) ||
                (hit_x >= point_x && c_vx > stop_v)
            )
        )) {
            successes++;
            break;
        }
    } while (
        ( get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING)) &&
        ( get_grid_bool(c_vx / GRID_SPACING, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool(c_vx / GRID_SPACING, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) )
    ); */

    if (debug_prr) {
        if (successes == prev_success) {
            temp_dgp(c_vx, c_vy, DG_RED); // No vertical success
            add_dgl(player_x, player_y, c_vx, c_vy, DG_RED);
        } else {
            temp_dgp(c_vx, c_vy, DG_PURPLE); // Vertical success
        }
    }

    return successes == 2;
}

int project(float distance) {
    return WINDOW_HEIGHT / (distance / fp_scale);
}

unsigned char shade(unsigned char color, float distance) {
    return color - ((distance / fp_render_distance) * fp_brightness);
}

rgb shade_rgb(rgb color, float distance) {
    return brighten(color, -(distance / fp_render_distance) * fp_brightness);
}

float perc(int percent) {
    return (percent / 100.0f);
}

void calc_grid_cam_zoom_p(void) {
    grid_cam_zoom_p = perc(grid_cam_zoom);
}

void calc_grid_cam_center(void) {
    grid_cam_center_x = grid_cam_x + round( (WINDOW_WIDTH / 2) / grid_cam_zoom_p );
    grid_cam_center_y = grid_cam_y + round( (WINDOW_HEIGHT / 2) / grid_cam_zoom_p );
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

void push_player_forward(int force) {
    player_x_velocity += cos(player_angle) * force;
    player_y_velocity += sin(player_angle) * force;
}

void push_player_right(int force) {
    player_angle += M_PI_2;
    push_player_forward(force);
    player_angle -= M_PI_2;
}
/*
//   1 2 3 4 5 6 7 8 9 10111213141516171819202122232425
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 1
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 2
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 3
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1}, // 4
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 5
    {1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 6
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 7
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 8
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 9
    {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1}, // 10
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 11
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 12
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 13
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
*/
const Uint8 horiz_textures[GRID_HEIGHT + 1][GRID_LENGTH] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 1
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0}, // 2
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 3
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 4
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 5
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 6
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 7
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

const Uint8 vertical_textures[GRID_HEIGHT][GRID_LENGTH + 1] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 1
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 2
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 3
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 4
    {0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 5
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, // 6
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

const Uint8 *state;
void setup(void) {
    // Intiailize bit representation
    bit_rep_init();

    // Initialize keyboard state
    state = SDL_GetKeyboardState(NULL);

    // Set fps floor gradient start color based on floor color of grid
    fp_bg_bottom = grid_fill_nonsolid;

    // Calculate 360 degrees in screen pixels
    pixel_fov_circumfrence = (WINDOW_WIDTH / fov) * (M_PI * 2);
    radians_per_pixel = fov / WINDOW_WIDTH;

    grid_enc = (bool_cont *) malloc(ceil( (float) ((GRID_LENGTH * GRID_HEIGHT) / SIZE_BOOL_CONT) ));

    int new_grid[GRID_HEIGHT][GRID_LENGTH] = {
        /*
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,0,0,0,0,1,0,1,0,0,1,0,0,1},
        {1,1,1,0,0,0,0,1,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,1,1,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,0,0,0,0,0,0,1,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,1},
        {1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        */
        /*
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
        */
        
    //   1 2 3 4 5 6 7 8 9 10111213141516171819202122232425
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, // 1
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 2
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 3
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1}, // 4
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 5
        {1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 6
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 7
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 8
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 9
        {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1}, // 10
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 11
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 12
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 13
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
       
        /*
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0}
        */
    };

    // Write grid to bit representation
    for (int row = 0; row < GRID_HEIGHT; row++) {
        for (int col = 0; col < GRID_LENGTH; col++) {
            bit_assign(
                &grid_enc[((GRID_LENGTH * row) + col) / (SIZE_BOOL_CONT * 8)],
                ((GRID_LENGTH * row) + col) % (SIZE_BOOL_CONT * 8),
                new_grid[row][col]
            );
        }
    }

    // Set player start pos
    reset_player();
    reset_grid_cam();
    grid_cam_zoom_p = perc(grid_cam_zoom);
    calc_grid_cam_center();

    // Load sprite images
    DIR *sprite_dir = opendir(SPRITE_PATH);
    if (sprite_dir) {
        struct dirent *entry;
        SDL_Texture *new_sprite;
        while ((entry = readdir(sprite_dir))) {
            if (entry->d_type == DT_REG) {
                // Create full relative path to sprite file
                char *full_path;
                asprintf(&full_path, SPRITE_PATH "%s", entry->d_name);

                // Get sprite
                new_sprite = IMG_LoadTexture(renderer, full_path);
                free(full_path);

                if (new_sprite) {
                    // Allocate space for texture and name
                    sprites = realloc(sprites, ++num_sprites * sizeof(SDL_Texture *));
                    sprite_names = realloc(sprite_names, num_sprites * sizeof(char *));

                    // Add texture to list
                    sprites[num_sprites - 1] = new_sprite;

                    // Add name to list
                    // Allocate strlen - 3 because the .png should be cut off but leave room for terminating character
                    int new_name_size = strlen(entry->d_name) - 3;
                    sprite_names[num_sprites - 1] = malloc(new_name_size);
                    strlcpy(sprite_names[num_sprites - 1], entry->d_name, new_name_size);

                    // Set sprite not found index if not already set
                    if (sprite_not_found_num == -1 && strcmp(sprite_names[num_sprites - 1], SPRITE_NOT_FOUND_NAME) == 0)
                        sprite_not_found_num = num_sprites - 1;
                } else
                    printf("Could not load sprite %s\n", entry->d_name);
            }
        }
        closedir(sprite_dir);
    } else {
        printf("Could not open sprite directory\n");
    }

    // Debug mobjs
    #define mobj(x, y, name) mobj_create(x * GRID_SPACING, y * GRID_SPACING, name)
    
    // Army of MJs
    for (float x = 14.9; x < 24; x += 2) {
        for (int y = 11; y < 21; y += 2) {
            mobj(x, y, "mj1");
        }
    }
    
    // Debug mobjs against starting wall
    for (int x = 6; x < 15; x += 2) {
        mobj(x, 1.1, "1");
    }
    
    // At corner in starting area
    //mobj(12, 3.9, "1");

    // Against end wall
    for (int y = 4; y < 10; y += 2) {
        mobj(19.9, y, "2");
    }

    // Other starting wall
    for (float x = 7.1; x < 12; x += 2) {
        mobj(x, 4.9, "1");
    }

    // Around first corner
    mobj(7, 12.9, "3");
}

Uint8 prev_state[SDL_NUM_SCANCODES];
int key_just_pressed(int scancode) {
    return state[scancode] && !prev_state[scancode];
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type) {
        case SDL_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_MOUSEBUTTONUP:
            if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = FALSE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (event.button.button == SDL_BUTTON_LEFT) left_mouse_down = TRUE;
            break;
        case SDL_MOUSEWHEEL:
            zoom_grid_cam_center(-event.wheel.y);
            break;
        case SDL_MOUSEMOTION:
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

        if (key_just_pressed(SDL_SCANCODE_2)) set_view(VIEW_GRID);
        if (key_just_pressed(SDL_SCANCODE_1)) set_view(VIEW_FPS);

        if (key_just_pressed(SDL_SCANCODE_C)) toggle(&grid_follow_player);

        // if (state[SDL_SCANCODE_UP]) fp_scale *= 1.01f;
        // if (state[SDL_SCANCODE_DOWN]) fp_scale /= 1.01f;

    } else if (view_mode == VIEW_TERMINAL && event.type == SDL_KEYDOWN) {
        char key = event.key.keysym.sym;

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

void update(void) {
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - last_frame_time);

    // Only delay if we are too fast to update this frame
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
        SDL_Delay(time_to_wait);
    }
    // Get a delta time factor converted to seconds to be used to update my objects
    float delta_time = (SDL_GetTicks() - last_frame_time) / 1000.0f;

    last_frame_time = SDL_GetTicks();

    static float prev_player_x;
    static float prev_player_y;

    // Rotate and apply force to player by input
    push_player_forward(player_movement_accel * vertical_input);
    push_player_right(player_movement_accel * horizontal_input);
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
            player_x_velocity -= cos(player_velocity_angle) * player_movement_decel;
            if (vel_is_pos && player_x_velocity < 0)
                player_x_velocity = 0;
            else if (!vel_is_pos && player_x_velocity > 0)
                player_x_velocity = 0;
        }
        if (player_y_velocity != 0) {
            int vel_is_pos = player_y_velocity > 0;
            player_y_velocity -= sin(player_velocity_angle) * player_movement_decel;
            if (vel_is_pos && player_y_velocity < 0)
                player_y_velocity = 0;
            else if (!vel_is_pos && player_y_velocity > 0)
                player_y_velocity = 0;
        }
    }

    // Wall collisions
    int northeast_collision = get_grid_bool_coords(player_x + player_radius, player_y - player_radius);
    int southeast_collision = get_grid_bool_coords(player_x + player_radius, player_y + player_radius);
    int southwest_collision = get_grid_bool_coords(player_x - player_radius, player_y + player_radius);
    int northwest_collision = get_grid_bool_coords(player_x - player_radius, player_y - player_radius);

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

    //printf("player velocity: (%f, %f), angle: (%f deg)\n", player_x_velocity, player_y_velocity, rad_deg(player_angle));

    prev_player_x = player_x;
    prev_player_y = player_y;

    // Move player by velocity
    player_x += player_x_velocity * delta_time;
    player_y += player_y_velocity * delta_time;
    assign_i_player_pos();

    if (show_player_trail) fill_dgp(player_x, player_y, DG_YELLOW);

    if (grid_follow_player) {
        grid_cam_x = round( player_x - ((WINDOW_WIDTH / 2) / grid_cam_zoom_p) );
        grid_cam_y = round( player_y - ((WINDOW_HEIGHT / 2) / grid_cam_zoom_p) );
    }

    
    //printf("player pos: (%f, %f) facing %f (%f deg)\n", player_x, player_y, player_angle, rad_deg(player_angle));
    //printf("player velocity: (%f, %f)\n", player_x_velocity, player_y_velocity);
    

    vertical_input = 0;
    horizontal_input = 0;
    rotation_input = 0;
}

void render(void) {
    // Set background
    if      (view_mode == VIEW_FPS)      set_draw_color_rgb(C_YELLOW);
    else if (view_mode == VIEW_GRID)     set_draw_color_rgb(grid_bg);
    else if (view_mode == VIEW_TERMINAL) set_draw_color_rgb(terminal_bg);
    SDL_RenderClear(renderer);

    if ((view_mode == VIEW_FPS && fp_show_walls) || show_player_vision || grid_casting) { // Raycasting
        float ray_dists[WINDOW_WIDTH];

        // Raycast and draw walls
        for (int ray_i = 0; ray_i < WINDOW_WIDTH; ray_i++) {
            // Calculate ray angle
            float relative_ray_angle = (radians_per_pixel * ray_i) - (fov / 2);
            float ray_angle = player_angle + relative_ray_angle; 
            if (ray_angle < 0) ray_angle += M_PI * 2;
            else if (ray_angle >= M_PI * 2) ray_angle -= M_PI * 2;

            char horiz_hit;
            xy hit = raycast(player_x, player_y, ray_angle, &horiz_hit);
            Uint8 texture_index;
            if (horiz_hit) {
                texture_index = horiz_textures[(int) hit.y / GRID_SPACING][(int) hit.x / GRID_SPACING];
            } else {
                texture_index = vertical_textures[(int) hit.y / GRID_SPACING][(int) hit.x / GRID_SPACING];
            }
            char draw_reverse = (ray_angle < M_PI && horiz_hit) || (M_PI_2 < ray_angle && ray_angle < M_PI + M_PI_2 && !horiz_hit);

            if ((view_mode == VIEW_FPS && fp_show_walls) || grid_casting) {
                // Get hit distance
                ray_dists[ray_i] = fp_dist(hit.x, hit.y, ray_angle);

                if (view_mode == VIEW_FPS && fp_show_walls) {
                    int wall_height = project(ray_dists[ray_i]);

                    // Draw texture column
                    float relative_wall_x = fmodf(horiz_hit ? hit.x : hit.y, GRID_SPACING);
                    int texture_x = (relative_wall_x / GRID_SPACING) * TEXTURE_WIDTH;
                    texture_x = draw_reverse ? TEXTURE_WIDTH - texture_x - 1 : texture_x;
                    float texture_row_height = (float) wall_height / TEXTURE_WIDTH;
                    float row_y = (WINDOW_HEIGHT - wall_height) / 2.0f;
                    int end = roundf(row_y) - 1;
                    for (int texture_y = 0; texture_y < TEXTURE_WIDTH; texture_y++) {
                        int start = end;
                        row_y += texture_row_height;
                        end = roundf(row_y);
                        draw_rect_rgb(ray_i, start + 1, 1, end - start, shade_rgb(textures[texture_index][texture_y][texture_x], ray_dists[ray_i]));
                    }

                    // Draw floor and ceiling tile pixels below wall
                    int end_floor = ceilf(((float) WINDOW_HEIGHT - wall_height) / 2);
                    for (int pixel_y = 0; pixel_y < end_floor; pixel_y += FLOOR_RES) {
                        // Player height reference
                        float dist_to_point = ((float) (WINDOW_HEIGHT / 2) / ((WINDOW_HEIGHT / 2) - (pixel_y + (FLOOR_RES / 2)))) * fp_scale;
                        float proj_dist_to_point = dist_to_point / cosf(relative_ray_angle);
                        
                        float point_x = (cosf(ray_angle) * proj_dist_to_point) + player_x;
                        float point_y = (sinf(ray_angle) * proj_dist_to_point) + player_y;

                        int texture_x = fmodf(point_x, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);
                        int texture_y = fmodf(point_y, GRID_SPACING) * ((float) TEXTURE_WIDTH / GRID_SPACING);

                        int height = pixel_y + FLOOR_RES > end_floor ? end_floor - pixel_y : FLOOR_RES;
                        rgb shaded_color = shade_rgb(textures[2][texture_y][texture_x], proj_dist_to_point);
                        draw_rect_rgb(ray_i, pixel_y, 1, height, shaded_color);
                        draw_rect_rgb(ray_i, WINDOW_HEIGHT - pixel_y - height, 1, height, shaded_color);
                    }
                }
            }
        }

        if (view_mode == VIEW_FPS || grid_casting) {
            // Store positions and distances of sprites on screen
            for (mobj *o = mobj_head; o; o = o->next) {
                // Get angle relative to left edge of player vision
                float angle = atan2f(o->y - player_y, o->x - player_x);
                if (angle < 0) angle += (M_PI * 2);

                float mobj_distance = fp_dist(o->x, o->y, angle);

                // Store, sorted farthest to closest, for rendering
                sprite_proj *new_proj = sprite_proj_create(mobj_distance, angle, o);
                
                if (sprite_proj_head) {
                    if (sprite_proj_head->dist < mobj_distance) {
                        new_proj->next = sprite_proj_head;
                        sprite_proj_head = new_proj;
                    } else {
                        // Find closest greater sprite
                        sprite_proj *greater = sprite_proj_head;
                        for (;greater->next && greater->next->dist > mobj_distance; greater = greater->next);

                        // Insert this sprite after the lesser sprite
                        sprite_proj *lesser = greater->next;
                        greater->next = new_proj;
                        new_proj->next = lesser;
                    }
                } else {
                    sprite_proj_head = new_proj;
                }
            }

            // Render sprites
            for (sprite_proj *sprite = sprite_proj_head; sprite; sprite = sprite->next) {
                int sprite_height = project(sprite->dist);

                // Scale sprite width using height
                int image_width, image_height;
                SDL_QueryTexture(sprites[sprite->obj->sprite_num], NULL, NULL, &image_width, &image_height);
                int sprite_width = ((float) sprite_height / image_height) * image_width;
          
                // Calculate screen x pos
                float relative_angle = sprite->angle - (player_angle - (fov / 2));
                if (relative_angle > M_PI * 2) relative_angle -= M_PI * 2;
                else if (relative_angle < 0) relative_angle += M_PI * 2;
                int screen_x = (WINDOW_WIDTH / fov) * relative_angle;

                // End if sprite would draw completely off screen
                if (screen_x > WINDOW_WIDTH + (sprite_width / 2) && screen_x < pixel_fov_circumfrence - (sprite_width / 2))
                    continue;

                // Shade sprite by distance
                unsigned char sprite_shading = shade(255, sprite->dist);
                SDL_SetTextureColorMod(sprites[sprite->obj->sprite_num], sprite_shading, sprite_shading, sprite_shading);

                // Get start and end of drawing and how much was skipped
                int start_x = roundf(screen_x - (sprite_width / 2));
                if (start_x > pixel_fov_circumfrence - sprite_width)
                    start_x -= pixel_fov_circumfrence;
                
                int end_x = start_x + sprite_width;
                int skipped = start_x;
                start_x = max(start_x, 0);
                skipped = start_x - skipped;
                
                // Draw columns to scale the sprite by distance
                SDL_Rect dest = {start_x, (WINDOW_HEIGHT / 2) - (sprite_height / 2), 1, sprite_height};
                float texture_incr = (float) image_width / sprite_width;
                float texture_col = skipped * texture_incr;
                SDL_Rect source = {0, 0, ceilf(texture_incr), image_height};

                while (dest.x < end_x && dest.x < WINDOW_WIDTH) {
                    // Only draw column if it is in front of its corresponding wall
                    if (ray_dists[dest.x] > sprite->dist) {
                        // float angle_to_point = ((dest.x * radians_per_pixel) - (fov / 2)) + player_angle;
                        // float rel_angle_to = angle_to_point - sprite->angle;
                        // float dist_from_center = sprite->dist * tanf(rel_angle_to);

                        // float sprite_part_x = sprite->obj->x - (dist_from_center * sinf(player_angle));
                        // float sprite_part_y = sprite->obj->y + (dist_from_center * cosf(player_angle));

                        source.x = texture_col;
                        if (view_mode == VIEW_FPS) {
                            SDL_RenderCopy(renderer, sprites[sprite->obj->sprite_num], &source, &dest);
                        }
                    }
                    dest.x++;
                    texture_col += texture_incr;
                }
            }

            sprite_proj_destroy_all();
        }
    }

    if (view_mode == VIEW_GRID) { // Grid rendering
        // Grid box fill
        for (int row = 0; row < GRID_HEIGHT; row++) {
            for (int col = 0; col < GRID_LENGTH; col++) {
                g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING,
                get_grid_bool(col, row) ? grid_fill_solid : grid_fill_nonsolid);
            }
        }

        if (show_grid_lines) { // Grid lines
            // Vertical
            for (int i = 0; i < GRID_LENGTH + 1; i++) {
                g_draw_rect_rgb(
                    (i * GRID_SPACING) - ceil(grid_line_width / 2), 
                    0,
                    grid_line_width,
                    GRID_HEIGHT * GRID_SPACING,
                    grid_line_fill
                );
            }
            // Horizontal
            for (int i = 0; i < GRID_LENGTH + 1; i++) {
                g_draw_rect_rgb(
                    0,
                    (i * GRID_SPACING) - ceil(grid_line_width / 2),
                    GRID_LENGTH * GRID_SPACING,
                    grid_line_width,
                    grid_line_fill
                );
            }
        }

        // Map objects
        for (mobj *temp = mobj_head; temp; temp = temp->next) {
            g_draw_scale_point_rgb(temp->x, temp->y, grid_mobj_radius, mobj_color);
        }

        // Fill debug grid points
        for (struct fill_dgp *p_i = fill_dgp_head; p_i; p_i = p_i->next) {
            g_draw_point_rgb(p_i->x, p_i->y, dgp_radius, DG_COLORS[p_i->color_index]);
        }

        // Player direction pointer
        g_draw_scale_point_rgb(
            i_player_x + round(cos(player_angle) * grid_player_pointer_dist),
            i_player_y + round(sin(player_angle) * grid_player_pointer_dist),
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
            asprintf(&coords_text, "(%d, %d) %s", (int) grid_mouse_x, (int) grid_mouse_y, get_grid_bool_coords(grid_mouse_x, grid_mouse_y) ? "true" : "false");
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

    // Clear dg things
    num_temp_dgps = 0;
    num_dgls = 0;

    SDL_RenderPresent(renderer);
}

void free_memory(void) {
    // Grid
    free(grid_enc);

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

    setup();
    debugging_start();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    debugging_end();
    destroy_window();
    free_memory();

    return 0;
}