#pragma once
#include <SFML/Graphics.hpp>
#include "GameEntity.hpp"
#include "ComponentType.hpp"
#include "SpriteComponent.hpp"
#include "CollisionComponent.hpp"
#include "TransformComponent.hpp"
#include "TileType.hpp"
#include <vector>
#include <memory>
#include <iostream>

class Map {
public:
    Map(int tileSize, int width, int height)
        : mTileSize(tileSize), mWidth(width), mHeight(height) {
        mEntities.resize(mHeight, std::vector<std::shared_ptr<GameEntity>>(mWidth, nullptr));
    }

    void Load(const std::vector<std::vector<int>>& mapData) {
        mMapData = mapData;
        for (int y = 0; y < mapData.size(); ++y) {
            for (int x = 0; x < mapData[y].size(); ++x) {
                auto tile = CreateTileEntity(mapData[y][x], x, y);
                if (tile) {
                    mEntities[y][x] = tile;
                }
            }
        }
    }

    void Render(sf::RenderWindow& window) {
        for (const auto& row : mEntities) {
            for (const auto& entity : row) {
                if (entity && entity->GetTileType() != TileType::Empty) {
                    entity->Render(window);
                }
            }
        }
    }

    void Update() {
        for (const auto& row : mEntities) {
            for (const auto& entity : row) {
                if (entity) entity->Update();
            }
        }
    }

    std::shared_ptr<GameEntity> GetTileEntity(int x, int y) const {
        if (x >= 0 && x < mWidth && y >= 0 && y < mHeight) {
            return mEntities[y][x];
        }
        return nullptr;
    }

    int GetTileTypeAt(int x, int y) const {
        if (y >= 0 && y < mMapData.size() && x >= 0 && x < mMapData[y].size()) {
            return mMapData[y][x];
        }
        return -1;
    }

    void SetTileTypeAt(int x, int y, TileType type) const {
        if (x >= 0 && x < mWidth && y >= 0 && y < mHeight) {
            // Update the map data at (x, y)
            const_cast<Map*>(this)->mMapData[y][x] = static_cast<int>(type);

            // Update the entity's tile type if an entity exists at (x, y)
            if (mEntities[y][x]) {
                mEntities[y][x]->SetTileType(type);
            }
        }
    }

    int mTileSize;
    int mWidth;
    int mHeight;

private:
    std::vector<std::vector<int>> mMapData;
    std::vector<std::vector<std::shared_ptr<GameEntity>>> mEntities;

    std::shared_ptr<GameEntity> CreateTileEntity(int tileType, int x, int y) {
        auto tile = GameEntity::Create();
        ShapeType shapeType = (tileType == 1) ? ShapeType::Rectangle : ShapeType::Circle;
        auto transform = std::make_shared<TransformComponent>(tile, shapeType);
        auto sprite = std::make_shared<SpriteComponent>(tile);
        auto collision = std::make_shared<CollisionComponent>(tile, shapeType);

        switch (tileType) {
        case 1: {  // Wall tile
            tile->SetTileType(TileType::Wall);
            sprite->CreateSpriteComponent("../assets/t.png");
            transform->SetRectangleSize(static_cast<float>(mTileSize), static_cast<float>(mTileSize));
            collision->SetRectangleSize(static_cast<float>(mTileSize), static_cast<float>(mTileSize));
            transform->SetPosition(static_cast<float>(x * mTileSize), static_cast<float>(y * mTileSize));
            break;
        }
        case 2: {  // Regular Pellet (small circle)
            tile->SetTileType(TileType::Pellet);
            sprite->CreateSpriteComponent("../assets/d.png");
            int pelletSize = mTileSize / 4;
            transform->SetCircleRadius(static_cast<float>(pelletSize) / 2.0f);
            collision->SetCircleRadius(static_cast<float>(pelletSize) / 2.0f);
            transform->SetPosition(static_cast<float>(x * mTileSize + (mTileSize - pelletSize) / 2),
                static_cast<float>(y * mTileSize + (mTileSize - pelletSize) / 2));
            transform->SetScale(static_cast<float>(pelletSize) / sprite->GetSpriteComponent().getTexture()->getSize().x,
                static_cast<float>(pelletSize) / sprite->GetSpriteComponent().getTexture()->getSize().y);
            break;
        }
        case 3: {  // Power Pellet (larger circle)
            tile->SetTileType(TileType::PowerPellet);
            sprite->CreateSpriteComponent("../assets/d.png");
            int pelletSize = mTileSize / 2;
            transform->SetCircleRadius(static_cast<float>(pelletSize) / 2.0f);
            collision->SetCircleRadius(static_cast<float>(pelletSize) / 2.0f);
            transform->SetPosition(static_cast<float>(x * mTileSize + (mTileSize - pelletSize) / 2),
                static_cast<float>(y * mTileSize + (mTileSize - pelletSize) / 2));
            transform->SetScale(static_cast<float>(pelletSize) / sprite->GetSpriteComponent().getTexture()->getSize().x,
                static_cast<float>(pelletSize) / sprite->GetSpriteComponent().getTexture()->getSize().y);
            break;
        }
        case 4: {  // door tile
            tile->SetTileType(TileType::Wall);
            sprite->CreateSpriteComponent("../assets/door.png");
            transform->SetRectangleSize(static_cast<float>(mTileSize), static_cast<float>(mTileSize));
            collision->SetRectangleSize(static_cast<float>(mTileSize), static_cast<float>(mTileSize));
            transform->SetPosition(static_cast<float>(x * mTileSize), static_cast<float>(y * mTileSize));
            break;
        }
        default:
            return nullptr;
        }

        tile->AddComponent(transform);
        tile->AddComponent(sprite);
        if (collision) tile->AddComponent(collision);

        return tile;
    }
};
