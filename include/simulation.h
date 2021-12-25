#ifndef SIMULATION_H
#define SIMULATION_H

#include "raylib.h"

void sim_init_world();
void sim_run();
void sim_render();

void sim_make_alive(int x, int y);
Vector2 sim_screen_to_world(int x, int y);

#endif  // SIMULATION_H