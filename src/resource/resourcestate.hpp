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

		sounds.bgm.stop();
		sounds.bgm.destroy();

		sounds.carNoises.stop();
		sounds.carNoises.destroy();

		sounds.clear();

		sounds.context.buffers.clear();
		sounds.context.sources.clear();
	}
};
