#include "contiki.h"

PROCESS(ble_process, "BLE Process");

PROCESS_THREAD(ble_process, ev, data)
{
  PROCESS_BEGIN();

  //handshake
    //send some character and wait for reply
  // ble setup
    // disable uart 2 shell input
    uart2_serial_input_exit();
    // enable direct control over contiki through tilde prefix
    if (data == '~') shell_input(++data, strlen(data));
    // set atributes contents
    // read atribute contents
    // get seen device's MAC address
    // setup direct communication with smartphone
  // sending data

  // receiving data

  PROCESS_END();
}

void ble_init()
{
  process_start(&ble_process);
}
