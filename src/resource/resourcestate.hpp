#pragma once

#include "shadermanager.hpp"
#include "texturemanager.hpp"

struct ResourceState {
	ShaderManager shaders;
	TextureManager textures;

	void clear() {
		shaders.clear();
		textures.clear();
	}
};

// this MUST be done on the render thread. Thanks OpenGL
void init(ResourceState& state);