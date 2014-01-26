#include <p1kern.h>
#include <stdio.h>
#include <timer.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>
void *tick_addr;
int handler_install(void (*tickback)(unsigned int))
{
  tick_addr=tickback;
  *(idt_base()+TIMER_IDT_ENTRY)=timer_wrapper;
  outb(TIMER_MODE_IO_PORT,TIMER_SQUARE_WAVE);
  outb(TIMER_PERIOD_IO_PORT,0x9C);
  outb(TIMER_PERIOD_IO_PORT,0x2E);
  return 0;
}
