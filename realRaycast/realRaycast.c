#include <math.h>
#include <string.h>
#include "./constants.h"
#include <dirent.h>

// Extra SDL functions
#include "../SDLStart.h"

// Grid encoding
#include "./grid.h"

// Font
#include "../BasicFont/BasicFont.h"

// Linked list generation
#include "../../linkedList.h"

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
#define g_draw_scale_point(x, y, radius, r, g, b) g_draw_point(x, y, round(radius * grid_cam_zoom_p), r, g, b)
#define g_draw_scale_point_rgb(x, y, radius, color) g_draw_scale_point(x, y, radius, color.r, color.g, color.b)

#define rad_deg(radians) radians * (180 / M_PI)
#define deg_rad(degrees) degrees * (M_PI / 180)

enum VIEW_MODE {
    VIEW_GRID,
    VIEW_FPS,
    VIEW_TERMINAL
} view_mode, prev_view_mode;

void set_view(enum VIEW_MODE mode) { prev_view_mode = view_mode; view_mode = mode; }

// Grid
int grid_length;
int grid_height;
bool_cont* grid_enc;
// Grid visual
rgb grid_bg = {255, 0, 255};
rgb grid_fill_nonsolid = {235, 235, 235};
// blue {105, 165, 255}
rgb grid_fill_solid = {100, 110, 100};
int grid_line_width = 1;
rgb grid_line_fill = C_BLACK;
int show_grid_lines = FALSE;
// Grid Visual Camera
int grid_cam_x;
int grid_cam_y;
int grid_cam_zoom;
float grid_cam_zoom_p;
int grid_cam_zoom_min;
int grid_cam_zoom_max;
int grid_cam_center_x;
int grid_cam_center_y;
// Player
float player_x;
int i_player_x;
float player_y;
int i_player_y;
int player_radius = 10;
float player_x_velocity;
float player_y_velocity;
float player_velocity_angle;
int player_max_velocity = 300;
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
int grid_follow_player = TRUE;
int show_player_vision = FALSE;
int show_player_trail = FALSE;

// Grid mobjs
int grid_mobj_radius = 7;

// First person rendering
rgb fp_bg_top = {255, 255, 230}; //{137, 125, 116}
rgb fp_bg_top_goal;
rgb fp_bg_bottom;
rgb fp_bg_bottom_goal;
rgb wall_color = {150, 150, 150};
int fp_brightness = 100;
float fp_scale = 0.02f;
int fp_render_distance = 2000;
int fp_render_distance_scr = (WINDOW_HEIGHT / 2) + 250;
int fp_show_walls = TRUE;

// User Input Variables
int shift = FALSE;
int left_mouse_down = FALSE;
int prev_mouse_x = -1;
int prev_mouse_y = -1;
char horizontal_input;
char vertical_input;
char rotation_input = 0;

// Mobjs and sprites
int sprite_not_found_num = -1;
int num_sprites = 0;
char** sprite_names = NULL;

rgb mobj_color = C_BLUE;
int num_mobjs = 0;
__linked_list_all_add__(
    mobj, float x; float y; int sprite_num,
    (float x, float y, char* sprite_name),
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
SDL_Texture** sprites = NULL;
#define SPRITE_NOT_FOUND_NAME "sprite_not_found"

__linked_list_all__(
    sprite_proj, int x; float dist; int sprite_num,
    (int x, float dist, int sprite_num),
        item->x = x;
        item->dist = dist;
        item->sprite_num = sprite_num
)

// Debugging
#include "./debugging.h"


// Grid Graphics
void g_draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b) {
    int real_length = ceil(length * grid_cam_zoom_p);
    int real_width = ceil(width * grid_cam_zoom_p);
    draw_rect(
        round( (x - grid_cam_x) * grid_cam_zoom_p ),
        round( (y - grid_cam_y) * grid_cam_zoom_p ),
        real_length,
        real_width,
        r, g, b
    );
}

void g_draw_rect_rgb(int x, int y, int length, int width, rgb color) {
    g_draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void g_draw_point(int x, int y, float radius, unsigned char r, unsigned char g, unsigned char b) {
    draw_point(
        round((x - grid_cam_x) * grid_cam_zoom_p),
        round((y - grid_cam_y) * grid_cam_zoom_p),
        radius,
        r, g, b
    );
}

void g_draw_point_rgb(int x, int y, float radius, rgb color) {
    g_draw_point(x, y, radius, color.r, color.g, color.b);
}

// Grid 
int get_grid_bool(int x, int y) {
    return bit_bool(
        grid_enc[((grid_length * y) + x) / (SIZE_BOOL_CONT * 8)],
        ((grid_length * y) + x) % (SIZE_BOOL_CONT * 8)
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
    return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
}

xy raycast(float x, float y, float angle) {
    float c_hx, c_hy;
    float d_hx, d_hy;
    float c_vx, c_vy;
    float d_vx, d_vy;
    float c_epsilon_h, d_epsilon_h;
    float c_epsilon_v, d_epsilon_v;
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
        c_epsilon_h = sqrt(pow(GRID_SPACING - ((int) x % GRID_SPACING), 2) + pow(GRID_SPACING - ((int) y % GRID_SPACING), 2));
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

    float diagonal_dist = fabs((M_PI_4) - alpha) / (M_PI);
    c_epsilon_h = (sqrt(pow(c_hx - x, 2) + pow(c_hy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_h = (sqrt(pow(d_hx, 2) + pow(d_hy, 2)) / GRID_SPACING) * diagonal_dist;
    c_epsilon_v = (sqrt(pow(c_vx - x, 2) + pow(c_vy - y, 2)) / GRID_SPACING) * diagonal_dist;
    d_epsilon_v = (sqrt(pow(d_vx, 2) + pow(d_vy, 2)) / GRID_SPACING) * diagonal_dist;

    while (!(
        !(
            ( 0 < (c_hx / GRID_SPACING) && (c_hx / GRID_SPACING) < grid_length ) &&
            ( 0 < (c_hy / GRID_SPACING) && (c_hy / GRID_SPACING) < grid_height )
            
        ) || (
            ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, (c_hy / GRID_SPACING) - 1) ) ||
            ( get_grid_bool((c_hx - (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) || get_grid_bool((c_hx + (c_epsilon_h / 2)) / GRID_SPACING, c_hy / GRID_SPACING) )
        ))
    ) {
        c_hx += d_hx; c_hy += d_hy;
        c_epsilon_h += d_epsilon_h;
    }

    while (!(
        !(
            ( 0 < (c_vx / GRID_SPACING) && (c_vx / GRID_SPACING) < grid_length ) &&
            ( 0 < (c_vy / GRID_SPACING) && (c_vy / GRID_SPACING) < grid_height )
        ) || (
            ( get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool((c_vx / GRID_SPACING) - 1, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING)) ||
            ( get_grid_bool(c_vx / GRID_SPACING, (c_vy - (c_epsilon_v / 2)) / GRID_SPACING) || get_grid_bool(c_vx / GRID_SPACING, (c_vy + (c_epsilon_v / 2)) / GRID_SPACING) )
        ))
    ) {
        c_vx += d_vx; c_vy += d_vy;
        c_epsilon_v += d_epsilon_v;
    }

    if (point_dist(c_hx, c_hy, x, y) < point_dist(c_vx, c_vy, x, y)) {
        return (xy) {c_hx, c_hy};
    } else {
        return (xy) {c_vx, c_vy};
    }
}

int project(float distance) {
    return (1.0f / (distance * fp_scale)) * WINDOW_HEIGHT;
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
    grid_cam_center_x = grid_cam_x + round( (WINDOW_WIDTH / 2) * (1.0f / grid_cam_zoom_p) );
    grid_cam_center_y = grid_cam_y + round( (WINDOW_HEIGHT / 2) * (1.0f / grid_cam_zoom_p) );
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
    grid_cam_x = old_grid_cam_center_x - round( (WINDOW_WIDTH / 2) * (1.0f / grid_cam_zoom_p) );
    grid_cam_y = old_grid_cam_center_y - round( (WINDOW_HEIGHT / 2) * (1.0f / grid_cam_zoom_p) );
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

const Uint8 *state;
void setup(void) {
    // Intiailize bit representation
    bit_rep_init();

    // Initialize keyboard state
    state = SDL_GetKeyboardState(NULL);

    // Set fps floor gradient start color based on floor color of grid
    fp_bg_bottom = grid_fill_nonsolid;

    // Calculate first person floor and ceiling gradient ending colors
    fp_bg_bottom_goal = get_gradient_color_dist(fp_bg_bottom, C_BLACK, fp_render_distance_scr, WINDOW_HEIGHT / 2);
    fp_bg_top_goal = get_gradient_color_dist(fp_bg_top, C_BLACK, fp_render_distance_scr, WINDOW_HEIGHT / 2);

    // Initialize grid
    grid_length = 25;
    grid_height = 25;

    grid_enc = (bool_cont *) malloc(ceil( (float) ((grid_length * grid_height) / SIZE_BOOL_CONT) ));

    int new_grid[25][25] = {
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
        {1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1}, // 9
        {1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1}, // 10
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 11
        {1,0,0,1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 12
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 13
        {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 14
        {1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,1}, // 15
        {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 16
        {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 17
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 18
        {1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 19
        {1,0,0,0,0,0,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 20
        {1,1,1,1,1,1,1,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1}, // 21
        {1,0,0,0,0,0,1,0,0,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1}, // 22
        {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1}, // 23
        {1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,1}, // 24
        {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}  // 25
    };

    // Write grid to bit representation
    for (int row = 0; row < grid_height; row++) {
        for (int col = 0; col < grid_length; col++) {
            bit_assign(
                &grid_enc[((grid_length * row) + col) / (SIZE_BOOL_CONT * 8)],
                ((grid_length * row) + col) % (SIZE_BOOL_CONT * 8),
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
    DIR* sprite_dir = opendir(SPRITE_PATH);
    if (sprite_dir) {
        struct dirent* entry;
        SDL_Texture* new_sprite;
        while ((entry = readdir(sprite_dir))) {
            if (entry->d_type == DT_REG) {
                // Create full relative path to sprite file
                char* full_path;
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
                    size_t new_name_size = strlen(entry->d_name) - 3;
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
    } else
        printf("Could not open sprite directory\n");

    // Debug mobjs
    #define new_mobj(x, y, name) mobj_create(x * GRID_SPACING, y * GRID_SPACING, name)
    new_mobj(12, 2.5, "person");
    new_mobj(12, 3,   "person");
    new_mobj(12, 3.5, "person");
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
            if (prev_mouse_x != -1 && prev_mouse_y != -1 && left_mouse_down) {
                grid_cam_x -= round( (event.motion.x - prev_mouse_x) * (1.0f / grid_cam_zoom_p) );
                grid_cam_y -= round( (event.motion.y - prev_mouse_y) * (1.0f / grid_cam_zoom_p) );
            }
            prev_mouse_x = event.motion.x;
            prev_mouse_y = event.motion.y;
    }

    shift = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];

    if (state[SDL_SCANCODE_ESCAPE]) game_is_running = FALSE;

    if (key_just_pressed(SDL_SCANCODE_SLASH)) {
        if (view_mode == VIEW_TERMINAL) set_view(prev_view_mode);
        else set_view(VIEW_TERMINAL);
    }

    if (view_mode == VIEW_FPS || view_mode == VIEW_GRID) {
        if (state[SDL_SCANCODE_RETURN] && !debug_menu_was_open) open_debug_menu = TRUE;

        if (state[SDL_SCANCODE_RIGHT]) rotation_input++;
        if (state[SDL_SCANCODE_LEFT]) rotation_input--;

        if (state[SDL_SCANCODE_R]) {
            reset_grid_cam();
            reset_player();
        }
        if (key_just_pressed(SDL_SCANCODE_P)) print_debug();

        if (state[SDL_SCANCODE_W]) vertical_input++;
        if (state[SDL_SCANCODE_S]) vertical_input--;
        if (state[SDL_SCANCODE_A]) horizontal_input--;
        if (state[SDL_SCANCODE_D]) horizontal_input++;

        if (key_just_pressed(SDL_SCANCODE_2)) set_view(VIEW_GRID);
        if (key_just_pressed(SDL_SCANCODE_1)) set_view(VIEW_FPS);

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

    // Debug menu opening
    if (open_debug_menu && !(debug_menu_was_open && !reopen_debug_menu)) debug_menu();
    else debug_menu_was_open = FALSE;

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

    /*
    // North
    if (
        (
            northeast_collision && (
                !(southeast_collision && !northwest_collision) || (
                    player_x_velocity != 0 &&
                    (( ((int) prev_player_x + player_radius) / GRID_SPACING ) == ( ((int) player_x + player_radius) / GRID_SPACING ))
                )
            )
        ) || (
            northwest_collision && (
                !(southwest_collision && !northeast_collision) || (
                    player_x_velocity != 0 &&
                    (( ((int) prev_player_x - player_radius) / GRID_SPACING ) == ( ((int) player_x - player_radius) / GRID_SPACING ))
                )
            )
        )
    ) {
        player_y = (((int) player_y / GRID_SPACING) * GRID_SPACING) + player_radius;
        player_y_velocity = 0;
        
        printf(
            "north collision: east now_x (%d), east prev_x (%d)\n",
            ((int) player_x + player_radius) / GRID_SPACING,
            ((int) prev_player_x + player_radius) / GRID_SPACING
        );
        
        // open_debug_menu = TRUE;
    }

    // South
    if (
        (southeast_collision && !(northeast_collision && !southwest_collision)) ||
        (southwest_collision && !(northwest_collision && !southeast_collision))
    ) {
        player_y = ((((int) player_y / GRID_SPACING) + 1) * GRID_SPACING) - player_radius;
        player_y_velocity = 0;
        //puts("south collision");
    }

    // East
    if (
        (northeast_collision && !(northwest_collision && !southeast_collision)) ||
        (southeast_collision && !(southwest_collision && !northeast_collision))
    ) {
        player_x = ((((int) player_x / GRID_SPACING) + 1) * GRID_SPACING) - player_radius;
        player_x_velocity = 0;
        //puts("east collision");
    }

    // West
    if (
        (northwest_collision && !(northeast_collision && !southwest_collision)) ||
        (southwest_collision && !(southeast_collision && !northwest_collision))
    ) {
        player_x = (((int) player_x / GRID_SPACING) * GRID_SPACING) + player_radius;
        player_x_velocity = 0;
        //puts("west collision");
    }
    */

    prev_player_x = player_x;
    prev_player_y = player_y;

    // Move player by velocity
    player_x += player_x_velocity * delta_time;
    player_y += player_y_velocity * delta_time;
    assign_i_player_pos();

    if (show_player_trail) fill_dgp(player_x, player_y, DGP_YELLOW);

    if (grid_follow_player) {
        grid_cam_x = round( player_x - ((WINDOW_WIDTH / 2) * (1.0f / grid_cam_zoom_p)) );
        grid_cam_y = round( player_y - ((WINDOW_HEIGHT / 2) * (1.0f / grid_cam_zoom_p)) );
    }

    
    //printf("player pos: (%f, %f) facing %f (%f deg)\n", player_x, player_y, player_angle, rad_deg(player_angle));
    //printf("player velocity: (%f, %f)\n", player_x_velocity, player_y_velocity);
    

    vertical_input = 0;
    horizontal_input = 0;
    rotation_input = 0;
}

void render(void) {
    // Set background
    if (view_mode == VIEW_FPS) set_draw_color_rgb(C_BLACK);
    else if (view_mode == VIEW_GRID) set_draw_color_rgb(grid_bg);
    else if (view_mode == VIEW_TERMINAL) set_draw_color_rgb(terminal_bg);
    SDL_RenderClear(renderer);

    if (view_mode == VIEW_FPS) {
        // Floor
        vertical_gradient(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, WINDOW_HEIGHT / 2, fp_bg_bottom_goal, fp_bg_bottom);
        // Ceiling
        vertical_gradient(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT / 2, fp_bg_top, fp_bg_top_goal);
        
    }

    if ((view_mode == VIEW_FPS && fp_show_walls) || show_player_vision) { // Raycasting
        float ray_distances[WINDOW_WIDTH];

        // Get ray distances
        for (int ray_i = 0; ray_i < WINDOW_WIDTH; ray_i++) {
            // Calculate ray angle
            float ray_angle = (player_angle - (fov / 2)) + ((fov / WINDOW_WIDTH) * ray_i);
            if (ray_angle < 0) ray_angle += M_PI * 2;
            else if (ray_angle >= M_PI * 2) ray_angle -= M_PI * 2;

            // Raycast
            xy hit = raycast(player_x, player_y, ray_angle);

            if (view_mode == VIEW_FPS && fp_show_walls) {
                ray_distances[ray_i] = cos(ray_angle - player_angle) * sqrtf( powf(hit.x - player_x, 2) + powf(hit.y - player_y, 2) );
            }
        }

        // Draw walls
        for (int i = 0; i < WINDOW_WIDTH; i++) {
            int height = project(ray_distances[i]);
            draw_rect_rgb(i, (WINDOW_HEIGHT / 2) - (height / 2), 1, height, shade_rgb(wall_color, ray_distances[i]));
        }

        // Store positions and distances of sprites on screen
        for (mobj* o = mobj_head; o; o = o->next) {
            // Get angle relative to left edge of player vision
            float angle = atan2f(o->y - player_y, o->x - player_x);
            if (angle < 0) angle += (M_PI * 2);
            
            float relative_angle = angle - (player_angle - (fov / 2));
            if (relative_angle > M_PI * 2) relative_angle -= M_PI * 2;
            else if (relative_angle < 0) relative_angle += M_PI * 2;

            // Calculare screen x pos
            int screen_x = ((float) WINDOW_WIDTH / fov) * relative_angle;

            // Calculate distance to mobj
            float mobj_distance = point_dist(player_x, player_y, o->x, o->y);

            // Store, sorted farthest to closest, for rendering if not behind wall
            if (screen_x >= 0 && screen_x < WINDOW_WIDTH && ray_distances[screen_x] > mobj_distance) {
                sprite_proj* new_proj = sprite_proj_create(screen_x, mobj_distance, o->sprite_num);
                
                if (sprite_proj_head) {
                    if (sprite_proj_head->dist < mobj_distance) {
                        new_proj->next = sprite_proj_head;
                        sprite_proj_head = new_proj;
                    } else {
                        // Find closest greater sprite
                        sprite_proj* greater = sprite_proj_head;
                        for (;greater->next && greater->next->dist > mobj_distance; greater = greater->next);

                        // Insert this sprite after the lesser sprite
                        sprite_proj* lesser = greater->next;
                        greater->next = new_proj;
                        new_proj->next = lesser;
                    }
                } else {
                    sprite_proj_head = new_proj;
                }
            }
        }

        // Render mobjs
        for (sprite_proj* sprite = sprite_proj_head; sprite; sprite = sprite->next) {
            int sprite_height = project(sprite->dist);

            // Scale sprite width using height
            int image_width, image_height;
            SDL_QueryTexture(sprites[sprite->sprite_num], NULL, NULL, &image_width, &image_height);
            int sprite_width = ((float) sprite_height / image_height) * image_width;

            unsigned char sprite_shading = shade(255, sprite->dist);
            SDL_SetTextureColorMod(sprites[sprite->sprite_num], sprite_shading, sprite_shading, sprite_shading);

            // Draw columns to scale the sprite by distance
            float texture_col = 0;
            float texture_incr = (float) image_width / sprite_width;
            SDL_Rect source = {0, 0, ceilf(texture_incr), image_height};
            SDL_Rect dest = {sprite->x - (sprite_width / 2), (WINDOW_HEIGHT / 2) - (sprite_height / 2), 1, sprite_height};
            for (int sprite_col_i = 0; sprite_col_i < sprite_width; sprite_col_i++) {
                dest.x++;
                if (ray_distances[dest.x] > sprite->dist) {
                    source.x = texture_col;
                    SDL_RenderCopy(renderer, sprites[sprite->sprite_num], &source, &dest);
                }
                texture_col += texture_incr;
            }
        }

        sprite_proj_destroy_all();
    }

    // Map view
    if (view_mode == VIEW_GRID) { // Grid rendering
        // Grid box fill
        for (int row = 0; row < grid_height; row++) {
            for (int col = 0; col < grid_length; col++) {
                g_draw_rect_rgb(col * GRID_SPACING, row * GRID_SPACING, GRID_SPACING, GRID_SPACING,
                get_grid_bool(col, row) ? grid_fill_solid : grid_fill_nonsolid);
            }
        }

        if (show_grid_lines) { // Grid lines
            // Vertical
            for (int i = 0; i < grid_length + 1; i++) {
                g_draw_rect_rgb(
                    (i * GRID_SPACING) - ceil(grid_line_width / 2), 
                    0,
                    grid_line_width,
                    grid_height * GRID_SPACING,
                    grid_line_fill
                );
            }
            // Horizontal
            for (int i = 0; i < grid_length + 1; i++) {
                g_draw_rect_rgb(
                    0,
                    (i * GRID_SPACING) - ceil(grid_line_width / 2),
                    grid_length * GRID_SPACING,
                    grid_line_width,
                    grid_line_fill
                );
            }
        }

        // Map objects
        for (mobj* temp = mobj_head; temp; temp = temp->next) {
            g_draw_scale_point_rgb(temp->x, temp->y, grid_mobj_radius, mobj_color);
        }

        // Fill debug grid points
        for (struct fill_dgp* p_i = fill_dgp_head; p_i; p_i = p_i->next) {
            g_draw_point_rgb(p_i->x, p_i->y, dgp_radius, DGP_COLORS[p_i->color_index]);
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
                dgp_radius * (temp_dgp_list[i].color_index == DGP_RED ? 1.5 : 1),
                DGP_COLORS[temp_dgp_list[i].color_index]);
        }

        if (show_grid_crosshairs) {
            draw_rect_rgb(WINDOW_WIDTH / 2, 0, 1, WINDOW_HEIGHT, C_WHITE);
            draw_rect_rgb(0, WINDOW_HEIGHT / 2, WINDOW_WIDTH, 1, C_WHITE);
        }

    } else if (view_mode == VIEW_TERMINAL) { // Terminal view

        // Output from previous command
        BF_DrawTextRgb(DT_console_text, 0, 0, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);
    
        // Prompt string
        BF_FillTextRgb(TERMINAL_PROMPT, terminal_font_size, WINDOW_WIDTH, terminal_font_color, FALSE);

        // Input text
        BF_FillTextRgb(terminal_input->text, terminal_font_size, WINDOW_WIDTH, terminal_font_color, TRUE);
    }

    if (num_temp_dgps != 0) {
        free(temp_dgp_list); temp_dgp_list = NULL;
        num_temp_dgps = 0;
    }

    SDL_RenderPresent(renderer);
}

void free_memory(void) {
    // Grid
    free(grid_enc);

    // Fill dgps
    struct fill_dgp* p_i = fill_dgp_head;
    while (p_i) {
        struct fill_dgp* next = p_i->next;
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
        //puts("update");
        render();
        //puts("render");
    }

    debugging_end();
    destroy_window();
    free_memory();

    return 1;
}