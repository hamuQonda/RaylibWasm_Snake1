#include "raylib.h"
#include <raymath.h>
#include <vector>

const int screenWidth = 800;
const int screenHeight = 600;
const float ballSpeed = 150.0f; // �{�[���̈ړ����x�i�s�N�Z�����b�j
const float angleIncrement = 360.0f / 24.0f; // �Ȃ���p�x�̑���
const float ballSpacing = 20.0f; // �{�[���Ԃ̋���
const int numFood = 10; // �G�T�̐�
const int initialNumObstacles = 15; // �����̏�Q���̐�

struct Ball {
    Vector2 position;
    float radius;
    float angle; // ���݂̈ړ������̊p�x�i�x�j
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

// �}�E�X�J�[�\�����{�[���̐i�s�����ɑ΂��č��E�ǂ���ɂ��邩�𔻒肷��֐�
int GetMouseDirectionRelativeToBall(const Vector2& ballPosition, float ballAngle, const Vector2& mousePosition) {
    Vector2 direction = { cos(ballAngle * PI / 180.0f), sin(ballAngle * PI / 180.0f) };
    Vector2 toMouse = { mousePosition.x - ballPosition.x, mousePosition.y - ballPosition.y };
    float crossProduct = direction.x * toMouse.y - direction.y * toMouse.x;

    if (crossProduct > 0) return 1;  // �}�E�X�͐i�s�����̍����ɂ���
    if (crossProduct < 0) return -1; // �}�E�X�͐i�s�����̉E���ɂ���
    return 0; // �}�E�X�͐i�s�����̐^��ɂ���
}

// �����_���ɃG�T��z�u����֐�
void InitFood(std::vector<Food>& food) {
    for (auto& f : food) {
        f.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        f.radius = 5.0f;
        f.active = true;
    }
}

// �����_���ɏ�Q����z�u����֐�
void InitObstacles(std::vector<Obstacle>& obstacles) {
    for (auto& o : obstacles) {
        o.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
        o.radius = 5.0f; // ��Q���̔��a��5.0f�ɐݒ�
    }
}

// ��Q����1�����_���ɔz�u����֐�
void AddObstacle(std::vector<Obstacle>& obstacles) {
    Obstacle newObstacle;
    newObstacle.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
    newObstacle.radius = 5.0f; // ��Q���̔��a��5.0f�ɐݒ�
    obstacles.push_back(newObstacle);
}

int main() {
    InitWindow(screenWidth, screenHeight, "Ball Movement Example");

    std::vector<Ball> balls(3);
    balls[0].position = { screenWidth / 2.0f, screenHeight / 2.0f };
    balls[0].radius = 5.0f; // �w�r�̔��a��5.0f�ɐݒ�
    balls[0].angle = 0.0f;

    // �c��̃{�[����擪�̃{�[���̈ʒu������Ԋu�Ŕz�u
    for (int i = 1; i < balls.size(); ++i) {
        balls[i].radius = 5.0f; // �w�r�̔��a��5.0f�ɐݒ�
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

            // �p�x�̍����v�Z���A�K�؂ȕ����Ƀ{�[������]������
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

            // �擪�̃{�[���̈ʒu���X�V
            balls[0].position.x += ballSpeed * cos(balls[0].angle * PI / 180.0f) * GetFrameTime();
            balls[0].position.y += ballSpeed * sin(balls[0].angle * PI / 180.0f) * GetFrameTime();

            // �c��̃{�[�����擪�̃{�[����ǐ�����悤�Ɉʒu���X�V
            for (int i = 1; i < balls.size(); ++i) {
                balls[i].position = Vector2Lerp(balls[i].position, balls[i - 1].position, 0.2f);
            }

            // �G�T�Ƃ̏Փ˔���
            for (auto& f : food) {
                if (f.active && CheckCollisionCircles(balls[0].position, balls[0].radius, f.position, f.radius)) {
                    f.active = false;
                    score++;
                    f.position = { (float)GetRandomValue(0, screenWidth), (float)GetRandomValue(0, screenHeight) };
                    f.active = true;
                    AddObstacle(obstacles); // �G�T��H�ׂ邲�Ƃɏ�Q����1�ǉ�
                }
            }

            // ��Q���Ƃ̏Փ˔���
            for (const auto& o : obstacles) {
                if (CheckCollisionCircles(balls[0].position, balls[0].radius, o.position, o.radius)) {
                    gameOver = true;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK); // �o�b�N�O���E���h�̐F�����ɕύX

        for (const auto& f : food) {
            if (f.active) {
                DrawCircleV(f.position, f.radius, RED); // �G�T�̐F��ԂɕύX
            }
        }

        for (const auto& o : obstacles) {
            DrawCircleV(o.position, o.radius, DARKGRAY); // ��Q���̐F���_�[�N�O���C�ɕύX
        }

        for (const auto& ball : balls) {
            DrawCircleV(ball.position, ball.radius, GREEN); // �w�r�̐F��΂ɕύX
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
            // �Q�[�������Z�b�g
            balls[0].position = { screenWidth / 2.0f, screenHeight / 2.0f };
            balls[0].angle = 0.0f;
            for (int i = 1; i < balls.size(); ++i) {
                balls[i].position = balls[i - 1].position;
            }
            InitFood(food);
            obstacles.clear();
            InitObstacles(obstacles); // ��Q���������l�ɖ߂�
            score = 0;
            gameOver = false;
        }
    }

    CloseWindow();

    return 0;
}
