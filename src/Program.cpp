#include "Program.hpp"
#include <vector>

struct Star {
    float x;
    float y;
    float speed;
    int size;
};

static std::vector<Star> stars;
static bool starsInitialized = false;

Program::Program() {
    Background::sideWalls = std::pair<HitBox, HitBox>{ 
        HitBox(0, 0, 10, GetScreenHeight()), 
        HitBox(GetScreenWidth() - 10, 0, 10, GetScreenHeight())
    };

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{350, 150}, 
            new SpEnemy(350, 150)
        });

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{600, 150}, 
            new SpEnemy(600, 150)
        });

    for (int i = 0; i < 30; i++) {
        float x;  
        float y;
        if(i < 10){
            x = 250 + (i * 50);
            y = 200;
        } else if(i < 20){
            x = 250 + ((i - 10) * 50);
            y = 250;
        } else {
            x = 250 + ((i - 20) * 50);
            y = 300;
        }

        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{x, y}, 
            new StdEnemy(x, y)
        });
    }

    // Bonus: initialize animated stars
    if (!starsInitialized) {
        for (int i = 0; i < 80; i++) {
            stars.push_back(Star{
                (float)GetRandomValue(0, GetScreenWidth()),
                (float)GetRandomValue(0, GetScreenHeight()),
                (float)GetRandomValue(1, 4),
                GetRandomValue(1, 3)
            });
        }
        starsInitialized = true;
    }
}

void Program::UpdateStars() {
    for (Star& s : stars) {
        s.y += s.speed;

        if (s.y > GetScreenHeight()) {
            s.y = 0;
            s.x = (float)GetRandomValue(0, GetScreenWidth());
            s.speed = (float)GetRandomValue(1, 4);
            s.size = GetRandomValue(1, 3);
        }
    }
}

void Program::DrawStars() {
    for (const Star& s : stars) {
        DrawCircle((int)s.x, (int)s.y, (float)s.size, WHITE);
    }
}

void Program::Update() {
    UpdateStars();

    for (Animation& a : Animation::animations) a.update();
    for (int i = 0; i < Animation::animations.size(); i++) {
        if (Animation::animations[i].done) {
            Animation::animations.erase(Animation::animations.begin() + i);
            i--;
        }
    }

    pauseFrames = std::max(pauseFrames - 1, 0);

    if (!startup && !paused && !gameOver && pauseFrames <= 0) {
        Enemy::ManageEnemies(player->hitBox, score);
        StdEnemy::attackReset();
        ManageEnemyRespawns();
        player->update();

        for (std::pair<std::pair<float, float>, Enemy*> p : Enemy::enemies) {
            if (p.second && HitBox::Collision(player->hitBox, p.second->hitBox)) {
                Animation::animations.push_back(
                    Animation(player->position.first, player->position.second, 16, 0, 33, 34, 30, 30, 3, ImageManager::SpriteSheet)
                );

                PlaySound(SoundManager::gameOver);
                Projectile::projectiles.clear();
                player->position.first = GetScreenWidth() / 2 - 15;
                p.second->health = 0;
                pauseFrames = 120;
                lives--;
            }
        }

        for (Projectile& p : Projectile::projectiles) { 
            p.update();

            // Phase 1: enemy projectile hits player
            if (p.ID != 0 && HitBox::Collision(player->hitBox, p.getHitBox())) {
                PlayerReset();
                break;
            }
        }

        if (lives <= 0 && pauseFrames <= 0) {
            gameOver = true;
            StopSound(SoundManager::BGM);
        } else if (lives <= 5 && score >= bonusLivesThreshold) {
            lives++;
            bonusLivesThreshold += 1000;
            PlaySound(SoundManager::lifeUp);
        } else if (lives > 5) {
            lives = 5;
            StopSound(SoundManager::lifeUp);
        }

        Projectile::CleanProjectiles();
        Projectile::ProjectileCollision();
    }
}

void Program::Draw() {
    background.Draw();
    DrawStars();

    if (pauseFrames <= 0 && !gameOver) player->draw();
    for (Animation& a : Animation::animations) a.draw();

    for (int i = 0; i < lives; i++) {
        DrawTexturePro(
            ImageManager::SpriteSheet,
            Rectangle{0, 0, 17, 18}, 
            Rectangle{10.0f + i * 30, GetScreenHeight() - 30.0f, 20, 20}, 
            Vector2{0, 0},
            0,
            WHITE
        );
    }

    for (Projectile p : Projectile::projectiles) p.draw();
    for (std::pair<std::pair<float, float>, Enemy*>& p : Enemy::enemies) {
        if (p.second) p.second->draw();
    }

    if (startup) DrawStartup();
    if (paused) DrawPauseScreen();
    if (gameOver) DrawGameOver();
    DrawScore();
    DrawLives();
}

void Program::ManageEnemyRespawns() {
    delay = std::max(delay - 1, 0);
       respawnCooldown -= 1;
//Difficulty based on score
    if (respawnCooldown <= 0)
    {
        if(score <= 1000){
              respawnCooldown = 1080;
        } else if (score <= 2500){
                 respawnCooldown = 400;
        } else {
                   respawnCooldown = 150;
        }
        for (std::pair<std::pair<float, float>, Enemy*>& p : Enemy::enemies) {
            if (!p.second && p.first.second != 150) {
                int eType = GetRandomValue(1, 3);

                if (eType == 1) {
                    p.second = new StEnemy(GetScreenWidth() / 2 - 15, 0, true);
                } else {
                    p.second = new StdEnemy(GetScreenWidth() / 2 - 15, 0, true);
                }

                respawns++;
                break;
            } else if (!p.second && p.first.second == 150) {
                p.second = new SpEnemy(GetScreenWidth() / 2 - 15, 0, true);
                respawns++;
                break;
            }
        }
    }

    if (respawns >= 4) {
        count = 4;
        respawns = 0;
    }

    if (count > 0 && delay <= 0) {
        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{0, 0}, 
            new DyEnemy(GetScreenWidth(), 300)
        });

        count--;
        delay = 20;
    }
}

void Program::DrawStartup() {
    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 125});
    DrawText("Galaga", (GetScreenWidth() / 2 - 237), 75, 144, WHITE);
    DrawText("Press Enter", (GetScreenWidth() / 2) - 75, GetScreenHeight() / 2, 24, GRAY);
}

void Program::DrawPauseScreen() {
    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 125});
    DrawText("Paused", (GetScreenWidth() / 2) - 85, GetScreenHeight() / 2 - 60, 48, WHITE);
    DrawText("Press Enter", (GetScreenWidth() / 2) - 75, GetScreenHeight() / 2, 24, GRAY);
}

void Program::DrawScore() {
    std::string text = "Score: " + std::to_string(score);
    DrawText(text.c_str(), 20, 20, 30, WHITE);
}

void Program::DrawLives() {
    std::string text = "Lives: " + std::to_string(lives);
    DrawText(text.c_str(), 850, 20, 30, WHITE);
}

void Program::DrawGameOver() {
    DrawRectangle(0, 0, (float)GetScreenWidth(), (float)GetScreenHeight(), Color{0, 0, 0, 125});
    DrawText("Game Over", (GetScreenWidth() / 2) - 380, 50, 144, WHITE);
    DrawText("Press Enter", (GetScreenWidth() / 2) - 75, GetScreenHeight() / 2, 24, GRAY);
}

void Program::KeyInputs() {
    if ((!gameOver && !startup && IsKeyPressed('P')) || (paused && IsKeyPressed(KEY_ENTER))) paused = !paused;
    if (!paused && !startup && IsKeyPressed('O')) gameOver = !gameOver;
    if (!gameOver && !paused && IsKeyPressed('I')) startup = !startup;
    if (IsKeyPressed('H')) HitBox::drawHitbox = !HitBox::drawHitbox;
    
    if (gameOver && IsKeyPressed(KEY_ENTER)) {
        PlaySound(SoundManager::BGM);//BGM Addition
        gameOver = false;
        score = 0;
        Reset();
    }

    if (startup && IsKeyPressed(KEY_ENTER)) {
        startup = false;
        PlaySound(SoundManager::BGM);//BGM Addition
    }

    if (!startup && !paused && !gameOver && pauseFrames <= 0) player->keyInputs();

    if (IsKeyPressed('K')) {
        score += 500;
    }
}

void Program::PlayerReset() {
    Animation::animations.push_back(
        Animation(player->position.first, player->position.second, 16, 0, 33, 34, 30, 30, 3, ImageManager::SpriteSheet)
    );

    PlaySound(SoundManager::gameOver);
    Projectile::projectiles.clear();
    player->position.first = GetScreenWidth() / 2 - 15;
    pauseFrames = 120;
    lives--;
}

void Program::Reset() {
    Enemy::enemies.clear();
    StdEnemy::attackInProgress = false;
    player = new Player((GetScreenWidth() / 2) - 15, GetScreenHeight() * 0.75f);
    respawnCooldown = 1080;
    respawns = 0;
    count = 0;
    delay = 0;
    lives = 3;
    bonusLivesThreshold = 1000;
//Enemy Respawn after Game Over
     for (int i = 0; i < 30; i++) {
        float x;  
        float y;
        if(i < 10){
            x = 250 + (i * 50);
            y = 200;
        } else if(i < 20){
            x = 250 + ((i - 10) * 50);
            y = 250;
        } else {
            x = 250 + ((i - 20) * 50);
            y = 300;
        }

        Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{x, y}, 
            new StdEnemy(x, y)
        });
    }

       Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{350, 150}, 
            new SpEnemy(350, 150)
        });

    Enemy::enemies.push_back(std::pair<std::pair<float, float>, Enemy*> {
            std::pair<float, float>{600, 150}, 
            new SpEnemy(600, 150)
        });
}