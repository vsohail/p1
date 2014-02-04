/** @file kbd.h
 *  @brief function definition for the keyboard wrapper
 *
 *  @author Sohil Habib (snhabib)
 */

/** @brief kbd_wrapper is the wrapper for the keyboard handler
 *
 *  The wrapper saves the current state on stack,
 *  calls the handler, pops the old state and then returns.
 *
 *  @return Void.
 */
void kbd_wrapper();
