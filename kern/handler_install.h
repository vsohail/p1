#define MAX_SIZE 256

void (*tick_addr)(unsigned int);
unsigned int key_history[MAX_SIZE];
unsigned int front;
unsigned int rear;
unsigned int max_size;
unsigned int curr_size;
unsigned int remove_q();
unsigned int insert_q(unsigned int val);
int handler_install(void (*tickback)(unsigned int));
