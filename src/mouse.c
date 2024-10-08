#include "mouse.h"

#include <fcntl.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"

int mouse = 0;

bool setup_mouse(void) {
    mouse = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (mouse < 0) {
        printf("could not open uinput\n");
        return false;
    }

    ioctl(mouse, UI_SET_EVBIT, EV_KEY);
    ioctl(mouse, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(mouse, UI_SET_EVBIT, EV_REL);
    ioctl(mouse, UI_SET_RELBIT, REL_X);
    ioctl(mouse, UI_SET_RELBIT, REL_Y);

    struct uinput_user_dev uidev = {0};
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE,
             "Logitech, Inc. M105 Optical Mouse");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor = 0x046D;
    uidev.id.product = 0xC077;
    uidev.id.version = 1;

    // Write device description
    if (write(mouse, &uidev, sizeof(uidev)) < 0) {
        perror("error while writing device description\n");
        close(mouse);
        return false;
    }

    // Create the input device
    if (ioctl(mouse, UI_DEV_CREATE) < 0) {
        perror("error creating mouse device\n");
        close(mouse);
        return false;
    }
    printf("created mouse device\n");

    return true;
}

void close_mouse(void) {
    if (mouse > 0) {
        ioctl(mouse, UI_DEV_DESTROY);
        close(mouse);
        printf("destroyed mouse device\n");
        exit(0);
    }
}

void terminate_mouse(int _) { close_mouse(); }

void move_mouse(i32 x, i32 y) {
    // fix sudden weird turns
    if (x > 50 || y > 50) {
        return;
    }
#if !DEBUG_MOUSE
    struct input_event ev = {0};

    // Move along X-axis
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = x;
    if (write(mouse, &ev, sizeof(struct input_event)) < 0) {
        perror("error writing x movement\n");
        return;
    }

    // Move along Y-axis
    ev.type = EV_REL;
    ev.code = REL_Y;
    ev.value = y;
    if (write(mouse, &ev, sizeof(struct input_event)) < 0) {
        perror("error writing y movement\n");
        return;
    }

    // Emit a SYN event (synchronize)
    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    if (write(mouse, &ev, sizeof(struct input_event)) < 0) {
        perror("error writing syn event\n");
        return;
    }
#else
    printf("mouse move: %d/%d\n", x, y);
#endif
}
