#ifndef COMMON_H
#define COMMON_H

#define LIBUSB_TARGET_MAJOR 1
#define LIBUSB_TARGET_MICRO 14

typedef enum {
    RED = 0,
    BLUE,
    GREEN,
    WHITE
} LEDState;

#endif // COMMON_H
