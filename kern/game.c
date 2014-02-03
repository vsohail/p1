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

#include <handler_install.h>
#include <game_controller.h>

/* libc includes. */
#include <stdio.h>

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* memory includes. */
#include <lmm.h>                    /* lmm_remove_free() */

/* x86 specific includes */
#include <x86/seg.h>                /* install_user_segs() */
#include <x86/interrupt_defines.h>  /* interrupt_setup() */
#include <x86/asm.h>                /* enable_interrupts() */


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
  game_run();
  return 0;
}
