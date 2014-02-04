/** @file timer_handler.c
 *  @brief the file contains the code for the timer handler
 *
 *  @author Sohil Habib (snhabib)
 *  @bug No known bugs.
 */

/* necessary includes */
#include <stdio.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>
#include <x86/interrupt_defines.h>
#include <handler_install.h>

/* the number of clock ticks*/
static unsigned int numTicks;

/** @brief the handler for the timer
 *
 *  incrememnts the number of ticks and sends it to tick_addr
 *  and then acknowledges the interrupt received
 *
 *  @return Void.
 */
void timer_handler()
{
  numTicks++;
  tick_addr(numTicks);
  outb(INT_CTL_PORT,INT_ACK_CURRENT);
}
