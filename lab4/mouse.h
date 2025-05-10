#ifndef MOUSE_H
#define MOUSE_H

#include <lcom/lcf.h>

int(mouse_subscribe_int)();
int(mouse_unsubscribe_int)();
void (mouse_ih)();
void print_mouse_packet();
int(write_argument_to_mouse)(uint8_t argument);
int(enable_mouse)();
int(disable_mouse)();

#endif
