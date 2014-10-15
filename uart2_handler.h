extern process_event_t UART2_SEND;
extern process_event_t uart2_sent;

void uart2_handler_init();
void uart2_log_init();
void uart2_log_exit();
void uart2_serial_input_init();
void uart2_serial_input_exit();

PROCESS_NAME(uart2_process);

