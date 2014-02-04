/** @file handler_install.c
 *  @brief This file initializes the install handlers
 *
 *  Sets the IDT's with appropriate values and also intializes
 *  the buffer used by the keyboard. Contains the declarations of
 *  the insert and remove keyboard buffer functions.
 *
 *  @author Sohil Habib (snhabib)
 *  @bug No known bugs.
 */

/* necessary includes */
#include <p1kern.h>
#include <stdio.h>
#include <timer.h>
#include <kbd.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>
#include <x86/keyhelp.h>
#include <console.h>
#include <x86/seg.h>

/* size of the keyboard buffer */
#define MAX_SIZE 256

#define LOWER_HALF 0xFFFF
#define UPPER_HALF 0xFFFF0000
#define TRAP_GATE_DEFAULT 0x8F

/* fucntion pointer to tick function */
void (*tick_addr)(unsigned int);

/* front of the queue */
unsigned int front;

/* rear of the queue */
unsigned int rear;

/* current size of th queue */
unsigned int curr_size;

/* the keyboard buffer */
unsigned int key_history[MAX_SIZE];

unsigned int remove_q();
unsigned int insert_q(unsigned int val);

/** @brief insert_q inserts the value into queue buffer
 *
 *  the code takes care of the condition wherin buffer is full and
 *  overwrites the oldest value. rest all functions are like a
 *  normal queue.
 *
 *  @param val value to insert
 *  @return unsigned returns the value inserted
 */
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

/** @brief remove_q remove the value from the queue buffer
 *
 *  all functions are like a
 *  normal queue.
 *
 *  @return unsigned returns the value deleted
 */
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

/** @brief installs and sets up the timer,keyboard handlers and the
 *         console
 *
 *  @return int 0 on success
 */
int handler_install(void (*tickback)(unsigned int))
{
  // initialize the console
  console_init();

  // backup address of tick
  tick_addr=tickback;

  // setup timer IDT
  void *base;
  base=(idt_base()+(TIMER_IDT_ENTRY)*8);
  *(unsigned *)base=((SEGSEL_KERNEL_CS<<16) | (((unsigned)timer_wrapper)&LOWER_HALF));
  base += 4;
  *(unsigned *)base=(((unsigned)timer_wrapper)&UPPER_HALF) | (TRAP_GATE_DEFAULT<<8);

  // initialize timer wave type and value
  outb(TIMER_MODE_IO_PORT,TIMER_SQUARE_WAVE);
  outb(TIMER_PERIOD_IO_PORT,(TIMER_RATE/100)&0xFF);
  outb(TIMER_PERIOD_IO_PORT,((TIMER_RATE/100)&0xFF00)>>8);

  // initialize queue buffer defaults
  front=0;
  rear=-1;
  curr_size=0;

  // initialize keyboard IDT
  base=(idt_base()+(KEY_IDT_ENTRY)*8);
  *(unsigned *)base=((SEGSEL_KERNEL_CS<<16) | (((unsigned)kbd_wrapper)&LOWER_HALF));
  base += 4;
  *(unsigned *)base=(((unsigned)kbd_wrapper)&UPPER_HALF) | (TRAP_GATE_DEFAULT<<8);

  return 0;
}
