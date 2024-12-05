#pragma once
#include <SFML/Graphics.hpp>
#include "ComponentType.hpp"
#include <memory>

class GameEntity;

class Component
{
public:
	Component(std::shared_ptr<GameEntity> gameEntity)
		: mGameEntity(gameEntity) {}
	virtual ~Component() = default;

	virtual void Input() {}
	virtual void Update() {}
	virtual void Render(sf::RenderWindow& ref) {}

	virtual ComponentType GetType() = 0;

	//void SetGameEntity(std::shared_ptr<GameEntity> entity) {
	//	mGameEntity = entity;
	//}

protected:
	std::weak_ptr<GameEntity> mGameEntity;

	// Utility function to get a shared_ptr to the GameEntity
	std::shared_ptr<GameEntity> GetGameEntity() const {
		return mGameEntity.lock();
	}
};

