/** @file handler_install.h
 *  @brief function definition for the buffer insert,delete
 *         and the handler installer
 *
 *  @author Sohil Habib (snhabib)
 */

void (*tick_addr)(unsigned int);
unsigned int remove_q();
unsigned int insert_q(unsigned int val);
int handler_install(void (*tickback)(unsigned int));
