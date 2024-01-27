#pragma once

#include <string>
#include <unordered_map>

#include <sndx/render/shader.hpp>

#include <sndx/util/resourcemanager.hpp>

class ShaderManager : public sndx::ResourceManager<sndx::ShaderProgram> {
public:
	void addShader(const std::string& id, sndx::ShaderProgram&& program) {
		this->insert({id, std::move(program)});
	}

	void setTime(float time) {
		for (const auto& [id, shader] : *this) {
			shader.use();
			shader.uniform("time", time);
		}
	}

	const sndx::ShaderProgram& getShader(const std::string& id) const {
		return this->at(id);
	}
};

void loadShaderFolder(const std::filesystem::directory_iterator& folder, ShaderManager& manager);