#pragma once
#include <SFML/Graphics.hpp>
#include "GameEntity.hpp"
#include "ShapeType.hpp"
#include "Direction.hpp"
#include "Map.hpp"
#include "GameEntityManager.hpp"
#include "GhostMode.hpp"
#include "GhostType.hpp"
#include <iostream>
#include <memory>
#include <vector>

class Application {
public:
    Application() : mMap(32, 20, 22), mManager(mMap) {
        InitializeGame();
    }

    void InitializeGame() {

        // Initialize the map
        std::vector<std::vector<int>> mapData = {
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
            {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
            {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
            {1,3,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,3,1},
            {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
            {1,2,1,1,2,1,2,1,1,1,1,1,1,2,1,2,1,1,2,1},
            {1,2,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,2,1},
            {1,1,1,1,2,1,1,1,0,1,1,0,1,1,1,2,1,1,1,1},
            {0,0,0,1,2,1,0,0,0,0,0,0,0,0,1,2,1,0,0,0},
            {1,1,1,1,2,1,0,1,1,4,1,1,1,0,1,2,1,1,1,1},
            {0,0,0,0,2,0,0,1,0,0,0,0,1,0,0,2,0,0,0,0},
            {1,1,1,1,2,1,0,1,1,1,1,1,1,0,1,2,1,1,1,1},
            {0,0,0,1,2,1,0,0,0,0,0,0,0,0,1,2,1,0,0,0},
            {1,1,1,1,2,1,0,1,1,1,1,1,1,0,1,2,1,1,1,1},
            {1,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,1},
            {1,2,1,1,2,1,1,1,2,1,1,2,1,1,1,2,1,1,2,1},
            {1,3,2,1,2,2,2,2,2,0,0,2,2,2,2,2,1,2,3,1},
            {1,1,2,1,2,1,2,1,1,1,1,1,1,2,1,2,1,2,1,1},
            {1,2,2,2,2,1,2,2,2,1,1,2,2,2,1,2,2,2,2,1},
            {1,2,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,2,1},
            {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
        };
        mMap.Load(mapData);

        // Initialize Pac-Man
        mPacman = GameEntity::Create();
        auto tc = std::make_shared<TransformComponent>(mPacman, ShapeType::Circle);
        auto sc = std::make_shared<SpriteComponent>(mPacman);
        mPacmanInput = std::make_shared<InputComponent>(mPacman, mMap);
        auto cc = std::make_shared<CollisionComponent>(mPacman, ShapeType::Circle);

        float pacmanRadius = 14.0f;
        tc->SetCircleRadius(pacmanRadius);
        cc->SetCircleRadius(pacmanRadius);
        sc->CreateSpriteComponent("../assets/p.png");
        tc->SetScale(0.5f, 0.5f);
        tc->SetPosition(306.0f, 512.0f);

        mPacman->AddComponent(tc);
        mPacman->AddComponent(mPacmanInput);
        mPacman->AddComponent(sc);
        mPacman->AddComponent(cc);
        mManager.SetPacman(mPacman);

        // Initialize ghosts
        InitializeGhost(GhostType::Blinky, "../assets/blinky.png", { 32, 32 }, { 2.0f, 2.0f }, GhostMode::Scatter);
        InitializeGhost(GhostType::Pinky, "../assets/pinky.png", { 288, 320 }, { 2.0f, 2.0f }, GhostMode::InHouse);
        InitializeGhost(GhostType::Inky, "../assets/inky.png", { 320, 320 }, { 2.0f, 2.0f }, GhostMode::InHouse);
        InitializeGhost(GhostType::Clyde, "../assets/clyde.png", { 352, 320 }, { 2.0f, 2.0f }, GhostMode::InHouse);

        // Load font - Scores
        if (!mFont.loadFromFile("../assets/Winter Lemon.ttf")) {
            std::cerr << "Failed to load font!" << std::endl;
        }
        // Set up score text
        mScoreText.setFont(mFont);
        mScoreText.setCharacterSize(22);
        mScoreText.setFillColor(sf::Color::White);
        mScoreText.setPosition(20, 740);
        UpdateScoreDisplay();


        // Set up score text
        mLifeText.setFont(mFont);
        mLifeText.setCharacterSize(22);
        mLifeText.setFillColor(sf::Color::White);
        mLifeText.setPosition(530, 740);
        UpdateLifeDisplay();

        // Set up Game Over text
        mResultText.setFont(mFont);
        mResultText.setCharacterSize(52);
        mResultText.setFillColor(sf::Color::Red);
        mResultText.setPosition(180, 340);
        UpdateResultDisplay();

        // Set up Win text
        mWinText.setFont(mFont);
        mWinText.setCharacterSize(32);
        mWinText.setFillColor(sf::Color::White);
        mWinText.setPosition(100, 340);
        UpdateWinDisplay();
    }

    void Loop(float targetFPS) {
        sf::RenderWindow window(sf::VideoMode(640, 850), "Pac-Man");
        const sf::Time frameTime = sf::seconds(1.0f / targetFPS);
        sf::Clock clock;
        sf::Clock fpsClock;
        int frameCount = 0;

        while (mGameOn) {
            sf::Time elapsed = clock.restart();

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    window.close();
            }

            if (isResetting) {
                if (resetDelayClock.getElapsedTime().asSeconds() >= resetDelayTime) {
                    isResetting = false;
                }
                else {
                    continue;
                }
            }

            if (mPacmanInput->Won() && !mGameOverSequence) {
                mFreezeState = true;
                mGameOverSequence = true;
                mGameOverClock.restart();
                UpdateWinDisplay();
            }
            if (mGameOverSequence) {
                if (mGameOverClock.getElapsedTime().asSeconds() >= 2.0f) {
                    mGameOn = false;
                }

            }
            // Power-up logic
            if (mPacmanInput->IsPowerUp() && !isPowerUpActive) {
                isPowerUpActive = true;
                powerUpClock.restart();
                SetGhostsMode(GhostMode::Frightened, "../assets/blue_ghost.png");
            }

            if (isPowerUpActive && powerUpClock.getElapsedTime().asSeconds() >= 7.0f) {
                isPowerUpActive = false;
                mPacmanInput->SetPowerUp(false);
                ResetGhostsToNormalMode();
            }
            if (!mFreezeState) {
                // Normal game update logic
                mPacman->Input();
                mPacman->Update();
                mMap.Update();
                for (auto& ghost : mGhosts) {
                    ghost->Update();
                }
                HandleCollisions();
            }

            if (mGameOverSequence && mGameOverClock.getElapsedTime().asSeconds() >= 2.0f) {
                mGameOn = false;
            }
            // Render
            window.clear(sf::Color::Black);
            mMap.Render(window);
            mPacman->Render(window);
            for (const auto& ghost : mGhosts) {
                ghost->Render(window);
            }
            // Update score display
            UpdateScoreDisplay();
            UpdateLifeDisplay();
            UpdateResultDisplay();
            UpdateWinDisplay();
            window.draw(mScoreText);
            window.draw(mLifeText);
            window.draw(mResultText);
            window.draw(mWinText);
            window.display();

            // Frame rate limiting and FPS calculation
            sf::Time sleepTime = frameTime - clock.getElapsedTime();
            if (sleepTime > sf::Time::Zero) {
                sf::sleep(sleepTime);
            }
            frameCount++;
            if (fpsClock.getElapsedTime().asSeconds() >= 1.0f) {
                //std::cout << "FPS: " << frameCount / fpsClock.getElapsedTime().asSeconds() << std::endl;
                frameCount = 0;
                fpsClock.restart();
            }
        }
    }

private:
    void InitializeGhost(GhostType type, const std::string& spritePath, sf::Vector2f position, sf::Vector2f scale, GhostMode mode) {
        auto ghost = GameEntity::Create();
        auto tc = std::make_shared<TransformComponent>(ghost, ShapeType::Circle);
        auto sc = std::make_shared<SpriteComponent>(ghost);
        auto cc = std::make_shared<CollisionComponent>(ghost, ShapeType::Circle);
        auto ai = std::make_shared<AIComponent>(ghost, mMap, type, mManager);


        tc->SetCircleRadius(15.f);
        cc->SetCircleRadius(15.f);
        tc->SetScale(scale.x, scale.y);
        tc->SetPosition(position.x, position.y);
        sc->CreateSpriteComponent(spritePath);

        ghost->AddComponent(tc);
        ghost->AddComponent(sc);
        ghost->AddComponent(cc);
        ghost->AddComponent(ai);
        ai->SetGhostMode(mode);
        mGhosts.push_back(ghost);
        mManager.AddGhost(ghost);
    }

    void SetGhostsMode(GhostMode mode, const std::string& spritePath) {
        for (auto& ghost : mGhosts) {
            auto ai = ghost->GetComponent<AIComponent>(ComponentType::AIComponent);
            ai->SetGhostMode(mode);
            auto sprite = ghost->GetComponent<SpriteComponent>(ComponentType::SpriteComponent);
            sprite->UpdateSpriteComponent(spritePath);
        }
    }

    void ResetGhostsToNormalMode() {
        for (auto& ghost : mGhosts) {
            auto ai = ghost->GetComponent<AIComponent>(ComponentType::AIComponent);
            ai->SetGhostMode(GhostMode::Chase);
            ai->mHasBeenEaten = false;
            // Update sprite paths based on ghost type
            auto sprite = ghost->GetComponent<SpriteComponent>(ComponentType::SpriteComponent);
            auto ghostSprite = ghost->GetComponent<SpriteComponent>(ComponentType::SpriteComponent);
            if (ai->GetGhostType() == GhostType::Blinky) {
                ghostSprite->UpdateSpriteComponent("../assets/blinky.png");
            }
            else if (ai->GetGhostType() == GhostType::Inky) {
                ghostSprite->UpdateSpriteComponent("../assets/inky.png");
            }
            else if (ai->GetGhostType() == GhostType::Pinky) {
                ghostSprite->UpdateSpriteComponent("../assets/pinky.png");
            }
            else {
                ghostSprite->UpdateSpriteComponent("../assets/clyde.png");
            }

        }
    }

    void HandleCollisions() {
        auto pacmanIC = mPacman->GetComponent<InputComponent>(ComponentType::InputComponent);
        auto pacmanPosition = mPacman->GetComponent<TransformComponent>(ComponentType::TransformComponent)->GetPosition();

        for (auto& ghost : mGhosts) {
            auto aiComponent = ghost->GetComponent<AIComponent>(ComponentType::AIComponent);
            aiComponent->SetPlayerPosition(pacmanPosition.x, pacmanPosition.y);

            if (mPacman->CheckCollision(*ghost)) {
                if (isPowerUpActive) {
                    if (!aiComponent->mHasBeenEaten) {
                        auto ghostTC = ghost->GetComponent<TransformComponent>(ComponentType::TransformComponent);
                        auto ghostSprite = ghost->GetComponent<SpriteComponent>(ComponentType::SpriteComponent);
                        // Set only this ghost to Eaten mode
                        aiComponent->SetGhostMode(GhostMode::Eaten);
                        aiComponent->mHasBeenEaten = true;
                        // Add 200 points only once
                        pacmanIC->UpdateScore(200);

                        // Change only this ghost's sprite to eyes
                        ghostSprite->UpdateSpriteComponent("../assets/eye.png");
                    }
                }
                else {
                    if (mPacLife > 0) {
                        mPacLife--;
                    }

                    if (mPacLife <= 0) {
                        mGameOverSequence = true;
                        mFreezeState = true;
                        mGameOverClock.restart();
                        if (mGameOverSequence && mGameOverClock.getElapsedTime().asSeconds() >= 2.0f) {
                            mGameOn = false;
                        }
                    }
                    else {
                        auto tc = mPacman->GetComponent<TransformComponent>(ComponentType::TransformComponent);
                        // Reset Pac-Man position
                        tc->SetPosition(306, 512);
                        // Reset ghosts' positions
                        for (auto& ghost : mGhosts) {
                            auto aiComponent = ghost->GetComponent<AIComponent>(ComponentType::AIComponent);
                            if (aiComponent->GetGhostType() == GhostType::Blinky) {
                                ghost->GetComponent<TransformComponent>(ComponentType::TransformComponent)->SetPosition(32, 32);
                                aiComponent->SetGhostMode(GhostMode::Chase);
                            }
                            if (aiComponent->GetGhostType() == GhostType::Pinky) {
                                ghost->GetComponent<TransformComponent>(ComponentType::TransformComponent)->SetPosition(288, 320);
                                aiComponent->SetGhostMode(GhostMode::InHouse);
                            }
                            if (aiComponent->GetGhostType() == GhostType::Inky) {
                                ghost->GetComponent<TransformComponent>(ComponentType::TransformComponent)->SetPosition(320, 320);
                                aiComponent->SetGhostMode(GhostMode::InHouse);
                            }
                            if (aiComponent->GetGhostType() == GhostType::Clyde) {
                                ghost->GetComponent<TransformComponent>(ComponentType::TransformComponent)->SetPosition(352, 320);
                                aiComponent->SetGhostMode(GhostMode::InHouse);
                            }
                        }
                        // Reset ghost positions if needed
                        isResetting = true;
                        resetDelayClock.restart();
                        break;
                    }

                }

            }
        }
    }
    void UpdateScoreDisplay() {
        int currentScore = mPacmanInput->GetScore();
        mScoreText.setString("Score: " + std::to_string(currentScore));
    }

    void UpdateLifeDisplay() {
        mLifeText.setString("Lives: " + std::to_string(mPacLife));
    }

    void UpdateResultDisplay() {
        if (mPacLife == 0) {
            mResultText.setString("Game Over!");
        }
    }

    void UpdateWinDisplay() {
        if (mPacmanInput->Won()) {
            mWinText.setString("You Won! Final Score: " + std::to_string(mPacmanInput->GetScore()));
        }
    }

    Map mMap;
    GameEntityManager mManager;
    std::shared_ptr<GameEntity> mPacman;
    std::shared_ptr<InputComponent> mPacmanInput;
    std::vector<std::shared_ptr<GameEntity>> mGhosts;
    sf::Clock powerUpClock;
    sf::Clock mGameOverClock;
    bool mGameOverSequence = false;
    bool isPowerUpActive{ false };
    bool isResetting{ false };
    bool mGameOn{ true };
    bool mFreezeState{ false };
    int mPacLife{ 3 };
    sf::Clock resetDelayClock;
    sf::Font mFont;
    sf::Text mScoreText;
    sf::Text mLifeText;
    sf::Text mResultText;
    sf::Text mWinText;
    const float resetDelayTime{ 3.0f };
};
