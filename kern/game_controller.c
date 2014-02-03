/** @file game.c
 *  @brief A kernel with timer, keyboard, console support
 *
 *  This file contains the kernel's main() function.
 *
 *  It sets up the drivers and starts the game.
 *
 *  @author Michael Berman (mberman)
 *  @bug No known bugs.
 */

#include <p1kern.h>
#include <handler_install.h>
#include <video_defines.h>

/* libc includes. */
#include <stdio.h>

/* x86 specific includes */
#include <RNG/mt19937int.h>
#include <string.h>

#define NUM_ROW 10
#define NUM_COL 15
#define SCREEN_X 80
#define SCREEN_Y 25
#define BIG_BUFF 32
#define SMALL_BUFF 8
#define BLUE (BGND_BLUE | FGND_WHITE)
#define RED (BGND_RED | FGND_WHITE)
#define GREEN (BGND_GREEN | FGND_WHITE)
#define BLACK (BGND_BLACK | FGND_WHITE)

void tick(unsigned int numTicks);
void game_init();
void display_prompts();
void set_block(unsigned int r,unsigned int c,int color);
void set_game_cursor(int r,int c,char ch);
void render_mesh();
void deleteblocks(int row, int col,int init);
void display_string(char *str,int row,int col,int color);
void compact();
void home_screen();
void complete_screen();
void home_prompts();
void instruction_screen();
void game_run();
int game_complete();

unsigned int color_arr[NUM_ROW][NUM_COL];
unsigned int score;
unsigned int row_pos;
unsigned int game_seed;
int last_color;
int game_time;
int high_score;
enum game_state {exit,pause,resume,complete} curr_state;
int selected_area_size;
int combo_multiplier;
int combo_color;
unsigned int col_pos;

void game_run()
{
  hide_cursor();
  curr_state=exit;
  home_screen();
  do {
    game_init();
    if(curr_state==complete)
      complete_screen();
    if(curr_state==exit) {
      home_screen();
    }
  }while(curr_state!=exit);
}
void complete_screen()
{
  int pos;
  char prompt[BIG_BUFF];
  snprintf(prompt,BIG_BUFF,"Congratulations!!");
  pos=(NUM_COL*2)-15;
  display_string(prompt,0,pos,BLUE);
  pos+=strlen(prompt)+1;
  snprintf(prompt,BIG_BUFF,"Game");
  display_string(prompt,0,pos,RED);
  pos+=strlen(prompt)+1;
  snprintf(prompt,BIG_BUFF,"Over");
  display_string(prompt,0,pos,GREEN);
  display_string("'r' to restart",NUM_ROW-1,NUM_COL*2,BLACK);
  display_string("'e' to exit",NUM_ROW,NUM_COL*2,BLACK);
  char c;
  while(1) {
    c=readchar();
    if(c=='r') {
      curr_state=resume;
      return;
    }
    if(c=='e') {
      curr_state=exit;
      return;
    }
  }
}
void render_mesh()
{
  int i,j;
  for(i=0;i<NUM_ROW;i++) {
    for(j=0;j<NUM_COL;j++) {
      set_block(i,j,color_arr[i][j]);
    }
  }
}
void instruction_screen()
{
  char prompt[BIG_BUFF];
  int y_pos;
  int x_pos;
  set_term_color(BLACK);
  y_pos=(SCREEN_Y/2)-6;
  x_pos=SCREEN_X/2;
  clear_console();
  snprintf(prompt,BIG_BUFF,"!!Instructions!!");
  display_string(prompt,y_pos,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"+---+");
  display_string(prompt,y_pos+2,x_pos-5,BLACK);
  snprintf(prompt,BIG_BUFF,"| W |");
  display_string(prompt,y_pos+3,x_pos-5,BLACK);
  snprintf(prompt,BIG_BUFF,"+---+");
  display_string(prompt,y_pos+4,x_pos-5,BLACK);
  snprintf(prompt,BIG_BUFF,"+---++---++---+");
  display_string(prompt,y_pos+5,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"TO MOVE CURSOR");
  display_string(prompt,y_pos+6,x_pos-30,BLACK);
  snprintf(prompt,BIG_BUFF,"| A || S || D |");
  display_string(prompt,y_pos+6,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"+---++---++---+");
  display_string(prompt,y_pos+7,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"+-------------+");
  display_string(prompt,y_pos+8,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"|    SPACE    |");
  display_string(prompt,y_pos+9,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"TO DELETE BLOCK");
  display_string(prompt,y_pos+9,x_pos-30,BLACK);
  snprintf(prompt,BIG_BUFF,"+-------------+");
  display_string(prompt,y_pos+10,x_pos-10,BLACK);
  snprintf(prompt,BIG_BUFF,"'p' to PAUSE, 'r' to RESUME");
  display_string(prompt,y_pos+11,x_pos-30,BLACK);
  snprintf(prompt,BIG_BUFF,"'e' to EXIT");
  display_string(prompt,y_pos+12,x_pos-30,BLACK);
  if(game_time) {
    snprintf(prompt,BIG_BUFF,"Current time: ");
    display_string(prompt,SCREEN_Y-2,1,BLACK);
    snprintf(prompt,BIG_BUFF,"Score: ");
    x_pos=strlen(prompt);
    display_string(prompt,SCREEN_Y-2,(NUM_COL*4)-11,BLACK);
  }
  snprintf(prompt,BIG_BUFF,"Press 'y' to go back.");
  display_string(prompt,SCREEN_Y-1,1,BLACK);
  snprintf(prompt,BIG_BUFF,"Sohil Habib 2014");
  x_pos=strlen(prompt);
  display_string(prompt,SCREEN_Y-1,SCREEN_X-(x_pos+2),BLACK);
}
void home_screen()
{
  char c;
  home_prompts();
  score=0;
  game_time=0;
  while(1)
  {
    c=readchar();
    if(c=='x') {
      instruction_screen();
      while(readchar()!='y');
      clear_console();
      home_prompts();
      continue;
    }
    if(c=='s') {
      curr_state=resume;
      return;
    }
  }
}
void home_prompts()
{
  char prompt[BIG_BUFF];
  set_term_color(BLACK);
  clear_console();
  snprintf(prompt,BIG_BUFF,"!!SAME GAME!!");
  display_string(prompt,SCREEN_Y/2,SCREEN_X/2-10,BLACK);
  snprintf(prompt,BIG_BUFF,"HIGHSCORE: %d",high_score);
  display_string(prompt,SCREEN_Y/2+1,SCREEN_X/2-10,BLACK);
  snprintf(prompt,BIG_BUFF,"'s' to Start");
  display_string(prompt,SCREEN_Y/2+3,SCREEN_X/2-10,BLACK);
  display_prompts();
}
void game_start()
{
  char c;char combo[SMALL_BUFF]="";
  while(1){
    c=readchar();
    if(c=='w') {
      if(row_pos-1==-1)
        continue;
      set_game_cursor(row_pos,col_pos,'\0');
      row_pos--;
      set_game_cursor(row_pos,col_pos,'|');
    }
    if(c=='a') {
      if(col_pos-1==-1)
        continue;
      set_game_cursor(row_pos,col_pos,'\0');
      col_pos--;
      set_game_cursor(row_pos,col_pos,'|');
    }
    if(c=='s') {
      if(row_pos+1==10)
        continue;
      set_game_cursor(row_pos,col_pos,'\0');
      row_pos++;
      set_game_cursor(row_pos,col_pos,'|');
    }
    if(c=='d') {
      if(col_pos+1==15)
        continue;
      set_game_cursor(row_pos,col_pos,'\0');
      col_pos++;
      set_game_cursor(row_pos,col_pos,'|');
    }
    if(c==' ') {
      int curr_color=color_arr[row_pos][col_pos];
      if(curr_color==BLACK)
        continue;
      deleteblocks(row_pos,col_pos,0);
      if(selected_area_size) {
        compact();
        selected_area_size++;
        if(last_color==curr_color) {
          combo_color=curr_color;
          combo_multiplier++;
        }
        else {
          last_color=curr_color;
          char *temp=combo;
          while((*temp)!='\0') {
            *temp=' ';temp++;
          }
          display_string(combo,(SCREEN_Y/2)+1,SCREEN_X-6,BLACK);
          combo_color=curr_color;
          combo_multiplier=1;
        }
        score+=selected_area_size*combo_multiplier;
        snprintf(combo,SMALL_BUFF,"%dX",combo_multiplier);
        display_string(combo,(SCREEN_Y/2)+1,SCREEN_X-6,combo_color);
        selected_area_size=0;
        render_mesh();
        if(!game_complete())
          set_game_cursor(row_pos,col_pos,'|');
        else {
          set_game_cursor(row_pos,col_pos,'\0');
          break;
        }
      }
    }
    if(c=='x') {
      instruction_screen();
      curr_state=pause;
      while(readchar()!='y');
      set_term_color(BLACK);
      clear_console();
      curr_state=resume;
      render_mesh();
      display_prompts();
      set_game_cursor(row_pos,col_pos,'|');
      display_string(combo,(SCREEN_Y/2)+1,SCREEN_X-6,combo_color);
      continue;
    }
    if(c=='p') {
      curr_state=pause;
      set_game_cursor(row_pos,col_pos,'\0');
      display_string("PAUSED, press 'r' to RESUME",0,(SCREEN_X/2)-5,RED);
      while(readchar()!='r');
      curr_state=resume;
      set_term_color(BLACK);
      clear_console();
      render_mesh();
      display_prompts();
      set_game_cursor(row_pos,col_pos,'|');
      display_string(combo,(SCREEN_Y/2)+1,SCREEN_X-6,combo_color);
      continue;
    }
    if(c=='e')
    {
      curr_state=exit;
      return;
    }
  }
}
void game_init()
{
  set_term_color(BLACK);
  clear_console();
  int color,i,j;
  for(i=0;i<NUM_ROW;i++) {
    for(j=0;j<NUM_COL;j++) {
      color=sgenrand(game_seed)%3;
      if(color==0)
        color_arr[i][j]=BLUE;
      if(color==1)
        color_arr[i][j]=RED;
      if(color==2)
        color_arr[i][j]=GREEN;
      set_block(i,j,color_arr[i][j]);
    }
  }
  score=0;
  game_time=0;
  display_prompts();
  row_pos=0;col_pos=0;
  set_game_cursor(row_pos,col_pos,'|');
  last_color=-1;
  combo_color=-1;
  selected_area_size=0;
  combo_multiplier=1;
  game_start();
  return;
}
void set_game_cursor(int r,int c,char ch)
{
  set_term_color(color_arr[r][c]);
  r*=2;c*=4;
  c+=2;r+=2;
  set_cursor(r,c+1);
  putbyte(ch);
  putbyte(ch);
  set_cursor(r+1,c+1);
  putbyte(ch);
  putbyte(ch);

}
void display_prompts()
{
  char prompt[BIG_BUFF];
  int x_pos;
  if(curr_state!=exit) {
    snprintf(prompt,BIG_BUFF,"Current time: ");
    display_string(prompt,SCREEN_Y-2,1,BLACK);
    snprintf(prompt,BIG_BUFF,"Score: ");
    x_pos=strlen(prompt);
    display_string(prompt,SCREEN_Y-2,(NUM_COL*4)-11,BLACK);
    snprintf(prompt,BIG_BUFF,"!!MULTIPLIER!!");
    x_pos=strlen(prompt);
    display_string(prompt,SCREEN_Y/2,SCREEN_X-(x_pos+2),BLACK);
  }
  snprintf(prompt,BIG_BUFF,"Press 'x' to show instructions");
  display_string(prompt,SCREEN_Y-1,1,BLACK);
  snprintf(prompt,BIG_BUFF,"Sohil Habib 2014");
  x_pos=strlen(prompt);
  display_string(prompt,SCREEN_Y-1,SCREEN_X-(x_pos+2),BLACK);
}
void display_string(char *str,int row,int col,int color)
{
  set_term_color(color);
  set_cursor(row,col);
  putbytes(str,strlen(str));
}
void set_block(unsigned int r,unsigned int c,int color)
{
  int i,j;
  r*=2;c*=4;
  c+=2;r+=2;
  for(i=0;i<2;i++)
    for(j=0;j<4;j++)
      draw_char(r+i,c+j,'\0',color);
}
/** @brief Tick function, to be called by the timer interrupt handler
 *
 *  In a real game, this function would perform processing which
 *  should be invoked by timer interrupts.
 *
 **/
void tick(unsigned int numTicks)
{
  if(curr_state==resume) {
    game_time++;
  }
  game_seed=numTicks;
  int sec=game_time/100;
  int msec=game_time%100;
  char buf[BIG_BUFF];
  int color;
  get_term_color(&color);
  if(game_time) {
    snprintf(buf,BIG_BUFF,"%d.%ds",sec,msec);
    display_string(buf,SCREEN_Y-2,15,BLACK);
    snprintf(buf,BIG_BUFF,"%d",score);
    display_string(buf,SCREEN_Y-2,(NUM_COL*4)-4,BLACK);
  }
  set_term_color(color);
}
void compact()
{
  int i,j,k;
  for(i=0;i<NUM_COL;i++) {
    for(j=NUM_ROW-1;j>0;j--) {
      if(color_arr[j][i]==BLACK) {
        for(k=j-1;k>=0;k--) {
          if(color_arr[k][i]!=BLACK) {
            color_arr[j][i]=color_arr[k][i];
            color_arr[k][i]=BLACK;
            j--;
          }
        }
        break;
      }
    }
  }
  int count;
  for(i=NUM_COL-1;i>0;i--) {
    count=0;
    for(j=0;j<NUM_ROW;j++) {
      if(color_arr[j][i]==BLACK) {
        count++;
      }
    }
    if(count==NUM_ROW) {
      for(k=i-1;k>=0;k--) {
        count=0;
        for(j=0;j<NUM_ROW;j++) {
          if(color_arr[j][k]==BLACK) {
            count++;
          }
        }
        if(count!=NUM_ROW) {
          for(j=0;j<NUM_ROW;j++) {
            color_arr[j][i]=color_arr[j][k];
            color_arr[j][k]=BLACK;
          }
          i--;
        }
      }
      break;
    }
  }
}
int game_complete()
{
  int row,col;
  for(col = NUM_COL-1; col >=0; col--)
  {
    for(row = NUM_ROW-1; row > 0; row--)
    {
      int color = color_arr[row][col];
      if(color == BLACK)
        break;
      else
      {
        if(color_arr[row - 1][col] == color)
          return 0;
        else if(col-1 >= 0)
          if(color_arr[row][col-1] == color)
            return 0;
      }
    }
  }
  if(score>high_score)
    high_score=score;
  curr_state=complete;
  return 1;

}
void deleteblocks(int row, int col,int init)
{
  if(row < 0 || row >= NUM_ROW || col < 0 || col >= NUM_COL)
    return;
  int color = color_arr[row][col];
  if(init) {
    color_arr[row][col] = BLACK;
    selected_area_size++;
  }
  if(row - 1 >= 0) {
    if(color_arr[row - 1][col] == color) {
      color_arr[row][col] = BLACK;
      deleteblocks(row - 1, col,1);
    }
  }
  if(row + 1 < NUM_ROW) {
    if(color_arr[row + 1][col] == color) {
      color_arr[row][col]= BLACK;
      deleteblocks(row + 1,col,1);
    }
  }
  if(col - 1 >= 0) {
    if(color_arr[row][col - 1] == color) {
      color_arr[row][col] = BLACK;
      deleteblocks(row, col - 1,1);
    }
  }
  if(col + 1 < NUM_COL) {
    if(color_arr[row][col + 1] == color) {
      color_arr[row][col] = BLACK;
      deleteblocks(row, col + 1,1);
    }
  }
}
