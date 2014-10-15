#include "net/rime.h"

void mesh_init();
void mesh_exit();
void mesh_set_dest(rimeaddr_t dest);

PROCESS_NAME(mesh_process);
extern process_event_t mesh_send_event;
extern process_event_t mesh_sent;
