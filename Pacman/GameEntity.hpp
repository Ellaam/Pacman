#pragma once

#include <SFML/Graphics.hpp>

#include "ComponentType.hpp"
#include "ShapeType.hpp"
#include "Component.hpp"
#include "TileType.hpp"
#include "GhostMode.hpp"
#include "GhostType.hpp"
#include<iostream>
#include<memory>
#include <map>

//Forward Declarions to avoid circular dependencies;
//class Component;
class TransformComponent;
class CollisionComponent;

class GameEntity : public std::enable_shared_from_this<GameEntity>
{
public:

	virtual ~GameEntity() = default;


	static std::shared_ptr<GameEntity> Create() {
		return std::shared_ptr<GameEntity>(new GameEntity());
	}

	virtual void Input() {
		for (auto& [key, value] : mComponents) {
			mComponents[key]->Input();
		}
	}

	virtual void Update() {
		for (auto& [key, value] : mComponents) {
			mComponents[key]->Update();
		}
	}
	virtual void Render(sf::RenderWindow& ref) {
		// Calls the Render method of each component
		for (const auto& component : mComponents) {
			component.second->Render(ref); // Call Render on each component
		}
	}

	template<typename Type>
	void AddComponent(std::shared_ptr<Type> component) {
		mComponents[component->GetType()] = component;
	}


	template<typename Type>
	std::shared_ptr<Type> GetComponent(ComponentType type) {
		auto it = mComponents.find(type);
		if (it != mComponents.end()) {
			return std::dynamic_pointer_cast<Type>(it->second);
		}
		//std::cout << "Error: The compoent you searched for is not found - nullptr is returned" << std::endl;
		return nullptr;
	}

	void SetTileType(TileType type) {
		mTileType = type;
	}

	TileType GetTileType() const {
		return mTileType;
	}

	bool IsActive() const {
		return mActive;
	}

	void SetActive(bool value) {
		mActive = value;
	}


	bool CheckCollision(GameEntity& other);

protected:
	GameEntity() = default;

private:
	std::map <ComponentType, std::shared_ptr<Component>> mComponents;
	bool mActive{ true };
	TileType mTileType{ TileType::Empty };
};

//// Explicit template instantiations
//template void GameEntity::AddComponent<SpriteComponent>(std::shared_ptr<SpriteComponent> C);
//template void GameEntity::AddComponent<TransformComponent>(std::shared_ptr<TransformComponent> C);
//template void GameEntity::AddComponent<CollisionComponent>(std::shared_ptr<CollisionComponent> C);
//template void GameEntity::AddComponent<InputComponent>(std::shared_ptr<InputComponent> C);
//
//template std::shared_ptr<SpriteComponent> GameEntity::GetComponent<SpriteComponent>(ComponentType type);
//template std::shared_ptr<TransformComponent> GameEntity::GetComponent<TransformComponent>(ComponentType type);
//template std::shared_ptr<CollisionComponent> GameEntity::GetComponent<CollisionComponent>(ComponentType type);
//template std::shared_ptr<InputComponent> GameEntity::GetComponent<InputComponent>(ComponentType type);