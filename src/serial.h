#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

bool setup_serial(void);
void close_serial(void);
void move_mouse(i32 x, i32 y);

#endif