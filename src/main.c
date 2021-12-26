#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_STANDARD_VARARGS
#include "nuklear.h"

#include "simulation.h"

const Vector2 g_window_size = {1920, 1080};

bool g_running = false;
int g_generation = 1;
int g_num_of_live_cells = 0;

float g_speed = 1.0f;
Camera2D g_camera;
float g_camera_speed = 300.0f;

Font g_ray_font;
struct nk_context g_nk_ctx;
struct nk_user_font g_nk_font;

/*      UTILITY     */
Vector2 to_ray_vec(struct nk_vec2i vec);
Color to_ray_color(struct nk_color color);

/*      RENDERING       */
void render();
void render_gui();

/*      FONTS       */
float calc_text_width(nk_handle handle, float height, const char* text, int len);
void load_fonts();

void handle_input(float delta);
void end();

int main()
{
    InitWindow(g_window_size.x, g_window_size.y, "Game Of Life");

    SetTargetFPS(30);

    g_camera.target = (Vector2){0, 0};
    g_camera.offset = (Vector2){g_window_size.x / 2, g_window_size.y / 2};
    g_camera.zoom = 1.0f;

    load_fonts();
    if (nk_init_default(&g_nk_ctx, &g_nk_font) == 0)
    {
        printf("Error: failed to initialize Nuklear context.\n");
        return EXIT_FAILURE;
    }

    sim_init_world();

    double elapsed_time = 0;
    while (!WindowShouldClose())
    {
        handle_input(GetFrameTime());

        if (GetTime() - elapsed_time >= (double)(1.0f / g_speed) && g_running)
        {
            sim_run();
            elapsed_time = GetTime();
            g_generation++;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        render();

        EndDrawing();
    }

    end();
    return EXIT_SUCCESS;
}

Vector2 to_ray_vec(struct nk_vec2i vec)
{
    return (Vector2){vec.x, vec.y};
}

Color to_ray_color(struct nk_color color)
{
    return (Color){color.r, color.g, color.b, color.a};
}

void render()
{
    BeginMode2D(g_camera);
    g_num_of_live_cells = sim_render();
    EndMode2D();

    enum nk_panel_flags flags =
        NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_MOVABLE;

    if (nk_begin(&g_nk_ctx, "Menu", (struct nk_rect){0, 0, 300, 300}, flags))
    {
        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        if (nk_button_label(&g_nk_ctx, g_running ? "Pause" : "Start"))
        {
            g_running = !g_running;
        }

        nk_layout_row_begin(&g_nk_ctx, NK_STATIC, 20, 2);
        {
            nk_layout_row_push(&g_nk_ctx, 50);
            nk_label(&g_nk_ctx, "Speed", NK_TEXT_LEFT);
            nk_layout_row_push(&g_nk_ctx, 240);
            nk_slider_float(&g_nk_ctx, 1, &g_speed, 10, 0.5);
        }

        nk_value_int(&g_nk_ctx, "Gen: ", g_generation);
        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        nk_value_int(&g_nk_ctx, "Alive: ", g_num_of_live_cells);

        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        if (nk_button_label(&g_nk_ctx, "Nuke"))
        {
            sim_nuke();
            g_generation = 1;
            g_running = false;
        }

        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        nk_text(&g_nk_ctx, "Controls:", 8, NK_TEXT_LEFT);
        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        nk_text(&g_nk_ctx, "\tRMB - place a living cell.",
                strlen("\tRMB - place a living cell."), NK_TEXT_LEFT);
        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        nk_text(&g_nk_ctx, "\tn - zoom out.", strlen("\tn - zoom out."), NK_TEXT_LEFT);
        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        nk_text(&g_nk_ctx, "\tm - zoom int.", strlen("\tm - zoom int."), NK_TEXT_LEFT);
        nk_layout_row_dynamic(&g_nk_ctx, 20, 1);
        nk_text(&g_nk_ctx, "\tWASD - move the camera.",
                strlen("\tWASD - move the camera."), NK_TEXT_LEFT);
    }
    nk_end(&g_nk_ctx);

    render_gui();
}

void render_gui()
{
    const struct nk_command* cmd = 0;
    nk_foreach(cmd, &g_nk_ctx) switch (cmd->type)
    {
        case NK_COMMAND_NOP:
            break;
        case NK_COMMAND_SCISSOR:
            break;
        case NK_COMMAND_LINE:
        {
            struct nk_command_line* c = (struct nk_command_line*)cmd;
            DrawLineEx(to_ray_vec(c->begin), to_ray_vec(c->end), c->line_thickness,
                       to_ray_color(c->color));
            break;
        }
        case NK_COMMAND_CURVE:
            break;
        case NK_COMMAND_RECT:
        {
            struct nk_command_rect* c = (struct nk_command_rect*)cmd;
            Rectangle rec = {c->x, c->y, c->w, c->h};
            DrawRectangleLinesEx(rec, c->line_thickness, to_ray_color(c->color));
            break;
        }
        case NK_COMMAND_RECT_FILLED:
        {
            struct nk_command_rect_filled* c = (struct nk_command_rect_filled*)cmd;
            DrawRectangleV((Vector2){c->x, c->y}, (Vector2){c->w, c->h},
                           to_ray_color(c->color));
            break;
        }
        case NK_COMMAND_RECT_MULTI_COLOR:
            break;
        case NK_COMMAND_CIRCLE:
        {
            struct nk_command_circle* c = (struct nk_command_circle*)cmd;
            DrawCircleLines(c->x, c->y, c->w, to_ray_color(c->color));
            break;
        }
        case NK_COMMAND_CIRCLE_FILLED:
        {
            struct nk_command_circle_filled* c = (struct nk_command_circle_filled*)cmd;
            DrawCircle(c->x + c->w / 2, c->y + c->h / 2, 8, to_ray_color(c->color));
            break;
        }
        case NK_COMMAND_ARC:
            break;
        case NK_COMMAND_ARC_FILLED:
            break;
        case NK_COMMAND_TRIANGLE:
            break;
        case NK_COMMAND_TRIANGLE_FILLED:
            break;
        case NK_COMMAND_POLYGON:
            break;
        case NK_COMMAND_POLYGON_FILLED:
            break;
        case NK_COMMAND_POLYLINE:
            break;
        case NK_COMMAND_TEXT:
        {
            // I can't use dynamic_cast, because commands are all C structs :(.
            struct nk_command_text* c = (struct nk_command_text*)cmd;
            DrawTextEx(g_ray_font, c->string, (Vector2){c->x, c->y}, c->height, 2,
                       to_ray_color(c->foreground));
            break;
        }
        case NK_COMMAND_IMAGE:
            break;
        case NK_COMMAND_CUSTOM:
            break;
    }

    nk_clear(&g_nk_ctx);
}

float calc_text_width(nk_handle handle, float height, const char* text, int len)
{
    return MeasureTextEx(*(Font*)handle.ptr, text, height, 1).x;
}

void load_fonts()
{
    g_ray_font = LoadFontEx("res/Roboto-Light.ttf", 16, NULL, 250);

    g_nk_font.userdata.ptr = &g_ray_font;
    g_nk_font.height = g_ray_font.baseSize;
    g_nk_font.width = calc_text_width;
}

void handle_input(float delta)
{
    nk_input_begin(&g_nk_ctx);

    nk_input_motion(&g_nk_ctx, GetMouseX(), GetMouseY());
    nk_input_button(&g_nk_ctx, NK_BUTTON_LEFT, GetMouseX(), GetMouseY(),
                    IsMouseButtonDown(MOUSE_BUTTON_LEFT));
    nk_input_button(&g_nk_ctx, NK_BUTTON_RIGHT, GetMouseX(), GetMouseY(),
                    IsMouseButtonDown(MOUSE_BUTTON_RIGHT));

    nk_input_end(&g_nk_ctx);

    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
    {
        Vector2 mouse_pos = sim_screen_to_world(
            (GetMouseX() - g_camera.offset.x + g_camera.target.x) * g_camera.zoom,
            (GetMouseY() - g_camera.offset.y + g_camera.target.y) * g_camera.zoom);
        sim_make_alive(mouse_pos.x, mouse_pos.y);
    }

    if (IsKeyPressed(KEY_N))
    {
        g_camera.zoom /= 2.0f;
    }
    else if (IsKeyPressed(KEY_M))
    {
        g_camera.zoom *= 2.0f;
    }

    if (g_camera.zoom > 10.0f)
        g_camera.zoom = 10.0f;

    if (IsKeyDown(KEY_W))
    {
        g_camera.target.y -= g_camera_speed * delta;
    }
    if (IsKeyDown(KEY_S))
    {
        g_camera.target.y += g_camera_speed * delta;
    }
    if (IsKeyDown(KEY_A))
    {
        g_camera.target.x -= g_camera_speed * delta;
    }
    if (IsKeyDown(KEY_D))
    {
        g_camera.target.x += g_camera_speed * delta;
    }
}

void end()
{
    CloseWindow();
    UnloadFont(g_ray_font);
    nk_free(&g_nk_ctx);
}
