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
/* Think about where this declaration
 * should be... probably not here!
 */

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */
#include <malloc.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* memory includes. */
#include <lmm.h>                    /* lmm_remove_free() */

/* x86 specific includes */
#include <x86/seg.h>                /* install_user_segs() */
#include <x86/interrupt_defines.h>  /* interrupt_setup() */
#include <x86/asm.h>                /* enable_interrupts() */
#include <RNG/mt19937int.h>
#include <string.h>

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
unsigned int score;
unsigned int row_pos;
int last_color;
int game_time;
int high_score;
enum game_state {exit,pause,resume,complete} curr_state;
int selected_area_size;
int combo_multiplier;
int combo_color;
unsigned int col_pos;
unsigned int color_arr[10][15];
#define BLUE (BGND_BLUE | FGND_WHITE)
#define RED (BGND_RED | FGND_WHITE)
#define GREEN (BGND_GREEN | FGND_WHITE)
#define BLACK (BGND_BLACK | FGND_WHITE)
/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.  It simply sets up the
 *  drivers and passes control off to game_run().
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
  /*
   * Initialize device-driver library.
   */
  handler_install(tick);
  enable_interrupts();
  game_run();
  return 0;
}
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
  display_string("Congratulations!!",0,15,BLUE);
  display_string("Game",0,34,RED);
  display_string("Over",0,40,GREEN);
  display_string("'r' to restart",9,30,BLACK);
  display_string("'e' to exit",10,30,BLACK);
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
  int color,i,j;
  for(i=0;i<10;i++) {
    for(j=0;j<15;j++) {
      color=color_arr[i][j];
      if(color==-1)
        set_block(i,j,BLACK);
      if(color==0)
        set_block(i,j,BLUE);
      if(color==1)
        set_block(i,j,RED);
      if(color==2)
        set_block(i,j,GREEN);
    }
  }
}
void instruction_screen()
{
  char prompt[32];
  set_term_color(BLACK);
  clear_console();
  snprintf(prompt,32,"!!Instructions!!");
  display_string(prompt,6,30,BLACK);
  snprintf(prompt,32,"+---+");
  display_string(prompt,8,35,BLACK);
  snprintf(prompt,32,"| W |");
  display_string(prompt,9,35,BLACK);
  snprintf(prompt,32,"+---+");
  display_string(prompt,10,35,BLACK);
  snprintf(prompt,32,"+---++---++---+");
  display_string(prompt,11,30,BLACK);
  snprintf(prompt,32,"TO MOVE CURSOR");
  display_string(prompt,11,10,BLACK);
  snprintf(prompt,32,"| A || S || D |");
  display_string(prompt,12,30,BLACK);
  snprintf(prompt,32,"+---++---++---+");
  display_string(prompt,13,30,BLACK);
  snprintf(prompt,32,"+-------------+");
  display_string(prompt,16,30,BLACK);
  snprintf(prompt,32,"|    SPACE    |");
  display_string(prompt,17,30,BLACK);
  snprintf(prompt,32,"TO DELETE BLOCK");
  display_string(prompt,17,10,BLACK);
  snprintf(prompt,32,"+-------------+");
  display_string(prompt,18,30,BLACK);
  snprintf(prompt,32,"'p' to PAUSE, 'r' to RESUME");
  display_string(prompt,19,10,BLACK);
  snprintf(prompt,32,"'e' to EXIT");
  display_string(prompt,20,10,BLACK);
  if(game_time) {
    snprintf(prompt,32,"Current time: ");
    display_string(prompt,23,1,BLACK);
    snprintf(prompt,32,"Score: ");
    display_string(prompt,23,49,BLACK);
  }
  snprintf(prompt,32,"Press 'y' to go back.");
  display_string(prompt,24,1,BLACK);
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
  char prompt[32];
  set_term_color(BLACK);
  clear_console();
  snprintf(prompt,32,"!!SAME GAME!!");
  display_string(prompt,6,30,BLACK);
  snprintf(prompt,32,"HIGHSCORE: %d",high_score);
  display_string(prompt,7,30,BLACK);
  snprintf(prompt,32,"'s' to Start");
  display_string(prompt,9,30,BLACK);
  display_prompts();
}
void game_start()
{
  char c;char combo[8]="";
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
      if(curr_color==-1)
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
          display_string(combo,13,67,BLACK);
          combo_color=curr_color;
          combo_multiplier=1;
        }
        score+=selected_area_size*combo_multiplier;
        snprintf(combo,8,"%dX",combo_multiplier);
        if(curr_color==0) {
          display_string(combo,13,67,BLUE);
        }
        if(curr_color==1) {
          display_string(combo,13,67,RED);
        }
        if(curr_color==2) {
          display_string(combo,13,67,GREEN);
        }
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
      if(combo_color==0) {
        display_string(combo,13,67,BLUE);
      }
      if(combo_color==1) {
        display_string(combo,13,67,RED);
      }
      if(combo_color==2) {
        display_string(combo,13,67,GREEN);
      }
      continue;
    }
    if(c=='p') {
      curr_state=pause;
      set_game_cursor(row_pos,col_pos,'\0');
      display_string("PAUSED, press 'r' to RESUME",0,35,RED);
      while(readchar()!='r');
      curr_state=resume;
      set_term_color(BLACK);
      clear_console();
      render_mesh();
      display_prompts();
      set_game_cursor(row_pos,col_pos,'|');
      if(combo_color==0) {
        display_string(combo,13,67,BLUE);
      }
      if(combo_color==1) {
        display_string(combo,13,67,RED);
      }
      if(combo_color==2) {
        display_string(combo,13,67,GREEN);
      }
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
  for(i=0;i<10;i++) {
    for(j=0;j<15;j++) {
      color=genrand()%3;
      if(color==0)
        set_block(i,j,BLUE);
      if(color==1)
        set_block(i,j,RED);
      if(color==2)
        set_block(i,j,GREEN);
      color_arr[i][j]=color;
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
  if(color_arr[r][c]==0)
    set_term_color(BLUE);
  else if(color_arr[r][c]==1)
    set_term_color(RED);
  else if(color_arr[r][c]==2)
    set_term_color(GREEN);
  else if(color_arr[r][c]==-1)
    set_term_color(BLACK);
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
  char prompt[32];
  if(curr_state!=exit) {
    snprintf(prompt,32,"Current time: ");
    display_string(prompt,23,1,BLACK);
    snprintf(prompt,32,"Score: ");
    display_string(prompt,23,49,BLACK);
    snprintf(prompt,32,"!!MULTIPLIER!!");
    display_string(prompt,12,63,BLACK);
  }
  snprintf(prompt,32,"Press 'x' for intructions");
  display_string(prompt,24,1,BLACK);
  snprintf(prompt,32,"Sohil Habib 2014");
  display_string(prompt,24,62,BLACK);
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
  int sec=game_time/100;
  int msec=game_time%100;
  char buf[32];
  int color;
  get_term_color(&color);
  if(game_time) {
    snprintf(buf,32,"%d.%ds",sec,msec);
    display_string(buf,23,15,BLACK);
    snprintf(buf,32,"%d",score);
    display_string(buf,23,56,BLACK);
  }
  set_term_color(color);
}
void compact()
{
  int i,j,k;
  for(i=0;i<15;i++) {
    for(j=9;j>0;j--) {
      if(color_arr[j][i]==-1) {
        for(k=j-1;k>=0;k--) {
          if(color_arr[k][i]!=-1) {
            color_arr[j][i]=color_arr[k][i];
            color_arr[k][i]=-1;
            j--;
          }
        }
        break;
      }
    }
  }
  int count;
  for(i=14;i>0;i--) {
    count=0;
    for(j=0;j<10;j++) {
      if(color_arr[j][i]==-1) {
        count++;
      }
    }
    if(count==10) {
      for(k=i-1;k>=0;k--) {
        count=0;
        for(j=0;j<10;j++) {
          if(color_arr[j][k]==-1) {
            count++;
          }
        }
        if(count!=10) {
          for(j=0;j<10;j++) {
            color_arr[j][i]=color_arr[j][k];
            color_arr[j][k]=-1;
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
  for(col = 14; col >=0; col--)
  {
    for(row = 9; row > 0; row--)
    {
      int color = color_arr[row][col];
      if(color == -1)
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
  if(row < 0 || row >= 10 || col < 0 || col >= 15)
    return;
  int color = color_arr[row][col];
  if(init) {
    color_arr[row][col]=-1;
    selected_area_size++;
  }
  if(row - 1 >= 0) {
    if(color_arr[row - 1][col] == color) {
      color_arr[row][col]=-1;
      deleteblocks(row - 1, col,1);
    }
  }
  if(row + 1 < 10) {
    if(color_arr[row + 1][col] == color) {
      color_arr[row][col]=-1;
      deleteblocks(row + 1,col,1);
    }
  }
  if(col - 1 >= 0) {
    if(color_arr[row][col - 1] == color) {
      color_arr[row][col]=-1;
      deleteblocks(row, col - 1,1);
    }
  }
  if(col + 1 < 15) {
    if(color_arr[row][col + 1] == color) {
      color_arr[row][col]=-1;
      deleteblocks(row, col + 1,1);
    }
  }
}
