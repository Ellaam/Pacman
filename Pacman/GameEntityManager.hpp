#pragma once
//#include <SFML/Graphics.hpp>
#include "GameEntity.hpp"
#include "TransformComponent.hpp"
#include "InputComponent.hpp"
#include "Direction.hpp"
#include "Map.hpp"
#include "AIComponent.hpp"
#include "GhostType.hpp"

class GameEntityManager
{
public:
    GameEntityManager(Map& gameMap) : map(gameMap) {}

    void AddGhost(std::shared_ptr<GameEntity> ghost) {
        mGhosts.push_back(ghost);
    }

    void SetPacman(std::shared_ptr<GameEntity> pacmanEntity) {
        mPacman = pacmanEntity;
    }

    sf::Vector2f GetGhostPosition(GhostType type) const {
        for (const auto& ghost : mGhosts) {
            if (ghost) {
                auto aiComponent = ghost->GetComponent<AIComponent>(ComponentType::AIComponent);
                if (aiComponent && aiComponent->GetGhostType() == type) {
                    auto tc = ghost->GetComponent<TransformComponent>(ComponentType::TransformComponent);
                    return tc->GetPosition();
                }
            }
        }
        return sf::Vector2f(0, 0);
    }

    sf::Vector2f GetPacmanPosition() const {
        if (mPacman) {
            auto tc = mPacman->GetComponent<TransformComponent>(ComponentType::TransformComponent);
            return tc->GetPosition();
        }
        else {
            std::cout << "Posistion: Pacman is null." << std::endl;
        }
        return sf::Vector2f(0, 0);  // Add a default return value
    }

    Direction GetPacmanDirection() const {
        if (mPacman) {
            auto ic = mPacman->GetComponent<InputComponent>(ComponentType::InputComponent);
            return ic->GetCurrentDirection();
        }
        else {
            std::cout << "Pacman is null." << std::endl;
        }
        return Direction::Right;  // Add a default return value
    }


private:
    std::vector<std::shared_ptr<GameEntity>> mGhosts;
    std::shared_ptr<GameEntity> mPacman;
    Map& map;
};

