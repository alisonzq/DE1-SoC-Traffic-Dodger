#include <stdint.h>

#define PIXEL_BUFF_ADDR 0xc8000000
#define CHAR_BUFF_ADDR 0xc9000000

void VGA_clear_pixelbuff() {
    for (int i = 0; i < 320 * 240; i++) {
        VGA_draw_point(i % 320, i / 320, 0x0000); //clear the pixel buffer
    }
}

void VGA_draw_point(int x, int y, short c) {
    if (x >= 0 && x < 320 && y >= 0 && y < 240) {
        volatile uint16_t *pixel_base_addr = (volatile uint16_t *)(PIXEL_BUFF_ADDR | (y << 10) | (x << 1));
        *pixel_base_addr = c;
    }
}

void VGA_write_char(int x, int y, char c) {
    if (x >= 0 && x < 80 && y >= 0 && y < 60) {
        volatile uint8_t *char_base_addr = (volatile uint8_t*)(CHAR_BUFF_ADDR | (y << 7) | x);
        *char_base_addr = c;
    }
}

void VGA_clear_charbuff() {
    for (int i = 0; i < 80 * 60; i++) {
        VGA_write_char(i % 80, i / 80, ' '); //clear the character buffer
    }
}

void draw_test_screen ()
{
    VGA_clear_pixelbuff();
    VGA_clear_charbuff();
	int SCREEN_HEIGHT=240;
	int SCREEN_WIDTH=320;
   for (int y = 0; y < SCREEN_HEIGHT; y++) {
        for (int x = 0; x < SCREEN_WIDTH; x++) {
            // Calculate color components to create a smooth gradient
            unsigned int red = (x * 31 / SCREEN_WIDTH) << 11;      // Horizontal red gradient
            unsigned int green = (y * 63 / SCREEN_HEIGHT) << 5;    // Vertical green gradient
            unsigned int blue = ((x + y) * 31 / (SCREEN_WIDTH + SCREEN_HEIGHT));  // Diagonal blue gradient

            // Combine red, green, and blue into a 16-bit color
            unsigned int color = red | green | blue;

            // Draw the pixel at (x, y) with the calculated color
            VGA_draw_point(x, y, color);
        }
    }


    const char *message = "Hello World!";
    int x = 20;
    int y = 5;
    
    for (int i = 0; message[i] != '\0'; i++) {
        VGA_write_char(x++, y, message[i]);
    }
}

int main() {
	draw_test_screen();
	return 0;
}