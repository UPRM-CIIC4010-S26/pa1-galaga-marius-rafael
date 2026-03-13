#pragma once

#include <iostream>
#include <string>
#include "raylib-cpp/raylib-cpp.hpp"
#include "Background.hpp"
#include "Player.hpp"
#include "StdEnemy.hpp"
#include "SpEnemy.hpp"
#include "StEnemy.hpp"
#include "DyEnemy.hpp"
class Program {
    private:
        Background background = Background();
        Player* player = new Player((GetScreenWidth() / 2) - 15, GetScreenHeight() * 0.75f);
        int respawnCooldown = 1080;
        int respawns = 0;
        int count = 0;
        int delay = 0;
        int lives = 3;
        int pauseFrames = 0;
        int score = 0;
        int bonusLivesThreshold = 1000;

        bool startup = true;
        bool paused = false;
        bool gameOver = false;
        struct Star {
        float x;
        float y;
        float speed;
        int size;
    };

    std::vector<Star> stars;
    bool starsInitialized = false;

    void UpdateStars();
    void DrawStars();

    public:
        Program();
        
        void Update();
        void Draw();
        void ManageEnemyRespawns();
        void DrawStartup();
        void DrawScore(); //Shows score on screen
        void DrawLives(); //Shows the amount of lives the player currently has
        void DrawPauseScreen();
        void DrawGameOver();
        void KeyInputs();
        void PlayerReset();
        void Reset();
        ~Program() {}
};