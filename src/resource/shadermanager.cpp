#include "shadermanager.hpp"

#include <unordered_map>

void loadShaderFolder(const std::filesystem::directory_iterator& folder, ShaderManager& manager) {

	std::unordered_map<std::string, std::vector<std::string>> components{};

	for (const auto& entry : folder) {
		if (entry.is_directory()) {
			loadShaderFolder(std::filesystem::directory_iterator(entry.path()), manager);
		}
		else if (entry.is_regular_file()) {
			const auto& path = entry.path();
			
			if (auto it = components.find(path.stem().string()); it != components.end()) {
				it->second.emplace_back(path.string());
			}
			else {
				components[path.stem().string()] = { path.string() };
			}
		}
	}

	for (const auto& [id, filenames] : components) {
		auto shader = sndx::programFromFiles(filenames);

		if (shader.has_value()) {
			manager.addShader(id, std::move(*shader));
		}
	}
}