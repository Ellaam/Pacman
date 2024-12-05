#include "AIComponent.hpp"
#include "GameEntityManager.hpp"
#include<iostream>


AIComponent::AIComponent(std::shared_ptr<GameEntity> gameEntity, Map& map, GhostType type, GameEntityManager& manager)
    : Component(gameEntity), mMap(map), mType(type),mManager(manager), mMode(GhostMode::Chase) {

}

void AIComponent::Update() {

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
    
    // get the ghost's current position
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();
    // Update timers and handle mode transitions
    
    Direction newDirection;

    // Update timers based on current mode
    if (mMode == GhostMode::InHouse) {
        newDirection = HandleInHouseMode(); 
    }
    else if (mMode == GhostMode::Scatter) {
        auto scatterTarget = GetScatterTarget(); // Return the scatter target based on ghost type
        newDirection = GetBestDirection(scatterTarget.x, scatterTarget.y);
    }
    else if (mMode == GhostMode::Frightened) {
        newDirection = HandleFrightenedMode();
    }
    else if (mMode == GhostMode::Eaten) {
        newDirection = HandleEatenMode();
    }
    else if (mMode == GhostMode::Chase) {
        newDirection = HandleChaseMode();
    }

    // Check if the new direction is valid
    bool canMove = false;
    switch (newDirection) {
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

    // Move in the new direction if possible
    if (canMove) {
        mCurrentDirection = newDirection;
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
    else {

        // If can't move in the new direction, try to continue in the current direction
        canMove = !CheckSurroundingTiles(
            currentPosition.x + (mCurrentDirection == Direction::Right ? mSpeed : (mCurrentDirection == Direction::Left ? -mSpeed : 0)),
            currentPosition.y + (mCurrentDirection == Direction::Down ? mSpeed : (mCurrentDirection == Direction::Up ? -mSpeed : 0)),
            mMap
        );
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
    }

    // Wrap-around boundaries
    if (currentPosition.x < -mTileSize) {
        currentPosition.x = 640 - mSpeed;
    }
    else if (currentPosition.x > 640) {
        currentPosition.x = -mTileSize + mSpeed;
    }

    tc->SetPosition(currentPosition.x, currentPosition.y);
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
        mSpeedMultiplier = 4; // Speed up in Eaten mode
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
    //auto entity = GetGameEntity();
    //auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    //auto currentPosition = tc->GetPosition();

    auto targetTile = GetChaseTargetPosition();
    Direction bestDirection = GetBestDirection(targetTile.x, targetTile.y);
    
    // DELETE THIS, AS the GETBESTDirection already takes care of it

    //if (!CanMove(bestDirection)) {
    //    std::vector<Direction> validDirections;
    //    //Ghosts started shaking after starting i from 1 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //    for (int i = 0; i < 4; ++i) {
    //        Direction dir = static_cast<Direction>(i);
    //        if (CanMove(dir) && dir != ReverseDirection(mCurrentDirection)) {
    //            validDirections.push_back(dir);
    //        }
    //    }
    //    if (!validDirections.empty()) {
    //        std::sort(validDirections.begin(), validDirections.end(),
    //            [&](Direction a, Direction b) {
    //                float aX, aY, bX, bY;
    //                GetNextPosition(currentPosition.x, currentPosition.y, a, aX, aY);
    //                GetNextPosition(currentPosition.x, currentPosition.y, b, bX, bY);
    //                return DistanceSquared(aX, aY, targetTile.x, targetTile.y) <
    //                    DistanceSquared(bX, bY, targetTile.x, targetTile.y);
    //            });
    //        bestDirection = validDirections[0];
    //    }
    //    else {
    //        bestDirection = ReverseDirection(mCurrentDirection);
    //    }
    //}
    return bestDirection;
}

Direction AIComponent::HandleEatenMode() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    if (DistanceSquared(currentPosition.x, currentPosition.y, mHomeBase.x, mHomeBase.y) < mTileSize * mTileSize) {
        // If ghost reaches home base, reset to InHouse mode
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

    return GetBestDirection(mHomeBase.x, mHomeBase.y);
}  

Direction AIComponent::HandleFrightenedMode() {

    //CAN BE REPLACED WITH THE CHOOSE DIRECTION FUNCTION - TO DO: REMOVE THE CODE BELOW AND REPLACE WITH THAT FUNCTION
    // Get a random direction from valid choices
    std::vector<Direction> validDirections;
    // Also i=0 causes shaking ++++++++++++++++++++++++++++++++++++++++++
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        if (CanMove(dir) && dir != ReverseDirection(mCurrentDirection)) {
            validDirections.push_back(dir);
        }
    }

    // If no valid direction, reverse
    if (validDirections.empty()) {
        return ReverseDirection(mCurrentDirection);
    }

    // Choose randomly from valid directions
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
                if (mMode == GhostMode::Eaten ||mMode == GhostMode::InHouse) {
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

bool AIComponent::IsInsideGhostHouse() {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto ic = entity->GetComponent<AIComponent>(ComponentType::AIComponent);
    auto currentPosition = tc->GetPosition();

    // Define the ghost house boundaries
    int ghostHouseLeft = 256;
    int ghostHouseRight = 352;
    int ghostHouseTop = 256;
    int ghostHouseBottom = 352;

    return (currentPosition.x >= ghostHouseLeft && currentPosition.x <= ghostHouseRight &&
        currentPosition.y >= ghostHouseTop && currentPosition.y <= ghostHouseBottom);
}

Direction AIComponent::ChooseDirection() {

    std::vector<Direction> possibleDirections;

    // Check all 4 possible directions
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        if (CanMove(dir) && dir != ReverseDirection(mCurrentDirection)) {
            possibleDirections.push_back(dir);
        }
    }

    // If no possible directions are available, the ghost can reverse direction or choose randomly
    if (possibleDirections.empty()) {
        return ReverseDirection(mCurrentDirection);
    }

    // Randomly choose from possible directions
    std::uniform_int_distribution<> dist(0, possibleDirections.size() - 1);
    return possibleDirections[dist(mRng)];
}

bool AIComponent::CanMove(Direction direction) {

    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    float nextX = currentPosition.x;
    float nextY = currentPosition.y;

    // Calculate the next position based on the direction
    switch (direction) {
    case Direction::Up:    nextY -= mTileSize; break;
    case Direction::Down:  nextY += mTileSize; break;
    case Direction::Left:  nextX -= mTileSize; break;
    case Direction::Right: nextX += mTileSize; break;
    default: break;
    }

    // Check if the new position is out of bounds
    if (nextX < 0 || nextY < 0 || nextX >= 20 * mTileSize || nextY >= 22 * mTileSize) {
        return false; 
    }

    // Convert nextX, nextY to tile-based coordinates
    int tileX = static_cast<int>(std::floor(nextX / mTileSize));
    int tileY = static_cast<int>(std::floor(nextY / mTileSize));


    // Check the tile type (1 is wall)
    return mMap.GetTileTypeAt(tileX, tileY) != 1;
}

void AIComponent::Move(Direction direction) {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    bool hasMoved = false;

    // Try to move in the new direction
    if (CanMove(direction)) {
        switch (direction) {
        case Direction::Up: currentPosition.y -= mSpeed; hasMoved = true; break;
        case Direction::Down: currentPosition.y += mSpeed; hasMoved = true; break;
        case Direction::Left: currentPosition.x -= mSpeed; hasMoved = true; break;
        case Direction::Right: currentPosition.x += mSpeed; hasMoved = true; break;
        default: break;
        }
    }

    // Align the position to the tile center after movement
    if (hasMoved) {
        /*currentPosition.x = std::round(currentPosition.x / mTileSize) * mTileSize;
        currentPosition.y = std::round(currentPosition.y / mTileSize) * mTileSize;*/

        tc->SetPosition(currentPosition.x, currentPosition.y);
    }
}

Direction AIComponent::GetBestDirection(float targetX, float targetY) {
    auto entity = GetGameEntity();
    auto tc = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
    auto currentPosition = tc->GetPosition();

    std::vector<Direction> possibleDirections;
    for (int i = 0; i < 4; ++i) {
        Direction dir = static_cast<Direction>(i);
        if (CanMove(dir) && dir != ReverseDirection(mCurrentDirection)) {
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

Direction AIComponent::ReverseDirection(Direction dir) {
    switch (dir) {
    case Direction::Up: return Direction::Down;
    case Direction::Down: return Direction::Up;
    case Direction::Left: return Direction::Right;
    case Direction::Right: return Direction::Left;
 /*   default: return Direction::None;*/
    }
}

void AIComponent::GetNextPosition(float currentX, float currentY, Direction dir, float& outX, float& outY) {
    switch (dir) {
    case Direction::Up:    outX = currentX; outY = currentY - mTileSize; break;
    case Direction::Down:  outX = currentX; outY = currentY + mTileSize; break;
    case Direction::Left:  outX = currentX - mTileSize; outY = currentY; break;
    case Direction::Right: outX = currentX + mTileSize; outY = currentY; break;
    default: outX = currentX; outY = currentY; break;
    }
}

float AIComponent::DistanceSquared(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return dx * dx + dy * dy;
}

