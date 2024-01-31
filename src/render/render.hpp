
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

inline void render(Gamestate& state, ResourceState& resources, float time, int mWidth, int mHeight) {
    resources.shaders.setTime(time);

    glClearColor(0.03f, 0.04f, 0.05f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    resources.shaders.getShader("sprite").use();

    double truckTime = 1.5;

    

    if (state.state == State::Game || state.state == State::GameOver) {
        glBindFramebuffer(GL_FRAMEBUFFER, gameFBO);

        glViewport(0, 0, 800, 600);

        glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        resources.textures.getTexture("bg").bind();

        SpriteData data{
            glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)),
            glm::vec4(state.pos.x * 0.01f, 0.0, 1.0, 1.0),
            glm::vec4(1.0f)
        };

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);


        resources.textures.getTexture("obstacle.PizzaEater").bind();

        std::vector<SpriteData> obstacles{};
        obstacles.reserve(state.track.size());

        for (size_t x = std::ceil(state.pos.x) - 2; x < state.track.size(); ++x) {

            const auto& ob = state.track[x];

            if (ob == Obstacle::Pit) continue;

            auto delta = float(x) - (state.pos.x);

            obstacles.emplace_back(
                glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f + delta * 0.2f, -1.0f + 0.05f, state.pos.z)), glm::vec3(0.2f, 0.2f, 1.0)),
                resources.textures.getTexcoords(std::string("obstacle.") + ObstacleToName(ob)),
                glm::vec4(1.0f)
            );
            
        }

        if (state.image) {
            obstacles.emplace_back(
                glm::rotate(glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5, 0.0, state.pos.z)), glm::vec3(0.5f, 0.5f, 1.0)), glm::radians(time), glm::vec3(0.0, 0.0, 1.0)),
                resources.textures.getTexcoords("obstacle.silly"),
                glm::vec4((cos(time) + 1.0) / 2.0, (sin(time * 3) + 1.0) / 2.0, (sin(time * 1.1) + 1.0) / 2.0, 1.0f)
            );
        }

        if (!obstacles.empty()) [[likely]] {
            spriteVBO.setData(obstacles);

            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, obstacles.size());
        }


        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


        resources.textures.getTexture("truck.NinjaFNAFsTheWhoa").bind();

        data = {
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.8f, state.pos.y - 1.0f + 0.2f, state.pos.z)), glm::vec3(0.2f)),
            resources.textures.getTexcoords(std::string("truck.") + (state.animFrame % 2 == 0 ? "0" : "1")),
            glm::vec4(1.0f)
        };

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);


        //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, mWidth * 0.75f, mHeight * 0.75f);

        auto& post = resources.shaders.getShader("post");
        post.use();

        unsigned int flags = (unsigned int)state.edging | ((unsigned int)state.blurring << 1) |
            ((unsigned int)state.posterize << 2) | ((unsigned int)state.warping << 3) |
            ((unsigned int)state.megaWarping << 4) | ((unsigned int)state.shake << 5) |
            ((unsigned int)state.invert << 6);

        post.uniform("Flags", flags);


        fboTexture.bind();

        data = {
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.225f, 0.3f, 0.0f)), glm::vec3(1.2f, 1.0f, 1.0f)),
            glm::vec4(0.0, 0.0, 1.0, 1.0),
            glm::vec4(1.0f)
        };

        if (state.rotate) {
            data.transform = glm::rotate<float>(data.transform, glm::radians(sin(time * 0.1) * 180), glm::vec3(1.0, 1.0, 0.0));
        }

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
    else if (state.state == State::Credits) {
        glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        resources.textures.getTexture("credits").bind();

        SpriteData data{
            glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)),
            resources.textures.getTexcoords("credits"),
            glm::vec4(1.0f)
        };

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
    else if (state.state == State::Menu) {

        resources.textures.getTexture("intro").bind();

        SpriteData data{
           glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.21f, 0.3f, 0.0f)), glm::vec3(1.2f, 1.0f, 1.0f)),
           glm::vec4(0.0, 0.0, 1.0, 1.0),
           glm::vec4(1.0f)
        };
        
        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

        resources.textures.getTexture("any_key").bind();

        data = {
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.21f, 0.05f, 0.0f)), glm::vec3(0.6f, 0.2f, 1.0f)),
            glm::vec4(0.0, 0.0, 1.0, 1.0),
            glm::vec4((cos(float(state.animFrame) / 7.0) + 1.0) / 4.0 + 0.5)
        };

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }
    
    
    resources.shaders.getShader("sprite").use();
    if (state.state == State::Game || state.state == State::Menu || state.state == State::GameOver) {
        
        resources.textures.getTexture("console.Gaming").bind();
        std::vector<SpriteData> home{};
        home.reserve(4);

        home.emplace_back(
            glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(-0.225, -0.4, 0.0)), glm::vec3(1.5, 2.5, 1.0)),
            resources.textures.getTexcoords("console.tv"),
            glm::vec4(1.0f)
        );

        home.emplace_back(
            glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.8, -0.25, 0.0)), glm::vec3(0.4, 1.5, 1.0)),
            resources.textures.getTexcoords("console.lamp"),
            glm::vec4(1.0f)
        );

        home.emplace_back(
            glm::rotate<float>(glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(0.0, -0.55, 0.0)), glm::vec3(0.5, 0.2, 1.0)), glm::radians(state.rotation), glm::vec3(0.0, 0.0, 1.0)),
            resources.textures.getTexcoords("console.cartridge"),
            glm::vec4(1.0f)
        );

        home.emplace_back(
            glm::scale(glm::translate(glm::mat4(1.0), glm::vec3(-0.1, -0.75, 0.0)), glm::vec3(1.0, 0.4, 1.0)),
            resources.textures.getTexcoords("console.console"),
            glm::vec4(1.0f)
        );


        spriteVBO.setData(home);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, home.size());
    }



    resources.textures.getTexture("death.ItHasNeverBeenMoreOver").bind();

    if (state.state != State::GameOver && state.state != State::Credits) {
        SpriteData data{
            glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.7, -0.165, 0.0)), glm::vec3(0.1f)),
            resources.textures.getTexcoords("death.truck"),
            glm::vec4(1.0f)
        };

        spriteVBO.setData(std::array<SpriteData, 1>{ data });

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
    }

    if (state.state == State::GameOver) {
        
        double progress = std::min((state.timer / truckTime) * (state.timer / truckTime), 1.0);

        resources.sounds.carNoises.setGain(progress);

        std::vector<SpriteData> deathStuff{};

        if (state.timer >= truckTime && state.pos.y < 0.0) {
            state.pos.y = 5.0;
            resources.sounds.carNoises.stop();
        }

        if (state.timer >= truckTime - 0.4 && state.pos.y < -0.2 && state.pos.y > -10.0) {
            state.pos.y = -10.1;

            resources.sounds.sfx.stop();
            resources.sounds.sfx.setBuffer(resources.sounds.at("glass"));
            resources.sounds.sfx.setGain(1.0).setSpeed(1.0).play();
        }

        deathStuff.emplace_back(
            glm::scale(glm::translate(glm::mat4(1.0f), glm::mix(glm::vec3(0.7, -0.165, 0.0), glm::vec3(0.0), 0.0)), glm::mix(glm::vec3(0.1), glm::vec3(2.0f), progress)),
            resources.textures.getTexcoords("death.truck"),
            glm::vec4(1.0f)
        );

        if (state.timer >= truckTime) {
            deathStuff.emplace_back(
                glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)),
                resources.textures.getTexcoords("death.glass"),
                glm::vec4(1.0f)
            );
        }

        if (state.timer >= truckTime + 2.0 && state.pos.y == 5.0) {
            state.pos.y = 6.0;

            resources.sounds.carNoises.setBuffer(resources.sounds.at("oh_no")).setGain(1.0).setParam(AL_LOOPING, AL_FALSE).play();
        }

        if (state.timer >= truckTime + 2.0) {
            deathStuff.emplace_back(
                glm::scale(glm::mat4(1.0f), glm::vec3(2.0)),
                resources.textures.getTexcoords("death.game_over"),
                glm::vec4(1.0f)
            );
        }

        if (state.timer >= truckTime + 5.25) {
            state.reset();
            state.rotation = 0.0;

            state.state = State::Menu;

            resources.sounds.bgm.setGain(0.75f);
            resources.sounds.setBGM("menu");
        }

        spriteVBO.setData(deathStuff);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, deathStuff.size());
    }
}