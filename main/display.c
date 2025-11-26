#include "driver/spi_master.h"
#include "display.h"
#include <string.h>
#include <assert.h>
#include "esp_err.h"


// Digits and arrows
static const uint8_t ARROW_DOUBLE_UP[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b00000000,
    0b00011000,
    0b00111100,
    0b01111110,
    0b00000000
};
static const uint8_t ARROW_DOUBLE_DOWN[8] = {
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000
};
static const uint8_t ARROW_DIAG_DOWN[8] = {
    0b00000000,
    0b01100000,
    0b00110000,
    0b00011001,
    0b00001111,
    0b00000111,
    0b00001111,
    0b00000000
};
static const uint8_t ARROW_DIAG_UP[8] = {
    0b00001111,
    0b00000111,
    0b00001111,
    0b00011001,
    0b00110000,
    0b01100000,
    0b00000000,
    0b00000000
};
static const uint8_t ARROW_FLAT[8] = {
    0b00001000,
    0b00001100,
    0b01111110,
    0b01111111,
    0b01111110,
    0b00001100,
    0b00001000,
    0b00000000

};
static const uint8_t ARROW_DOWN[8] = {
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b01111110,
    0b00111100,
    0b00011000,
    0b00000000
};
static const uint8_t ARROW_UP[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00000000
};
static const uint8_t DIGIT_9[8] = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b00111110,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00000000
};
static const uint8_t DIGIT_8[8] = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b00111100,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000

};
static const uint8_t DIGIT_7[8] = {
    0b01111110,
    0b00000110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b00110000,
    0b00110000,
    0b00000000
};
static const uint8_t DIGIT_6[8] = {
    0b00111100,
    0b01100110,
    0b01100000,
    0b01111100,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};
static const uint8_t DIGIT_5[8] = {
    0b01111110,
    0b01100000,
    0b01111100,
    0b00000110,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00000000
};
static const uint8_t DIGIT_4[8] = {
    0b00001100,
    0b00011100,
    0b00111100,
    0b01101100,
    0b01111110,
    0b00001100,
    0b00001100,
    0b00000000
};
static const uint8_t DIGIT_3[8] = {
    0b00111100,
    0b01100110,
    0b00000110,
    0b00011100,
    0b00000110,
    0b01100110,
    0b00111100,
    0b00000000

};
static const uint8_t DIGIT_2[8] = {
    0b00111100,
    0b01100110,
    0b00000110,
    0b00001100,
    0b00011000,
    0b00110000,
    0b01111110,
    0b00000000

};
static const uint8_t DIGIT_1[8] = {
    0b00011000,
    0b00111000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b00011000,
    0b01111110,
    0b00000000

};
static const uint8_t DIGIT_0[8] = {
    0b00111100,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b01100110,
    0b00111100,
    0b00000000
};
static const uint8_t DASH[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b01111110,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};
static const uint8_t CLEAR[8] = {
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};


spi_device_handle_t max7219_handle;
uint8_t framebuffer[8][4] = {0};


void max7219_send_cmd(uint16_t cmd)
{
    // Build the 8-byte SPI buffer (4 chips × 16 bits each)
    uint8_t buf[8];

    // Fill buffer with values for all 4 chips
    buf[0] = (cmd >> 8) & 0xFF;
    buf[1] = cmd & 0xFF;

    buf[2] = (cmd >> 8) & 0xFF;
    buf[3] = cmd & 0xFF;

    buf[4] = (cmd >> 8) & 0xFF;
    buf[5] = cmd & 0xFF;

    buf[6] = (cmd >> 8) & 0xFF;
    buf[7] = cmd & 0xFF;

    spi_transaction_t t = {
        .length    = 8 * 8,
        .tx_buffer = buf,
    };

    // Blocking transmit — CS toggled automatically
    esp_err_t ret = spi_device_transmit(max7219_handle, &t);
    assert(ret == ESP_OK);
}

void max7219_clear(void) {
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT0, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT1, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT2, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT3, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT4, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT5, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT6, 0x00)); 
    max7219_send_cmd(MAX7219_CMD(MAX7219_REG_DIGIT7, 0x00)); 
}

void max7219_init(void) {
    // Initialize all required settings for each max7219 in the chain of 4
    max7219_send_cmd(MAX7219_SHUTDOWN_INIT);   
    max7219_send_cmd(MAX7219_TEST_INIT);
    max7219_send_cmd(MAX7219_DECODE_INIT);
    max7219_send_cmd(MAX7219_SCAN_INIT);       
    max7219_send_cmd(MAX7219_INTENSITY_INIT);   

    memset(framebuffer, 0, sizeof(framebuffer));


    max7219_clear();
}

const uint8_t* get_glyph(int code) {
    switch (code) {
        case SYM_DIGIT_0: return DIGIT_0;
        case SYM_DIGIT_1: return DIGIT_1;
        case SYM_DIGIT_2: return DIGIT_2;
        case SYM_DIGIT_3: return DIGIT_3;
        case SYM_DIGIT_4: return DIGIT_4;
        case SYM_DIGIT_5: return DIGIT_5;
        case SYM_DIGIT_6: return DIGIT_6;
        case SYM_DIGIT_7: return DIGIT_7;
        case SYM_DIGIT_8: return DIGIT_8;
        case SYM_DIGIT_9: return DIGIT_9;

        case SYM_ARROW_UP:      return ARROW_UP;
        case SYM_ARROW_DOWN:    return ARROW_DOWN;
        case SYM_ARROW_RIGHT:   return ARROW_FLAT;
        case SYM_ARROW_UR:      return ARROW_DIAG_UP;
        case SYM_ARROW_DR:      return ARROW_DIAG_DOWN;
        case SYM_DOUBLE_UP:     return ARROW_DOUBLE_UP;
        case SYM_DOUBLE_DOWN:   return ARROW_DOUBLE_DOWN;

        case SYM_DASH:        return DASH;
        case SYM_CLEAR:       return CLEAR;

        default:              return DASH;
    }
}


void write_glyph_to_framebuffer(const uint8_t* glyph, uint8_t chip) {
    for(int row = 0; row < 8; row ++) {
        framebuffer[row][chip] = glyph[row];
    }
}

void update_framebuffer(int code0, int code1, int code2, int code3) {
    // Get all glyphs
    const uint8_t* glyph0 = get_glyph(code0);
    const uint8_t* glyph1 = get_glyph(code1);
    const uint8_t* glyph2 = get_glyph(code2);
    const uint8_t* glyph3 = get_glyph(code3);


    // Write each to framebuffer
    write_glyph_to_framebuffer(glyph0, 0);
    write_glyph_to_framebuffer(glyph1, 1);
    write_glyph_to_framebuffer(glyph2, 2);
    write_glyph_to_framebuffer(glyph3, 3);

}

void max7219_write_row(uint16_t cmd3, uint16_t cmd2, uint16_t cmd1, uint16_t cmd0)
{
    // Build the 8-byte SPI buffer (4 chips × 16 bits each)
    uint8_t buf[8];

    // Fill buffer with values for all 4 chips
    buf[0] = (cmd3 >> 8) & 0xFF;
    buf[1] = cmd3 & 0xFF;

    buf[2] = (cmd2 >> 8) & 0xFF;
    buf[3] = cmd2 & 0xFF;

    buf[4] = (cmd1 >> 8) & 0xFF;
    buf[5] = cmd1 & 0xFF;

    buf[6] = (cmd0 >> 8) & 0xFF;
    buf[7] = cmd0 & 0xFF;

    spi_transaction_t t = {
        .length    = 8 * 8,
        .tx_buffer = buf,
    };

    // Blocking transmit — CS toggled automatically
    esp_err_t ret = spi_device_transmit(max7219_handle, &t);
    assert(ret == ESP_OK);
}

int convert_trend(const char *trend) {
    if (!trend) {
        return SYM_DASH;
    }

    if (strcmp(trend, "Flat") == 0)
        return SYM_ARROW_RIGHT;

    if (strcmp(trend, "SingleUp") == 0)
        return SYM_ARROW_UP;

    if (strcmp(trend, "SingleDown") == 0)
        return SYM_ARROW_DOWN;

    if (strcmp(trend, "DoubleUp") == 0)
        return SYM_DOUBLE_UP;

    if (strcmp(trend, "DoubleDown") == 0)
        return SYM_DOUBLE_DOWN;

    if (strcmp(trend, "FortyFiveDown") == 0)
        return SYM_ARROW_DR;


    if (strcmp(trend, "FortyFiveUp") == 0)
        return SYM_ARROW_UR;

    return SYM_DASH;
}

void write_framebuffer_to_max7219() {
    // Row variables for each chip
    uint8_t chip3;
    uint8_t chip2;
    uint8_t chip1;
    uint8_t chip0;

    // Command variables for each chip
    // These are 16 bit. See MAX7219_CMD for more details
    uint16_t cmd3;
    uint16_t cmd2;
    uint16_t cmd1;
    uint16_t cmd0;

    uint8_t reg;

    for (int row = 0; row < 8; row ++) {
        // First digit is noop, so use offset
        reg = MAX7219_REG_DIGIT0 + row;

        chip3 = framebuffer[row][3];
        chip2 = framebuffer[row][2];
        chip1 = framebuffer[row][1];
        chip0 = framebuffer[row][0];

        // Setup commands for each chip
        cmd3 = MAX7219_CMD(reg, chip3);
        cmd2 = MAX7219_CMD(reg, chip2);
        cmd1 = MAX7219_CMD(reg, chip1);
        cmd0 = MAX7219_CMD(reg, chip0);

        // Send row to display
        max7219_write_row(cmd3, cmd2, cmd1, cmd0);

    }
}

// Set all display matrices to -----
void set_loading_display(void) {
    // Update framebuffer with values
    update_framebuffer(SYM_DASH, SYM_DASH, SYM_DASH, SYM_DASH);

    // Send framebuffer data to the display
    write_framebuffer_to_max7219();
}

// Update display with blood sugar data
void update_display(int bg_level, const char *trend) {
    int trend_glyph = convert_trend(trend);

    int ones = bg_level % 10;
    int tens = (bg_level / 10) % 10;
    int hundreds = (bg_level >= 100) ? (bg_level / 100) : SYM_CLEAR;

    // Update framebuffer with values
    update_framebuffer(ones, tens, hundreds, trend_glyph);

    // Send framebuffer data to the display
    write_framebuffer_to_max7219();
}   