#include <string.h>
#include <stdlib.h>
#include "../../SDL/SDL3Start.h"

#define BF_NUM_CHARS 47
#define BF_CHAR_WIDTH 5
#define BF_CHAR_HEIGHT 5

const static int BF_CHARS[BF_NUM_CHARS][BF_CHAR_HEIGHT][BF_CHAR_WIDTH] = {
    { // A
        {0,1,1,1,0},
        {1,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,0,0,0,1}
    },
    { // B
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,0}
    },
    { // C
        {0,1,1,1,0},
        {1,0,0,0,0},
        {1,0,0,0,0},
        {1,0,0,0,0},
        {0,1,1,1,0}
    },
    { // D
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,0}
    },
    { // E
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,0},
        {1,0,0,0,0},
        {1,1,1,1,1}
    }, // F
    {
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,1,1,1,0},
        {1,0,0,0,0},
        {1,0,0,0,0}
    },
    { // G
        {1,1,1,1,1},
        {1,0,0,0,0},
        {1,0,1,1,1},
        {1,0,0,0,1},
        {1,1,1,1,1}
    },
    { // H
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,1,1,1,1},
        {1,0,0,0,1},
        {1,0,0,0,1}
    },
    { // I
        {0,1,1,1,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,1,1,1,0}
    },
    { // J
        {1,1,1,1,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {1,0,1,0,0},
        {1,1,1,0,0}
    },
    { // K
        {1,0,0,1,0},
        {1,0,1,0,0},
        {1,1,0,0,0},
        {1,0,1,0,0},
        {1,0,0,1,0}
    },
    { // L
        {1,0,0,0,0},
        {1,0,0,0,0},
        {1,0,0,0,0},
        {1,0,0,0,0},
        {1,1,1,1,0}
    },
    { // M
        {1,0,0,0,1},
        {1,1,0,1,1},
        {1,0,1,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1}
    },
    { // N
        {1,0,0,0,1},
        {1,1,0,0,1},
        {1,0,1,0,1},
        {1,0,0,1,1},
        {1,0,0,0,1}
    },
    { // O
        {0,1,1,1,0},
        {1,0,0,0,1},
        {1,0,1,0,1},
        {1,0,0,0,1},
        {0,1,1,1,0}
    },
    { // P
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,1,1,1,0},
        {1,0,0,0,0},
        {1,0,0,0,0}
    },
    { // Q
        {0,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,1,1},
        {0,1,1,1,1}
    },
    { // R
        {1,1,1,1,0},
        {1,0,0,0,1},
        {1,1,1,1,0},
        {1,0,0,1,0},
        {1,0,0,0,1}
    },
    { // S
        {0,1,1,1,1},
        {1,0,0,0,0},
        {0,1,1,1,0},
        {0,0,0,0,1},
        {1,1,1,1,0}
    },
    { // T
        {1,1,1,1,1},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0}
    },
    { // U
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {0,1,1,1,0}
    },
    { // V
        {1,0,0,0,1},
        {1,0,0,0,1},
        {0,1,0,1,0},
        {0,1,0,1,0},
        {0,0,1,0,0}
    },
    { // W
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,1,0,1},
        {1,1,0,1,1}
    },
    { // X
        {1,0,0,0,1},
        {0,1,0,1,0},
        {0,0,1,0,0},
        {0,1,0,1,0},
        {1,0,0,0,1}
    },
    { // Y
        {1,0,0,0,1},
        {0,1,0,1,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0}
    },
    { // Z
        {1,1,1,1,1},
        {0,0,0,1,0},
        {0,0,1,0,0},
        {0,1,0,0,0},
        {1,1,1,1,1}
    },
    { // 0
        {0,1,1,1,0},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {1,0,0,0,1},
        {0,1,1,1,0}
    },
    { // 1
        {0,0,1,0,0},
        {0,1,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,1,1,1,0}
    },
    { // 2
        {0,1,1,0,0},
        {1,0,0,1,0},
        {0,0,1,0,0},
        {0,1,0,0,0},
        {1,1,1,1,0}
    },
    { // 3
        {1,1,1,0,0},
        {0,0,0,1,0},
        {1,1,1,0,0},
        {0,0,0,1,0},
        {1,1,1,0,0}
    },
    { // 4
        {1,0,0,1,0},
        {1,0,0,1,0},
        {1,1,1,1,0},
        {0,0,0,1,0},
        {0,0,0,1,0}
    },
    { // 5
        {0,1,1,1,0},
        {1,0,0,0,0},
        {1,1,1,1,0},
        {0,0,0,1,0},
        {1,1,1,1,0}
    },
    { // 6
        {1,1,1,1,0},
        {1,0,0,0,0},
        {1,1,1,1,0},
        {1,0,0,1,0},
        {1,1,1,1,0}
    },
    { // 7
        {1,1,1,1,0},
        {0,0,0,1,0},
        {0,0,1,0,0},
        {0,1,0,0,0},
        {1,0,0,0,0}
    },
    { // 8
        {0,1,1,0,0},
        {1,0,0,1,0},
        {0,1,1,0,0},
        {1,0,0,1,0},
        {0,1,1,0,0}
    },
    { // 9
        {1,1,1,1,0},
        {1,0,0,1,0},
        {1,1,1,1,0},
        {0,0,0,1,0},
        {1,1,1,1,0}
    },
    { // period (.)
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,1,1,0,0},
        {0,1,1,0,0}
    },
    { // question mark (?)
        {0,1,1,0,0},
        {1,0,0,1,0},
        {0,0,1,0,0},
        {0,0,0,0,0},
        {0,0,1,0,0}
    },
    { // exclamation mark (!)
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,0,0,0},
        {0,0,1,0,0}
    },
    { // greater than (>)
        {1,1,0,0,0},
        {0,0,1,1,0},
        {0,0,0,0,1},
        {0,0,1,1,0},
        {1,1,0,0,0}
    },
    { // underscore (_)
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {1,1,1,1,1}
    },
    { // colon (:)
        {1,1,0,0,0},
        {1,1,0,0,0},
        {0,0,0,0,0},
        {1,1,0,0,0},
        {1,1,0,0,0}
    },
    { // apostrophe (')
        {0,0,1,0,0},
        {0,0,1,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0},
        {0,0,0,0,0}
    },
    { // open square bracket ([) 
        {0,1,1,1,0},
        {0,1,0,0,0},
        {0,1,0,0,0},
        {0,1,0,0,0},
        {0,1,1,1,0}
    },
    { // close square bracket (])
        {0,1,1,1,0},
        {0,0,0,1,0},
        {0,0,0,1,0},
        {0,0,0,1,0},
        {0,1,1,1,0}
    },
    { // open parenthesis (() 
        {0,0,1,1,1},
        {0,1,0,0,0},
        {0,1,0,0,0},
        {0,1,0,0,0},
        {0,0,1,1,1}
    },
    { // close parenthesis ())
        {1,1,1,0,0},
        {0,0,0,1,0},
        {0,0,0,1,0},
        {0,0,0,1,0},
        {1,1,1,0,0}
    }
};

const char *BF_ALLCHARS = "abcdefghijklmnopqrstuvwxyz0123456789.?!>_:'[]()";

typedef struct {
    char lower;
    char upper;
} char_key;

#define BF_NUM_CHAR_KEYS 7
const char_key BF_CHAR_KEYS[BF_NUM_CHAR_KEYS] = {
    {'1', '!'},
    {'/', '?'},
    {'.', '>'},
    {'-', '_'},
    {';', ':'},
    {'9', '('},
    {'0', ')'}
};

char BF_GetUpperCharKey(char lower_key) {
    for (int i = 0; i < BF_NUM_CHAR_KEYS; i++) {
        if (BF_CHAR_KEYS[i].lower == lower_key) {
            return BF_CHAR_KEYS[i].upper;
        }
    }

    return -1;
}

int BF_GetCharIndex(char character) {
    // If character is uppercase (char values 65 through 90), change it to lowercase (add 32 to char value)
    if (65 <= character && character <= 90) character += 32;

    // Find index of character in array
    for (int i = 0; i < BF_NUM_CHARS; i++) {
        if (BF_ALLCHARS[i] == character) return i;
    }

    // If the character was not found, return error
    return -1;
}

size_t BF_char_x = 0;
size_t BF_char_y = 0;

size_t BF_char_line_x = 0;

void BF_SetTextPos(int x, int y) { BF_char_line_x = x; BF_char_x = x; BF_char_y = y; }

void BF_FillText(char *text, int font_size, int wrap_length, unsigned char font_color_r, unsigned char font_color_g, unsigned char font_color_b, char draw_cursor) {
    size_t length = strlen(text);

    // Iterate though each character in the text
    for (size_t i = 0; i < length; i++) {
        char this_char = text[i];

        // If the character is a newline, go to beginning of next line
        if (this_char == '\n') {
            BF_char_x = BF_char_line_x;
            BF_char_y += font_size * (BF_CHAR_HEIGHT + 1);
            continue;
        }

        // Get index of character in graphics array
        int char_index = BF_GetCharIndex(this_char);

        // If the character is in the set, draw it
        if (char_index != -1) {
            for (int row = 0; row < BF_CHAR_HEIGHT; row++) {
                for (int col = 0; col < BF_CHAR_WIDTH; col++) {
                    if (BF_CHARS[char_index][row][col])
                    draw_rect(
                        BF_char_x + (col * font_size),
                        BF_char_y + (row * font_size),
                        font_size, font_size,
                        font_color_r, font_color_g, font_color_b
                    );
                }
            }
        // Otherwise, if the character is not a space, don't advance the character placement
        // (unrecognized character)
        } else if (this_char != ' ') continue;

        // Advance to next character x position
        BF_char_x += font_size * (BF_CHAR_WIDTH + 1);

        // If the next character will exceed the wrap length, go to next line
        if (wrap_length != -1 && (BF_char_x - BF_char_line_x) + (font_size * 5) >= wrap_length) {
            BF_char_x = BF_char_line_x;
            BF_char_y += font_size * (BF_CHAR_HEIGHT + 1);
        }
    }

    // Draw cursor
    if (draw_cursor) {
        draw_rect(BF_char_x, BF_char_y, font_size * BF_CHAR_WIDTH, font_size * BF_CHAR_HEIGHT, font_color_r, font_color_g, font_color_b);
    }
}

void BF_FillTextRgb(char *text, int font_size, int wrap_length, rgb font_color, int draw_cursor) {
    BF_FillText(text, font_size, wrap_length, font_color.r, font_color.g, font_color.b, draw_cursor);
}

void BF_DrawText(char *text, int x, int y, int font_size, int wrap_length, unsigned char color_r, unsigned char color_g, unsigned char color_b, int show_cursor) {
    BF_SetTextPos(x, y);
    BF_FillText(text, font_size, wrap_length, color_r, color_g, color_b, show_cursor);
}

void BF_DrawTextRgb(char *text, int x, int y, int font_size, int wrap_length, rgb color, char show_cursor) {
    BF_DrawText(text, x, y, font_size, wrap_length, color.r, color.g, color.b, show_cursor);
}

typedef struct {
    char* text;
    size_t len;
    int alloc;
} alstring;

alstring *alstring_init(int alloc) {
    alstring* s = (alstring *) malloc(sizeof(*s));
    s->text = malloc(sizeof(char) * alloc);
    s->len = 1;
    s->alloc = alloc;
    return s;
}

void alstring_append(alstring *str, char c) {
    if (str->len % str->alloc == 0) {
        str->text = realloc(str->text, sizeof(char) * (str->len + str->alloc));
        //printf("allocated %zu\n", str->len + str->alloc);
    }
    str->text[str->len - 1] = c;
    str->len++;
    str->text[str->len - 1] = '\0';
}

void alstring_pop(alstring *str) {
    if (str->len > 1) {
        if (--str->len % str->alloc == 0) {
            str->text = realloc(str->text, sizeof(char) * str->len);
            //printf("allocated %zu\n", str->len);
        }
        str->text[str->len - 1] = '\0';
    }
}

void alstring_clear(alstring *str) {
    str->text = realloc(str->text, sizeof(char));
    str->text[0] = '\0';
    str->len = 1;
}

void alstring_destroy(alstring *str) {
    free(str->text);
    free(str);
}