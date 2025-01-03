#include <raylib.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define WINDOW_TITLE "Squared"
#define TARGET_FPS 60

class FontManager
{
public:
    Font displayFont;
    Font scoreFont;
    Font healthFont;

    void LoadFonts()
    {
        displayFont = LoadFont("Resources/Fonts/DisplayFont.ttf");
        scoreFont = LoadFont("Resources/Fonts/ScoreFont.otf");
        healthFont = LoadFont("Resources/Fonts/ScoreFont.otf");
    }

    void UnloadFonts()
    {
        UnloadFont(displayFont);
        UnloadFont(scoreFont);
        UnloadFont(healthFont);
    }
};

class TextureManager
{
public:
    Texture2D playerTexture;
    Texture2D enemyTexture;
    Texture2D projectileTexture;
    Texture2D healthTexture;
    Texture2D powerUpTexture;
    Texture2D lifeTexture;

    void LoadTextures()
    {
        playerTexture = LoadTexture("Resources/Assets/Player.png");
        enemyTexture = LoadTexture("Resources/Assets/Enemy.png");
        projectileTexture = LoadTexture("Resources/Assets/Projectile.png");
        healthTexture = LoadTexture("Resources/Assets/Health.png");
        powerUpTexture = LoadTexture("Resources/Assets/PowerUp.png");
        lifeTexture = LoadTexture("Resources/Assets/Life.png");
    }

    void UnloadTextures()
    {
        UnloadTexture(playerTexture);
        UnloadTexture(enemyTexture);
        UnloadTexture(projectileTexture);
        UnloadTexture(healthTexture);
        UnloadTexture(powerUpTexture);
        UnloadTexture(lifeTexture);
    }
};

void SetupGameWindow(TextureManager& TM, FontManager& FM)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
    SetTargetFPS(TARGET_FPS);
    FM.LoadFonts();
    TM.LoadTextures();
}

class Projectile
{
protected:
    Vector2 projectilePosition;
    Vector2 projectileDirection;
    int projectileSpeed;
    int projectileDamage;
    int projectileSize;
    bool isActive;

public:
    Projectile(Vector2 position, Vector2 direction, int speed, int damage, int size)
        : projectilePosition(position), projectileDirection(direction), projectileSpeed(speed), projectileDamage(damage), projectileSize(size), isActive(true) {}

    void Update()
    {
        if (isActive)
        {
            projectilePosition.x += projectileDirection.x * projectileSpeed;
            projectilePosition.y += projectileDirection.y * projectileSpeed;
            if (projectilePosition.x < 0 || projectilePosition.x > SCREEN_WIDTH || projectilePosition.y < 0 || projectilePosition.y > SCREEN_HEIGHT)
            {
                isActive = false;
            }
        }
    }

    void Draw(TextureManager& TM)
    {
        if (isActive)
        {
            DrawTextureEx(TM.projectileTexture, projectilePosition, 0, 1, WHITE);
        }
    }

    static void Shoot(std::vector<Projectile>& projectileObjects, Vector2 startPosition, Vector2 targetPosition, int speed, int damage, int size)
    {
        Vector2 projectileDirection = {targetPosition.x - startPosition.x, targetPosition.y - startPosition.y};
        float projectileMagnitude = sqrt(projectileDirection.x * projectileDirection.x + projectileDirection.y * projectileDirection.y);

        if (projectileMagnitude > 0)
        {
            projectileDirection.x /= projectileMagnitude;
            projectileDirection.y /= projectileMagnitude;
        }

        projectileObjects.emplace_back(startPosition, projectileDirection, speed, damage, size);
    }

    bool IsActive() const { return isActive; }
    Vector2 GetPosition() const { return projectilePosition; }
    int GetDamage() const { return projectileDamage; }
    int GetSize() const { return projectileSize; }
};

class Entity
{
public:
    virtual void Draw(TextureManager& TM) = 0;

    int GetHealth() { return entityHealth; }
    void SetHealth(int health) { entityHealth = health; }

    int GetSpeed() { return entitySpeed; }
    void SetSpeed(int speed) { entitySpeed = speed; }

    int GetDamage() { return entityDamage; }
    void SetDamage(int damage) { entityDamage = damage; }

    int GetCollisionDamage() { return entityCollisionDamage; }
    void SetCollisionDamage(int damage) { entityCollisionDamage = damage; }

    int GetSize() { return entitySize; }
    void SetSize(int size) { entitySize = size; }

    Vector2 GetPosition() { return entityPosition; }
    void SetPosition(Vector2 position) { entityPosition = position; }

protected:
    int entityHealth = 100;
    int entitySpeed = 5;
    int entityDamage = 10;
    int entityCollisionDamage = 50;
    int entitySize = 32;
    Vector2 entityPosition = {0, 0};
};

class Player : public Entity
{
public:
    void Draw(TextureManager& TM) override
    {
        DrawTextureEx(TM.playerTexture, entityPosition, 0, 1, WHITE);
    }

    void Move(std::vector<Projectile>& projectileObjects)
    {
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))
            entityPosition.x += entitySpeed;
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))
            entityPosition.x -= entitySpeed;
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))
            entityPosition.y -= entitySpeed;
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S))
            entityPosition.y += entitySpeed;

        if (IsKeyPressed(KEY_SPACE))
        {
            Vector2 mousePosition = GetMousePosition();
            Projectile::Shoot(projectileObjects, entityPosition, mousePosition, 10, 20, 5);
        }

        if (entityPosition.x > SCREEN_WIDTH) entityPosition.x = 0;
        if (entityPosition.x < 0) entityPosition.x = SCREEN_WIDTH;
        if (entityPosition.y > SCREEN_HEIGHT) entityPosition.y = 0;
        if (entityPosition.y < 0) entityPosition.y = SCREEN_HEIGHT;
    }
};

class Enemy : public Entity
{
public:
    void Draw(TextureManager& TM) override
    {
        DrawTextureEx(TM.enemyTexture, entityPosition, 0, 1, WHITE);
        DrawRectangle(entityPosition.x, entityPosition.y - 10, entitySize, 5, RED);
        DrawRectangle(entityPosition.x, entityPosition.y - 10, entitySize * (entityHealth / 100.0f), 5, GREEN);
    }

    void Move(Vector2 playerPosition)
    {
        Vector2 direction = {playerPosition.x - entityPosition.x, playerPosition.y - entityPosition.y};
        float distance = sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0)
        {
            direction.x /= distance;
            direction.y /= distance;
            entityPosition.x += direction.x * entitySpeed;
            entityPosition.y += direction.y * entitySpeed;
        }
    }
};

class GameManager
{
public:
    void Initialize(TextureManager& TM, FontManager& FM)
    {
        this->TM = &TM;
        this->FM = &FM;

        PC.SetHealth(100);
        PC.SetSpeed(5);
        PC.SetPosition({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});

        isGameRunning = true;

        enemyUnits.clear();
        for (int i = 0; i < numEnemies; i++)
        {
            Enemy enemy;
            enemy.SetPosition({(float)GetRandomValue(0, SCREEN_WIDTH), (float)GetRandomValue(0, SCREEN_HEIGHT)});
            enemy.SetSpeed(3);
            enemyUnits.push_back(enemy);
        }
    }

    void Update()
    {
        ClearBackground(RAYWHITE);

        HandlePlayer();
        HandleEnemies();
        HandleProjectiles();
        HandleCollision();

        DisplayHealth();
    }

    bool GameShouldClose() const { return gameShouldClose; }

private:
    void HandlePlayer()
    {
        PC.Draw(*TM);
        PC.Move(projectileObjects);
    }

    void HandlePlayerInput()
    {
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_Q))
        {
            gameShouldClose = true;
        }
        if (IsKeyPressed(KEY_R))
        {
            Initialize(*TM, *FM);
        }
    }

    void HandleEnemies()
    {
        for (auto& enemy : enemyUnits)
        {
            enemy.Draw(*TM);
            enemy.Move(PC.GetPosition());
        }
    }

    void HandleProjectiles()
    {
        for (auto& projectile : projectileObjects)
        {
            projectile.Update();
            projectile.Draw(*TM);
        }
            projectileObjects.erase( std::remove_if(projectileObjects.begin(), projectileObjects.end(), [](const Projectile& p) { return !p.IsActive(); }),
            projectileObjects.end());
    }

    void HandleCollision()
    {
        for (size_t i = 0; i < enemyUnits.size(); ++i)
        {
            if (CheckCollisionRecs( {PC.GetPosition().x, PC.GetPosition().y, (float)PC.GetSize(), (float)PC.GetSize()}, {enemyUnits[i].GetPosition().x, enemyUnits[i].GetPosition().y, (float)enemyUnits[i].GetSize(), (float)enemyUnits[i].GetSize()}))
            {
                PC.SetHealth(PC.GetHealth() - enemyUnits[i].GetCollisionDamage());
                enemyUnits.erase(enemyUnits.begin() + i);
                --i;
                continue;
            }

            for (size_t j = 0; j < projectileObjects.size(); ++j)
            {
                if (CheckCollisionRecs( {projectileObjects[j].GetPosition().x, projectileObjects[j].GetPosition().y, (float)projectileObjects[j].GetSize(), (float)projectileObjects[j].GetSize()}, {enemyUnits[i].GetPosition().x, enemyUnits[i].GetPosition().y, (float)enemyUnits[i].GetSize(), (float)enemyUnits[i].GetSize()}))
                {
                    enemyUnits[i].SetHealth(enemyUnits[i].GetHealth() - projectileObjects[j].GetDamage());
                    projectileObjects.erase(projectileObjects.begin() + j);

                    if (enemyUnits[i].GetHealth() <= 0)
                    {
                        enemyUnits.erase(enemyUnits.begin() + i);
                        --i;
                    }
                    break;
                }
            }
        }
    }

    void DisplayHealth()
    {
        std::string healthText = "Health: " + std::to_string(PC.GetHealth());
        if (PC.GetHealth() > 0)
        {
            DrawTextEx(FM->healthFont, healthText.c_str(), {10, 10}, 24, 0, BLACK);
        }
        else
        {
            GameIsOver();
        }
    }

    void GameIsOver()
    {
        HandlePlayerInput(); // Handle Player Input to Restart or Quit
        char* GameOverMessage = "You died! Press R to Restart or Q to Quit."; // Char* for DrawTextEx Only :)
        float GameOverMessageWidth = MeasureTextEx(FM->displayFont, GameOverMessage, 24, 0).x;
        DrawTextEx(FM->displayFont, GameOverMessage, {(SCREEN_WIDTH - GameOverMessageWidth) / 2, SCREEN_HEIGHT / 2}, 24, 0, BLACK);
        // TODO: Remove Enemies & Projectiles
    }

    TextureManager* TM;
    FontManager* FM;

    Player PC;
    std::vector<Enemy> enemyUnits;
    std::vector<Projectile> projectileObjects;

    bool isGameRunning = true;
    bool gameShouldClose = false;

    int numEnemies = 5;
};

void CleanUp(FontManager& FM, TextureManager& TM)
{
    FM.UnloadFonts();
    TM.UnloadTextures();
    CloseWindow();
}

int main()
{
    GameManager GM;
    FontManager FM;
    TextureManager TM;
    SetupGameWindow(TM, FM);
    GM.Initialize(TM, FM);
    while (!WindowShouldClose() && !GM.GameShouldClose())
    {
        BeginDrawing();
        GM.Update();
        EndDrawing();
    }
    CleanUp(FM, TM);
    return 0;
}
