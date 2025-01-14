#ifndef SDLStart_h
#define SDLStart_h

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

// RGB colors
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} rgb;

#define C_BLACK (rgb) {0, 0, 0}
#define C_WHITE (rgb) {255, 255, 255}
#define C_YELLOW (rgb) {255, 255, 0}
#define C_RED (rgb) {255, 0, 0}
#define C_PURPLE (rgb) {255, 0, 255}
#define C_BLUE (rgb) {0, 0, 255}

// SDL stuff
int game_is_running = FALSE;
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

int last_frame_time = 0;

int initialize_window(char* window_title) {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL.\n");
        return FALSE;
    }
    window = SDL_CreateWindow(
        window_title, //Title
        SDL_WINDOWPOS_CENTERED, //X
        SDL_WINDOWPOS_CENTERED, //Y
        WINDOW_WIDTH, //Width
        WINDOW_HEIGHT, //Height
        0 //Flags
    );
    if (!window) {
        fprintf(stderr, "Error creating SDL Window.\n");
        return FALSE;
    }

    renderer = SDL_CreateRenderer(window, -1, 0); //Window, display driver, flags
    if (!renderer) {
        fprintf(stderr, "Error creating SDL Renderer.\n");
        return FALSE;
    }

    return TRUE;
}

void destroy_window() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

// Utilities
int max(int val1, int val2) {
    if (val1 > val2) return val1; else return val2;
}

int min(int val1, int val2) {
    if (val1 < val2) return val1; else return val2;
}

int bounds(int minVal, int val, int maxVal) {
    return max( min(val, maxVal), minVal );
}

// Colors
rgb brighten(rgb color, int offset) {
    int new_r = color.r;
    int new_g = color.g;
    int new_b = color.b;
    new_r += offset; new_g += offset; new_b += offset;
    new_r = bounds(0, new_r, 255);
    new_g = bounds(0, new_g, 255);
    new_b = bounds(0, new_b, 255);
    return (rgb) {new_r, new_g, new_b};
}

void print_rgb(rgb color) {
    printf("(%d, %d, %d)", color.r, color.g, color.b);
}

// Graphics
void draw_rect_a(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    SDL_Rect rect = {x, y, length, width};
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    SDL_RenderFillRect(renderer, &rect);
}

void draw_rect_a_rgb(int x, int y, int length, int width, rgb color, int a) {
    draw_rect_a(x, y, length, width, color.r, color.g, color.b, a);
}

void draw_rect(int x, int y, int length, int width, unsigned char r, unsigned char g, unsigned char b) {
    draw_rect_a(x, y, length, width, r, g, b, 255);
}

void set_draw_color_rgb(rgb color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
}

void draw_rect_rgb(int x, int y, int length, int width, rgb color) {
    draw_rect(x, y, length, width, color.r, color.g, color.b);
}

void draw_rect_bordered(int x, int y, int length, int width, unsigned char fill_r, unsigned char fill_g, unsigned char fill_b,
    unsigned char border_r, unsigned char border_g, unsigned char border_b) {
    SDL_Rect rect = {x, y, length, width};
    SDL_SetRenderDrawColor(renderer, fill_r, fill_g, fill_b, 255);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, border_r, border_g, border_b, 255);
    SDL_RenderDrawRect(renderer, &rect);
}

void draw_rect_bordered_rgb(int x, int y, int length, int width, rgb fill, rgb border) {
    draw_rect_bordered(x, y, length, width, fill.r, fill.g, fill.b, border.r, border.g, border.b);
}

void draw_point(int x, int y, float radius, unsigned char r, unsigned char g, unsigned char b) {
    draw_rect_bordered(x - radius, y - radius, round(radius * 2), round(radius * 2), r, g, b, 0, 0, 0);
}

void draw_point_rgb(int x, int y, float radius, rgb color) {
    draw_point(x, y, radius, color.r, color.g, color.b);
}

void vertical_gradient(int x, int y, int width, int height, rgb top_color, rgb bottom_color) {
    float c_r = (float) (top_color.r - bottom_color.r) / -height;
    float c_g = (float) (top_color.g - bottom_color.g) / -height;
    float c_b = (float) (top_color.b - bottom_color.b) / -height;

    float grad_r = top_color.r;
    float grad_g = top_color.g;
    float grad_b = top_color.b;

    for (int i = 0; i < height; i++) {
        draw_rect(x, y + i, width, 1, round(grad_r), round(grad_g), round(grad_b));
        grad_r += c_r;
        grad_g += c_g;
        grad_b += c_b;
    }
}

rgb get_gradient_color_dist(rgb start, rgb end, int length, int dist) {
    float c_r = (float) (start.r - end.r) / -length;
    float c_g = (float) (start.g - end.g) / -length;
    float c_b = (float) (start.b - end.b) / -length;

    return (rgb) { roundf(start.r + (c_r * dist)), roundf(start.g + (c_g * dist)), roundf(start.b + (c_b * dist)) };
}

#endif