#include "raylib.h"
#include <raymath.h>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;
const float ballSpeed = 150.0f; // ボールの移動速度（ピクセル毎秒）
const float angleIncrement = 360.0f / 24.0f; // 曲がる角度の増分
const float ballSpacing = 20.0f; // ボール間の距離
const int numFood = 10; // エサの数
const int initialNumObstacles = 15; // 初期の障害物の数

struct Ball {
    Vector2 position;
    float radius;
    float angle; // 現在の移動方向の角度（度）
};

struct Food {
    Vector2 position;
    float radius;
    bool active;
};

struct Obstacle {
    Vector2 position;
    float radius;
};

// マウスカーソルがボールの進行方向に対して左右どちらにあるかを判定する関数
int GetMouseDirectionRelativeToBall(const Vector2& ballPosition, float ballAngle, const Vector2& mousePosition) {
    Vector2 direction = { cos(ballAngle * PI / 180.0f), sin(ballAngle * PI / 180.0f) };
    Vector2 toMouse = { mousePosition.x - ballPosition.x, mousePosition.y - ballPosition.y };
    float crossProduct = direction.x * toMouse.y - direction.y * toMouse.x;

    if (crossProduct > 0) return 1;  // マウスは進行方向の左側にある
    if (crossProduct < 0) return -1; // マウスは進行方向の右側にある
    return 0; // マウスは進行方向の真上にある
}

// ランダムにエサを配置する関数
void InitFood(std::vector<Food>& food) {
    for (auto& f : food) {
        f.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        f.radius = 5.0f;
        f.active = true;
    }
}

// ランダムに障害物を配置する関数
void InitObstacles(std::vector<Obstacle>& obstacles) {
    for (auto& o : obstacles) {
        o.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        o.radius = 5.0f; // 障害物の半径を5.0fに設定
    }
}

// 障害物を1つランダムに配置する関数
void AddObstacle(std::vector<Obstacle>& obstacles) {
    Obstacle newObstacle;
    newObstacle.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
    newObstacle.radius = 5.0f; // 障害物の半径を5.0fに設定
    obstacles.push_back(newObstacle);
}

int main() {
    InitWindow(screenWidth, screenHeight, "Ball Movement Example");

    std::vector<Ball> balls(3);
    balls[0].position = { screenWidth / 2.0f, screenHeight / 2.0f };
    balls[0].radius = 5.0f; // ヘビの半径を5.0fに設定
    balls[0].angle = 0.0f;

    // 残りのボールを先頭のボールの位置から一定間隔で配置
    for (int i = 1; i < balls.size(); ++i) {
        balls[i].radius = 5.0f; // ヘビの半径を5.0fに設定
        balls[i].angle = 0.0f;
        balls[i].position = balls[i - 1].position;
    }

    std::vector<Food> food(numFood);
    InitFood(food);

    std::vector<Obstacle> obstacles(initialNumObstacles);
    InitObstacles(obstacles);

    int score = 0;
    bool gameOver = false;
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (!gameOver) {
            Vector2 mousePosition = GetMousePosition();
            float targetAngle = atan2(mousePosition.y - balls[0].position.y, mousePosition.x - balls[0].position.x) * (180.0f / PI);

            // 角度の差を計算し、適切な方向にボールを回転させる
            float angleDifference = targetAngle - balls[0].angle;
            if (angleDifference > 180) angleDifference -= 360;
            if (angleDifference < -180) angleDifference += 360;

            if (fabs(angleDifference) > angleIncrement) {
                if (angleDifference > 0) {
                    balls[0].angle += angleIncrement;
                }
                else {
                    balls[0].angle -= angleIncrement;
                }
            }
            else {
                balls[0].angle = targetAngle;
            }

            // 先頭のボールの位置を更新
            balls[0].position.x += ballSpeed * cos(balls[0].angle * PI / 180.0f) * GetFrameTime();
            balls[0].position.y += ballSpeed * sin(balls[0].angle * PI / 180.0f) * GetFrameTime();

            // 残りのボールが先頭のボールを追随するように位置を更新
            for (int i = 1; i < balls.size(); ++i) {
                balls[i].position = Vector2Lerp(balls[i].position, balls[i - 1].position, 0.2f);
            }

            // エサとの衝突判定
            for (auto& f : food) {
                if (f.active && CheckCollisionCircles(balls[0].position, balls[0].radius, f.position, f.radius)) {
                    f.active = false;
                    score++;
                    f.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
                    f.active = true;
                    AddObstacle(obstacles); // エサを食べるごとに障害物を1つ追加
                }
            }

            // 障害物との衝突判定
            for (const auto& o : obstacles) {
                if (CheckCollisionCircles(balls[0].position, balls[0].radius, o.position, o.radius)) {
                    gameOver = true;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK); // バックグラウンドの色を黒に変更

        for (const auto& f : food) {
            if (f.active) {
                DrawCircleV(f.position, f.radius, RED); // エサの色を赤に変更
            }
        }

        for (const auto& o : obstacles) {
            DrawCircleV(o.position, o.radius, DARKGRAY); // 障害物の色をダークグレイに変更
        }

        for (const auto& ball : balls) {
            DrawCircleV(ball.position, ball.radius, GREEN); // ヘビの色を緑に変更
        }

        if (gameOver) {
            DrawText("Game Over! Press R to Retry", screenWidth / 2 - MeasureText("Game Over! Press R to Retry", 20) / 2, screenHeight / 2, 20, DARKGRAY);
        }
        else {
            DrawText("Move the ball with your mouse direction", 10, 10, 20, DARKGRAY);
            DrawText(TextFormat("Score: %d", score), 10, 40, 20, DARKGRAY);
        }

        EndDrawing();

        if (gameOver && IsKeyPressed(KEY_R)) {
            // ゲームをリセット
            balls[0].position = { screenWidth / 2.0f, screenHeight / 2.0f };
            balls[0].angle = 0.0f;
            for (int i = 1; i < balls.size(); ++i) {
                balls[i].position = balls[i - 1].position;
            }
            InitFood(food);
            obstacles.clear();
            InitObstacles(obstacles); // 障害物を初期値に戻す
            score = 0;
            gameOver = false;
        }
    }

    CloseWindow();

    return 0;
}
