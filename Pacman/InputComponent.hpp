#pragma once
#include <SFML/Graphics.hpp>
#include "Component.hpp"
#include "GameEntity.hpp"
#include "TransformComponent.hpp"
#include "Direction.hpp"
#include "Map.hpp"
#include <iostream>
#include <array>
#include <cmath>

class InputComponent : public Component {
public:
    InputComponent(std::shared_ptr<GameEntity> gameEntity, Map& map)
        : Component(gameEntity), mMap(map) {}
    ~InputComponent() override = default;

    ComponentType GetType() override {
        return ComponentType::InputComponent;
    }


    bool CheckSurroundingTiles(float x, float y, const Map& map) {
        float tile_x = x / mTileSize;
        float tile_y = y / mTileSize;

        for (int t = 0; t < 4; t++) {
            int xx = 0;
            int yy = 0;

            switch (t) {
            case 0: xx = static_cast<int>(floor(tile_x)); yy = static_cast<int>(floor(tile_y)); break;
            case 1: xx = static_cast<int>(ceil(tile_x)); yy = static_cast<int>(floor(tile_y)); break;
            case 2: xx = static_cast<int>(floor(tile_x)); yy = static_cast<int>(ceil(tile_y)); break;
            case 3: xx = static_cast<int>(ceil(tile_x)); yy = static_cast<int>(ceil(tile_y)); break;
            }

            if (0 <= xx && xx < 20 && 0 <= yy && yy < 22) {
                if (map.GetTileTypeAt(xx, yy) == static_cast<int>(TileType::Wall)) {
                    return true; // There is a wall
                }
            }

            if (map.GetTileTypeAt(xx, yy) == static_cast<int>(TileType::Pellet)) {
                // Consume the pellet
                auto pallet = map.GetTileEntity(xx, yy);
                if (pallet) {
                    pallet->SetTileType(TileType::Empty);
                    const_cast<Map&>(map).SetTileTypeAt(xx, yy, TileType::Empty);
                    mScore += 1; // Increment score
                    mSmallp -= 1;
                    //std::cout << "Pellet collected! Score: " << mScore << std::endl;
                    if (mLargP == 0 and mSmallp == 0) {
                        std::cout << "You Won! Your final score is: " << mScore << std::endl;
                        mWon = true;
                    }

                }
            }

            if (map.GetTileTypeAt(xx, yy) == static_cast<int>(TileType::PowerPellet)) {

                // Consume the pellet
                auto largePallet = map.GetTileEntity(xx, yy);
                if (largePallet) {
                    largePallet->SetTileType(TileType::Empty);
                    const_cast<Map&>(map).SetTileTypeAt(xx, yy, TileType::Empty);
                    mScore += 10; // Increment score
                    mLargP -= 1;
                    //std::cout << "Large Pellet collected! Score: " << mScore << std::endl;
                    if (mLargP == 0 and mSmallp == 0) {
                        //std::cout << "You Won! Your final score is: " << mScore << std::endl;
                        mWon = true;
                    }

                    mLargePalletEatenCount += 1;
                    mPowerUp = true;

                }
            }
        }
        return false; // No wall found
    }


    void Input() override {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            queuedDirection = Direction::Left;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            queuedDirection = Direction::Right;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            queuedDirection = Direction::Up;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            queuedDirection = Direction::Down;
        }
    }

    void Update() override {
        auto entity = GetGameEntity();
        auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
        auto currentPosition = tc->GetPosition();


        // Check if the queued direction is available
        bool canSwitchToQueued = false;
        switch (queuedDirection) {
        case Direction::Left:
            canSwitchToQueued = !CheckSurroundingTiles(currentPosition.x - mSpeed, currentPosition.y, mMap);
            break;
        case Direction::Right:
            canSwitchToQueued = !CheckSurroundingTiles(currentPosition.x + mSpeed, currentPosition.y, mMap);
            break;
        case Direction::Up:
            canSwitchToQueued = !CheckSurroundingTiles(currentPosition.x, currentPosition.y - mSpeed, mMap);
            break;
        case Direction::Down:
            canSwitchToQueued = !CheckSurroundingTiles(currentPosition.x, currentPosition.y + mSpeed, mMap);
            break;
            /*       case Direction::None:
                       break;*/
        }

        // Switch to queued direction if it's valid
        if (canSwitchToQueued) {
            mCurrentDirection = queuedDirection;
        }

        // Check if the current direction is still valid
        bool canMove = false;
        switch (mCurrentDirection) {
        case Direction::Left:
            canMove = !CheckSurroundingTiles(currentPosition.x - mSpeed, currentPosition.y, mMap);
            break;
        case Direction::Right:
            canMove = !CheckSurroundingTiles(currentPosition.x + mSpeed, currentPosition.y, mMap);
            break;
        case Direction::Up:
            canMove = !CheckSurroundingTiles(currentPosition.x, currentPosition.y - mSpeed, mMap);
            break;
        case Direction::Down:
            canMove = !CheckSurroundingTiles(currentPosition.x, currentPosition.y + mSpeed, mMap);
            break;
            //case Direction::None:
            //    break;
        }

        // Move in the current direction if possible
        if (canMove) {
            switch (mCurrentDirection) {
            case Direction::Left:
                currentPosition.x -= mSpeed;
                break;
            case Direction::Right:
                currentPosition.x += mSpeed;
                break;
            case Direction::Up:
                currentPosition.y -= mSpeed;
                break;
            case Direction::Down:
                currentPosition.y += mSpeed;
                break;
                //case Direction::None:
                //    break;
            }
        }

        // Wrap-around boundaries
        if (currentPosition.x < -mTileSize) {
            currentPosition.x = 640 - mSpeed;
        }
        else if (currentPosition.x > 640) {
            currentPosition.x = -mTileSize + mSpeed;
        }
        currentPosition.y = std::max(0.0f, std::min(currentPosition.y, 704.0f - mTileSize));

        tc->SetPosition(currentPosition.x, currentPosition.y);
    }

    void ResetPosition(const sf::Vector2f& position) {
        auto entity = GetGameEntity();
        auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
        tc->SetPosition(position.x, position.y);
    }

    void Render(sf::RenderWindow& ref) override {}

    Direction GetCurrentDirection() const {
        return mCurrentDirection;

    }

    bool IsPowerUp() {
        return mPowerUp;
    }

    bool Won() {
        return mWon;
    }

    void SetPowerUp(bool value) {
        mPowerUp = value;
    }

    int GetLpalletEatenCount() const {
        return mLargePalletEatenCount;
    }

    void ResetLargePalletEatenCount() {
        mLargePalletEatenCount = 0;
    }

    void UpdateScore(int val) {
        mScore += val;
    }

    int GetScore() const {
        return mScore;
    }

private:
    Direction mCurrentDirection = Direction::Right;
    Direction queuedDirection = Direction::Right; // New variable for queued direction
    int mSpeed{ 2 };
    Map& mMap;
    int mTileSize{ 32 };
    int mScore{ 0 };
    int mLargP{ 4 };
    int mSmallp{ 154 };
    int mLargePalletEatenCount{ 0 };
    bool mPowerUp{ false };
    bool mWon{ false };
};
