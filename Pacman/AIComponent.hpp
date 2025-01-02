#pragma once
#include "Component.hpp"
#include "Direction.hpp"
#include "Map.hpp"
#include "GhostType.hpp"
#include "GhostMode.hpp"
#include <SFML/System/Vector2.hpp>
#include <random>


class GameEntityManager; // Forward declaration

class AIComponent : public Component
{
public:
    AIComponent(std::shared_ptr<GameEntity> gameEntity, Map& map, GhostType type, GameEntityManager& manager);
    ~AIComponent() override = default;

    void Input() override {}
    void Update() override;
    void Render(sf::RenderWindow& ref) override {}

    ComponentType GetType() override { return ComponentType::AIComponent; }

    // Why do I even have this in here? this shouldn't be possible
    void SetPlayerPosition(float x, float y);

    // Setters and Getters for state
    void SetGhostMode(GhostMode mode);
    GhostMode GetMode() const { return mMode; }

    // Setters and Getters for state
    void SetMode(GhostType type) { mType = type; }
    GhostType GetGhostType() const { return mType; }


    Direction HandleFrightenedMode();
    Direction HandleInHouseMode();
    Direction HandleEatenMode();
    Direction HandleChaseMode();

    sf::Vector2f GetScatterTarget() const;

    void SetSpeed(float speed) {
        mSpeed = speed;
    }
    bool IsInsideGhostHouse();
    bool mHasBeenEaten = false;
    bool mInHouse = true;

    Direction DetermineNewDirection();
    void UpdateModeAndSpeed();

private:

    GameEntityManager& mManager;
    int mSpeed{ 1 };
    int mBaseSpeed = 1; // Default speed
    int mSpeedMultiplier = 1; // Multiplier for different modes
    int mTileSize{ 32 };
    Map& mMap;
    GhostMode mMode = GhostMode::Scatter;
    GhostType mType;
    Direction mCurrentDirection = Direction::Right;
    float mPlayerPositionX, mPlayerPositionY;


    std::mt19937 mRng{std::random_device{}()};
    sf::Vector2f mBScatterTaret {514, 32}; // Top-right corner
    sf::Vector2f mPScatterTaret {32, 32}; // Top-left corner
    sf::Vector2f mIScatterTaret {32, 672}; // Bottom-right corner
    sf::Vector2f mCScatterTaret {514, 514};
    sf::Vector2f mHomeBase {320, 352};// Bottom-left corner


    bool CheckSurroundingTiles(float x, float y, const Map& map);
    //DELETE This funtion, it's not doing anything
    /*Direction ChooseDirection();*/

    void Move(Direction direction);
    sf::Vector2f GetChaseTargetPosition();
    Direction GetBestDirection(float targetX, float targetY);
    Direction ReverseDirection(Direction dir);
    void GetNextPosition(float currentX, float currentY, Direction dir, float& outX, float& outY);
    float DistanceSquared(float x1, float y1, float x2, float y2);
    float mEatenTimer = 0.0f;
    float mScatterTimer = 0.0f;
    float mChaseTimer = 0.0f;
    float mModeTimer = 0.0f; // Tracks time spent in the current mode

    const float SCATTER_DURATION = 7.0f; // Scatter mode duration (in seconds)
    const float CHASE_DURATION = 20.0f; // Chase mode duration (in seconds)
    const float FRIGHTENED_DURATION = 5.0f; // Frightened mode duration (in seconds)
};

