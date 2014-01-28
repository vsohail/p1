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
unsigned int score;
unsigned int row_pos;
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
  game_init();
  while (1) {
    continue;
  }

  return 0;
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
void game_run()
{
  char c;
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
  }
}
void game_init()
{
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
  hide_cursor();
  display_prompts();
  row_pos=0;col_pos=0;
  set_game_cursor(row_pos,col_pos,'|');
  game_run();
}
void set_game_cursor(int r,int c,char ch)
{
  if(color_arr[r][c]==0)
    set_term_color(BLUE);
  else if(color_arr[r][c]==1)
    set_term_color(RED);
  else if(color_arr[r][c]==2)
    set_term_color(GREEN);
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
  set_term_color(BLACK);
  char prompt[25];
  set_cursor(23,1);
  sprintf(prompt,"Current time: ");
  putbytes(prompt,strlen(prompt));
  set_cursor(23,49);
  sprintf(prompt,"Score: ");
  putbytes(prompt,strlen(prompt));
  set_cursor(24,1);
  sprintf(prompt,"Press F1 for intructions.");
  putbytes(prompt,strlen(prompt));
  set_cursor(24,62);
  sprintf(prompt,"Sohil Habib 2014");
  putbytes(prompt,strlen(prompt));
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
  int sec=numTicks/100;
  int msec=numTicks%100;
  char buf[10];
  int color;
  get_term_color(&color);
  set_term_color(BLACK);
  sprintf(buf,"%d.%ds",sec,msec);
  set_cursor(23,15);
  putbytes(buf,strlen(buf));
  sprintf(buf,"%d",score);
  set_cursor(23,56);
  putbytes(buf,strlen(buf));
  set_term_color(color);
}
