#include "AIComponent.hpp"
#include "GameEntityManager.hpp"
#include<iostream>


AIComponent::AIComponent(std::shared_ptr<GameEntity> gameEntity, Map& map, GhostType type, GameEntityManager& manager)
    : Component(gameEntity), mMap(map), mType(type), mManager(manager), mMode(GhostMode::Chase) {

}

void AIComponent::Update() {

    UpdateModeAndSpeed();
    Direction newDirection = DetermineNewDirection();
    Move(newDirection);
    std::cout << "mMode: " << static_cast<int>(mMode) << std::endl;
    std::cout << "Is in house? " << IsInsideGhostHouse() << std::endl;

}

void AIComponent::SetGhostMode(GhostMode mode) {
    mMode = mode;
    mModeTimer = 0.0f;
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
            case Direction::Up: return sf::Vector2f(pacManPosition.x, pacManPosition.y - 4 * mTileSize);
            case Direction::Down: return sf::Vector2f(pacManPosition.x, pacManPosition.y + 4 * mTileSize);
            case Direction::Left: return sf::Vector2f(pacManPosition.x - 4 * mTileSize, pacManPosition.y);
            case Direction::Right: return sf::Vector2f(pacManPosition.x + 4 * mTileSize, pacManPosition.y);
            default:return pacManPosition;
            }
            break;
        }
        case GhostType::Inky: {
            // Blinky's position
            sf::Vector2f blinkyPosition = mManager.GetGhostPosition(GhostType::Blinky);
            sf::Vector2f targetPoint;

            switch (mCurrentDirection) {
            case Direction::Up: targetPoint = sf::Vector2f(pacManPosition.x - 2 * mTileSize, pacManPosition.y - 2 * mTileSize);
                break;
            case Direction::Down: targetPoint = sf::Vector2f(pacManPosition.x, pacManPosition.y + 2 * mTileSize);
                break;
            case Direction::Left: targetPoint = sf::Vector2f(pacManPosition.x - 2 * mTileSize, pacManPosition.y);
                break;
            case Direction::Right: targetPoint = sf::Vector2f(pacManPosition.x + 2 * mTileSize, pacManPosition.y);
                break;
            default: targetPoint = pacManPosition;
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
        SetGhostMode(GhostMode::InHouse);
        mHasBeenEaten = false;

        // Reset ghost appearance immediately
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


Direction AIComponent::HandleInHouseMode() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    // Define the exit point of the ghost house
    sf::Vector2f exitPoint(488.f, 256.f);

    // Check if ghost has reached the exit point
    if (DistanceSquared(currentPosition.x, currentPosition.y, exitPoint.x, exitPoint.y) < mTileSize * mTileSize) {
        SetGhostMode(GhostMode::Scatter);
        auto sprite = entity->GetComponent<SpriteComponent>(ComponentType::SpriteComponent);
        switch (mType) {
        case GhostType::Blinky: sprite->UpdateSpriteComponent("../assets/blinky.png"); break;
        case GhostType::Pinky: sprite->UpdateSpriteComponent("../assets/pinky.png"); break;
        case GhostType::Inky: sprite->UpdateSpriteComponent("../assets/inky.png"); break;
        case GhostType::Clyde: sprite->UpdateSpriteComponent("../assets/clyde.png"); break;
        }

        return Direction::Up;
    }
    return GetBestDirection(exitPoint.x, exitPoint.y);
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

bool AIComponent::IsInsideGhostHouse() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto ic = entity->GetComponent<AIComponent>(ComponentType::AIComponent);
    auto currentPosition = tc->GetPosition();
    //std::cout << currentPosition.x << " , " << currentPosition.y << std::endl;

    // Define the ghost house boundaries
    int ghostHouseLeft = 256;
    int ghostHouseRight = 351;
    int ghostHouseTop = 255;
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
                if (mMode == GhostMode::Eaten || mMode == GhostMode::InHouse || IsInsideGhostHouse()) {
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

    if (mMode == GhostMode::Eaten) {
    mSpeedMultiplier = 4;
    }
    else if (mMode == GhostMode::Frightened) {
        mSpeedMultiplier = 1;
    }
    else {
        mSpeedMultiplier = 1; // Normal speed for other modes
    }

    std::vector<Direction> possibleDirections;
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        float nextX = currentPosition.x;
        float nextY = currentPosition.y;

        int speed = (static_cast<int>(currentPosition.x) % mSpeedMultiplier == 0 &&
            static_cast<int>(currentPosition.y) % mSpeedMultiplier == 0) ?
            mSpeed * mSpeedMultiplier : mSpeed;

        switch (dir) {
        case Direction::Up:    nextY -= speed; break;
        case Direction::Down:  nextY += speed; break;
        case Direction::Left:  nextX -= speed; break;
        case Direction::Right: nextX += speed; break;
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
    if (mMode == GhostMode::Eaten) {
        mSpeedMultiplier = 4;
    }
    else if (mMode == GhostMode::Frightened) {
        mSpeedMultiplier = 1;
    }
    else {
        mSpeedMultiplier = 1; // Normal speed for other modes
    }
    int speed = (static_cast<int>(currentPosition.x) % mSpeedMultiplier == 0 &&
        static_cast<int>(currentPosition.y) % mSpeedMultiplier == 0) ?
        mSpeed * mSpeedMultiplier : mSpeed;

    float nextX = currentPosition.x;
    float nextY = currentPosition.y;

    switch (direction) {
    case Direction::Up:    nextY -= speed; break;
    case Direction::Down:  nextY += speed; break;
    case Direction::Left:  nextX -= speed; break;
    case Direction::Right: nextX += speed; break;
    }
    if (nextX < 0) {
        nextX = 20 * mTileSize - speed;
    }
    else if (nextX >= 20 * mTileSize) {
        nextX = speed;
    }

    if (!CheckSurroundingTiles(nextX, nextY, mMap)) {
        currentPosition.x = nextX;
        currentPosition.y = nextY;
        mCurrentDirection = direction;
        tc->SetPosition(currentPosition.x, currentPosition.y);
    }
}

void AIComponent::UpdateModeAndSpeed() {

    float deltaTime = 1.0f / 120.0f;
    mModeTimer += deltaTime;

    // Handle mode transitions
    if (IsInsideGhostHouse() && mMode == GhostMode::Eaten) {
        SetGhostMode(GhostMode::InHouse);
    }
    else if (mMode == GhostMode::InHouse && !IsInsideGhostHouse()) {
        SetGhostMode(GhostMode::Scatter);
    }
    else if (mMode == GhostMode::Scatter && mModeTimer >= SCATTER_DURATION) {
        SetGhostMode(GhostMode::Chase);
    }
    else if (mMode == GhostMode::Chase && mModeTimer >= CHASE_DURATION) {
        SetGhostMode(GhostMode::Scatter);
    }
    else if (mMode == GhostMode::Frightened && mModeTimer >= FRIGHTENED_DURATION) {
        SetGhostMode(GhostMode::Chase);
    }

}

Direction AIComponent::DetermineNewDirection() {
    /*std::cout << static_cast<int>(mMode) << std::endl;*/
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
    if (mMode == GhostMode::Eaten) {
        mSpeedMultiplier = 4;
    }
    else if (mMode == GhostMode::Frightened) {
        mSpeedMultiplier = 1;
    }
    else {
        mSpeedMultiplier = 1; // Normal speed for other modes
    }
    int speed = (static_cast<int>(currentX) % mSpeedMultiplier == 0 && static_cast<int>(currentY) % mSpeedMultiplier == 0) ?
        mSpeed * mSpeedMultiplier : mSpeed;

    switch (dir) {
    case Direction::Up:    outX = currentX; outY = currentY - speed; break;
    case Direction::Down:  outX = currentX; outY = currentY + speed; break;
    case Direction::Left:  outX = currentX - speed; outY = currentY; break;
    case Direction::Right: outX = currentX + speed; outY = currentY; break;
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