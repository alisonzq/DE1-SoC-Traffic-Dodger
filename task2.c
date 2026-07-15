#include <stdint.h>

#define PIXEL_BUFF_ADDR 0xc8000000
#define CHAR_BUFF_ADDR 0xc9000000
#define PS2_ADDR 0xff200100

void VGA_write_char(int x, int y, char c) {
    if (x >= 0 && x < 80 && y >= 0 && y < 60) {
        volatile uint8_t *char_base_addr = (volatile uint8_t*)(CHAR_BUFF_ADDR | (y << 7) | x);
        *char_base_addr = c;
    }
}

void VGA_draw_point(int x, int y, short c) {
    if (x >= 0 && x < 320 && y >= 0 && y < 240) {
        volatile uint16_t *pixel_base_addr = (volatile uint16_t *)(PIXEL_BUFF_ADDR | (y << 10) | (x << 1));
        *pixel_base_addr = c;
    }
}

void VGA_clear_pixelbuff() {
    for (int i = 0; i < 320 * 240; i++) {
        VGA_draw_point(i % 320, i / 320, 0x0000); //clear the pixel buffer
    }
}

 void VGA_clear_charbuff() {
    for (int i = 0; i < 80 * 60; i++) {
        VGA_write_char(i % 80, i / 80, ' '); //clear the character buffer
    }
}

int read_PS2_data(char *data) {
    int RVALID = ((*(volatile int *)PS2_ADDR) >> 15) & 0x1;
    if(RVALID == 1) {
        *data = *(volatile int *)PS2_ADDR;
    }
	return RVALID;
}

void write_hex_digit(unsigned int x,unsigned int y, char c) {
    if (c > 9) {
        c += 55;
    } else {
        c += 48;
    }
    c &= 255;
    VGA_write_char(x,y,c);
}
void write_byte_kbrd(unsigned int x,unsigned int y, unsigned int c) {
   char lower=c>>4 &0x0F;
   write_hex_digit(x,y,lower);
   char upper=c&0x0F;
   write_hex_digit(x+1,y,upper);
   return;
}

void input_loop_fun() {
    unsigned int x = 0;
    unsigned int y = 0;
	VGA_clear_pixelbuff();
    VGA_clear_charbuff();

    while (y<=59) {
    
			char data;
            char r2 = read_PS2_data(&data);

            if (r2 != 0) {  // Check if data is available

				write_byte_kbrd(x,y,data); 
                x += 3;
                if (x > 79) {
                    y++;
                    x = 0;
                }

                if (y > 59) {  // Check if loop should exit
                    return;  // End of input loop
                }
            }
    }
}


int main() {
	input_loop_fun();
	return 0;
}
