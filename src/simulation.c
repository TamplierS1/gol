#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "simulation.h"

#define WIDTH 400
#define HEIGHT 400

bool g_current_world[WIDTH * HEIGHT];
bool g_future_world[WIDTH * HEIGHT];
Vector2 g_cell_size = {8, 8};

// TODO: the cells wrap around.
static bool is_outside_bounds(int x, int y)
{
    return x < 0 || x > WIDTH || y < 0 || y > HEIGHT;
}

static bool* get_cell(int x, int y)
{
    if (is_outside_bounds(x, y))
        return NULL;
    return &g_current_world[x + y * WIDTH];
}

static bool evolve(int x, int y)
{
    if (is_outside_bounds(x, y))
        return false;

    int live_neighbours = 0;
    for (int x1 = x - 1; x1 <= x + 1; x1++)
    {
        for (int y1 = y - 1; y1 <= y + 1; y1++)
        {
            if ((x1 == x && y1 == y) || x <= 0 || x >= WIDTH || y <= 0 || y >= HEIGHT)
                continue;
            if (*get_cell(x1, y1) == true)
                live_neighbours++;
        }
    }

    return (*get_cell(x, y) == true && live_neighbours == 2) || live_neighbours == 3;
}

void sim_init_world()
{
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            *get_cell(x, y) = false;
            g_future_world[x + y * WIDTH] = false;
        }
    }
}

void sim_run()
{
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            g_future_world[x + y * WIDTH] = evolve(x, y);
        }
    }

    memcpy(&g_current_world, &g_future_world, WIDTH * HEIGHT);
}

void sim_render()
{
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            if (*get_cell(x, y) == true)
            {
                DrawRectangle(x * g_cell_size.x, y * g_cell_size.y, g_cell_size.x,
                              g_cell_size.y, WHITE);
            }
        }
    }
}

void sim_nuke()
{
    sim_init_world();
}

void sim_make_alive(int x, int y)
{
    if (is_outside_bounds(x, y))
        return;
    *get_cell(x, y) = true;
}

Vector2 sim_screen_to_world(int x, int y)
{
    return (Vector2){x / g_cell_size.x, y / g_cell_size.y};
}
