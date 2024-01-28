#pragma once

#include <vector>
#include <random>

#include <sndx/util/weightedvector.hpp>

#include "resource/resourcestate.hpp"

enum class State {
	Menu,
	Game,
	Credits,
	GameOver
};

enum class Obstacle {
	None,
	Barrier,
	Pit,
};

inline std::string ObstacleToName(Obstacle ob) {
	switch (ob) {
	case Obstacle::None:
		return "none";
	case Obstacle::Barrier:
		return "barrier";
	case Obstacle::Pit:
		return "pit";
	default:
		return "";
	}
}

constexpr float gravity = 1.0f;

inline void generateTrack(std::vector<Obstacle>& out, int atLeast) {
	static sndx::WeightedVector<Obstacle> weights{};

	static std::random_device rd;
	static std::mt19937 gen(rd());

	static std::uniform_int_distribution<> distribHole(0, 1);
	static std::uniform_int_distribution<> distribGap(3, 7);

	auto c = 0;

	while (c < atLeast) {
		auto holeS = distribHole(gen);
		auto gapS = distribGap(gen);

		for (auto i = 0; i <= holeS; ++i) {
			out.emplace_back(Obstacle::Pit);
		}

		for (auto i = 0; i < gapS; ++i) {
			out.emplace_back(Obstacle::None);
		}

		c += holeS + gapS;
	}
}

struct Gamestate {
	State state = State::Menu;

	glm::vec3 pos{};
	glm::vec3 velocity{ glm::vec3(2.0f, 0.0f, 0.0f)};

	std::vector<Obstacle> track{};
	size_t animFrame{};

	bool hovering = false;

	float hp = 1.0;
	bool tilted = false;

	bool edging = false;
	bool blurring = false;
	bool posterize = false;
	bool warping = false;
	bool megaWarping = false;
	bool shake = false;
	bool invert = false;
	bool rotate = false;
	bool image = false;

	bool audioSkipping = false;

	float rotation = 0.0;

	double timer{};


	void reset() {
		edging = false;
		blurring = false;
		posterize = false;
		warping = false;
		megaWarping = false;
		shake = false;
		invert = false;
		rotate = false;
		image = false;

		audioSkipping = false;
	}


	void start(ResourceState& resources) {
		reset();

		resources.sounds.bgm.setGain(0.5f);
		resources.sounds.setBGM("bgm");

		resources.sounds.sfx.stop();
		resources.sounds.sfx.setBuffer(resources.sounds["start"]);
		resources.sounds.sfx.setGain(1.0).setSpeed(1.0).play();


		hovering = false;

		animFrame = 0;
		pos.x = 2.0;
		pos.y = 0.0;
		track = std::vector{ 16, Obstacle::None };
	}

	void update(double dt, ResourceState& resources) {

		if (state == State::Game) {
			timer += dt;

			if (timer >= 0.1) {
				timer = 0.0;
				++animFrame;

				if (std::abs(rotation) <= 5.0) {
					tilted = false;
					reset();
				}
				else {
					tilted = true;
					megaWarping = std::abs(rotation) >= 90.0;

					if (megaWarping) {
						audioSkipping = true;
					}

					if (animFrame % 21 == 0) {
						static std::random_device rd;
						static std::mt19937 gen(rd());

						static std::uniform_int_distribution<> distrib(0, 9);

						switch (distrib(gen)) {
						case 0:
							edging = true;
							break;
						case 1:
							blurring = true;
							break;
						case 2:
							posterize = true;
							break;
						case 3:
							warping = true;
							break;
						case 4:
							shake = true;
							break;
						case 5:
							invert = true;
							break;
						case 6:
							rotate = true;
							break;
						case 7:
							audioSkipping = true;
							break;
						case 8:
							image = true;
							break;
						case 9:
							pos.y += 1.25;
							velocity.y = gravity * dt;
							hovering = true;
							break;
						default:
							megaWarping = true;
							break;
						}
					}
				}
			}

			if (!tilted) {
				reset();
			}

			if (floor(pos.x) >= track.size() - 16) {
				generateTrack(track, 16);
			}

			pos += velocity * float(dt);
			velocity.y -= gravity * dt;

			if (pos.y < -0.4f) {
				state = State::GameOver;
				resources.sounds.bgm.stop();

				resources.sounds.carNoises.stop();
				resources.sounds.carNoises.setBuffer(resources.sounds["v8"]);
				resources.sounds.carNoises.setParam(AL_LOOPING, AL_TRUE).setGain(0.1f).setSpeed(1.0).play();
				timer = 0.0;
			}

			if (track.at(size_t(ceil(pos.x - 0.2))) != Obstacle::Pit) {
				if (pos.y <= 0.0f && pos.y > -0.1f && velocity.y < 0.0f) {
					velocity.y = 0.0f;
					pos.y = 0.0f;

					if (hovering) {
						hovering = false;
						resources.sounds.carNoises.stop();
					}

					if (!resources.sounds.carNoises.playing()) {
						resources.sounds.carNoises.setBuffer(resources.sounds["redline"]);
						resources.sounds.carNoises.setParam(AL_LOOPING, AL_TRUE).setSpeed(1.0).play();
					}
				}
			}
		}
		else if (state == State::GameOver) {
			timer += dt;

			
		}
		else if (state == State::Menu) {
			timer += dt;

			if (timer >= 0.1) {
				timer = 0.0;
				++animFrame;
			}
		}
	}
	
};