#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#define TILE_SIZE 32
#define TITLE "Pacman"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define MAP_ROWS 11
#define MAP_COLS 20
#define collisionTolerance 7
bool IsBlocked(Rectangle rect);
void resetGame();
bool GameOver = false;
bool Paused = false;

int score = 0;
typedef enum { NONE, UP, DOWN, LEFT, RIGHT } Direction;

typedef struct Enemy {
    Vector2 pos;
    Vector2 dir;
    float speed;
    Color color;
} Enemy;

int maze[MAP_ROWS][MAP_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1},
    {1,0,1,0,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,1,0,0,0,0,1,1,0,0,0,1,0,0,0,0,1},
    {1,0,1,1,1,0,1,1,0,1,1,0,1,1,0,1,1,1,0,1},
    {1,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1},
    {1,0,1,0,1,1,0,1,1,1,1,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

void drawMaze(){
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            if (maze[row][col] == 1) {
                DrawRectangle(col * TILE_SIZE, row * TILE_SIZE, TILE_SIZE, TILE_SIZE, BLUE);
            }
        }
    }
}

void drawPellets(){
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            if (maze[row][col] == 0) {
                float centerX = col * TILE_SIZE + TILE_SIZE / 2;
                float centerY = row * TILE_SIZE + TILE_SIZE / 2;
                DrawCircle(centerX, centerY, 3, WHITE); 

            }
        }
    }
}

void isGameOver(){
    for (int row = 0; row < MAP_ROWS; row++) {
        for (int col = 0; col < MAP_COLS; col++) {
            if (maze[row][col] == 0) {
                return; 
            }
        }
    }
    
    DrawText("Game Over! All pellets eaten!", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 20, RED);
    DrawText("Press R to restart", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, 20, RED);
}

void addScoreBox(){
    DrawRectangleRounded((Rectangle){20, 352, 120, 30}, 0.3f, 10, DARKGRAY);
    DrawText(TextFormat("Score: %d", score), 25, 357, 20, RAYWHITE);
}

Direction GetRandomDirection(Vector2 pos) {
    Direction possibleDirs[4];
    int count = 0;

    Vector2 testDirs[4] = {
        {0, -1}, {0, 1}, {-1, 0}, {1, 0}
    };
    Direction dirsEnum[4] = {UP, DOWN, LEFT, RIGHT};

    for (int i = 0; i < 4; i++) {
        Rectangle testBox = {
            pos.x - TILE_SIZE / 2 + testDirs[i].x * TILE_SIZE,
            pos.y - TILE_SIZE / 2 + testDirs[i].y * TILE_SIZE,
            TILE_SIZE, TILE_SIZE
        };

        if (!IsBlocked(testBox)) {
            possibleDirs[count++] = dirsEnum[i];
        }
    }

    if (count == 0) return NONE;
    return possibleDirs[GetRandomValue(0, count - 1)];
}

void DrawEnemy(Enemy enemy) {
    DrawCircleV(enemy.pos, TILE_SIZE / 2 - collisionTolerance +2, enemy.color);
}

bool IsBlocked(Rectangle rect) {
    int topLeftRow = rect.y / TILE_SIZE ;
    int topLeftCol = rect.x / TILE_SIZE ;

    int topRightCol = (rect.x + rect.width - 1) / TILE_SIZE ;
    int bottomRow = (rect.y + rect.height - 1) / TILE_SIZE ;
    int bottomCol = topRightCol;

    int bottomLeftCol = topLeftCol;

    return (
        maze[topLeftRow][topLeftCol] == 1 ||
        maze[topLeftRow][topRightCol] == 1 ||
        maze[bottomRow][bottomLeftCol] == 1 ||
        maze[bottomRow][bottomCol] == 1
    );
}

Enemy UpdateEnemy(Enemy enemy) {
    Rectangle nextBox = {
        enemy.pos.x - TILE_SIZE / 2,
        enemy.pos.y - TILE_SIZE / 2,
        TILE_SIZE,
        TILE_SIZE
    };

    nextBox.x += enemy.dir.x * enemy.speed;
    nextBox.y += enemy.dir.y * enemy.speed;

    if (!IsBlocked(nextBox)) {
        enemy.pos.x += enemy.dir.x * enemy.speed;
        enemy.pos.y += enemy.dir.y * enemy.speed;
    } else {
        // If collision, pick a new direction
        // For now: random valid direction
        Direction newDir = GetRandomDirection(enemy.pos);
        switch (newDir) {
            case UP: enemy.dir = (Vector2){0, -1}; break;
            case DOWN: enemy.dir = (Vector2){0, 1}; break;
            case LEFT: enemy.dir = (Vector2){-1, 0}; break;
            case RIGHT: enemy.dir = (Vector2){1, 0}; break;
            default: break;
        }
    }

    return enemy;
}

void OnCollsion(Enemy enemy, Rectangle pacmanPos) {
    Rectangle enemyBox = {
        enemy.pos.x - TILE_SIZE / 2,
        enemy.pos.y - TILE_SIZE / 2,
        TILE_SIZE,
        TILE_SIZE
    };
    if (CheckCollisionRecs(enemyBox, pacmanPos)) {
      
        GameOver = true;
        DrawText("Game Over! You Died!", SCREEN_WIDTH / 2 -250 , SCREEN_HEIGHT / 2-100, 50, WHITE);
        Paused = true;
        
    }
}


int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE);
    SetTargetFPS(60);
    Direction previosDir = NONE;
    Direction currentDir = NONE;
    Rectangle pacmanPos = { SCREEN_WIDTH / 2-TILE_SIZE/2 + collisionTolerance, SCREEN_HEIGHT / 2-TILE_SIZE/2 + collisionTolerance,TILE_SIZE - collisionTolerance, TILE_SIZE - collisionTolerance};
    float speed = 2.0f;
    Enemy enemy1 = { {TILE_SIZE * 2 -16, TILE_SIZE * 2- 16}, {1, 0}, 1.5f, RED };
    while (!WindowShouldClose()) {
        // Direction input
        if(!Paused){
        if (IsKeyDown(KEY_RIGHT)) currentDir = RIGHT;
        if (IsKeyDown(KEY_LEFT))  currentDir = LEFT;
        if (IsKeyDown(KEY_UP))    currentDir = UP;
        if (IsKeyDown(KEY_DOWN))  currentDir = DOWN;
        if (IsKeyReleased(KEY_RIGHT)) previosDir = RIGHT;
        if (IsKeyReleased(KEY_LEFT))  previosDir = LEFT;
        if (IsKeyReleased(KEY_UP))    previosDir = UP;
        if (IsKeyReleased(KEY_DOWN))  previosDir = DOWN; 
      
        Rectangle nextPos = pacmanPos;
        switch (currentDir) {
            case UP:    nextPos.y -= speed;  break;
            case DOWN:  nextPos.y += speed;  break;
            case LEFT:  nextPos.x -= speed;  break;
            case RIGHT: nextPos.x += speed;  break;
            default: break;
        }

      
        int nextRow = nextPos.y / TILE_SIZE;
        int nextCol = nextPos.x / TILE_SIZE;

      
        if (!IsBlocked(nextPos)) {
            pacmanPos = nextPos; 
        } else {
            currentDir = previosDir; 
        }

        int pacRow = (int)(pacmanPos.y / TILE_SIZE);
        int pacCol = (int)(pacmanPos.x / TILE_SIZE);

  
        if (maze[pacRow][pacCol] == 0) {
            maze[pacRow][pacCol] = 2; 
        score++;
        }
        enemy1 = UpdateEnemy(enemy1);

    }
        Vector2 pacmanPosV ={
            pacmanPos.x + pacmanPos.width / 2 ,
            pacmanPos.y + pacmanPos.height / 2 
        };
        if(IsKeyPressed(KEY_P)){
            Paused = !Paused;
        }
        
        // Rendering
        BeginDrawing();
        ClearBackground(BLACK);
        drawMaze();
        drawPellets();
        DrawCircleV(pacmanPosV ,TILE_SIZE / 2 - collisionTolerance + 2, YELLOW);
        addScoreBox();
        DrawEnemy(enemy1);
        OnCollsion(enemy1, pacmanPos);
        if (Paused && !GameOver) {
            DrawText("Game Paused", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2 - 120, 40, WHITE);
            DrawText("Press P to continue", SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 -50, 40, WHITE);
        }
        isGameOver();
        EndDrawing();   
    }

    CloseWindow();
    return 0;
}
