#include "raylib.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>



enum Direction{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

typedef struct{
    int pos_x;
    int pos_y;
    enum Direction direction;
} Ant;

enum CellState{
    WHITE_CELL,
    BLACK_CELL,
};

typedef struct {
    enum CellState state;
} GridCell;

// should probably just use a rectangle struct
typedef struct {
    int width;
    int height;
    // int depth; // for 3D
} Size2D;

void update(Size2D* size, GridCell grid[size->height][size->width], Ant* ant, int iterations);
// is it even worth putting this in a function?
void draw(Size2D* size, GridCell grid[size->height][size->width]);

int main(void) {
    // Vector2 center = {width / 2, height / 2};
    InitWindow(900, 900, "Langton's Ant");
    int grid_w = 300;
    int grid_h = 300;

    Size2D grid_size = {grid_w, grid_h};
    GridCell (*grid)[grid_h] = malloc(grid_w * grid_h * sizeof(GridCell));

    // instantiate grid
    for (int y = 0; y < grid_h; y++) {
        for (int x = 0; x < grid_w; x++) {
            GridCell* cell = &grid[y][x];
            cell->state = WHITE_CELL;
        }
    }

    int iterations = 250;
    Ant ant = {
        grid_size.width/2,
        grid_size.height/2,
        0,
    };

    SetTargetFPS(30);
    while (!WindowShouldClose()) {
        printf("%d, %d \n", ant.pos_x, ant.pos_y);
        update(&grid_size, grid, &ant, 500);
        draw(&grid_size, grid);
    }
    CloseWindow();

    return 0;
}

void update(Size2D* size, GridCell grid[size->height][size->width], Ant* ant, int iterations) {
    // This can be optimised probably
    for(int i = 0; i < iterations; i++){
        GridCell* ant_cell = &grid[ant->pos_y][ant->pos_x];
        if (ant->pos_x > size->width || ant->pos_x < 0 || ant->pos_y < 0 || ant->pos_y > size->height){  
            ant->direction = (ant->direction+2)%4;
        }else{
            if (ant_cell->state == BLACK_CELL){
                ant->direction = (ant->direction-1)%4;
            }else{
                ant->direction = (ant->direction+1)%4;
            }
            ant_cell->state = (ant_cell->state+1)%2;
        }
        

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

void draw(Size2D* size, GridCell grid[size->height][size->width]) {
    BeginDrawing();
    ClearBackground(BLACK);

    Vector2 rect_size = {
        (float)GetScreenWidth() / (float)size->width,
        (float)GetScreenHeight() / (float)size->height,
    };

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        int x_cord = GetMouseX() / rect_size.x;
        int y_cord = GetMouseY() / rect_size.y;
        grid[y_cord][x_cord].state = BLACK_CELL;
    }

    for (int y = 0; y < size->height; y++) {
        for (int x = 0; x < size->width; x++) {
            GridCell* cell = &grid[y][x];
            Vector2 pos = {rect_size.x * x, rect_size.y * y};

            if (cell->state == WHITE_CELL){
                Color col = {255,255,255,255};
                
                DrawRectangleV(pos, rect_size, col);
            }
        }
    }

    DrawText(TextFormat("x:%d. y:%d", GetMouseX(), GetMouseY()), GetMouseX(), GetMouseY(), 20, RED);
    EndDrawing();
}
