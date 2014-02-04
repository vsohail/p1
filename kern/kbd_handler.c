/** @file kbd_handler.c
 *  @brief the file contains the code for the keyboard handler and the readchar api function
 *         needed to get input from kyboard
 *
 *  @author Sohil Habib (snhabib)
 *  @bug No known bugs.
 */

/* necessary includes */
#include <stdio.h>
#include <x86/asm.h>
#include <x86/keyhelp.h>
#include <handler_install.h>
#include <x86/interrupt_defines.h>
#include <p1kern.h>

int readchar(void)
{
  unsigned int val=remove_q();
  if((int)val==-1)
    return -1;
  int code=process_scancode(val);
  if(KH_HASDATA(code)) {
    if(KH_ISMAKE(code)) {
    return KH_GETCHAR(code);
    }
  }
  return -1;
}

/** @brief the handler for the keyboard
 *
 *  inserts the key entered into the buffer
 *  and acknowledges interrupt recieved
 *
 *  @return Void.
 */
void kbd_handler()
{
  insert_q((unsigned)ind(KEYBOARD_PORT));
  outb(INT_CTL_PORT,INT_ACK_CURRENT);
}
