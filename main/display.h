#include "driver/spi_master.h"

#ifndef DISPLY_H
#define DISPLAY_H
// The max7219 handle we will use throughout
extern spi_device_handle_t max7219_handle;

// Register address macros
#define MAX7219_REG_NOOP        0x00
#define MAX7219_REG_DIGIT0      0x01
#define MAX7219_REG_DIGIT1      0x02
#define MAX7219_REG_DIGIT2      0x03
#define MAX7219_REG_DIGIT3      0x04
#define MAX7219_REG_DIGIT4      0x05
#define MAX7219_REG_DIGIT5      0x06
#define MAX7219_REG_DIGIT6      0x07
#define MAX7219_REG_DIGIT7      0x08

#define MAX7219_REG_DECODE      0x09
#define MAX7219_REG_INTENSITY   0x0A
#define MAX7219_REG_SCANLIMIT   0x0B
#define MAX7219_REG_SHUTDOWN    0x0C
#define MAX7219_REG_TEST        0x0F

// Structure 1 led row worth of data into [register][value]
#define MAX7219_CMD(reg, val) (((reg) << 8) | (val))

// Max7219 setting inits
#define MAX7219_DECODE_INIT         MAX7219_CMD(MAX7219_REG_DECODE, 0x00)
#define MAX7219_INTENSITY_INIT      MAX7219_CMD(MAX7219_REG_INTENSITY, 0x00)
#define MAX7219_SCAN_INIT           MAX7219_CMD(MAX7219_REG_SCANLIMIT, 0x07)
#define MAX7219_SHUTDOWN_INIT       MAX7219_CMD(MAX7219_REG_SHUTDOWN, 0x01)
#define MAX7219_TEST_INIT            MAX7219_CMD(MAX7219_REG_TEST, 0x00)

#define SYM_DIGIT_0     0
#define SYM_DIGIT_1     1
#define SYM_DIGIT_2     2
#define SYM_DIGIT_3     3
#define SYM_DIGIT_4     4
#define SYM_DIGIT_5     5
#define SYM_DIGIT_6     6
#define SYM_DIGIT_7     7
#define SYM_DIGIT_8     8
#define SYM_DIGIT_9     9
#define SYM_ARROW_UP    10
#define SYM_ARROW_DOWN  11
#define SYM_ARROW_RIGHT 12
#define SYM_ARROW_UR    13
#define SYM_ARROW_DR    14
#define SYM_DASH        15
#define SYM_CLEAR       16
#define SYM_DOUBLE_UP   17
#define SYM_DOUBLE_DOWN 18

// Represents the entire matrix
extern uint8_t framebuffer[8][4];
void max7219_init(void);
void update_display(int bg_level, const char *trend);
void set_loading_display(void);
const uint8_t* get_glyph(int code);

#endif