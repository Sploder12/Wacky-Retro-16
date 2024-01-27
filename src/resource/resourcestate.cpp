#include "resourcestate.hpp"

#include <future>

void loadShaders(ResourceState* state) {
    loadShaderFolder(std::filesystem::directory_iterator("resources/shaders/"), state->shaders);
}

void loadTextures(ResourceState* state) {
    loadTextureResource("resources/textures/sprites.json", state->textures);
}

void init(ResourceState& state) {
    state.clear();

    // textures are slow but there is nothing we can do about it,
    // thanks stbi
    loadTextures(&state);

    // shaders are quick
    loadShaders(&state);

}