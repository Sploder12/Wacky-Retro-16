#include "texturemanager.hpp"

#include <sndx/util/stringmanip.hpp>
#include <sndx/data.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define __STDC_LIB_EXT1__
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include <future>

sndx::Texture missingTexture;

void loadMissingTexture() {
	sndx::ImageData img;
	img.channels = 3;
	img.height = 64;
	img.width = 64;

	img.data.resize(img.channels * img.width * img.height);

	bool on = false;
	for (size_t height = 0; height < img.height; ++height) {
		if (height % (img.height / 8) == 0) {
			on = !on;
		}

		for (size_t width = 0; width < img.width; ++width) {
			if (width % (img.width / 8) == 0) {
				on = !on;
			}

			if (on) {
				size_t index = height * img.width * img.channels + width * img.channels;
				img.data[index] = 255u;
				img.data[index + 2] = 255u;
			}
		}
	}

	missingTexture = sndx::textureFromImage(std::move(img), GL_RGB, GL_NEAREST);
}

[[nodiscard]]
const sndx::Texture& getMissingTexture() {
	if (missingTexture.id == 0) [[unlikely]] {
		loadMissingTexture();
	}
	
	return missingTexture;
}

[[nodiscard]]
const sndx::Texture& TextureManager::getTexture(std::string_view id) const {
	auto [baseId, subId] = sndx::splitFirst(id, '.');

	if (subId.empty()) {
		if (auto it = textures.find(std::string(baseId)); it != textures.end()) {
			return it->second;
		}
		
		return getMissingTexture();
	}

	if (auto it = atlases.find(std::string(baseId)); it != atlases.end()) {
		return it->second.tex;
	}

	return getMissingTexture();
}

[[nodiscard]]
glm::vec4 TextureManager::getTexcoords(std::string_view id) const {
	auto [baseId, subId] = sndx::splitFirst(id, '.');

	if (subId.empty()) {
		return glm::vec4(0.0, 0.0, 1.0, 1.0);
	}

	if (auto ait = atlases.find(std::string(baseId)); ait != atlases.end()) {
		if (auto it = ait->second.find(std::string(subId)); it != ait->second.entries.end()) {
			return glm::vec4(it->second.first, it->second.second);
		}
	}

	return glm::vec4(0.0, 0.0, 1.0, 1.0);
}

void TextureManager::unloadTexture(const std::string& id) {
	if (auto it = textures.find(id); it != textures.end()) {
		it->second.destroy();
		textures.erase(it);
	}
}

void TextureManager::unloadAtlas(const std::string& id) {
	if (auto it = atlases.find(id); it != atlases.end()) {
		it->second.destroy();
		atlases.erase(it);
	}
}

void TextureManager::addTexture(const std::string& id, const sndx::Texture& texture) {
	textures.insert({ id, texture });
}

void TextureManager::addAtlas(const std::string& id, const sndx::Atlas<std::string>& atlas) {
	atlases.insert({ id, atlas });
}

bool TextureManager::loadTexture(const std::string& id, const char* file) {
	auto tex = sndx::textureFromFile(file);
	if (!tex.has_value()) {
		return false;
	}

	textures.insert({ id, std::move(*tex) });
	return true;
}

std::optional<sndx::Atlas<std::string>> loadAtlas(const sndx::DataDict& data) {
	sndx::AtlasBuilder builder{};

	for (const auto& [id, obj] : data) {
		if (obj.holdsAlternative<sndx::Primitive>()) {
			const auto& prim = *obj.get<sndx::Primitive>();
			
			if (std::holds_alternative<std::string>(prim.data)) {
				auto img = sndx::imageFromFile(("resources/textures/" + std::get<std::string>(prim.data)).c_str(), 4, false);

				if (img.has_value()) {
					builder.insert(id, std::move(*img));
				}
			}
		}
	}

	if (builder.entries.empty()) {
		return std::nullopt;
	}

	return builder.buildAtlas();
}

bool loadTextureResource(const std::filesystem::path& path, TextureManager& manager) {
	auto data = sndx::decodeData<sndx::JSONdecoder>(path);
	if (!data.has_value()) {
		return false;
	}

	if (!data->holdsAlternative<sndx::DataDict>()) {
		return false;
	}

	const sndx::DataDict& dict = std::get<sndx::DataDict>(data->data);

	// LOL stbi isn't thread safe!
	std::vector<std::pair<std::string, std::optional<sndx::Atlas<std::string>>>> atlases{};

	std::vector<std::pair<std::string, std::string>> texturesToLoad{};

	for (const auto& [id, obj] : dict) {
		if (obj.holdsAlternative<sndx::DataDict>()) {
			atlases.emplace_back(id, loadAtlas(std::get<sndx::DataDict>(obj.data)));
		}
		else if (obj.holdsAlternative<sndx::Primitive>()) {
			const auto& prim = *obj.get<sndx::Primitive>();

			if (std::holds_alternative<std::string>(prim.data)) {
				texturesToLoad.emplace_back(id, "resources/textures/" + std::get<std::string>(prim.data));
			}
		}
	}

	for (auto& [id, file] : texturesToLoad) {
		auto img = sndx::imageFromFile(file.c_str());

		if (img.has_value()) {
			manager.addTexture(std::move(id), sndx::textureFromImage(std::move(*img), GL_RGBA, GL_NEAREST));
		}
	}

	for (auto& [id, atlas] : atlases) {
		if (atlas.has_value()) {
			manager.addAtlas(id, std::move(*atlas));
		}
	}

	return true;
}