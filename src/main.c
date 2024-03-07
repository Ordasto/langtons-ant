#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/*
Needs refactoring into headers and stuff.
Make a function to generate rules with a provided color palate or range

*/

enum Direction{
    UP,
    RIGHT,
    DOWN,
    LEFT,
    NUMBER_OF_DIRECTIONS
};

typedef struct Rule{
    // enum Direction direction; // Don't really need this
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

// should probably just use a rectangle struct
typedef struct {
    int width;
    int height;
    // int depth; // for 3D
} Size2D;

void update(Size2D* size, GridCell grid[size->height][size->width], Ant* ant, int iterations, Rule* rules, int rules_length);
void draw(Size2D* size, GridCell grid[size->height][size->width], Rule* rules);
Rule* create_rules(const char* rule_text, Color* colors);

int main(void) {
    InitWindow(900, 900, "Langton's Ant");
    int grid_w = 400;
    int grid_h = 400;

    Size2D grid_size = {grid_w, grid_h};
    GridCell (*grid)[grid_h] = malloc(grid_w * grid_h * sizeof(GridCell));

    // Rule symmetric[] = {
    //     new_rule(LEFT, BLACK),
    //     new_rule(LEFT, BLUE),
    //     new_rule(RIGHT, RED),
    //     new_rule(RIGHT, GREEN)
    // };

    const char* rule_text = "LRRRRRLLR";
    Rule* rules = create_rules(rule_text, NULL);

    int rule_length = sizeof(rule_text);
    printf("%d\n", rule_length);

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

    SetTargetFPS(-1);
    int iterations = 1000;
    while (!WindowShouldClose()) {
        // printf("%d, %d \n", ant.pos_x, ant.pos_y);
        // printf("%d\n", GetFPS());
        update(&grid_size, grid, &ant, iterations, rules, rule_length);
        draw(&grid_size, grid, rules);
    }
    CloseWindow();

    return 0;
}

void update(Size2D* size, GridCell grid[size->height][size->width], Ant* ant, int iterations, Rule* rules, int rules_length) {
    // This can be optimised probably
    for(int i = 0; i < iterations; i++){
        GridCell* ant_cell = &grid[ant->pos_y][ant->pos_x];
        if (ant->pos_x > size->width || ant->pos_x < 0 || ant->pos_y < 0 || ant->pos_y > size->height){  
            ant->direction = (ant->direction+2)%4;
        }

        ant->direction = (ant->direction+rules[ant_cell->rule_index].direction_modifier)%NUMBER_OF_DIRECTIONS;
        ant_cell->rule_index = (ant_cell->rule_index+1)%rules_length;


        if (ant->direction == UP){
            ant->pos_y-=1;
        }else if (ant->direction == RIGHT){
            ant->pos_x+=1;
        }else if (ant->direction == DOWN){
            ant->pos_y+=1;
        }else{
            ant->pos_x-=1;
        }

        // ant->pos_x = ant->pos_x%size->width;
        // ant->pos_y = ant->pos_y%size->height;

    }
    
}

void draw(Size2D* size, GridCell grid[size->height][size->width], Rule* rules) {
    // [Optimisation]
    // Could draw only the new squares and leave squares from previous draws
    
    BeginDrawing();
    ClearBackground(rules[0].color);


    Vector2 rect_size = {
        (float)GetScreenWidth() / (float)size->width,
        (float)GetScreenHeight() / (float)size->height,
    };

    // This should probably be put in 'update' or a seperate function.
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        int x_cord = GetMouseX() / rect_size.x;
        int y_cord = GetMouseY() / rect_size.y;
        // add ability to change size and color for mouse
        grid[y_cord][x_cord].rule_index = 2;
    }

    for (int y = 0; y < size->height; y++) {
        for (int x = 0; x < size->width; x++) {
            GridCell* cell = &grid[y][x];
            Vector2 pos = {rect_size.x * x, rect_size.y * y};
            Color cell_color = rules[cell->rule_index].color;
            // First rule is default state
            if(cell->rule_index != 0){
                // DrawRectangleV(pos, rect_size, rules[cell->rule_index].color);
                DrawRectangleV(pos, rect_size, rules[cell->rule_index].color);

            }
        }
    }
    DrawRectangle(0,0,50,25,WHITE);
    DrawText(TextFormat("FPS:%d", GetFPS()),5,5,10,BLACK);
    
    // DrawText(TextFormat("x:%d. y:%d", GetMouseX(), GetMouseY()), GetMouseX(), GetMouseY(), 20, RED);
    EndDrawing();
}


Rule* create_rules(const char* rule_text, Color* colors){
    Rule* rules = (Rule*)malloc(sizeof(rule_text)*sizeof(Rule));

    if(!colors){
        
    }
}

