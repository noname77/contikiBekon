//processes
PROCESS_NAME(rudolph1_process);

//events
extern process_event_t rudolph1_send_event;

//functions
void rudolph1_init();
void rudolph1_exit();
void rudolph1_pause();
void rudolph1_set_receive(int rec);
void rudolph1_set_filename(char* f);
