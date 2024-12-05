#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "ssd1306-lib/font.h"
#include "ssd1306-lib/ssd1306.h"
#include "ssd1306-lib/ssd1306.c"
#include "game-lib/game.h"
#include "game-lib/dots.c"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT PICO_DEFAULT_I2C
#define I2C_SDA PICO_DEFAULT_I2C_SDA_PIN
#define I2C_SCL PICO_DEFAULT_I2C_SCL_PIN

// Global Variables
bool DOWN = true;
bool RIGHT = true;

// When true, the game is running
bool START_SCREEN = false;
bool GAME = false;
bool PAUSE = true;
bool continue_game = true;
int SCORE = 0;

// Pixel Struct
struct XY {
    int x;
    int y;
};

// Pixel Array
// Max of 10 dots on the screen
struct Dot dots[10];

// Fill the dots array with random dots
void fillDots()
{
    for (int i = 0; i < 10; i++)
    {
        dots[i] = getRandomDot();
    }
}

// Initialize adc with the joystick pins
void init_joyStick() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

// Read the joystick input and output the direction the avatar should move
struct XY readJoyStick() {
    // Direction struct
    struct XY move = {0, 0};

    // Read the joystick input
    adc_select_input(0);
    uint adc_x_raw = adc_read();
    adc_select_input(1);
    uint adc_y_raw = adc_read();
    
    // Get x direction
    // If moving left
    if (adc_x_raw < 1500) {
        move.x = -1;
    }
    // if moving right
    else if (adc_x_raw > 2500) {
        move.x = 1;
    }

    // Get y direction
    // If moving down
    if (adc_y_raw < 1500) {
        move.y = 1;
    }
    // if moving up
    else if (adc_y_raw > 2500) {
        move.y = -1;
    }

    // Return the direction
    return move;
}

void drawScore(ssd1306_t display) {
    char formatted_score[10];
    sprintf(formatted_score, "Score: %d", SCORE);
    ssd1306_draw_string(&display, 75, 1, 1, formatted_score);
}

// Draw the game screen with game name and score
void init_gameScreen(ssd1306_t display) {
    // Yellow area is first 16 pixels
    // Fill in yellow area
    drawScore(display);

    ssd1306_draw_square(&display, 0, 14, 128, 2);
    ssd1306_draw_string(&display, 1, 1, 1, GAME_NAME);
    
}

void init_startScreen(ssd1306_t display) {
    ssd1306_draw_square(&display, 0, 15, 128, 2);
    ssd1306_draw_string(&display, 1, 0, 2, GAME_NAME);
    ssd1306_draw_string(&display, 1, 45, 1, "Press Button to Start");
    ssd1306_show(&display);
}

void init_pauseScreen(ssd1306_t display) {
    drawScore(display);
    ssd1306_draw_string(&display, 1, 0, 1, "Paused");
    ssd1306_draw_square(&display, 0, 15, 128, 2);

    ssd1306_draw_string(&display, 32, 25, 1, "Continue");
    ssd1306_draw_string(&display, 32, 50, 1, "Quit");

    ssd1306_show(&display);

}
// Draw player avatar on the screen
void drawAvatar(ssd1306_t display, int x, int y) {
    ssd1306_draw_square(&display, x, y, SQUARE_SIZE, SQUARE_SIZE);
    ssd1306_show(&display);
    sleep_ms(25);
    ssd1306_clear(&display);
}

// Check if the square has eaten a dot on the screen
bool checkCollision(struct XY avatar_center, struct Dot dot) {
    if (abs(avatar_center.x - dot.x) <= SQUARE_SIZE / 2 && abs(avatar_center.y - dot.y) <= SQUARE_SIZE / 2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Check if Y is within the screen
int checkY(int y, int size) {
    if (y > 64 - size)
    {
        y = 64 - size;
    }
    else if (y < 17)
    {
        y = 17;
    }
    
    return y;
}

// Check if X is within the screen
int checkX(int x, int size) {
    if (x > 128 - size)
    {
        x = 128 - size;
    }
    else if (x < 0)
    {
        x = 0;
    }
    return x;
}

// Increase the score
// Max score is 99 due to formatting
void increaseScore() {
    if (SCORE < 99)
    {
        SCORE++;
    }
}

int main()
{
    stdio_init_all();
    init_joyStick();

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(i2c_default, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c
    
    // ADC Initialisation + Joystick pins
    // Conversion factor for the ADC
    const float conversion_factor = 3.3f / (1 << 12);
    // Initialize the display
    ssd1306_t display;
    display.external_vcc = false;
    ssd1306_init(&display, 128, 64, 0x3C, i2c_default);
    ssd1306_clear(&display);

    // Avatar starting position
    int x = AVATAR_START_X;
    int y = AVATAR_START_Y;

    struct XY avatar_center = {x + SQUARE_SIZE / 2, y + SQUARE_SIZE / 2};

    bool down = true;
    bool right = true;

    // Fill the dots array with random dots
    fillDots();

    while(START_SCREEN) {
        init_startScreen(display);
    }

    while(PAUSE) {
        init_pauseScreen(display);
        struct XY move = readJoyStick();
        if (continue_game) {
            sleep_ms(25);
            ssd1306_clear(&display);
            ssd1306_draw_circle(&display, 20, 28, 2);
        }
        else {;
            sleep_ms(25);
            ssd1306_clear(&display);
            ssd1306_draw_circle(&display, 20, 53, 2);
        }
        if (move.y == -1) {
            continue_game = true;
        }
        else if (move.y == 1) {
            continue_game = false;
        }
    }

    // Game Loop
    while (GAME) {
        init_gameScreen(display);
        drawAvatar(display, x, y);
        
        // Get movement direction from joystick
        struct XY move = readJoyStick();
        // Adjust the x and y position of the avatar
        x += move.x;
        y += move.y;
        // Check if the avatar is within the screen
        x = checkX(x, SQUARE_SIZE);
        y = checkY(y, SQUARE_SIZE);

        avatar_center.x = x + SQUARE_SIZE / 2;
        avatar_center.y = y + SQUARE_SIZE / 2;
        
        // Draw dots from dots array
        for (int i = 0; i < 10; i++)
        {
            printf("Score: %d\n", SCORE);
            if (SCORE > 10) {
                printf("Moving dot\n");
                dots[i] = moveDot(dots[i]);
            }
            ssd1306_draw_circle(&display, dots[i].x, dots[i].y, PIXEL_RADIUS);
            bool collision = checkCollision(avatar_center, dots[i]);
            // If there is a collision, increase the score and replace the dot with a new one
            if (collision)
            {
                increaseScore();
                dots[i] = getRandomDot();
            }
        } 
    }
}
