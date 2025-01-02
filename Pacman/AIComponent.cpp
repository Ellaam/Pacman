#include "AIComponent.hpp"
#include "GameEntityManager.hpp"
#include<iostream>


AIComponent::AIComponent(std::shared_ptr<GameEntity> gameEntity, Map& map, GhostType type, GameEntityManager& manager)
    : Component(gameEntity), mMap(map), mType(type),mManager(manager), mMode(GhostMode::Chase) {

}

void AIComponent::Update() {

    UpdateModeAndSpeed();
    Direction newDirection = DetermineNewDirection();
    Move(newDirection);

    //// Wrap-around boundaries
    //if (currentPosition.x < -mTileSize) {
    //    currentPosition.x = 640 - mSpeed;
    //}
    //else if (currentPosition.x > 640) {
    //    currentPosition.x = -mTileSize + mSpeed;
    //}
}

void AIComponent::SetPlayerPosition(float x, float y) {
    mPlayerPositionX = x;
    mPlayerPositionY = y;
}

void AIComponent::SetGhostMode(GhostMode mode) {
    mMode = mode;
    mModeTimer = 0.0f;

    if (mode == GhostMode::Frightened) {
        mSpeedMultiplier = 1; // Slow down in Frightened mode
    }
    else if (mode == GhostMode::Eaten) {
        mSpeedMultiplier = 2; // Speed up in Eaten mode
    }
    else {
        mSpeedMultiplier = 2; // Normal speed for other modes
    }
}

sf::Vector2f AIComponent::GetScatterTarget() const {
    switch (mType) {
    case GhostType::Blinky: return mBScatterTaret;
    case GhostType::Pinky: return mPScatterTaret;
    case GhostType::Inky: return mIScatterTaret;
    case GhostType::Clyde: return mCScatterTaret;
    default: return sf::Vector2f(0, 0);
    }
}

sf::Vector2f AIComponent::GetChaseTargetPosition() {

    auto pacManPosition = mManager.GetPacmanPosition();
    auto pacManDirection = mManager.GetPacmanDirection();

    if (mMode == GhostMode::Chase) {
        switch (mType) {

        case GhostType::Blinky:
            return sf::Vector2f(pacManPosition.x, pacManPosition.y);

        case GhostType::Pinky: {
            switch (mCurrentDirection) {
            case Direction::Up:
                return sf::Vector2f(pacManPosition.x, pacManPosition.y - 4 * mTileSize);

            case Direction::Down:
                return sf::Vector2f(pacManPosition.x, pacManPosition.y + 4 * mTileSize);

            case Direction::Left:
                return sf::Vector2f(pacManPosition.x - 4 * mTileSize, pacManPosition.y);

            case Direction::Right:
                return sf::Vector2f(pacManPosition.x + 4 * mTileSize, pacManPosition.y);

            default:
                return pacManPosition;

            }
            break;
        }
        case GhostType::Inky: {
            // Blinky's position
            sf::Vector2f blinkyPosition = mManager.GetGhostPosition(GhostType::Blinky);

            sf::Vector2f targetPoint;

            switch (mCurrentDirection) {
            case Direction::Up:
                targetPoint = sf::Vector2f(pacManPosition.x - 2 * mTileSize, pacManPosition.y - 2 * mTileSize);
                break;
            case Direction::Down:
                targetPoint = sf::Vector2f(pacManPosition.x, pacManPosition.y + 2 * mTileSize);
                break;
            case Direction::Left:
                targetPoint = sf::Vector2f(pacManPosition.x - 2 * mTileSize, pacManPosition.y);
                break;
            case Direction::Right:
                targetPoint = sf::Vector2f(pacManPosition.x + 2 * mTileSize, pacManPosition.y);
                break;
            default:
                targetPoint = pacManPosition;
                break;
            }

            sf::Vector2f vector = targetPoint - blinkyPosition;
            return blinkyPosition + (vector * 2.0f);

        }
        case GhostType::Clyde: {
            // Clyde's position
            auto clydePosition = mManager.GetGhostPosition(GhostType::Clyde);
            float dx = pacManPosition.x - clydePosition.x;
            float dy = pacManPosition.y - clydePosition.y;
            if (std::sqrt(dx * dx + dy * dy) > 4 * mMap.mTileSize) {
                return pacManPosition;
            }
            else {
                return mCScatterTaret;
            }
        }
        }
    }

    return sf::Vector2f(0, 0);
}

Direction AIComponent::HandleChaseMode() {

    auto targetTile = GetChaseTargetPosition();
    Direction bestDirection = GetBestDirection(targetTile.x, targetTile.y);
    
    return bestDirection;
}

Direction AIComponent::HandleEatenMode() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    // If ghost is near the home base, reset its state
    if (DistanceSquared(currentPosition.x, currentPosition.y, mHomeBase.x, mHomeBase.y) < mTileSize * mTileSize) {
        mMode = GhostMode::InHouse;
        mEatenTimer = 0.0f;
        mHasBeenEaten = false;

        // Reset ghost appearance
        auto sprite = entity->GetComponent<SpriteComponent>(ComponentType::SpriteComponent);
        switch (mType) {
        case GhostType::Blinky: sprite->UpdateSpriteComponent("../assets/blinky.png"); break;
        case GhostType::Pinky: sprite->UpdateSpriteComponent("../assets/pinky.png"); break;
        case GhostType::Inky: sprite->UpdateSpriteComponent("../assets/inky.png"); break;
        case GhostType::Clyde: sprite->UpdateSpriteComponent("../assets/clyde.png"); break;
        }

        return Direction::Up;
    }

    // Otherwise, move towards the home base deterministically
    return GetBestDirection(mHomeBase.x, mHomeBase.y);

}

Direction AIComponent::HandleFrightenedMode() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    // Get possible directions
    std::vector<Direction> validDirections;
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        if (!CheckSurroundingTiles(currentPosition.x, currentPosition.y, mMap) &&
            dir != ReverseDirection(mCurrentDirection)) {
            validDirections.push_back(dir);
        }
    }

    // If no valid directions, reverse
    if (validDirections.empty()) {
        return ReverseDirection(mCurrentDirection);
    }

    // Choose a random valid direction
    std::uniform_int_distribution<> dist(0, validDirections.size() - 1);
    return validDirections[dist(mRng)];
}

Direction AIComponent::HandleInHouseMode() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    // Define the exit point of the ghost house
    sf::Vector2f exitPoint(488.f, 256.f); 

    // Check if ghost has reached the exit point
    if (DistanceSquared(currentPosition.x, currentPosition.y, exitPoint.x, exitPoint.y) < mTileSize * mTileSize) {
        mMode = GhostMode::Scatter; // Transition back to Scatter or another mode
        mModeTimer = 0.0f;
    }
    return GetBestDirection(exitPoint.x, exitPoint.y);
}

bool AIComponent::IsInsideGhostHouse() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto ic = entity->GetComponent<AIComponent>(ComponentType::AIComponent);
    auto currentPosition = tc->GetPosition();

    // Define the ghost house boundaries
    int ghostHouseLeft = 256;
    int ghostHouseRight = 352;
    int ghostHouseTop = 256;
    int ghostHouseBottom = 350;

    return (currentPosition.x >= ghostHouseLeft && currentPosition.x <= ghostHouseRight &&
        currentPosition.y >= ghostHouseTop && currentPosition.y <= ghostHouseBottom);
}

bool AIComponent::CheckSurroundingTiles(float x, float y, const Map& map) {
    
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

        if (0 <= xx && xx <= 20 && 0 <= yy && yy <= 22) {
            if (map.GetTileTypeAt(xx, yy) == 1) {
                return true; // There is a wall
            }
            else if (map.GetTileTypeAt(xx, yy) == 4) { // Door tile
                if (mMode == GhostMode::Eaten ||mMode == GhostMode::InHouse || IsInsideGhostHouse()) {
                    return false; // Allow passage for Eaten mode or exiting the house
                }
                else {
                    return true; // Block entry for other modes
                }
            }
        }
    }
    return false; // No wall found
}

Direction AIComponent::GetBestDirection(float targetX, float targetY) {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    std::vector<Direction> possibleDirections;
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        float nextX = currentPosition.x;
        float nextY = currentPosition.y;

        switch (dir) {
        case Direction::Up:    nextY -= mSpeed * mSpeedMultiplier; break;
        case Direction::Down:  nextY += mSpeed * mSpeedMultiplier; break;
        case Direction::Left:  nextX -= mSpeed * mSpeedMultiplier; break;
        case Direction::Right: nextX += mSpeed * mSpeedMultiplier; break;
        }

        if (!CheckSurroundingTiles(nextX, nextY, mMap) && dir != ReverseDirection(mCurrentDirection)) {
            possibleDirections.push_back(dir);
        }
    }

    if (possibleDirections.empty()) {
        return ReverseDirection(mCurrentDirection);
    }

    return *std::min_element(possibleDirections.begin(), possibleDirections.end(),
        [&](Direction a, Direction b) {
            float posAX, posAY, posBX, posBY;
            GetNextPosition(currentPosition.x, currentPosition.y, a, posAX, posAY);
            GetNextPosition(currentPosition.x, currentPosition.y, b, posBX, posBY);
            return DistanceSquared(posAX, posAY, targetX, targetY) <
                DistanceSquared(posBX, posBY, targetX, targetY);
        });
}


void AIComponent::Move(Direction direction) {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    float nextX = currentPosition.x;
    float nextY = currentPosition.y;

    // Compute the next position based on direction
    switch (direction) {
    case Direction::Up:    nextY -= mSpeed * mSpeedMultiplier; break;
    case Direction::Down:  nextY += mSpeed * mSpeedMultiplier; break;
    case Direction::Left:  nextX -= mSpeed * mSpeedMultiplier; break;
    case Direction::Right: nextX += mSpeed * mSpeedMultiplier; break;
    }

    // Check if the target position is valid and move there
    if (!CheckSurroundingTiles(nextX, nextY, mMap)) {
        currentPosition.x = nextX;
        currentPosition.y = nextY;
        mCurrentDirection = direction;
        tc->SetPosition(currentPosition.x, currentPosition.y);
    }

    
}


void AIComponent::UpdateModeAndSpeed() {
    // Update mode timers and transitions
    // Update speed multiplier based on mode
    if (mMode == GhostMode::Eaten) {
        mSpeedMultiplier = 2;
    }
    else if (mMode == GhostMode::Frightened) {
        mSpeedMultiplier = 1;
    }
    else {
        mSpeedMultiplier = 1; // Normal speed for other modes
    }
    mSpeed = mBaseSpeed * mSpeedMultiplier;

    float deltaTime = 1.0f / 120.0f;
    mModeTimer += deltaTime;

    // Handle mode transitions
    if (mMode == GhostMode::InHouse && !IsInsideGhostHouse()) {
        mMode = GhostMode::Scatter;
        mModeTimer = 0.0f;
    }
    else if (mMode == GhostMode::Scatter && mModeTimer >= SCATTER_DURATION) {
        mMode = GhostMode::Chase;
        mModeTimer = 0.0f;
    }
    else if (mMode == GhostMode::Chase && mModeTimer >= CHASE_DURATION) {
        mMode = GhostMode::Scatter;
        mModeTimer = 0.0f;
    }
    else if (mMode == GhostMode::Frightened && mModeTimer >= FRIGHTENED_DURATION) {
        mMode = GhostMode::Chase; // Return to Chase after Frightened
        mModeTimer = 0.0f;
    }
}

Direction AIComponent::DetermineNewDirection() {
    switch (mMode) {
    case GhostMode::InHouse: return HandleInHouseMode();
    case GhostMode::Scatter: return GetBestDirection(GetScatterTarget().x, GetScatterTarget().y);
    case GhostMode::Frightened: return HandleFrightenedMode();
    case GhostMode::Eaten: return HandleEatenMode();
    case GhostMode::Chase: return HandleChaseMode();
    //default: return mCurrentDirection;
    }
}

void AIComponent::GetNextPosition(float currentX, float currentY, Direction dir, float& outX, float& outY) {
    switch (dir) {
    case Direction::Up:    outX = currentX; outY = currentY - (mSpeed * mSpeedMultiplier); break;
    case Direction::Down:  outX = currentX; outY = currentY + (mSpeed * mSpeedMultiplier); break;
    case Direction::Left:  outX = currentX - (mSpeed * mSpeedMultiplier); outY = currentY; break;
    case Direction::Right: outX = currentX + (mSpeed * mSpeedMultiplier); outY = currentY; break;
    default: outX = currentX; outY = currentY; break;
    }
}

Direction AIComponent::ReverseDirection(Direction dir) {
    switch (dir) {
    case Direction::Up: return Direction::Down;
    case Direction::Down: return Direction::Up;
    case Direction::Left: return Direction::Right;
    case Direction::Right: return Direction::Left;
        /*   default: return Direction::None;*/
    }
}

float AIComponent::DistanceSquared(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

