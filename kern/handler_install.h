void (*tick_addr)(unsigned int);
unsigned int remove_q();
unsigned int insert_q(unsigned int val);
int handler_install(void (*tickback)(unsigned int));
