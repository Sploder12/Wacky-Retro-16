
#include <sndx/util/window.hpp>

#include <sndx/render/vbo.hpp>

#include <sndx/render/imagedata.hpp>

#include <thread>
#include <functional>

#include "render/render.hpp"

#include "resource/resourcestate.hpp"

#define MINIMP3_IMPLEMENTATION
#include <minimp3/minimp3.h>
#include <minimp3/minimp3_ex.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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

		if (state.state == State::Credits) {
			state.state = State::Menu;
		}
		else if (state.state == State::Menu) {
			// NOTHING
		}
		else if (state.state == State::Game) {
			// @TODO
		}
	}
}

bool lrotating = false;
bool rrotating = false;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		if (state.state == State::Credits) {
			state.state = State::Menu;
		}
		else if (state.state == State::Menu) {
			if (key == GLFW_KEY_C) {
				state.state = State::Credits;
			}
			else {
				state.state = State::Game;
				state.start(resources);
			}
		}
		else if (state.state == State::Game) {
			if (key == GLFW_KEY_SPACE) {
				if (!state.hovering) {
					state.velocity.y = 1.0f;

					resources.sounds.carNoises.stop();
					resources.sounds.carNoises.setBuffer(resources.sounds["limiter"]);
					resources.sounds.carNoises.setParam(AL_LOOPING, AL_FALSE).setSpeed(1.0).play();

					state.hovering = true;
				}
			}
			else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
				lrotating = true;
			}
			else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
				rrotating = true;
			}
		}
		else {
			lrotating = false;
			rrotating = false;
		}
	}
	else if (action == GLFW_RELEASE) {
		if (state.state == State::Game) {
			if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) {
				lrotating = false;
			}
			else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) {
				rrotating = false;
			}
		}
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

void framebuffer_size_callback(GLFWwindow* windo, int width, int height) {
	window.resize(glm::ivec2(width, height));
	window.setViewport();
}

std::function<void()> loop;
void main_loop() {
	loop();
}

#ifdef __EMSCRIPTEN__
int main() {
#else
#include <Windows.h>
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#endif

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();

	int mpX, mpY;
	int mWidth, mHeight;

	glfwGetMonitorWorkarea(monitor, &mpX, &mpY, &mWidth, &mHeight);

	window = sndx::createWindow(1, 1, "EXTREME OFF-ROAD HILL CHALLENGE", float(mWidth) / float(mHeight));
	glfwMakeContextCurrent(window);

	glfwSetWindowPos(window, int((mWidth / 2) - (window.dims.x / 2)), int((mHeight / 2) - (window.dims.y / 2)));


	auto icoO = sndx::imageFromFile("resources/textures/truck_frame_0.png", 4, false);
	if (icoO.has_value()) {
		GLFWimage img;
		img.width = icoO->width;
		img.height = icoO->height;
		img.pixels = icoO->data.data();
		icoO->data.clear();


		glfwSetWindowIcon(window, 1, &img);
	}


	glewInit();

	initRenderer();

	loadShaderFolder(std::filesystem::directory_iterator("resources/shaders/"), resources.shaders);

	resources.textures.loadTexture("intro", "resources/textures/intro.png");

	//static constexpr bool intro = true;

	loop = [&]() {
		static float ipercent = 0.01f;

		auto x = bounceInEasing(ipercent);


		auto width = x * mWidth;
		auto height = x * mHeight;

		glfwSetWindowPos(window, (mWidth / 2) - (width * 0.75f / 2), (mHeight / 2) - (height * 0.75f / 2));
		glfwSetWindowSize(window, std::max(1.0f, width * 0.75f), std::max(1.0f, height * 0.75f));

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

		ipercent += 0.01f;

		if (ipercent > 1.0f) {
			glfwSetWindowPos(window, (mWidth / 2) - (mWidth * 0.75f / 2), (mHeight / 2) - (mHeight * 0.75f / 2));
			glfwSetWindowSize(window, std::max(1.0f, mWidth * 0.75f), std::max(1.0f, mHeight * 0.75f));
			glViewport(0, 0, mWidth * 0.75f, mHeight * 0.75f);

			window.dims = glm::vec2(mWidth, mHeight);

			static double start = glfwGetTime();

			// LOAD RESOURCES HERE

			loadTextureResource("resources/sprites.json", resources.textures);

			loadAudioResource("resources/sounds.json", resources.sounds);

			// PLEASE
			resources.sounds.bgm.setGain(0.75f);
			resources.sounds.setBGM("menu");


			const auto& intro = resources.textures.getTexture("intro");
			intro.bind();

			loop = [&]() { // we do a little loading screen

				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);
				glfwSwapBuffers(window);
				glfwPollEvents();

				if (!(glfwGetTime() - start < 1.0)) {
					static float percent = 1.0f;

					loop = [&]() {
				
						glClearColor(0.03f, 0.04f, 0.05f, 1.0f);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						intro.bind();
						SpriteData data{
							glm::scale(glm::translate(glm::mat4(1.0f), glm::mix(glm::vec3(-0.21f, 0.3f, 0.0f), glm::vec3(0.0), percent)), glm::mix(glm::vec3(1.2f, 1.0f, 1.0f), glm::vec3(2.0), percent)),
							resources.textures.getTexcoords("intro"),
							glm::vec4(1.0)
						};

						spriteVBO.setData(std::array<SpriteData, 1>{ data });

						glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

						glfwSwapBuffers(window);
						glfwPollEvents();

						std::this_thread::sleep_for(std::chrono::milliseconds(5));

						percent -= 0.005f;

						if (percent < 0.0f) {
							start = glfwGetTime() - resources.sounds.bgm.tell().count();

							loop = [&]() {
								glClearColor(0.03f, 0.04f, 0.05f, 1.0f);
								glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

								intro.bind();
								SpriteData data{
									glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.21f, 0.3f, 0.0f)), glm::vec3(1.2f, 1.0f, 1.0f)),
									resources.textures.getTexcoords("intro"),
									glm::vec4(1.0)
								};

								spriteVBO.setData(std::array<SpriteData, 1>{ data });

								glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

								glfwSwapBuffers(window);
								glfwPollEvents();

								if (glfwGetTime() - start >= 4.8) {
									loop = [&]() {
										glClearColor(0.03f, 0.04f, 0.05f, 1.0f);
										glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

										intro.bind();
										SpriteData data{
											glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.21f, 0.3f, 0.0f)), glm::vec3(1.2f, 1.0f, 1.0f)),
											resources.textures.getTexcoords("intro"),
											glm::vec4(1.0)
										};

										spriteVBO.setData(std::array<SpriteData, 1>{ data });

										glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 1);

										resources.textures.getTexture("controls.WeAreSoBack").bind();
										std::vector<SpriteData> controls{};

										controls.emplace_back(
											glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f)),
											resources.textures.getTexcoords("controls.jump"),
											glm::vec4(1.0)
										);

										//if (glfwGetTime() - start > glm::mix(4.8f, 9.7f, 0.5f)) {
										controls.emplace_back(
											glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)), glm::vec3(1.0f, 1.0f, 1.0f)),
											resources.textures.getTexcoords("controls.tilt"),
											glm::vec4(1.0)
										);
										//}

										spriteVBO.setData(controls);

										glDrawArraysInstanced(GL_TRIANGLES, 0, 6, controls.size());

										glfwSwapBuffers(window);
										glfwPollEvents();

										if (glfwGetTime() - start >= 9.7) {
											// setup callbacks

											glfwSetCursorPosCallback(window, cursor_position_callback);
											glfwSetMouseButtonCallback(window, mouse_callback);
											glfwSetKeyCallback(window, key_callback);

											//glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

											resources.sounds.setSpeed(1.0);

											static double prev = glfwGetTime();

											glfwSwapInterval(1);

											static double skipTimer = prev;

											loop = [&]() {
												double now = glfwGetTime();

												double dt = std::min(now - prev, 0.1);
												prev = now;

												if (state.audioSkipping && state.state == State::Game && now - skipTimer >= 0.2) {
													try {
														resources.sounds.bgmSkip(-(now - skipTimer) * 0.99);
														skipTimer = now;
													}
													catch (...) {}
												}

												if (state.state != State::Game) {
													lrotating = false;
													rrotating = false;
												}

												if (lrotating && !rrotating) {
													state.rotation += 60.0f * dt;
												}
												else if (rrotating && !lrotating) {
													state.rotation -= 60.0f * dt;
												}

												if (state.rotation > 180.0f) {
													state.rotation = -180.0;
												}
												else if (state.rotation < -180.0f) {
													state.rotation = 180.0;
												}

												state.update(dt, resources);
												render(state, resources, now, mWidth, mHeight);

												glfwSwapBuffers(window);
												glfwPollEvents();

												glFinish();
											};
										}
									};
								}
							};
						}
					};
				}
			};
		}
	};
	
	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(main_loop, 0, true);
	#else
	while (!glfwWindowShouldClose(window)) {
		loop();
	}
	#endif	

	resources.clear();

	glfwTerminate();

	return 0;
}