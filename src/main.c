#include "raylib.h"
#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
[TODO]
Needs refactoring into headers and stuff.

Fix ratios other than 1:1 crashing.

Could add a RuleSet struct that contains the array of rules and it's length. Wouldn't need to provide rule_length to update.

Add a function to generate a color palatte either randomly or with some provided input.

Add UI to allow entering custom rules through UI. (Make sure this is safe)
Add keybind to hide/show UI.

[Performance]
This doesn't really matter right now. If I wan't to do some really large grids then maybe.
Profile and optimise hotpath (most likely draw).
    Current guess is that transitioning from drawing each rect to drawing a texture that contains the data might be faster.

Compare the performance of the if-else-if and % sections.
*/

const int direction_count = 4;
enum Direction {
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

typedef struct Rule {
    int direction_modifier;
    Color color;
} Rule;

Rule new_rule(enum Direction turn_to, Color color) {
    Rule new_rule;
    new_rule.color = color;

    if (turn_to == LEFT) {
        new_rule.direction_modifier = -1;
    } else if (turn_to == RIGHT) {
        new_rule.direction_modifier = 1;
    }

    return new_rule;
}

typedef struct {
    int pos_x;
    int pos_y;
    enum Direction direction;
} Ant;

typedef struct {
    int rule_index;
} GridCell;

typedef struct {
    int width;
    int height;
} Size2D;

typedef struct {
    Size2D size;
    Ant ant;
    Rule* rules;
    int rules_length;
    Camera2D camera;
    int iterations;
    bool running;
} State;

void update(State* state, GridCell grid[state->size.height][state->size.width]);
void draw(const State* state, const GridCell grid[state->size.height][state->size.width]);
void input_manager(State* state, GridCell grid[state->size.height][state->size.width]);

Rule* create_rules(const char* rule_text, int rule_size, Color* colors);
Color* gradient(int length, float r_multiplier, float g_multiplier, float b_multiplier, bool inverted);

Color* gradient(int length, float r_multiplier, float g_multiplier, float b_multiplier, bool inverted) {
    Color* colors = malloc(sizeof(Color) * length);
    for (int i = 0; i < length; i++) {
        // If inverted, 'index = (length-1) - i' else 'index = i'
        int index = inverted ? (length - 1) - i : i;
        colors[index].r = (255 / 7) * i * r_multiplier;
        colors[index].g = (255 / 7) * i * g_multiplier;
        colors[index].b = (255 / 7) * i * b_multiplier;
        colors[i].a = 255;
    }
    return colors;
}

Color* gray_scale(int length) { return gradient(length, 1.0, 1.0, 1.0, false); }

Color* gray_scale_inverted(int length) { return gradient(length, 1.0, 1.0, 1.0, true); }

int main(int argc, char* argv[]) {
    InitWindow(1000, 1000, "Langton's Ant");
    int grid_w = 800;
    int grid_h = 800;

    GridCell(*grid)[grid_h] = malloc(grid_w * grid_h * sizeof(GridCell));

    if (!grid) {
        printf("Failed to allocate memory for grid.");
        return -1;
    }

    // char rule_text[] = "RRLLLRLLLLL";
    char rule_text[] = "RLLR";
    int rule_length = strlen(rule_text);
    Rule* rules = create_rules(rule_text, rule_length, gray_scale_inverted(rule_length));

    // instantiate grid
    for (int y = 0; y < grid_h; y++) {
        for (int x = 0; x < grid_w; x++) {
            GridCell* cell = &grid[y][x];
            cell->rule_index = 0;
        }
    }

    SetTargetFPS(120);

    State state;
    // state.ant = ant;
    state.ant.pos_x = grid_w/2;
    state.ant.pos_y = grid_h/2;
    state.ant.direction = 0;
    
    state.camera.offset.x = GetScreenWidth() /2;
    state.camera.offset.y = GetScreenHeight() /2;
    state.camera.rotation = 0;
    state.camera.target.x = GetScreenWidth() / 2;
    state.camera.target.y = GetScreenHeight() / 2;
    state.camera.zoom = 1;

    state.iterations = 500;
    
    state.rules = rules;
    state.rules_length = rule_length;
    
    state.size.width = grid_w;
    state.size.height = grid_h;

    state.running = true;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            state.camera.target.x = GetScreenWidth() / 2;
            state.camera.target.y = GetScreenHeight() / 2;
            state.camera.target.x = GetScreenWidth() / 2;
            state.camera.target.y = GetScreenHeight() / 2;
        }

        input_manager(&state, grid);
        update(&state, grid);
        draw(&state, grid);
    }
    CloseWindow();

    free(grid);
    free(rules);
    return 0;
}

#ifdef _WIN32
int WinMain() { return main(0, NULL); }
#endif // _WIN32

void update(State* state, GridCell grid[state->size.height][state->size.width]) {
    // Kinda jank
    if(!state->running){
        return;
    }

    for (int i = 0; i < state->iterations; i++) {
        GridCell* ant_cell = &(grid[state->ant.pos_y][state->ant.pos_x]);
        int border = 5;
        // This if statement is very long.
        if (state->ant.pos_x >= state->size.width - border || state->ant.pos_x <= border || state->ant.pos_y <= border ||
            state->ant.pos_y >= state->size.height - border) {
            // state->ant.direction = (state->ant.direction+2) % 4;
            // Compare these two methods. Chances are the if-else-if is much faster than % but it's worth testing.
            state->ant.direction += 2;
            if (state->ant.direction > 3) {
                state->ant.direction -= 3;
            }
        } else {
            // this one works on linux
            state->ant.direction = (state->ant.direction + state->rules[ant_cell->rule_index].direction_modifier) % 4;

            // this one works on windows but not linux. I've no idea why
            //(state->ant.direction += rules[ant_cell->rule_index].direction_modifier;
            // if (state->ant.direction > 3) {
            //     state->ant.direction = 0;
            // }
            // else if (state->ant.direction < 0) {
            //     state->ant.direction = 3;
            // }

            // ant_cell->rule_index = (ant_cell->rule_index+1)%rules_length;
            ant_cell->rule_index += 1;
            if (ant_cell->rule_index >= state->rules_length) {
                ant_cell->rule_index = 0;
            }
        }

        // This could be improved
        if (state->ant.direction == UP) {
            state->ant.pos_y -= 1;
        } else if (state->ant.direction == RIGHT) {
            state->ant.pos_x += 1;
        } else if (state->ant.direction == DOWN) {
            state->ant.pos_y += 1;
        } else if (state->ant.direction == LEFT) {
            state->ant.pos_x -= 1;
        } else {
            printf("Invalid direction\n");
            exit(-1);
        }
    }
}

void draw(const State* state, const GridCell grid[state->size.height][state->size.width]) {
    // [Optimisation]
    // Could draw only the new squares and leave squares from previous draws.
    //      All squares will need to be redrawn on zoom in/out or camera movement
    // Screen width and height could be passed in. (minor)
    // Change the current system of drawing a rectangle for each tile into drawing a single texture that contains all the data.
    BeginDrawing();
    BeginMode2D(state->camera);

    ClearBackground(state->rules[0].color);

    Vector2 rect_size = {
        ((float)GetScreenWidth() / (float)state->size.width),
        ((float)GetScreenHeight() / (float)state->size.height),
    };

    for (int y = 0; y < state->size.height; y++) {
        for (int x = 0; x < state->size.width; x++) {
            GridCell* cell = &grid[y][x];
            if (cell->rule_index != 0) {
                Vector2 pos = {
                    (rect_size.x * x),
                    (rect_size.y * y),
                };
                DrawRectangleV(pos, rect_size, state->rules[cell->rule_index].color);
            }
        }
    }

    // Draws fps
    EndMode2D();
    // DrawRectangle(0,0,50,25,WHITE);
    //  DrawText(TextFormat("FPS:%d", GetFPS()), 5, 5, 10, BLACK);
    //  Draws mouse X and Y next to mouse
    //  DrawText(TextFormat("x:%d. y:%d", GetMouseX(), GetMouseY()), GetMouseX(), GetMouseY(), 20, RED);
    EndDrawing();
}

void input_manager(State* state, GridCell grid[state->size.height][state->size.width]) {

    Vector2 rect_size = {
        ((float)GetScreenWidth() / (float)state->size.width),
        ((float)GetScreenHeight() / (float)state->size.height),
    };
    // Consider scaling zoom incrememnt with current zoom to avoid zooming acceleration
    if (IsKeyDown(KEY_UP)) {
        state->camera.zoom += 0.01;
    }
    if (IsKeyDown(KEY_DOWN)) {
        state->camera.zoom -= 0.01;
    }
    if (IsKeyDown(KEY_W)) {
        state->camera.offset.y += 1;
    }
    if (IsKeyDown(KEY_S)) {
        state->camera.offset.y -= 1;
    }
    if (IsKeyDown(KEY_A)) {
        state->camera.offset.x += 1;
    }
    if (IsKeyDown(KEY_D)) {
        state->camera.offset.x -= 1;
    }
    if (IsKeyReleased(KEY_SPACE)) {
        state->running = !state->running;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mpos = GetMousePosition();
        Vector2 relative_mpos = GetScreenToWorld2D(mpos, state->camera);

        int x_cord = (int)(relative_mpos.x / rect_size.x);
        int y_cord = (int)(relative_mpos.y / rect_size.y);
        if (x_cord < state->size.width && y_cord < state->size.height) {
            grid[y_cord][x_cord].rule_index = 2;
        }
    }
}

Rule* create_rules(const char* rule_text, int rule_size, Color* colors) {
    Rule* rules = (Rule*)malloc(rule_size * sizeof(Rule));
    for (int i = 0; i < rule_size; i++) {
        if (rule_text[i] == 'L') {
            rules[i].direction_modifier = -1;
        } else if (rule_text[i] == 'R') {
            rules[i].direction_modifier = 1;
        }
    }

    if (colors == NULL) {
        for (int i = 0; i < rule_size; i++) {
            rules[i].color.r = rand() % 255;
            rules[i].color.g = rand() % 255;
            rules[i].color.b = rand() % 255;
            rules[i].color.a = 255;
        }
    } else {
        for (int i = 0; i < rule_size; i++) {
            rules[i].color = colors[i];
        }
    }
    return rules;
}
