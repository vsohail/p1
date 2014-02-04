/** @file timer.h
 *  @brief function definition for the timer wrapper
 *
 *  @author Sohil Habib (snhabib)
 */

/** @brief timer_wrapper is the wrapper for the timer handler
 *
 *  The wrapper saves the current state on stack,
 *  calls the handler, pops the old state and then returns.
 *
 *  @return Void.
 */
void timer_wrapper();
