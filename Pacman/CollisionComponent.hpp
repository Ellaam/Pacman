#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <iostream>
#include "GameEntity.hpp"
#include "Component.hpp"
#include "ShapeType.hpp"
#include "TransformComponent.hpp"


class CollisionComponent : public Component
{
public:
	CollisionComponent(std::shared_ptr<GameEntity> gameEntity, ShapeType shapeType)
		: Component(gameEntity), mShapeType(shapeType), mCircle(1), mRectangle(sf::Vector2f(1, 1)) {

	}
	~CollisionComponent() override = default;

	void Input() override {}
	void Update() override {
		auto entity = GetGameEntity();
		if (!entity) {
			std::cerr << "Error: GameEntity is not set." << std::endl;
			return;
		}

		else {
			if (auto transform = entity->GetComponent<TransformComponent>(ComponentType::TransformComponent)) {
				if (mShapeType == ShapeType::Circle) {
					mCircle.setPosition(transform->GetPosition());
				}
				else {
					mRectangle.setPosition(transform->GetPosition());
				}
			}
		}
	}



	void Render(sf::RenderWindow& ref) override {
		//if (mShapeType == ShapeType::Circle) {
		//	sf::Color outlineColor(255, 0, 0, 128); // Semi-transparent red
		//	mCircle.setFillColor(sf::Color::Transparent);
		//	mCircle.setOutlineColor(outlineColor);
		//	mCircle.setOutlineThickness(1);
		//	ref.draw(mCircle);
		//}
		//else if (mShapeType == ShapeType::Rectangle) {
		//	sf::Color outlineColor(255, 0, 0, 128);
		//	mRectangle.setFillColor(sf::Color::Transparent);
		//	mRectangle.setOutlineColor(outlineColor);
		//	mRectangle.setOutlineThickness(1);
		//	ref.draw(mRectangle);
		//}
	}

	ComponentType GetType() override {
		return ComponentType::CollisionComponent;

	}

	// Set position on both shapes
	void SetPosition(const sf::Vector2f& position) {
		mPosition = position;
		if (mShapeType == ShapeType::Circle) {
			mCircle.setPosition(mPosition);
		}
		else if (mShapeType == ShapeType::Rectangle) {
			mRectangle.setPosition(mPosition);
		}
	}

	sf::Vector2f GetPosition() const {
		return mPosition;
	}

	// Rectangle-specific methods
	void SetRectangleSize(float width, float height) {
		if (mShapeType == ShapeType::Rectangle) {
			mRectangle.setSize(sf::Vector2f(width, height));
		}
	}

	sf::Vector2f GetRectangleSize() const {
		return (mShapeType == ShapeType::Rectangle) ? mRectangle.getSize() : sf::Vector2f(0, 0);
	}

	// Set scale on both shapes
	void SetScale(const sf::Vector2f& scale) {
		mScale = scale;
		if (mShapeType == ShapeType::Circle) {
			mCircle.setScale(mScale);
		}
		else if (mShapeType == ShapeType::Rectangle) {
			mRectangle.setScale(mScale);
		}
	}

	sf::Vector2f GetScale() const {
		return mScale;
	}

	void SetCircleRadius(float radius) {
		if (mShapeType == ShapeType::Circle) {
			mCircle.setRadius(radius);
		}
	}

	float GetCircleRadius() const {
		return (mShapeType == ShapeType::Circle) ? mCircle.getRadius() : 0.0f;
	}

	ShapeType GetShapeType() {
		return mShapeType;
	}

	// Get the circle shape
	sf::CircleShape& GetCircleShape() {
		return mCircle;
	}

	// Get the rectangle shape
	sf::RectangleShape& GetRectangleShape() {
		return mRectangle;
	}



private:

	sf::Vector2f mPosition{ 0,0 };
	sf::Vector2f mScale {1, 1};
	sf::CircleShape mCircle;
	sf::RectangleShape mRectangle;
	ShapeType mShapeType;
};

