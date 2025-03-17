#include "../constantsTemplate.h"
#include "./BasicFont.h"
#include <stdio.h>

const Uint8* state;

const char* START_TEXT = "press a key!";
/*
#define ALLOC_PORTION 10
char* text;
size_t len = 0;
size_t font_size = 5;


void text_append(char c) {
    // If we are at the edge of an allocation portion, allocate another portion
    if (len % ALLOC_PORTION == 0) {
        printf("Allocating %zu\n", len + ALLOC_PORTION + 1);
        text = (char *) realloc(text, len + ALLOC_PORTION + 1);
    }

    // Increment length
    len++;

    // Add new character to end
    text[len - 1] = c;

    // Add terminating character
    text[len] = '\0';
}

void text_pop(void) {
    // Decrement length
    len--;

    // Replace last character with terminating character
    text[len] = '\0';

    // If we are at the edge of an allocation portion, reallocate without the excess portion
    if (len % ALLOC_PORTION == 0) {
        printf("Reallocating %zu\n", len + 1);
        text = (char *) realloc(text, len + 1);
    }
}
*/

alstring* input;
int font_size = 4;

void setup(void) {
    state = SDL_GetKeyboardState(NULL);

    input = alstring_init(5);

    for (int i = 0, end = strlen(START_TEXT); i < end; i++) {
        alstring_append(input, START_TEXT[i]);
    }
}

Uint8 prev_state[SDL_SCANCODE_COUNT];
int key_just_pressed(int scancode) {
    return state[scancode] && !prev_state[scancode];
}

void process_input(void) {
    SDL_Event event;
    SDL_PollEvent(&event);

    int shift = state[SDL_SCANCODE_RSHIFT] || state[SDL_SCANCODE_LSHIFT];

    switch (event.type) {
        case SDL_EVENT_QUIT:
            game_is_running = FALSE;
            break;
        case SDL_EVENT_KEY_DOWN: {
            char c = event.key.key;

            if (key_just_pressed(SDL_SCANCODE_UP)) {
                font_size++;
                break;
            }

            if (key_just_pressed(SDL_SCANCODE_DOWN)) {
                if (font_size > 1) font_size--;
                break;
            }

            if (key_just_pressed(SDL_SCANCODE_RETURN)) c = '\n';

            // Characters using the shift key
            // Question mark
            else if (c == '/' && shift) c = '?';
            // Exclamation mark
            else if (c == '1' && shift) c = '!';

            if (state[SDL_SCANCODE_BACKSPACE]) {
                alstring_pop(input);
            }
            else if (c == '\n' || c == ' ' || BF_GetCharIndex(c) != -1) {
                alstring_append(input, c);
            }

            break;
        }
    }

    if (state[SDL_SCANCODE_ESCAPE]) game_is_running = FALSE;

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
}

void render(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    BF_DrawText(input->text, 0, 0, font_size, WINDOW_WIDTH, 255, 255, 255, TRUE);

    SDL_RenderPresent(renderer);
}

int main() {
    printf("Start\n");

    game_is_running = initialize_window("Basic Font Test");

    setup();

    while (game_is_running) {
        process_input();
        update();
        render();
    }

    alstring_destroy(input);

    destroy_window();

    return 0;
}