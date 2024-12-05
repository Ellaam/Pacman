#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include<string>
#include<iostream>

#include "Component.hpp"
#include "ResourceManager.hpp"
#include "GameEntity.hpp"
#include "TransformComponent.hpp"

class SpriteComponent : public Component
{
public:

	SpriteComponent(std::shared_ptr<GameEntity> gameEntity) : Component(gameEntity) {}
	~SpriteComponent() override = default;

	void CreateSpriteComponent(const std::string& fileName) {
		std::shared_ptr<sf::Texture> texture = ResourceManager::GetInstance().Acquire(fileName);
		if (texture) {
			mSpriteComponent.setTexture(*texture);
			std::cout << "The image was loaded successfully from: " << fileName << std::endl;
		}
		else {
			std::cerr << "Failed to load texture from: " << fileName << std::endl;
		}
	}

	void UpdateSpriteComponent(std::string fileName) {
		CreateSpriteComponent(fileName);
	}

	const sf::Sprite& GetSpriteComponent() const {
		return mSpriteComponent;
	}

	ComponentType GetType() override {
		return ComponentType::SpriteComponent;
	}

	void Update() override {}
	void Input() override {}
	void Render(sf::RenderWindow& ref) override {

		if (auto entity = GetGameEntity()) {
			auto transform = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent);
			if (transform) {
				mSpriteComponent.setPosition(transform->GetPosition());
				mSpriteComponent.setScale(transform->GetScale());
				ref.draw(mSpriteComponent);
			}
			else {
				std::cerr << "Warning: TransformComponent not found for GameEntity in SpriteComponent." << std::endl;
			}
		}
		else {
			std::cerr << "Error: GameEntity is not set for SpriteComponent." << std::endl;
		}
	}

private:

	sf::Sprite mSpriteComponent;
};

