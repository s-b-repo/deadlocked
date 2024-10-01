#include "serial.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "config.h"

int mouse = 0;
struct termios tty = {0};

bool setup_serial(void) {
    if (access(ARDUINO_PORT, F_OK) != 0) {
        printf("port %s does not exist or could not be opened", ARDUINO_PORT);
        printf("check that the arduino is plugged in, and that the correct port is written in config.h");
        return false;
    }

    mouse = open(ARDUINO_PORT, O_WRONLY);

    if (mouse < 0) {
        printf("error while opening serial connection\n");
        return false;
    }

    if (tcgetattr(mouse, &tty) != 0) {
        printf("error while reading existing serial settings\n");
        return false;
    }

    // set flags
    tty.c_cflag &= ~PARENB;  // Clear parity bit, disabling parity (most common)
    tty.c_cflag &= ~CSTOPB;  // Clear stop field, only one stop bit used in
                             // communication (most common)
    tty.c_cflag |= CS8;      // 8 bits per byte (most common)

    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ECHO;  // Disable echo
    tty.c_lflag &= ~ISIG;  // Disable interpretation of INTR, QUIT and SUSP

    tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
                     ICRNL);  // Disable any special handling of received bytes

    tty.c_oflag &= ~OPOST;  // Prevent special interpretation of output bytes
                            // (e.g. newline chars)
    tty.c_oflag &=
        ~ONLCR;  // Prevent conversion of newline to carriage return/line feed

    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(mouse, TCSANOW, &tty) != 0) {
        printf("error while setting serial settings\n");
        return false;
    }
}

void close_serial(void) { close(mouse); }

void move_mouse(i32 x, i32 y) {
    char buffer[128] = {0};
    snprintf(buffer, sizeof(buffer), "%d;%y", x, y);
    write(mouse, buffer, sizeof(buffer));
}
