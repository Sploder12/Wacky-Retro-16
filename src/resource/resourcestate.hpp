#pragma once

#include "shadermanager.hpp"
#include "texturemanager.hpp"
#include "audiomanager.hpp"

struct ResourceState {
	ShaderManager shaders{};
	TextureManager textures{};
	AudioManager sounds{};

	void clear() {
		shaders.clear();
		textures.clear();
		sounds.clear();
	}
};

// this MUST be done on the render thread. Thanks OpenGL
void init(ResourceState& state);