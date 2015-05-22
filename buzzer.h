#define BUZZ_PIN 7

PROCESS_NAME(ringtone_process);
extern process_event_t ringtone_play_event;

int ttof(char* t);

void ringtone_init();
void ringtone_end();
