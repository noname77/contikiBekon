CONTIKI = ../..

CONTIKI_PROJECT = boot

APPS = serial-shell

### .c source files here
PROJECT_SOURCEFILES += commands.c uart2_handler.c  mesh_my.c rudolph1_my.c settings_read.c uarts_test.c SDcard.c lcd.c ble.c
# collect_my.c

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
