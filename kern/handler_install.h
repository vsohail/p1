void (*tick_addr)(unsigned int);
unsigned int key_history[256];
unsigned int front;
unsigned int rear;
unsigned int max_size;
unsigned int curr_size;
unsigned int remove_q();
unsigned int insert_q(unsigned int val);
int handler_install(void (*tickback)(unsigned int));
