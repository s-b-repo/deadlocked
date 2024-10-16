#include "config.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "buttons.h"

Config config = {.button = MOUSE_LEFT, .fov = 2.0f, .smooth = 5.0f, .multibone = true, .visibility_check = true};

char *trim(char *str) {
    char *end;

    // Trim leading spaces
    while (isspace((u8)*str)) str++;

    // All spaces? Return an empty string
    if (*str == 0) return str;

    // Trim trailing spaces
    end = str + strlen(str) - 1;
    while (end > str && isspace((u8)*end)) end--;

    // Write new null terminator character
    *(end + 1) = '\0';

    return str;
}

void to_lower_case(char *string) {
    for (i32 i = 0; string[i] != '\0'; i++) {
        string[i] = tolower(string[i]);
    }
}

i64 parse_i64(char *value) {
    for (i32 i = 0; i < strlen(value); i++) {
        if (!isdigit(value[i]) && value[i] != '-') {
            return 0;
        }
    }

    return strtoll(value, NULL, 10);
}

f32 parse_f32(char *value) {
    for (i32 i = 0; i < strlen(value); i++) {
        if (!isdigit(value[i]) && value[i] != '.') {
            return 0;
        }
    }

    return strtof(value, NULL);
}

bool parse_bool(char *value) {
    if (!strcmp(value, "true")) {
        return true;
    } else if (!strcmp(value, "false")) {
        return false;
    }

    for (i32 i = 0; i < strlen(value); i++) {
        if (!isdigit(value[i]) || value[i] == '-') {
            return false;
        }
    }

    return strtoll(value, NULL, 10) != 0;
}

void parse_line(char *string) {
    char *trimmed = trim(string);
    if (strlen(trimmed) < 2) {
        return;
    }

    char *right_space = strrchr(trimmed, ' ');
    char *left_space = strchr(trimmed, ' ');
    *left_space = '\0';

    const char *name = trimmed;
    char *value = right_space + 1;
    to_lower_case(value);

    if (!strcmp(name, "button")) {
        const i64 button = parse_i64(value);
        if (button <= KEYCODE_NONE) {
            return;
        }
        if (button > KEYCODE_SCROLLLOCKTOGGLE && button < MOUSE_LEFT) {
            return;
        }
        if (button > MOUSE_WHEEL_DOWN) {
            return;
        }

        config.button = button;
    } else if (!strcmp(name, "fov")) {
        f32 fov = parse_f32(value);
        if (fov < 0.1) {
            fov = 0.1;
        } else if (fov > 360.0) {
            fov = 360.0;
        }
        config.fov = fov;
    } else if (!strcmp(name, "smooth")) {
        f32 smooth = parse_f32(value);
        if (smooth < 1.0) {
            smooth = 0.0;
        } else if (smooth > MAX_SMOOTH) {
            smooth = MAX_SMOOTH;
        }
        config.smooth = smooth;
    } else if (!strcmp(name, "multibone")) {
        config.multibone = parse_bool(value);
    } else if (!strcmp(name, "visibility_check")) {
        config.visibility_check = parse_bool(value);
    } else {
        printf("config option not recognized: %s", name);
    }
}

void parse_config(FILE *file) {
    char string[1024] = {0};
    while (fgets(string, sizeof(string), file) != NULL) {
        parse_line(string);
    }
}

void create_config(FILE *file) {
    fprintf(file, "button %ld\n", config.button);
    fprintf(file, "fov %.2f\n", config.fov);
    fprintf(file, "smooth %.2f\n", config.smooth);
    fprintf(file, "multibone %s\n", config.multibone ? "true" : "false");
    fprintf(file, "visibility_check %s\n", config.visibility_check ? "true" : "false");
}
