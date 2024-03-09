#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
[TODO]
Needs refactoring into headers and stuff.

Fix ratios other than 1:1 crashing.

Could add a RuleSet struct that contains the array of rules and it's length. Wouldn't need to provide rule_length to update.

Add a function to generate rules with a provided color palatte or range 
Add a function to generate a color palatte either randomly or with some provided input.

Add keybinds to move/zoom "camera".

Add UI to allow entering custom rules through UI. (Make sure this is safe)
Add keybind to hide/show UI.


[Performance]
This doesn't really matter right now. If I wan't to do some really large grids then maybe.
Profile and optimise hotpath (most likely draw).
    Current guess is that transitioning from drawing each rect to drawing a texture that contains the data might be faster.

Compare the performance of the if-else-if and % sections.
*/

const int direction_count = 4;
enum Direction{
    UP,
    RIGHT,
    DOWN,
    LEFT,
};

typedef struct Rule{
    int direction_modifier;
    Color color;
} Rule;

Rule new_rule(enum Direction turn_to, Color color){
    Rule new_rule;
    new_rule.color = color;

    if(turn_to == LEFT){
        new_rule.direction_modifier = -1;
    }
    else if(turn_to == RIGHT){
        new_rule.direction_modifier = 1;
    }

    return new_rule;
}

typedef struct{
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


void update(Size2D* size, GridCell grid[size->height][size->width], Ant* ant, int iterations, Rule* rules, int rules_length);
void draw(Size2D* size, GridCell grid[size->height][size->width], Rule* rules, float zoom, Vector2 camera_position);
Rule* create_rules(const char* rule_text, int rule_size, Color* colors);
Color* gradient(int length, float r_multiplier, float g_multiplier, float b_multiplier, bool inverted);

Color* gradient(int length, float r_multiplier, float g_multiplier, float b_multiplier, bool inverted) {
    Color* colors = malloc(sizeof(Color) * length);
    for (int i = 0; i < length; i++) {
        // If inverted, 'index = (length-1) - i' else 'index = i'
        int index = inverted ? (length-1) - i : i;
        colors[index].r = (255 / 7) * i * r_multiplier;
        colors[index].g = (255 / 7) * i * g_multiplier;
        colors[index].b = (255 / 7) * i * b_multiplier;
        colors[i].a = 255;
    }
    return colors;
}

Color* gray_scale(int length) {
    return gradient(length, 1.0, 1.0, 1.0, false);
}

Color* gray_scale_inverted(int length) {
    return gradient(length, 1.0, 1.0, 1.0, true);
}

int main(int argc, char *argv[]) {
    InitWindow(1000, 1000, "Langton's Ant");
    float zoom = 1.0f;
    int grid_w = 800;
    int grid_h = 800;

    Size2D grid_size = {grid_w, grid_h};
    GridCell (*grid)[grid_h] = malloc(grid_w * grid_h * sizeof(GridCell));
    Vector2 camera_position = {
        0,
        0
    };

    if (!grid) {
        printf("Failed to allocate memory for grid.");
        return -1;
    }

    char rule_text[] = "RRLLLRLLLLL";
    //char rule_text[]  = "RLLR";
    int rule_length = strlen(rule_text);
    Rule* rules = create_rules(rule_text, rule_length, gray_scale_inverted(rule_length));

    // instantiate grid
    for (int y = 0; y < grid_h; y++) {
        for (int x = 0; x < grid_w; x++) {
            GridCell* cell = &grid[y][x];
            cell->rule_index = 0;
        }
    }

    Ant ant = {
        grid_size.width/2,
        grid_size.height/2,
        0,
    };

    SetTargetFPS(120);
    int iterations = 100;
    zoom = 1;
    while (!WindowShouldClose()) {
        
        // MOVE TO INPUT MANAGER
        // Add keybind to pause simulation and speedup/slowdown simulation (effect iterations)
        if (IsKeyReleased(KEY_UP)) {
            zoom += 0.1;
        }
        if (IsKeyReleased(KEY_DOWN)) {
            zoom -= 0.1;
        }
        if (IsKeyDown(KEY_W)) {
            camera_position.y -= 1;
        }
        if (IsKeyDown(KEY_S)) {
            camera_position.y += 1;
        }
        if (IsKeyDown(KEY_A)) {
            camera_position.x -= 1;
        }
        if (IsKeyDown(KEY_D)) {
            camera_position.x += 1;
        }
        update(&grid_size, grid, &ant, iterations, rules, rule_length);
        draw(&grid_size, grid, rules, zoom, camera_position);
    }
    CloseWindow();

    return 0;
}

#ifdef _WIN32
int WinMain() {
    return main(0, NULL);
}
#endif // _WIN32


void update(Size2D* size, GridCell grid[size->height][size->width], Ant* ant, int iterations, Rule* rules, int rules_length) {
    for(int i = 0; i < iterations; i++){
        GridCell* ant_cell = &grid[ant->pos_y][ant->pos_x];
        int border = 5;
        // This if statement is very long.
        if (ant->pos_x >= size->width - border || ant->pos_x <= border || ant->pos_y <= border || ant->pos_y >= size->height - border){  
            // ant->direction = (ant->direction+2) % 4;
            // Compare these two methods. Chances are the if-else-if is much faster than % but it's worth testing.
            ant->direction += 2;
            if (ant->direction > 3) {
                ant->direction -= 3;
            }
        }
        else {
            // ant->direction = (ant->direction+rules[ant_cell->rule_index].direction_modifier) % NUMBER_OF_DIRECTIONS;
            // Compare these two methods. Chances are the if-else-if is much faster than % but it's worth testing.
            int direction_modifier = rules[ant_cell->rule_index].direction_modifier;
            ant->direction += direction_modifier;

            if (ant->direction > 3) {
                ant->direction = 0;
            }
            else if (ant->direction < 0) {
                ant->direction = 3;
            }

            ant_cell->rule_index = (ant_cell->rule_index+1)%rules_length;
        }

        // This could be improved 
        if (ant->direction == UP){
            ant->pos_y-=1;
        }else if (ant->direction == RIGHT){
            ant->pos_x+=1;
        }else if (ant->direction == DOWN){
            ant->pos_y+=1;
        }else if (ant->direction == LEFT){
            ant->pos_x-=1;
        }else {
            printf("Invalid direction\n");
            exit(-1);
        }

    }
    
}

void draw(Size2D* size, GridCell grid[size->height][size->width], Rule* rules, float zoom, Vector2 camera_position) {
    // [Optimisation]
    // Could draw only the new squares and leave squares from previous draws.
    //      All squares will need to be redrawn on zoom in/out or camera movement
    // Screen width and height could be passed in. (minor)
    // Change the current system of drawing a rectangle for each tile into drawing a single texture that contains all the data.

    // Zoom = 1, stays at center of screen
    // zoom = 2, moves to bottom right, everything is moved screen/zoom 

    BeginDrawing();
    Vector2 center = {
        (GetScreenWidth() / 2),
        (GetScreenHeight() / 2),
    };

    Vector2 offset = center;
    offset.x += camera_position.x;
    offset.y += camera_position.y;

    Camera2D camera;
    camera.offset = offset;
    camera.rotation = 0;
    camera.target = center;
    camera.zoom = zoom;

    BeginMode2D(camera);

    ClearBackground(rules[0].color);

    Vector2 rect_size = {
        ((float)GetScreenWidth() / (float)size->width),
        ((float)GetScreenHeight() / (float)size->height),
    };

    // This should probably be put in 'update' or a seperate input handling function.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        // Need to translate screen coords to game coords
        // add ability to change size and color for mouse
        int x_cord = GetMouseX()/ rect_size.x;
        int y_cord = GetMouseY()/ rect_size.y;

        grid[y_cord][x_cord].rule_index = 2;
    }

    for (int y = 0; y < size->height; y++) {
        for (int x = 0; x < size->width; x++) {
            GridCell* cell = &grid[y][x];
            if (cell->rule_index != 0) {
                Vector2 pos = {
                    (rect_size.x * x),
                    (rect_size.y * y),
                };
                DrawRectangleV(pos, rect_size, rules[cell->rule_index].color);

            }
        }
    }
    //DrawRectangle(0,0,50,25,WHITE);
    // Draws fps
    EndMode2D();
    DrawText(TextFormat("FPS:%d", GetFPS()), 5, 5, 10, BLACK);
    DrawText(TextFormat("ZOOM:%f", zoom), 5, 20, 10, BLACK);
    // Draws mouse X and Y next to mouse
     DrawText(TextFormat("x:%d. y:%d", GetMouseX(), GetMouseY()), GetMouseX(), GetMouseY(), 20, RED);
    EndDrawing();
}

Rule* create_rules(const char* rule_text,int rule_size, Color* colors){
    Rule* rules = (Rule*)malloc(rule_size * sizeof(Rule));
    for (int i = 0; i < rule_size; i++) {
        if(rule_text[i] == 'L'){
            rules[i].direction_modifier = -1;
        } else if (rule_text[i] == 'R') {
            rules[i].direction_modifier = 1;
        }
    }

    if(colors == NULL){
        for(int i = 0; i < rule_size; i++){
            rules[i].color.r = rand()%255;
            rules[i].color.g = rand()%255;
            rules[i].color.b = rand()%255;
            rules[i].color.a = 255;
        }
    }
    else {
        for (int i = 0; i < rule_size; i++) {
            rules[i].color = colors[i];
        }
    }
    return rules;
}

