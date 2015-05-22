void audio_init();
void audio_stop();

PROCESS_NAME(audio_process);

extern process_event_t player_load_file_event;
extern process_event_t player_start_playback_event;
extern process_event_t player_pause_playback_event;
