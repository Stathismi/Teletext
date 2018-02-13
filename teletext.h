
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <assert.h>

/* Font stuff */
typedef unsigned short fntrow;
#define FNTWIDTH (int)(sizeof(fntrow)*8)
#define FNTHEIGHT 18
#define FNTCHARS 96
#define FNT1STCHAR 32

#define WIDTH 40
#define HEIGHT 25
#define ON_ERROR(STR) fprintf(stderr, STR); exit(EXIT_FAILURE)
#define ARGUMENTS_REQUIRED 3
#define MIN_CONTROL_CODE 128
#define NEW_BACKGROUND 0x9D
#define RED_GRAPHICS 0x91
#define WHITE_GRAPHICS 0x97
#define HOLD_GRAPHICS 0x9E
#define RELEASE_GRAPHICS 0x9F
#define ADJUST_COLOUR 16
#define BLACK_BACKGROUND 0x9C
#define MAKE_SPACE 0xA0
#define MIN_NON_BLOCK 0xC0
#define MAX_NON_BLOCK 0xDF
#define BASECODE 160
#define MAX_COLOUR 255
#define MAX_CONTROL_CODE 255
#define TOTAL_CHARACTERS 1000
#define CONTIGUOUS_CODE 0X99
#define SEPARATED_CODE 0x9A
#define SINGLE_CODE 0x8C
#define DOUBLE_CODE 0x8D
#define DELAY 5

/*Values for computing graphics code*/
#define TOP_LEFT_CODE 1
#define TOP_RIGHT_CODE 2
#define MIDDLE_LEFT_CODE 4
#define MIDDLE_RIGHT_CODE 8
#define BOTTOM_LEFT_CODE 16
#define BOTTOM_RIGHT_CODE 64

#define W_WIDTH 40*FNTWIDTH
#define W_HEIGHT 25*FNTHEIGHT
#define HALF_HEIGHT 2
#define LIT_ON 1
#define SEPARATE_SIXEL 2
#define SIXEL_HEIGHT FNTHEIGHT/3
#define SIXEL_WIDTH FNTWIDTH/2

#define TOP_SIXELS_ON 0XA3
#define TOP_MIDDLE_SIXELS_ON 0XAF
#define ALL_SIXELS_ON 0XFF
#define FIRST_ARGV_INPUT 1
#define SECOND_ARGV_INPUT 2


typedef enum height {Single, Double} height;
typedef enum graphics_mode {Alphanumeric, Graphics} graphics_mode;
typedef enum graphics_type {Contiguous, Separated} graphics_type;
typedef enum hold_graphics {Off, On} hold_graphics;
typedef enum colour {BLACK=0x80, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE} colour;

typedef struct sixel_blocks{
   int top_left;
   int top_right;
   int middle_left;
   int middle_right;
   int bottom_left;
   int bottom_right;
}sixel_blocks;

typedef struct screen{
   unsigned char character;
   int foreground;
   int background;
   int graphics_mode;
   int graphics_type;
   int height;
   int hold_graphics;
   sixel_blocks sixel;
}screen;

typedef struct SDL_Simplewin {
   SDL_bool finished;
   SDL_Window *win;
   SDL_Renderer *renderer;
}SDL_Simplewin;

/*main functions*/
void read_file(char* argv, screen screen_unit[HEIGHT][WIDTH]);
void adjust_screen(screen screen_unit[HEIGHT][WIDTH]);
void init_sixels(sixel_blocks *s_u);
void default_settings(screen *s_u);
void decode_control_code(screen *s_u, screen *prv);
void dec_graphics_mode(screen *s_u, screen *prv);
void dec_graphics_type(screen *s_u, screen *prv);
void dec_height(screen *s_u, screen *prv);
void dec_hold_graphics(screen *s_u, screen *prv);
void adjust_screen_unit(screen *s_u, screen *prv, sixel_blocks *prv_sixels);
void dec_foreground(screen *s_u, int *prv_foreground);
void dec_background(screen *s_u, int *prv_background, int prv_foreground);
void dec_sixels(screen *s_u, sixel_blocks *prv_sixels);
void activate_sixels(screen *s_u, unsigned char hex_value, sixel_blocks *prv_sixels);
void load_prv_sixel(screen *s_u, sixel_blocks *prv_sixels);
void adjust_character(screen *s_u);
void my_SDL_Init(SDL_Simplewin *sw);
void display_screen(SDL_Simplewin sw, screen screen_unit[HEIGHT][WIDTH], char* fname);
void read_font_file(fntrow fontdata[FNTCHARS][FNTHEIGHT], char *fname);
void dec_colour(int character,  SDL_Simplewin *sw);
void background_draw(SDL_Simplewin *sw, int row, int column, screen s_u);
void foreground_setup(screen s_u, fntrow fontdata[FNTCHARS][FNTHEIGHT],
   SDL_Simplewin *sw, int row, int column, int k, int prv_height);
void foreground_draw(screen s_u,SDL_Simplewin *sw, int row, int column, int i, int k);
void my_rect_draw(screen s_u, SDL_Simplewin *sw, int row, int column);
void lit_sixel(screen s_u, int graphics_type, SDL_Simplewin *sw, int row, int column);
void my_SDL_Events(SDL_Simplewin *sw);
