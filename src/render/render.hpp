
#include <array>

#include <glm/glm.hpp>

#include <sndx/render/vbo.hpp>
#include <sndx/render/vao.hpp>

#include "../gamestate.hpp"

#include "../resource/resourcestate.hpp"

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

inline unsigned int gameFBO;
inline sndx::Texture fboTexture;

inline void initRenderer() {
    quadVBO.setData(quadVerts);

    spriteVAO.bindVBO(quadVBO);
    spriteVAO.bindVBO(spriteVBO, 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glGenFramebuffers(1, &gameFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gameFBO);

    fboTexture = sndx::Texture(800, 600, GL_RGBA, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture.id, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline void render(Gamestate& state, ResourceState& resources, float time) {
    resources.shaders.setTime(time);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (state.state == State::Game) {
        glBindFramebuffer(GL_FRAMEBUFFER, gameFBO);

        glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        resources.shaders.getShader("sprite").use();

        resources.textures.getTexture("obstacle.PizzaEater").bind();

        std::vector<SpriteData> obstacles{};
        obstacles.reserve(state.track.size());

        for (size_t x = 0; x < state.track.size(); ++x) {

            const auto& ob = state.track[x];

            auto delta = float(x) - state.pos.x;

            obstacles.emplace_back(
                glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f + delta * 0.1f, -1.0f + 0.05f, state.pos.z)), glm::vec3(0.1f, 0.2f, 0.0)),
                resources.textures.getTexcoords(std::string("obstacle.") + ObstacleToName(ob)),
                glm::vec4(1.0f)
            );
            
        }

        if (!obstacles.empty()) [[likely]] {
            spriteVBO.setData(obstacles);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, obstacles.size());
        }


        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        resources.textures.getTexture("truck.NinjaFNAFsTheWhoa").bind();

        SpriteData data{
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, state.pos.y - 1.0f + 0.2f, state.pos.z)), glm::vec3(0.2f)),
            resources.textures.getTexcoords(std::string("truck.") + (state.animFrame % 2 == 0 ? "0" : "1")),
            glm::vec4(1.0f)
        };

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);


        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        auto& post = resources.shaders.getShader("post");
        post.use();

        post.uniform("Edge", state.edging);
        post.uniform("Blur", state.blurring);
        post.uniform("Posterize", state.posterize);
        post.uniform("Warp", state.warping);
        post.uniform("MegaWarp", state.megaWarping);
        post.uniform("Shake", state.shake);
        post.uniform("Invert", state.invert);

        fboTexture.bind();

        data = {
            glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)),
            glm::vec4(0.0, 0.0, 1.0, 1.0),
            glm::vec4(1.0f)
        };

        if (state.rotate) {
            data.transform = glm::rotate<float>(data.transform, glm::radians(sin(time * 0.1) * 180), glm::vec3(1.0, 1.0, 0.0));
        }

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
    else {
        state.state = State::Game;
    }
}