/** @file game_controller.c
 *  @brief The heart of the game. Contains all the routines
 *         necessary to startup, play and terminate a game.
 *
 *  This file contains the game entry point game_run,
 *  the game initializations, the game logic, and the
 *  various other functions and state variables needed for
 *  the functioning of the game.
 *
 *  @author Sohil Habib (snhabib)
 *  @bug No known bugs.
 */

#include <p1kern.h>
#include <video_defines.h>

/* libc includes. */
#include <stdio.h>

/* random function includes */
#include <RNG/mt19937int.h>

#include <string.h>

/* max number of rows in game mesh */
#define NUM_ROW 10

/* max number of columns in game mesh */
#define NUM_COL 15

/* screen max x coordinate */
#define SCREEN_X 80

/* screen max y coordinate */
#define SCREEN_Y 25

/* buffer size definitions */
#define BIG_BUFF 32
#define SMALL_BUFF 8

/* color definitions */
#define BLUE (BGND_BLUE | FGND_WHITE)
#define RED (BGND_RED | FGND_WHITE)
#define GREEN (BGND_GREEN | FGND_WHITE)
#define BLACK (BGND_BLACK | FGND_WHITE)

/* -- Function Definitions -- */
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

/* the array which maintains game mesh state */
unsigned int color_arr[NUM_ROW][NUM_COL];
/* the game score */
unsigned int score;
/* the row position of game cursor */
unsigned int row_pos;
/* the column position of game cursor */
unsigned int col_pos;
/* the seed to generate the random number for a game session */
unsigned int game_seed;
/* the color of the block last cleared */
int last_color;
/* in game time */
int game_time;
/* the high score!! */
int high_score;
/* the state of the game to effectively handle each case */
enum game_state {exit,pause,resume,complete} curr_state;
/* the area of the cleared block */
int selected_area_size;
/* the combo multiplier */
int combo_multiplier;
/* the color of the current combo multiplier */
int combo_color;

/** @brief game_run is the entry point of the game
 *
 *  It controls the state change from completed to exited
 *  and the initialization of each game session.
 *
 *  @return Void.
 */
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

/** @brief complete_screen is the screen shown to the user
 *         when the game completes.
 *
 *  This function runs when game_complete returns a true and
 *  changes the state of the game to complete. Moreover it
 *  freezes the game times and allows the user to either
 *  restart or exit the game.
 *
 *  @return Void.
 */
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

/** @brief render_mesh renders the game mesh.
 *
 *  This function is responsible for siplaying the
 *  color matrix on the screen after each user action.
 *
 *  @return Void.
 */
void render_mesh()
{
  int i,j;
  for(i=0;i<NUM_ROW;i++) {
    for(j=0;j<NUM_COL;j++) {
      set_block(i,j,color_arr[i][j]);
    }
  }
}

/** @brief instruction_screen renders the intruction
 *         string.
 *
 * This function is called when the user presses 'x'
 * and returns when the user presses 'y', It also pauses
 * the game.
 *
 * @return Void.
 */
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

/** @brief home_screen renders the game home screen logic.
 *
 *  It gives the user an option to either view the
 *  instructions or start a new game. Also shows the high
 *  score.
 *
 *  @return Void.
 */
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

/** @brief home_prompts is the function that displays the
 *         prompts shown on the home_screen
 *
 *  Separating the prompt from the logic provides flexibilty
 *  which changing screens.
 *
 *  @return Void.
 */
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

/** @brief game_start starts the game logic for a session
 *         of the game
 *
 *  Handles the movement of the game cursor in the mesh
 *  with the w,a,s,d keys and select a block with the space key.
 *  Allows the user to pause,resume and exit the game.
 *  Also contains the logic for instruction screen. This function
 *  also handles the scoring and the combo multiplier logic
 *
 *  @return Void.
 */
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

/** @brief game_init initilizes a game session.
 *
 *  The function resets the game state variables,
 *  generates a random mesh, populated with colors
 *  blue,red,green and then transfers control
 *  to game_start which starts the game.
 *
 *  @return Void.
 */
void game_init()
{
  set_term_color(BLACK);
  clear_console();
  int color,i,j;
  sgenrand((unsigned long)game_seed);
  for(i=0;i<NUM_ROW;i++) {
    for(j=0;j<NUM_COL;j++) {
      color=genrand()%3;
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

/** @brief set_game_cursor is resposible for moving
 *         the cursor to a specific position
 *
 *  @param r The row to move the cursor to
 *  @param c The column to move the cursor to
 *  @param ch The character to use for the cursor
 *  @return Void.
 */
void set_game_cursor(int r,int c,char ch)
{
  // The values of r,c are checked before they are sent
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

/** @brief display_prompts displays the prompts
 *         that are common to all the screens
 *
 *  The values to be displayed are handled by the
 *  state of the game.
 *
 *  @return Void.
 */
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

/** @brief wrapper function to display a string on the console.
 *
 *  Takes a string and its co-ordinates and color and prints it
 *  on the console at the appropriate position.
 *
 *  @param str The string pointe.r
 *  @param row The row number to put the string.
 *  @param col The column number to put the string.
 *  @param color The color to use while printing the string.
 *  @return Void.
 */
void display_string(char *str,int row,int col,int color)
{
  set_term_color(color);
  set_cursor(row,col);
  putbytes(str,strlen(str));
}

/** @brief sets a block to a particular color
 *         as per the game mesg requirements.
 *
 *  @return Void.
 */
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
 *  In a real game, this function would performs processing which
 *  should be invoked by timer interrupts. The function updates
 *  game score,time on screen and also helps initialize the seed
 *  Also performs differently for different game states.
 *
 *  @param numTicks the timer ticks
 */
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

/** @brief compact contains the logic to compact the block mesh
 *
 *  The function is divided into 2 halves
 *  - the falling of blocks
 *  - the column compaction
 *
 *  the first part seeks the first empty block in a column
 *  and keeps replacing all blocks from that empty block onwards
 *  with a filled block
 *
 *  the second part does the sma ething but with an empty
 *  column and then shifts the column inward(rightward in this case)
 *
 *  @return Void.
 */
void compact()
{
  int i,j,k;
  // block falling logic
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
  // column compaction algorithm
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

/** @brief contains the logic to detect game completion
 *
 *  apart from the obvious case of all the blocks being
 *  BLACK, the logic also checks every block adjacent to
 *  and above a certain block to not be of a certain color.
 *  It also updates the high score if necessary.
 *
 *  @return int 1 - if complete, 0 otherwise
 */
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

/** @brief contains the delete block logic
 *
 *  Apart from the first block, for every other block
 *  deletion is performed immediately. For each block,
 *  its adjacent(top,bottom,left,right) blocks are checked
 *  to be of same color and then recursively called on each
 *  such block
 *
 *  @param row The row position in mesh of the block selected
 *  @param col The col position in mesh of the block selected
 *  @param init Whether it is the initial block or no (0 for initial)
 *  @return Void.
 */
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
