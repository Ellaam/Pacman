#include "GameEntity.hpp"
#include "CollisionComponent.hpp"


bool GameEntity::CheckCollision(GameEntity& other) {
    auto thisCollision = GetComponent<CollisionComponent>(ComponentType::CollisionComponent);
    auto otherCollision = other.GetComponent<CollisionComponent>(ComponentType::CollisionComponent);

    if (!thisCollision || !otherCollision) {
        return false;
    }

    if (thisCollision->GetShapeType() == ShapeType::Circle && otherCollision->GetShapeType() == ShapeType::Circle) {
        // Circle vs Circle collision
        sf::Vector2f thisCenter = thisCollision->GetCircleShape().getPosition();
        float thisRadius = thisCollision->GetCircleShape().getRadius();

        sf::Vector2f otherCenter = otherCollision->GetCircleShape().getPosition();
        float otherRadius = otherCollision->GetCircleShape().getRadius();

        float distanceX = thisCenter.x - otherCenter.x;
        float distanceY = thisCenter.y - otherCenter.y;
        float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

        float radiusSum = thisRadius + otherRadius;
        return distanceSquared < (radiusSum * radiusSum);
    }

    // Handle other collision types if needed

    return false;
}