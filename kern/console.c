/** @file console.c
 *  @brief the console driver definitions.
 *
 *  This file contains all the console driver definitions
 *  as per the declarations in p1kern.h. All the function details
 *  are specified in the p1kern.h file
 *
 *  @author Sohil Habib (snhabib)
 *  @bug No known bugs.
 */

#include <p1kern.h>
#include <stdio.h>
#include <video_defines.h>
#include <x86/asm.h>
#include <string.h>

/*
 * The position and state of the cursor are
 * maintained by these variables variable.
 */
static unsigned int row_pos;
static unsigned int col_pos;
static unsigned int is_hidden;


/* Maintains the color of the console window. */
static unsigned int console_color;

void console_init()
{
  is_hidden=0;
  row_pos=0;
  col_pos=0;
  set_term_color(FGND_WHITE | BGND_BLACK);
  clear_console();
  set_cursor(0,0);
}

int putbyte( char ch )
{
  if(ch=='\b') {
    if(col_pos==0) {
      if(row_pos==0)
        return ch;
      col_pos=CONSOLE_WIDTH-1;
      row_pos--;
    }
    else
      col_pos--;
    draw_char(row_pos,col_pos,'\0',console_color);
    set_cursor(row_pos,col_pos);
    return ch;
  }
  if(ch=='\r') {
    col_pos=0;
    set_cursor(row_pos,col_pos);
    return ch;
  }
  if(ch!='\n') {
    *(char *)(CONSOLE_MEM_BASE + 2*(row_pos*CONSOLE_WIDTH+col_pos))=ch;
    *(char *)(CONSOLE_MEM_BASE + 2*(row_pos*CONSOLE_WIDTH+col_pos)+1)=console_color;
  }
  else
    col_pos=CONSOLE_WIDTH-1;
  if((++col_pos)==CONSOLE_WIDTH) {
    col_pos=0;
    row_pos++;
  }
  if(row_pos==CONSOLE_HEIGHT) {
    row_pos=CONSOLE_HEIGHT-1;
    memmove((void *)CONSOLE_MEM_BASE,(const void *)(CONSOLE_MEM_BASE+CONSOLE_WIDTH),2*CONSOLE_WIDTH*(CONSOLE_HEIGHT-1));
  }
  set_cursor(row_pos,col_pos);
  return ch;
}

void putbytes( const char *s, int len )
{
  for(;len>0;len--) {
    putbyte(*s);
    s++;
  }
}

int set_term_color( int color )
{
  if((unsigned)color>(BLINK | BGND_BLACK | FGND_WHITE))
    return -1;
  console_color=color;
  return 0;
}

void get_term_color( int *color )
{
  *color=console_color;
}

int set_cursor( int row, int col )
{
  if((unsigned)row>=CONSOLE_HEIGHT || (unsigned)col>=CONSOLE_WIDTH)
    return -1;
  row_pos=row;
  col_pos=col;
  if(!is_hidden) {
    int cursor_pos=row*CONSOLE_WIDTH + col;
    outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
    outb(CRTC_DATA_REG,(cursor_pos>>8));
    outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
    outb(CRTC_DATA_REG,(cursor_pos & 0x00FF));
  }
  return 0;
}

void get_cursor( int *row, int *col )
{
  *row=row_pos;
  *col=col_pos;
}

void hide_cursor()
{
  int cursor_pos=CONSOLE_HEIGHT*CONSOLE_WIDTH + CONSOLE_WIDTH;
  outb(CRTC_IDX_REG,CRTC_CURSOR_MSB_IDX);
  outb(CRTC_DATA_REG,(cursor_pos>>8));
  outb(CRTC_IDX_REG,CRTC_CURSOR_LSB_IDX);
  outb(CRTC_DATA_REG,(cursor_pos & 0x000F));
  is_hidden=1;
}

void show_cursor()
{
  is_hidden=0;
  set_cursor(row_pos,col_pos);
}

void clear_console()
{
  int row,col;
  for(row=0;row<CONSOLE_HEIGHT;row++) {
    for(col=0;col<CONSOLE_WIDTH;col++) {
      *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH+col))='\0';
      *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH+col)+1)=console_color;
    }
  }
  set_cursor(0,0);
}

void draw_char( int row, int col, int ch, int color )
{
  if((unsigned)row>=CONSOLE_HEIGHT || (unsigned)col>=CONSOLE_WIDTH || (unsigned)color>(BLINK | BGND_BLACK | FGND_WHITE))
    return;
  *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH+col))=ch;
  *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH+col)+1)=color;
}

char get_char( int row, int col )
{
  return *(char *)(CONSOLE_MEM_BASE + 2*(row*CONSOLE_WIDTH+col));
}
