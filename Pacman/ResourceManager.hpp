#pragma once
#include <SFML/Graphics.hpp>

#include<memory>
#include<unordered_map>
#include<string>


//This is a singleton

class ResourceManager
{
public:
	// Accessor for singleton instance
	static ResourceManager& GetInstance() {
		static ResourceManager instance; // Thread-safe and automatically cleaned up
		return instance;
	}

	// Acquire method to load and retrieve textures
	std::shared_ptr<sf::Texture> Acquire(const std::string& filepath) {
		// Check if texture is already loaded
		auto it = mTextureResources.find(filepath);
		if (it != mTextureResources.end()) {
			return it->second;
		}

		// Load and store the texture if not found
		auto texture = std::make_shared<sf::Texture>();
		if (texture->loadFromFile(filepath)) {
			mTextureResources[filepath] = texture;
		}
		return texture;
	}


private:

	ResourceManager() {}
	~ResourceManager() {}

	std::unordered_map<std::string, std::shared_ptr<sf::Texture>> mTextureResources;
};

