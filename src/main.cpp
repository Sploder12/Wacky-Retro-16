
#include <sndx/util/window.hpp>

#include <sndx/render/vbo.hpp>

#include <thread>

#include "render/render.hpp"

#include "resource/resourcestate.hpp"

ResourceState resources{};

sndx::Window window;

Gamestate state;

double mx = 0;
double my = 0;

double pmx = 0;
double pmy = 0;

bool firstMouse = true;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
	pmx = mx;
	pmy = my;

	mx = xpos;
	my = ypos;

	if (firstMouse) {
		pmx = mx;
		pmy = my;
	}

	// @TODO
}

void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		// @TODO
	}
}

float bounceInEasing(float x) {
	float n1 = 7.5625f;
	float d1 = 2.75f;

	if (x < 1.0f / d1) {
		return n1 * x * x;
	}
	else if (x < 2.0f / d1) {
		return n1 * (x -= 1.5f / d1) * x + 0.75f;
	}
	else if (x < 2.5f / d1) {
		return n1 * (x -= 2.25f / d1) * x + 0.9375f;
	}

	return n1 * (x -= 2.625f / d1) * x + 0.984375f;
}

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	int mpX, mpY;
	int mWidth, mHeight;

	glfwGetMonitorWorkarea(monitor, &mpX, &mpY, &mWidth, &mHeight);


	

	window = sndx::createWindow(1, 1, "Wacky Retro 16", float(mWidth) / float(mHeight));
	glfwMakeContextCurrent(window);

	glfwSetWindowPos(window, int((mWidth / 2) - (window.dims.x / 2)), int((mHeight / 2) - (window.dims.y / 2)));

	glewInit();

	initRenderer();

	loadShaderFolder(std::filesystem::directory_iterator("resources/shaders/"), resources.shaders);

	resources.textures.loadTexture("intro", "resources/textures/intro.png");

	bool intro = true;

	if (intro) {
		for (float percent = 0.01f; percent <= 1.0f; percent += 0.01f) {
			auto x = bounceInEasing(percent);


			auto width = x * mWidth;
			auto height = x * mHeight;


			glfwSetWindowPos(window, (mWidth / 2) - (width * 0.75f / 2), (mHeight / 2) - (height * 0.75f / 2));
			glfwSetWindowSize(window, width * 0.75f, height * 0.75f);

			glViewport(0, 0, width * 0.75f, height * 0.75f);

			spriteVAO.bind();

			const auto& shader = resources.shaders.getShader("sprite");
			shader.use();

			const auto& intro = resources.textures.getTexture("intro");
			intro.bind();

			SpriteData data{
				glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)),
				resources.textures.getTexcoords("intro"),
				glm::vec4(1.0f)
			};

			spriteVBO.setData(std::array<SpriteData, 1>{ data });

			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

			glfwSwapBuffers(window);

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	glfwSetWindowPos(window, (mWidth / 2) - (mWidth * 0.75f / 2), (mHeight / 2) - (mHeight * 0.75f / 2));
	glfwSetWindowSize(window, mWidth * 0.75f, mHeight * 0.75f);
	glViewport(0, 0, mWidth * 0.75f, mHeight * 0.75f);

	window.dims = glm::vec2(mWidth, mHeight);

	double start = glfwGetTime();
	
	// LOAD RESOURCES HERE
	loadTextureResource("resources/sprites.json", resources.textures);
	loadAudioResource("resources/sounds.json", resources.sounds);

	generateTrack(state.track, 64);
	// PLEASE

	if (intro) {
		while (glfwGetTime() - start < 1.0 && !glfwWindowShouldClose(window)) { // we do a little loading screen
			const auto& intro = resources.textures.getTexture("intro");
			intro.bind();

			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		for (float percent = 1.0f; percent >= 0.0f && !glfwWindowShouldClose(window); percent -= 0.005f) {

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			SpriteData data{
				glm::scale(glm::mat4(1.0f), glm::vec3(2.0f)),
				resources.textures.getTexcoords("intro"),
				glm::vec4(percent)
			};

			spriteVBO.setData(std::array<SpriteData, 1>{ data });

			glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

			glfwSwapBuffers(window);
			glfwPollEvents();

			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}

		start = glfwGetTime();
		while (glfwGetTime() - start < 1.0 && !glfwWindowShouldClose(window)) {
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}

	// setup callbacks

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_callback);

	resources.sounds.setBGM("bgm");
	resources.sounds.setSpeed(1.0);

	double prev = glfwGetTime();

	glfwSwapInterval(1);

	double skipTimer = prev;

	while (!glfwWindowShouldClose(window)) {

		double now = glfwGetTime();

		double dt = std::min(now - prev, 0.1);
		prev = now;

		if (now - skipTimer >= 0.2) {
			resources.sounds.bgmSkip(-(now - skipTimer) * 0.99);
			skipTimer = now;
		}

		state.update(dt);
		render(state, resources, now);

		glfwSwapBuffers(window);
		glfwPollEvents();

		glFinish();
	}
	

	return 0;
}