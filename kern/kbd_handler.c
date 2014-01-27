#include <stdio.h>
#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <x86/keyhelp.h>
#include <handler_install.h>
#include <p1kern.h>
#include <stdio.h>

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

void kbd_handler()
{
  insert_q((unsigned)ind(KEYBOARD_PORT));
  outb(INT_CTL_PORT,INT_ACK_CURRENT);
}
