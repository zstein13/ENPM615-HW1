#include <stdio.h>

// Dot struct
struct Dot
{
    int x;
    int y;
    bool left;
    bool down;
};

// Generate a random dot on the screen
struct Dot getRandomDot() {
    int x = rand() % 128;
    int y = 0;
    // Generate a random number between 17 and 64
    while (y < 17)
    {
        y = rand() % 63;
    }
    struct Dot dot = {x, y};
    return dot;
}

struct Dot moveDot(struct Dot dot)
{
    // Move dot left or right
    if (dot.left)
    {
        dot.x++;
    }
    else
    {
        dot.x--;
    }

    // Move dot up or down
    if (dot.down)
    {
        dot.y++;
    }
    else
    {
        dot.y--;
    }

    // Check if the dot is at the edge of the screen x-axis
    if (dot.x >= 128)
    {
        dot.left = false;
    }
    else if (dot.x <= 0)
    {
        dot.left = true;
    }

    // Check if the dot is at the edge of the screen y-axis
    if (dot.y >= 64)
    {
        dot.down = false;
    }
    else if (dot.y <= 17)
    {
        dot.down = true;
    }

    return dot;
}