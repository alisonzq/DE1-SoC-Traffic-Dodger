#include <stdint.h>

#define PIXEL_BUFF_ADDR 0xc8000000
#define CHAR_BUFF_ADDR 0xc9000000
#define PS2_ADDR 0xff200100
#define TIMER_INT_ADDR 0xFFFEC60C
#define TIMER_CTRL_ADDR 0xFFFEC608
#define TIMER_ADDR 0xFFFEC600

typedef struct {
    float x;
    float y;
    int spawned_next_car;
} TrafficCar;

#define MAX_TRAFFIC_CARS 3

TrafficCar traffic_cars[MAX_TRAFFIC_CARS];
int traffic_car_count = 0;

float player_position_x;
int score;
float speed = 1.0;
int is_moving_left = 0;
int is_moving_right = 0;
int car_width = 30;
int car_height = 48;

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


int timer_expired() {
    volatile int *timer_status = (volatile int *)TIMER_INT_ADDR; // timer interrupt status register
    return (*timer_status & 0x1); //return 1 if expired, 0 otherwise
}

void clear_timer_interrupt() {
    volatile int *timer_status = (volatile int *)TIMER_INT_ADDR; //timer interrupt status register
    *timer_status = 0x1; //clear the interrupt
}

void init_timer() {
    volatile int *timer_control = (volatile int *)TIMER_CTRL_ADDR; //timer control register
    *timer_control = 0x7; //enable the timer
    volatile int *timer_load_value = (volatile int *)TIMER_ADDR;
    *timer_load_value = 0x00989680; //0.05s
}

int read_PS2_data(char *data) {
    int RVALID = ((*(volatile int *)PS2_ADDR) >> 15) & 0x1;
    if(RVALID == 1) {
        *data = *(volatile int *)PS2_ADDR;
    }
	return RVALID;
}

unsigned int seed = 12345;  // You can set this to any starting value

// Function to generate a pseudo-random number
unsigned int pseudo_random() {
    // LCG parameters (from Numerical Recipes)
    seed = (1103515245 * seed + 12345) & 0x7fffffff;
    return seed;
}

// Function to get a pseudo-random number within a specific range [min, max]
unsigned int random_in_range(int min, int max) {
    return (pseudo_random() % (max - min + 1)) + min;
}

void draw_road()
{   
	int ROAD_HEIGHT=240;
	int ROAD_WIDTH=100;
    int LINE_WIDTH = 2;
    int LINE_SPACING = 20; 

    for (int y = 0; y < ROAD_HEIGHT; y++) {
        for (int x = 0; x < ROAD_WIDTH; x++) {
            if(x < 10) {
                //checkered pattern for borders
                if (y % 2 == 0 && x % 2 == 0) {
                    VGA_draw_point(x, y, 0xFFFF);
                } else {
                    VGA_draw_point(x, y, 0x0000);
                }
            } else if(x > 89) {
                //checkered pattern for borders
                if (y % 2 != 0 && x % 2 != 0) {
                    VGA_draw_point(x, y, 0xFFFF);
                } else {
                    VGA_draw_point(x, y, 0x0000);
                }
            } 
            else {
                //center yellow road lines
                if (x >= (50 - LINE_WIDTH / 2) && x < 50 + LINE_WIDTH / 2 && (y / LINE_SPACING) % 2 == 0) {
                    VGA_draw_point(x, y, 0xFFC0);  // yellow line
                } else {
                    VGA_draw_point(x, y, 0xF5F9);  // pink road
                }
            }
        }
    }

}

void draw_car(int position_x, int position_y, short color) {
    for (int y = 0; y < car_height; y++) {
        for (int x = 0; x < car_width; x++) {
            VGA_draw_point(position_x + x, position_y - y, color);
        }
    }
}

write_instruction(char *message, int y) {
    int length = 0;
    for (int i = 0; message[i] != '\0'; i++) {
        length++;
    }

    int x = (80 - length) / 2;  //center the message horizontally
    for (int i = 0; message[i] != '\0'; i++) {
        VGA_write_char(x++, y, message[i]);
    }
}

void spawn_traffic_car() {
    if(traffic_car_count > MAX_TRAFFIC_CARS) {
        return; //max traffic cars reached
    }
    int lane = random_in_range(1, 2);

    if(lane == 1) {
        traffic_cars[traffic_car_count].x = 14; //left lane
    } else if(lane == 2) {
        traffic_cars[traffic_car_count].x = 56; //right lane
    }
    traffic_cars[traffic_car_count].y = 0; //start above the screen, 
    traffic_car_count++;
}

void draw_traffic() {
    for (int i = 0; i < traffic_car_count; i++) {
        //spawn car if distance betweeen is 96
        if (traffic_cars[i].spawned_next_car == 0 && traffic_cars[i].y >= (96 + car_height)) {
            spawn_traffic_car();
            traffic_cars[i].spawned_next_car = 1; // car spawns only once after crossing 96
        }
        
        draw_car(traffic_cars[i].x, traffic_cars[i].y - (car_height - 1), 0xF5F9); // clear back line of car only to avoid glitching

        traffic_cars[i].y += speed; //move the car down

        int car_y = traffic_cars[i].y;
        int car_x = traffic_cars[i].x;

        //check if the car is within the screen (240+48)
        if (car_y <= (240 + car_height)) {
            draw_car(car_x, car_y, 0x00FF);
        } else {
            //increase speed and score
            speed = speed * 1.1;
            score++;

            //remove the car from the array
            for (int j = i; j < traffic_car_count - 1; j++) {
                traffic_cars[j] = traffic_cars[j + 1];
            }
            traffic_car_count--;
            i--; //decrement i to check the next car
        }
    }
}

void update_character_position(char data) {
    if (data == 0x1C) { // A key
        for (int y = 0; y < car_height; y++) {
            for (int x = 0; x < 6; x++) {
                //clear only 3 pixels on the right to avoid glitching
                VGA_draw_point((int)player_position_x + car_width - 1 - x, 239 - y, 0xF5F9); 
            }
        }
        player_position_x -= 6;
        if (player_position_x < 10) {
            player_position_x = 10; //prevent going out of bounds
        }
    } else if (data == 0x23) { // 'D' key
        for (int y = 0; y < car_height; y++) {
            for (int x = 0; x < 6; x++) {
                //clear only 3 pixels on the left to avoid glitching
                VGA_draw_point((int)player_position_x + x, 239 - y, 0xF5F9);
            }
        }
        player_position_x += 6; 
        if (player_position_x > (90-car_width)) { //89 - 30
            player_position_x = (90-car_width); // prevent going out of bounds
        }
    }
    draw_car((int)player_position_x, 239, 0xF800); // red car
}

int check_collision() {
    int collided = 0;
    for(int i = 0; i < traffic_car_count; i++) {
        int traffic_car_x = traffic_cars[i].x;
        int traffic_car_y = traffic_cars[i].y;

        //check if player car is within bounds of a traffic car
        if ((int)player_position_x < traffic_car_x + car_width && (int)player_position_x + car_width > traffic_car_x && 239 < traffic_car_y + car_height && 239 + car_height > traffic_car_y) {
            collided = 1; //collision detected
            break;
        }
    }

    return collided;

}

void init_game() {
    player_position_x = 20.0; // Initialize as float
    score = 0;
    speed = 1.0;
    traffic_car_count = 0;
    

    //reinitialize traffic cars
    for (int i = 0; i < MAX_TRAFFIC_CARS; i++) {
        traffic_cars[i].x = 0;
        traffic_cars[i].y = 0;
        traffic_cars[i].spawned_next_car = 0;
    }
    VGA_clear_charbuff();
    VGA_clear_pixelbuff();
    draw_road();
    draw_car((int)player_position_x, 239, 0xF800); //red car
    char *instruction = "Press S to start";
    write_instruction(instruction, 30);
    char *instructionA = "Press A to move left";
    write_instruction(instructionA, 33);
    char *instructionD = "Press D to move right";
    write_instruction(instructionD, 36);

}

int main() {
    init_timer();
    init_game();

    int is_first_open = 0;
    char data;
    int is_break_code = 0;
    int is_start_game = 0;

    while (1) { 
        char RVALID = read_PS2_data(&data);

        if (RVALID != 0) { 
            if (data == 0xF0) {
                is_break_code = 1; //set break code flag
            } else {
                if (is_break_code) {
                    is_break_code = 0; //reset break code flag
                    if (data == 0x1B && is_start_game == 0) {
                        if(is_first_open == 0) {
                            is_first_open = 1; //do not reinitialize if first time 
                        } else {
                            init_game(); //reinitialize game if gameover and press s
                        }
                        spawn_traffic_car();
                        is_start_game = 1;
                    }
                } else if(data == 0x1C || data == 0x23) { // 'A' or 'D' key
                    if(is_start_game == 1) {
                        update_character_position(data);
                    }
                }
            }
        } 

        if(is_start_game == 1 && timer_expired()) {

            clear_timer_interrupt(); //clear the timer interrupt
            draw_traffic();

            if(check_collision() == 1) {
                char *game_over = "Game Over!";
                write_instruction(game_over, 20);

                char scoreText[20];
                sprintf(scoreText, "Final Score: %d", score);
                write_instruction(scoreText, 22);

                is_start_game = 0;
            }

            if(is_start_game == 0) {
                continue; //skip other score display if game over
            }
            char scoreText[20];
            sprintf(scoreText, "Score: %d", score);
            write_instruction(scoreText, 22);
        }
    }
    return 0;
}
