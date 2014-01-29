#include <p1kern.h>
#include <stdio.h>
#include <timer.h>
#include <kbd.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>
#include <x86/keyhelp.h>
#include <console.h>
#include <x86/seg.h>
#include <simics.h>
#define MAX_SIZE 256

void (*tick_addr)(unsigned int);
unsigned int front;
unsigned int rear;
unsigned int curr_size;
unsigned int remove_q();
unsigned int insert_q(unsigned int val);
unsigned int key_history[MAX_SIZE];

unsigned int insert_q(unsigned int val)
{
  disable_interrupts();
  if((++curr_size)==MAX_SIZE+1) {
    curr_size--;
    key_history[front]=val;
    front=(front+1)%MAX_SIZE;
    rear=(rear+1)%MAX_SIZE;
    enable_interrupts();
    return val;
  }
  rear=(rear+1)%MAX_SIZE;
  key_history[rear]=val;
  enable_interrupts();
  return val;
}

unsigned int remove_q()
{
  disable_interrupts();
  unsigned int val;
  if((--curr_size)==-1) {
    curr_size++;
    enable_interrupts();
    return -1;
  }
  val=key_history[front];
  front=(front+1)%MAX_SIZE;
  enable_interrupts();
  return val;
}

int handler_install(void (*tickback)(unsigned int))
{
  console_init();

  tick_addr=tickback;
  void *base;
  base=(idt_base()+(TIMER_IDT_ENTRY)*8);
  *(unsigned *)base=((SEGSEL_KERNEL_CS<<16) | (((unsigned)timer_wrapper)&0xFFFF));
  base += 4;
  *(unsigned *)base=(((unsigned)timer_wrapper)&0xFFFF0000) | (0x8F<<8);
  outb(TIMER_MODE_IO_PORT,TIMER_SQUARE_WAVE);
  outb(TIMER_PERIOD_IO_PORT,0x9C);
  outb(TIMER_PERIOD_IO_PORT,0x2E);

  front=0;
  rear=-1;
  curr_size=0;
  base=(idt_base()+(KEY_IDT_ENTRY)*8);
  *(unsigned *)base=((SEGSEL_KERNEL_CS<<16) | (((unsigned)kbd_wrapper)&0xFFFF));
  base += 4;
  *(unsigned *)base=(((unsigned)kbd_wrapper)&0xFFFF0000) | (0x8F<<8);

  enable_interrupts();

  return 0;
}
