#include "raylib.h"
#include <raymath.h>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;

const float startR = 10.0f;         // ボール半径の初期値
const int startBallNum = 10;         // ボール数の初期値
const float nomalSpeed = 150.0f;    // 通常スピード（ピクセル毎秒）
const float highSpeed = 250.0f;     // 高速スピード（マウス左ボタン押下時）
const float angleIncrement = 360.0f / 0.75f; // 曲がる角度の増分
const int numFood = 10; // エサの数
const int initialNumObstacles = 1; // 初期の障害物の数

Vector2 ballDirection = { 1, 0 }; // 初期方向: 右
struct Ball {
    Vector2 position;
    float radius;
    float angle; // 現在の移動方向の角度（度）
    bool active;
};

struct Food {
    Vector2 position;
    float radius;
    bool active;
};

struct Obstacle {
    Vector2 position;
    float radius = 6.0f;
    bool active;
};

// ランダムにエサを配置する関数
void InitFood(std::vector<Food>& food) {
    for (auto& f : food) {
        f.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        f.radius = 7.0f;
        f.active = true;
    }
}

// ランダムに障害物を配置する関数
static void InitObstacles(std::vector<Obstacle>& obstacles) {
    for (int i = 0; i < initialNumObstacles; i++)
    {
        obstacles.push_back(Obstacle{});
    }
    for (auto& o : obstacles) {
        o.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        o.active = true;
    }
}

// 障害物を1つランダムに配置する関数
static void AddObstacle(std::vector<Obstacle>& obstacles) {
    Obstacle newObstacle;
    newObstacle.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
    newObstacle.active = true;
    obstacles.push_back(newObstacle);
}

static void InitSnake(std::vector<Ball>& balls) {
    for (int i = 0; i < startBallNum; i++)
    {
        balls.push_back(Ball{});
    }

    // 残りのボールを先頭のボールの位置から一定間隔で配置
    for (int i = 0; i < balls.size(); ++i) {
        balls[i].radius = startR; // ヘビの半径を設定
        balls[i].angle = 0.0f;
        balls[i].position = (i == 0) ? Vector2{ screenWidth / 3.0f, screenHeight / 2.0f } : // 頭の初期位置
            Vector2{ balls[i - 1].position.x - startR * 2.0f, balls[i - 1].position.y };    // 後は前の位置から一定間隔で配置
        //balls[i].position = balls[i - 1].position;
        balls[i].active = true;
    }
}

//// 多角形の中に点が存在するか判定する関数
//bool IsPointInPolygon(int polygonPointsCount, Vector2 polygon[], Vector2 point) {
//    bool isInside = false;
//    for (int i = 0, j = polygonPointsCount - 1; i < polygonPointsCount; j = i++) {
//        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
//            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
//            isInside = true;
//        }
//    }
//    return isInside;
//}

// ４つの点を含む最小の矩形を取得する関数
Rectangle GetRectangleFromPoints(const std::vector<Vector2>& points) {
    float minX = std::min_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.x < b.x; })->x;
    float minY = std::min_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.y < b.y; })->y;
    float maxX = std::max_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.x < b.x; })->x;
    float maxY = std::max_element(points.begin(), points.end(), [](Vector2 a, Vector2 b) { return a.y < b.y; })->y;
    return { minX, minY, maxX - minX, maxY - minY };
}

// ベクトルを指定角度で回転
static Vector2 RotateVector(Vector2 v, float degrees) {
    float radians = degrees * DEG2RAD;
    float cosTheta = cos(radians);
    float sinTheta = sin(radians);
    return { v.x * cosTheta - v.y * sinTheta, v.x * sinTheta + v.y * cosTheta };
}

enum GameScreen { START, GAMEPLAY, GAMEOVER, GAME_END };

int main() {
    InitWindow(screenWidth, screenHeight, "Ball Movement Example");
    SetTargetFPS(60);

    std::vector<Ball> balls{};
    InitSnake(balls);

    std::vector<Food> food(numFood);
    InitFood(food);

    std::vector<Obstacle> obstacles(initialNumObstacles);
    InitObstacles(obstacles);

    float stopTimer = 0.0f;
    int numFoodEaten = 0;
    int lifeNum = 3;
    int score = 0;
    bool gameOver = false;

    while (!WindowShouldClose()) {
        int hitBallNum = 0;
        std::vector<Vector2> vtx = { {},{},{},{} };
        if (!gameOver) {
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // 入力
            Vector2 mousePosition = GetMousePosition();
            float bSpeed = (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) ? highSpeed : nomalSpeed;
            /////////////////////////////////////////////////////////////////////////////////////////////////////////////
            // 更新
            float deltaTime = GetFrameTime();

            if (stopTimer <= 0.0f)
            {
                float targetAngle = atan2f(mousePosition.y - balls[0].position.y, mousePosition.x - balls[0].position.x) * (180.0f / PI);
                // 角度の差を計算し、適切な方向にボールを回転させる
                float angleDifference = targetAngle - balls[0].angle;
                if (angleDifference > 180) angleDifference -= 360;
                if (angleDifference < -180) angleDifference += 360;
                float turnAngle = angleIncrement * deltaTime;
                if (fabs(angleDifference) > turnAngle) {
                    if (angleDifference > 0) {
                        balls[0].angle += turnAngle;
                    }
                    else {
                        balls[0].angle -= turnAngle;
                    }
                }
                else {
                    balls[0].angle = targetAngle;
                }
                ballDirection = Vector2Normalize(Vector2Subtract(mousePosition, balls[0].position));

                Ball bkupBall = { balls[0].position, balls[0].radius, balls[0].angle };
                // 先頭のボールの位置を更新
                balls[0].position.x += bSpeed * cos((balls[0].angle * PI / 180.0f)) * deltaTime;
                balls[0].position.y += bSpeed * sin((balls[0].angle * PI / 180.0f)) * deltaTime;

                // 残りのボールが先頭のボールを追随するように位置を更新
                for (int i = 1; i < balls.size(); ++i) {
                    Vector2 bkPos = { balls[i].position.x, balls[i].position.y };
                    float am = (bSpeed == highSpeed) ? 0.3f : 0.19f;
                    //balls[i].position = Vector2Lerp(balls[i].position, balls[i - 1].position, am);
                    balls[i].position = Vector2Lerp(balls[i].position, bkupBall.position, am);
                    bkupBall = { bkPos, bkupBall.radius, bkupBall.angle };
                }
                //// 残りのボールが先頭のボールを追随するように位置を更新
                //for (int i = 1; i < balls.size(); ++i) {
                //    float am = bSpeed == highSpeed ? 0.27f : 0.19f;
                //    balls[i].position = Vector2Lerp(balls[i].position, balls[i - 1].position, am);
                //}

                // エサとの衝突判定
                for (auto& f : food) {
                    Rectangle recBall0 = {
                        balls[0].position.x - balls[0].radius, balls[0].position.y - balls[0].radius ,
                        balls[0].radius * 2, balls[0].radius * 2
                    };
                    Rectangle recFood = {
                        f.position.x - f.radius, f.position.y - f.radius ,f.radius * 2, f.radius * 2
                    };

                    if (f.active && CheckCollisionRecs(recBall0, recFood)) {
                        //if (f.active && CheckCollisionCircles(balls[0].position, balls[0].radius, f.position, f.radius)) {
                        f.active = false;
                        numFoodEaten++;
                        score = numFoodEaten * 10;
                        f.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
                        f.active = true;

                        if ((numFoodEaten % 2 == 0)) AddObstacle(obstacles); // エサをいくつか食べるごとに障害物を1つ追加

                        // ヘビの身体のボールを１つ増す
                        Ball addBall = { balls[0].position, startR, balls[0].angle, true };
                        balls.insert(balls.begin(), addBall);
                        //balls.push_back(addBall);
                        //balls.insert(balls.end(), addBall);
                    }
                }

                // 頭と体ball[15-19]との衝突判定
                if (balls.size() >= 15) {
                    for (int i = 15; i < balls.size(); i++)
                    {
                        if (i > 19) break;
                        if (CheckCollisionCircles(balls[0].position, balls[0].radius, balls[i].position, balls[i].radius)) {
                            hitBallNum = i;
                            break;
                        }
                    }
                }

                // ヘビで囲った中の障害物は消滅
                if (hitBallNum > 0) {
                    vtx[0] = balls[hitBallNum].position;
                    vtx[1] = balls[4].position;
                    vtx[2] = balls[9].position;
                    vtx[3] = balls[14].position;

                    for (int i = 0; i < obstacles.size(); i++)
                    {
                        Rectangle rect = GetRectangleFromPoints(vtx);
                        if (CheckCollisionPointRec(obstacles[i].position, rect)) {
                            //obstacles[i].active = false;
                            obstacles.erase(obstacles.begin() + i);
                            score += 50;
                        }
                    }
                }

                // 障害物との衝突判定
                for (auto& o : obstacles) {
                    Rectangle recBall0 = {
                        balls[0].position.x - balls[0].radius * 0.7f,balls[0].position.y - balls[0].radius * 0.7f,
                        balls[0].radius * 2 * 0.7f, balls[0].radius * 2 * 0.7f
                    };
                    Rectangle recObst = {
                        o.position.x - o.radius * 0.7f, o.position.y - o.radius * 0.7f,
                        o.radius * 2 * 0.7f, o.radius * 2 * 0.7f
                    };
                    if (o.active && CheckCollisionRecs(recBall0, recObst)) {
                        if (--lifeNum > 0) {
                            balls[0].active = false;
                            o.active = false;
                            stopTimer = 1.2f;
                        }
                        else {
                            o.active = false;
                            gameOver = true;
                        }
                    }
                    else if (o.active == false) {
                        o.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
                        o.active = true;
                    }
                }
            }
            else {
                stopTimer -= deltaTime;
                if (stopTimer < 0) {
                    stopTimer = 0.0f;
                    balls[0].active = true;
                }
            }

        }
        if (gameOver && IsKeyPressed(KEY_R)) {
            ballDirection = { 1, 0 };
            // ゲームをリセット
            balls.clear();
            InitSnake(balls);

            InitFood(food);

            obstacles.clear();
            InitObstacles(obstacles);

            stopTimer = 0.0f;
            numFoodEaten = 0;
            score = 0;
            lifeNum = 3;
            gameOver = false;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // 描画
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////
        BeginDrawing();
        ClearBackground({ 15,15,15,255 }); // バックグラウンドの色
        //DrawRectangle(80, 60, 640, 480, { 255,255,5,195 });
        //DrawRectangleGradientEx({ 80, 60, 640, 480 },GOLD,PINK, GOLD, PINK);
        DrawRectangleGradientEx({ 0, 0, 800, 600 }, BROWN, BLUE, GOLD, RED);

            // 囲い範囲テスト
        {
            if (hitBallNum > 0) {
                vtx[0] = balls[hitBallNum].position;
                vtx[1] = balls[4].position;
                vtx[2] = balls[9].position;
                vtx[3] = balls[14].position;
            }
            DrawLineV(vtx[0], vtx[1], YELLOW);
            DrawLineV(vtx[1], vtx[2], YELLOW);
            DrawLineV(vtx[2], vtx[3], YELLOW);
            DrawLineV(vtx[3], vtx[0], YELLOW);
        }

        // へび(player)----------------------------------------------------------------------------------------------
        Color color = balls[0].active ? Color{ 15, 255, 15, 255 } : GRAY;
        for (const auto& ball : balls) {
            //DrawCircleLinesV(ball.position, ball.radius, GREEN);
            //DrawCircleV(ball.position, ball.radius, GREEN);
            DrawCircleGradient(ball.position.x, ball.position.y, ball.radius, color, { 255,255,255,0 });
        }
        if (balls[0].active) {
            //DrawCircleV(balls[0].position, balls[0].radius / 2.5f, { 55,255,55,205 }); // 頭
                // 目
            Vector2 lEyeDir = RotateVector(ballDirection, -60.0f);
            Vector2 rEyeDir = RotateVector(ballDirection, 60.0f);
            float eyeRadius = balls[0].radius / 2.4f;
            Color eyesColor = Color{ 225,225,225,155 };
            DrawCircleV({ balls[0].position.x + lEyeDir.x * 3.5f, balls[0].position.y + lEyeDir.y * 3.5f }, eyeRadius, eyesColor);
            DrawCircleV({ balls[0].position.x + rEyeDir.x * 3.5f, balls[0].position.y + rEyeDir.y * 3.5f }, eyeRadius, eyesColor);

            DrawCircleV({ balls[0].position.x + lEyeDir.x * 5.0f, balls[0].position.y + lEyeDir.y * 5.0f }, eyeRadius / 2.2f, PURPLE);
            DrawCircleV({ balls[0].position.x + rEyeDir.x * 5.0f, balls[0].position.y + rEyeDir.y * 5.0f }, eyeRadius / 2.2f, PURPLE);

            // 5ボール碁との節
            for (size_t i = 0; i < balls.size(); i++)
            {
                if ((i + 1) % 10 == 0) {
                    DrawCircleV(balls[i].position, balls[i].radius / 3.2f, GREEN); // 5
                }
                else if ((i + 1) % 5 == 0) {
                    DrawCircleV(balls[i].position, balls[i].radius / 4.8f, GREEN); // 5
                }
            }
        }
        // へび(player)----------------------------------------------------------------------------------------------

        // エサ
        for (const auto& f : food) {
            if (f.active) {
                //DrawStar(f, YELLOW);
                //DrawCircleV(f.position, f.radius, RED); // エサの色を赤に変更
                DrawCircleGradient(f.position.x, f.position.y, f.radius, WHITE, { 255,255,0,0 });
            }
        }

        // 障害物
        for (const auto& o : obstacles) {
            //if (o.active) {
                //DrawCircleV(o.position, o.radius, DARKGRAY);
                DrawCircleGradient(o.position.x, o.position.y, o.radius * 1.5, RED, { 255,255,255,128 });
            //}
        }

        // メッセージ表示
        // スコア
        DrawText(TextFormat("Score: %05d", score), 4, 4, 20, RAYWHITE);
        DrawText(TextFormat("LIFE: %d", lifeNum), 180, 4, 20, RAYWHITE);

        if (gameOver) {
            DrawText("Game Over!", screenWidth / 2 - MeasureText("Game Over!", 60) / 2, screenHeight / 2 - 60, 60, DARKPURPLE);
            DrawText("Press R to Retry", screenWidth / 2 - MeasureText("Press R to Retry", 30) / 2, screenHeight / 2 + 10, 30, DARKPURPLE);
        }


        EndDrawing();

    }

    CloseWindow();

    return 0;
}
