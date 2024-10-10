#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>

#include "buttons.h"
#include "types.h"

#define CONFIG_FILE_NAME "config.txt"
#define MAX_SMOOTH 10.0

#define DEBUG_MOUSE false

typedef struct Config {
    i64 button;
    f32 fov;
    f32 smooth;
    bool multibone;
    bool visibility_check;
} Config;

void parse_config(FILE *file);
void create_config(FILE *file);

#endif
