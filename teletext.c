#include "teletext.h"

int main(int argc, char *argv[])
{
   SDL_Simplewin sw;
   screen screen_unit[HEIGHT][WIDTH];
   if(argc!=ARGUMENTS_REQUIRED){
      ON_ERROR("argc must be three words\n");
   }
   read_file(argv[FIRST_ARGV_INPUT], screen_unit);
   adjust_screen(screen_unit);
   my_SDL_Init(&sw);
   do{
      SDL_Delay(DELAY);
      display_screen(sw, screen_unit, argv[SECOND_ARGV_INPUT]);
      SDL_RenderPresent(sw.renderer);
      my_SDL_Events(&sw);
   }while(!sw.finished);
   return 0;
}

void read_file(char* argv, screen screen_unit[HEIGHT][WIDTH])
{
   int i, k;
   size_t itms;
   FILE *fp=fopen(argv, "rb");
   if(fp==NULL){
      ON_ERROR("Opening of file Failed\n");
   }
   for(i=0; i<HEIGHT; i++){
      for(k=0; k<WIDTH; k++){
         itms=fread(&(screen_unit[i][k].character), sizeof(unsigned char), 1, fp);
         if(itms!=1){
            ON_ERROR("Cannot read an item on file \n");
         }
         if(screen_unit[i][k].character<MIN_CONTROL_CODE){
            screen_unit[i][k].character+=MIN_CONTROL_CODE;
         }
      }
   }
   fclose(fp);
}

void adjust_screen(screen screen_unit[HEIGHT][WIDTH])
{
   int i, k;
   screen previous_settings;
   sixel_blocks prv_sixels;
   init_sixels(&prv_sixels);
   /*take one character at a time and decode it!*/
   for(i=0; i<HEIGHT; i++){
      for(k=0; k<WIDTH; k++){
         init_sixels(&screen_unit[i][k].sixel);
         if(k==0){
            /*set default values at the beggining of each line*/
            default_settings(&screen_unit[i][k]);
            previous_settings=screen_unit[i][k];
         }
         decode_control_code(&screen_unit[i][k], &previous_settings);
         adjust_screen_unit(&screen_unit[i][k], &previous_settings, &prv_sixels);
      }
   }

}

void init_sixels(sixel_blocks *sixel)
{
   sixel->bottom_right=0;
   sixel->bottom_left=0;
   sixel->middle_right=0;
   sixel->middle_left=0;
   sixel->top_right=0;
   sixel->top_left=0;
}

void default_settings(screen *s_u)
{
   s_u->foreground=WHITE;
   s_u->background=BLACK;
   s_u->graphics_mode=Alphanumeric;
   s_u->graphics_type=Contiguous;
   s_u->height=Single;
   s_u->hold_graphics=Off;
}

void decode_control_code(screen *s_u, screen *prv)
{
   dec_graphics_mode(s_u, prv);
   dec_graphics_type(s_u, prv);
   dec_height(s_u, prv);
   dec_hold_graphics(s_u, prv);
}

void dec_graphics_mode(screen *s_u, screen *prv)
{
   if((s_u->character>=RED) && (s_u->character<=WHITE)){
      s_u->graphics_mode=Alphanumeric;
      s_u->hold_graphics=Off;
      prv->graphics_mode=Alphanumeric;
      prv->hold_graphics=Off;
   }
   if((s_u->character>=RED_GRAPHICS) && (s_u->character<=WHITE_GRAPHICS)){
      s_u->graphics_mode=Graphics;
      s_u->graphics_type=Contiguous;
      prv->graphics_mode=Graphics;
      prv->graphics_type=Contiguous;
   }
}

void dec_graphics_type(screen *s_u, screen *prv)
{
   if(s_u->character==CONTIGUOUS_CODE){
      s_u->graphics_mode=Graphics;
      s_u->graphics_type=Contiguous;
      prv->graphics_mode=Graphics;
      prv->graphics_type=Contiguous;
   }
   if(s_u->character==SEPARATED_CODE){
      s_u->graphics_mode=Graphics;
      s_u->graphics_type=Separated;
      prv->graphics_mode=Graphics;
      prv->graphics_type=Separated;
   }
}

void dec_height(screen *s_u, screen *prv)
{
   if(s_u->character==SINGLE_CODE){
      s_u->height=Single;
      s_u->hold_graphics=Off;
      prv->height=Single;
      prv->hold_graphics=Off;
   }
   if(s_u->character==DOUBLE_CODE){
      s_u->height=Double;
      s_u->hold_graphics=Off;
      prv->height=Double;
      prv->hold_graphics=Off;
   }
}

void dec_hold_graphics(screen *s_u, screen *prv)
{
   if(s_u->character==HOLD_GRAPHICS){
      s_u->hold_graphics=On;
      prv->hold_graphics=On;
      s_u->graphics_type=Contiguous;
   }
   if(s_u->character==RELEASE_GRAPHICS){
      s_u->hold_graphics=Off;
      prv->hold_graphics=Off;
   }
}

void adjust_screen_unit(screen *s_u, screen *prv, sixel_blocks *prv_sixels)
{
   s_u->hold_graphics=prv->hold_graphics;
   s_u->graphics_type=prv->graphics_type;
   s_u->height=prv->height;
   s_u->graphics_mode=prv->graphics_mode;
   dec_foreground(s_u, &prv->foreground);
   dec_background(s_u, &prv->background, prv->foreground);
   dec_sixels(s_u, prv_sixels);
   adjust_character(s_u);
}

void dec_foreground(screen *s_u, int *prv_foreground)
{
   if((s_u->character>=RED) && (s_u->character<=WHITE)){
      s_u->foreground=s_u->character;
   }
   else if((s_u->character>=RED_GRAPHICS) && (s_u->character<=WHITE_GRAPHICS)){
      s_u->foreground=s_u->character-ADJUST_COLOUR;
   }
   else{
      s_u->foreground=*prv_foreground;
   }
   *prv_foreground=s_u->foreground;
}

void dec_background(screen *s_u, int *prv_background, int prv_foreground){

   if(s_u->character==BLACK_BACKGROUND){
      s_u->background=BLACK;
   }
   else if(s_u->character==NEW_BACKGROUND){
      s_u->background=prv_foreground;
   }
   else{
      s_u->background=*prv_background;
   }
   *prv_background=s_u->background;
}

void adjust_character(screen *s_u)
{
   if(s_u->character<MAKE_SPACE){
      s_u->character=MAKE_SPACE;
   }
}

void dec_sixels(screen *s_u, sixel_blocks *prv_sixels)
{
   if(s_u->graphics_mode==Graphics){
      /*need to activate only the block graphics, not letters*/
      if(s_u->character<MIN_NON_BLOCK || s_u->character>MAX_NON_BLOCK){
         activate_sixels(s_u, s_u->character, prv_sixels);
         s_u->character=MAKE_SPACE;
      }
   }
}

void activate_sixels(screen *s_u, unsigned char hex_value, sixel_blocks *prv_sixels)
{
   /*activate proper sixels*/
   int graphics_code=hex_value-BASECODE;
   if(graphics_code/BOTTOM_RIGHT_CODE==1){
      s_u->sixel.bottom_right=graphics_code/BOTTOM_RIGHT_CODE;
      graphics_code%=BOTTOM_RIGHT_CODE;
   }
   else{
      s_u->sixel.bottom_right=0;
   }

   if(graphics_code/BOTTOM_LEFT_CODE==1){
      s_u->sixel.bottom_left=graphics_code/BOTTOM_LEFT_CODE;
      graphics_code%=BOTTOM_LEFT_CODE;
   }
   else{
      s_u->sixel.bottom_left=0;
   }

   if(graphics_code/MIDDLE_RIGHT_CODE==1){
      s_u->sixel.middle_right=graphics_code/MIDDLE_RIGHT_CODE;
      graphics_code%=MIDDLE_RIGHT_CODE;
   }
   else{
      s_u->sixel.middle_right=0;
   }

   if(graphics_code/MIDDLE_LEFT_CODE==1){
      s_u->sixel.middle_left=graphics_code/MIDDLE_LEFT_CODE;
      graphics_code%=MIDDLE_LEFT_CODE;
   }
   else{
      s_u->sixel.middle_left=0;
   }

   if(graphics_code/TOP_RIGHT_CODE==1){
      s_u->sixel.top_right=graphics_code/TOP_RIGHT_CODE;
      graphics_code%=TOP_RIGHT_CODE;
   }
   else{
      s_u->sixel.top_right=0;
   }

   if(graphics_code/TOP_LEFT_CODE==1){
      s_u->sixel.top_left=graphics_code/TOP_LEFT_CODE;
   }
   else{
      s_u->sixel.top_left=0;
   }
   load_prv_sixel(s_u, prv_sixels);
}

void load_prv_sixel(screen *s_u, sixel_blocks *prv_sixels)
{
   /*load previous sixel when on hold graphics only*/
   if(s_u->hold_graphics==On){
      s_u->sixel.bottom_right=prv_sixels->bottom_right;
      s_u->sixel.bottom_left=prv_sixels->bottom_left;
      s_u->sixel.middle_right=prv_sixels->middle_right;
      s_u->sixel.middle_left=prv_sixels->middle_left;
      s_u->sixel.top_right=prv_sixels->top_right;
      s_u->sixel.top_left=prv_sixels->top_left;
   }
   else{/*store current sixels in case we need them*/
      prv_sixels->bottom_right=s_u->sixel.bottom_right;
      prv_sixels->bottom_left=s_u->sixel.bottom_left;
      prv_sixels->middle_right=s_u->sixel.middle_right;
      prv_sixels->middle_left=s_u->sixel.middle_left;
      prv_sixels->top_right=s_u->sixel.top_right;
      prv_sixels->top_left=s_u->sixel.top_left;
   }
}

void my_SDL_Init(SDL_Simplewin *sw)
{
   if (SDL_Init(SDL_INIT_VIDEO)!=0) {
      fprintf(stderr, "\nUnable to initialize SDL:  %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   }
   sw->finished = 0;
   sw->win= SDL_CreateWindow("Teletext",
                                       SDL_WINDOWPOS_UNDEFINED,
                                       SDL_WINDOWPOS_UNDEFINED,
                                       W_WIDTH, W_HEIGHT,
                                       SDL_WINDOW_SHOWN);
   if(sw->win == NULL){
      fprintf(stderr, "\nUnable to initialize SDL Window:  %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   }
   sw->renderer = SDL_CreateRenderer(sw->win, -1, 0);
   if(sw->renderer == NULL){
      fprintf(stderr, "\nUnable to initialize SDL Renderer:  %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   }

   if(SDL_SetRenderDrawColor(sw->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE)!=0){
      fprintf(stderr, "\nUnable to draw colour:  %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   }

   if(SDL_RenderClear(sw->renderer)!=0){
      fprintf(stderr, "\nUnable to clear render:  %s\n", SDL_GetError());
      SDL_Quit();
      exit(1);
   }
}

void display_screen(SDL_Simplewin sw, screen screen_unit[HEIGHT][WIDTH], char* fname)
{
   int i=0, k=0, row=0, column=0;
   fntrow fontdata[FNTCHARS][FNTHEIGHT];
   int prv_height=Single;
   read_font_file(fontdata, fname);

   for (i=0; i<HEIGHT; i++){
      for (k=0; k<WIDTH; k++){
         row=i*FNTHEIGHT;
         column=k*FNTWIDTH;
         prv_height=screen_unit[i-1][k].height;
         background_draw(&sw, row, column, screen_unit[i][k]);
         foreground_setup(screen_unit[i][k], fontdata, &sw, row, column, k, prv_height);
         if (screen_unit[i][k].graphics_mode == Graphics){
            my_rect_draw(screen_unit[i][k], &sw, row, column);
         }
      }
   }
}

void read_font_file(fntrow fontdata[FNTCHARS][FNTHEIGHT], char *fname)
{
   FILE *fp=fopen(fname, "rb");
   size_t itms;
   if(!fp){
      fprintf(stderr, "Can't open Font file %s\n", fname);
      exit(1);
   }
   itms=fread(fontdata, sizeof(fntrow), FNTCHARS*FNTHEIGHT, fp);
   if(itms!=FNTCHARS*FNTHEIGHT){
      fprintf(stderr, "Can't read all Font file %s (%d) \n", fname, (int)itms);
      exit(1);
   }
   fclose(fp);
}

void background_draw(SDL_Simplewin *sw, int row, int column, screen s_u)
{
   int i, k;
   for(i=0; i<FNTHEIGHT; i++){
      for(k=0; k<FNTWIDTH; k++){
         dec_colour(s_u.background, sw);
         if(SDL_RenderDrawPoint(sw->renderer, column+k, row+i) != 0){
            fprintf(stderr, "\nUnable to draw point:  %s\n", SDL_GetError());
            SDL_Quit();
            exit(1);
         }
      }
   }
}

void foreground_setup(screen s_u, fntrow fontdata[FNTCHARS][FNTHEIGHT],
   SDL_Simplewin *sw, int row, int column, int k, int prv_height)
   {
      int i=0;
      unsigned char chr=s_u.character-MIN_CONTROL_CODE;
      int adjusted_height=FNTHEIGHT;
      dec_colour(s_u.foreground, sw);

      if (s_u.height == Double){
         adjusted_height =adjusted_height/ HALF_HEIGHT;
         if (prv_height == Double){
            i=adjusted_height;                 /*select middle as the start*/
            adjusted_height=adjusted_height+i; /*select end*/
            row=row-FNTHEIGHT;                 /*make the shift for upwards*/
         }
      }

      for(; i<adjusted_height; i++){
         for(k=0; k<FNTWIDTH; k++){
            if(fontdata[chr-FNT1STCHAR][i] >> (FNTWIDTH-1-k)&1){
               foreground_draw(s_u, sw, row, column, i , k);
            }
         }
      }
   }

   void foreground_draw(screen s_u, SDL_Simplewin *sw, int row, int column, int i, int k)
   {
      if(s_u.height==Single){
         if(SDL_RenderDrawPoint(sw->renderer, column+k, row+i)!=0){
            fprintf(stderr, "\nUnable to draw point:  %s\n", SDL_GetError());
            SDL_Quit();
            exit(1);
         }
      }
      else{
         if(SDL_RenderDrawPoint(sw->renderer, column + k, (row +i*2))!=0){
            fprintf(stderr, "\nUnable to draw point:  %s\n", SDL_GetError());
            SDL_Quit();
            exit(1);
         }
         if(SDL_RenderDrawPoint(sw->renderer, column + k, (row +i*2)+1)!=0){
            fprintf(stderr, "\nUnable to draw point:  %s\n", SDL_GetError());
            SDL_Quit();
            exit(1);
         }
      }
   }

   void my_rect_draw(screen s_u, SDL_Simplewin *sw, int row, int column)
   {
      if(s_u.sixel.top_left==LIT_ON){
         lit_sixel(s_u, s_u.graphics_type, sw, row, column);
      }
      if(s_u.sixel.top_right==LIT_ON){
         lit_sixel(s_u, s_u.graphics_type, sw, row, column+SIXEL_WIDTH);
      }
      if(s_u.sixel.middle_left==LIT_ON){
         lit_sixel(s_u, s_u.graphics_type, sw, row+SIXEL_HEIGHT, column);
      }
      if(s_u.sixel.middle_right==LIT_ON){
         lit_sixel(s_u, s_u.graphics_type, sw, row+SIXEL_HEIGHT, column+SIXEL_WIDTH);
      }
      if(s_u.sixel.bottom_left==LIT_ON){
         lit_sixel(s_u, s_u.graphics_type, sw, row+(2*SIXEL_HEIGHT), column);
      }
      if(s_u.sixel.bottom_right==LIT_ON){
         lit_sixel(s_u, s_u.graphics_type, sw, row+(2*SIXEL_HEIGHT), column+SIXEL_WIDTH);
      }
   }

   void lit_sixel(screen s_u, int graphics_type, SDL_Simplewin *sw, int row, int column)
   {
      SDL_Rect rectangle;
      int tmp_height=SIXEL_HEIGHT, tmp_width=SIXEL_WIDTH;
      if(graphics_type==Separated){
         tmp_height=tmp_height-SEPARATE_SIXEL;
         tmp_width=tmp_width-SEPARATE_SIXEL;
      }
      rectangle.x=column;
      rectangle.y=row;
      rectangle.w=tmp_width;
      rectangle.h=tmp_height;
      dec_colour(s_u.foreground, sw);
      if(SDL_RenderFillRect(sw->renderer, &rectangle)!=0){
         fprintf(stderr, "\nUnable to fill rect: %s\n", SDL_GetError());
         SDL_Quit();
         exit(1);
      }
   }

   void dec_colour(int character,  SDL_Simplewin *sw)
   {
      int error=0;
      switch(character){
         case BLACK:
         error=SDL_SetRenderDrawColor(sw->renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
         break;
         case RED:
         error=SDL_SetRenderDrawColor(sw->renderer, MAX_COLOUR, 0, 0, SDL_ALPHA_OPAQUE);
         break;
         case GREEN:
         error=SDL_SetRenderDrawColor(sw->renderer, 0, MAX_COLOUR, 0, SDL_ALPHA_OPAQUE);
         break;
         case YELLOW:
         error=SDL_SetRenderDrawColor(sw->renderer, MAX_COLOUR, MAX_COLOUR, 0, SDL_ALPHA_OPAQUE);
         break;
         case BLUE:
         error=SDL_SetRenderDrawColor(sw->renderer, 0, 0, MAX_COLOUR, SDL_ALPHA_OPAQUE);
         break;
         case MAGENTA:
         error=SDL_SetRenderDrawColor(sw->renderer, MAX_COLOUR, 0, MAX_COLOUR, SDL_ALPHA_OPAQUE);
         break;
         case CYAN:
         error=SDL_SetRenderDrawColor(sw->renderer, 0, MAX_COLOUR, MAX_COLOUR, SDL_ALPHA_OPAQUE);
         break;
         case WHITE:
         error=SDL_SetRenderDrawColor(sw->renderer, MAX_COLOUR, MAX_COLOUR, MAX_COLOUR, SDL_ALPHA_OPAQUE);
         break;
      }
      if(error!=0){
         fprintf(stderr, "unable to draw colour");
         SDL_Quit();
         exit(1);
      }
   }

   void my_SDL_Events(SDL_Simplewin *sw)
   {
      SDL_Event event;
      while(SDL_PollEvent(&event))
      {
         switch (event.type){
            case SDL_QUIT:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_KEYDOWN:
            sw->finished=1;
         }
      }
   }
