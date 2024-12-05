#pragma once
#include <SFML/Graphics.hpp>
#include "Component.hpp"
#include "ShapeType.hpp"

class TransformComponent : public Component

{
public:

	TransformComponent(std::shared_ptr<GameEntity> gameEntity, ShapeType shapeType)
		: Component(gameEntity), mShapeType(shapeType), mCircle(1), mRectangle(sf::Vector2f(1, 1)) {}

	~TransformComponent() override {}
	void Input() override {}
	void Update() override {}
	void Render(sf::RenderWindow& ref) override {

	}

	ComponentType GetType() override {
		return ComponentType::TransformComponent;
	}

	void SetPosition(float x, float y) {
		mPosition = sf::Vector2f(x, y);
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
		return mRectangle.getSize();
	}


	void SetScale(float x, float y) {
		mScale = sf::Vector2f(x, y);
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
		return mCircle.getRadius();
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

	void SetOrigin(float x, float y) {
		if (mShapeType == ShapeType::Circle) {
			mCircle.setOrigin(x, y);
		}
		if (mShapeType == ShapeType::Rectangle) {
			mRectangle.setOrigin(x, y);
		}
	}

	sf::Vector2f GetOrigin() const {
		if (mShapeType == ShapeType::Circle) {
			return mCircle.getOrigin();
		}
		if (mShapeType == ShapeType::Rectangle) {
			return mRectangle.getOrigin();
		}
	}

private:

	sf::Vector2f mPosition{ 0,0 };
	sf::Vector2f mScale {1, 1};
	sf::CircleShape mCircle;
	sf::RectangleShape mRectangle;
	ShapeType mShapeType;

};

