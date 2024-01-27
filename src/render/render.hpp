
#include <array>

#include <glm/glm.hpp>

#include <sndx/render/vbo.hpp>
#include <sndx/render/vao.hpp>

constexpr std::array<glm::vec2, 6> quadVerts = {
        decltype(quadVerts)::value_type
        { -0.5f, -0.5f },
        { 0.5f, -0.5f },
        { 0.5f, 0.5f },

        { -0.5f, -0.5f },
        { 0.5f, 0.5f },
        { -0.5f, 0.5f }
};

inline sndx::VBO<glm::vec2> quadVBO;

struct SpriteData {
    glm::mat4 transform;
    glm::vec4 texoff;
    glm::vec4 tint;
};

inline sndx::VBO<glm::mat4, glm::vec4, glm::vec4> spriteVBO;

inline sndx::VAO spriteVAO;

inline void initRenderer() {
    quadVBO.setData(quadVerts);

    spriteVAO.bindVBO(quadVBO);
    spriteVAO.bindVBO(spriteVBO, 1);
}