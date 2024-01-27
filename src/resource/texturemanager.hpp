#pragma once

#include <string>
#include <unordered_map>
#include <filesystem>

#include <sndx/render/atlas.hpp>
#include <sndx/render/texture.hpp>

#include <sndx/util/atlasbuild.hpp>
#include <sndx/util/resourcemanager.hpp>

class TextureManager {
protected:
	sndx::ResourceManager<sndx::Atlas<std::string>> atlases;
	sndx::ResourceManager<sndx::Texture> textures;

public:
	void clear() {
		atlases.clear();
		textures.clear();
	}

public:
	[[nodiscard]]
	const sndx::Texture& getTexture(std::string_view id) const;

	[[nodiscard]]
	glm::vec4 getTexcoords(std::string_view id) const;

	void unloadTexture(const std::string& id);
	void unloadAtlas(const std::string& id);

	void addTexture(const std::string& id, const sndx::Texture& texture);
	void addAtlas(const std::string& id, const sndx::Atlas<std::string>& atlas);

	bool loadTexture(const std::string& id, const char* file);

	template <class... Ts>
	bool loadAtlas(const std::string& id, std::pair<std::string, Ts>... files) {
		static_assert(sizeof...(files) > 0);

		sndx::AtlasBuilder builder{};

		for (const auto& [subId, path] : { files... }) {
			
			std::optional<sndx::ImageData> img;
			if constexpr (std::is_same_v<std::decay_t<decltype(path)>, char*>) {
				img = sndx::imageFromFile(path);
			}
			else {
				img = sndx::imageFromFile(path.c_str());
			}

			if (!img.has_value()) {
				return false;
			}

			builder.insert(subId, std::move(*img));
		}

		addAtlas(id, builder.buildAtlas());
		return true;
	}
};

bool loadTextureResource(const std::filesystem::path& path, TextureManager& manager);