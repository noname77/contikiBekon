CONTIKI = ../..

CONTIKI_PROJECT = boot

APPS = serial-shell

CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

### .c source files here
PROJECT_SOURCEFILES += commands.c uart2_handler.c  mesh_my.c settings_read.c SDcard.c ble.c buzzer.c lcd.c clock_my.c rot_enc.c menu.c misc.c
# rudolph1_my.c audio.c collect_my.c uarts_test.c

all: $(CONTIKI_PROJECT)

include $(CONTIKI)/Makefile.include
