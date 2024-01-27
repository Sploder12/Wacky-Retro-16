
#include <sndx/util/window.hpp>

#include <sndx/render/vbo.hpp>

#include <thread>

#include "render/render.hpp"

#include "resource/resourcestate.hpp"

ResourceState resources{};

sndx::Window window;

inline float bounceInEasing(float x) {
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


	double start = glfwGetTime();

	window = sndx::createWindow(1, 1, "Wacky Retro 16", float(mWidth) / float(mHeight));
	glfwMakeContextCurrent(window);

	glfwSetWindowPos(window, (mWidth / 2) - (window.dims.x / 2), (mHeight / 2) - (window.dims.y / 2));

	glewInit();

	initRenderer();

	loadShaderFolder(std::filesystem::directory_iterator("resources/shaders/"), resources.shaders);

	loadTextureResource("resources/sprites.json", resources.textures);

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

	glfwSetWindowSize(window, mWidth * 0.75f, mHeight * 0.75f);


	while (!glfwWindowShouldClose(window)) {

		glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	

	return 0;
}