#include <stdio.h>
#include <timer.h>
#include <handler_install.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>
#include <x86/interrupt_defines.h>
static unsigned int numTicks;
void timer_handler()
{
  numTicks++;
  tick_addr(numTicks);
  outb(INT_CTL_PORT,INT_ACK_CURRENT);
}
