#pragma once

#include <vector>
#include <random>

#include <sndx/util/weightedvector.hpp>

enum class State {
	Menu,
	Game,
	Credits,
	GameOver
};

enum class Obstacle {
	None,
	Barrier,
};

inline std::string ObstacleToName(Obstacle ob) {
	switch (ob) {
	case Obstacle::None:
		return "none";
	case Obstacle::Barrier:
		return "barrier";
	default:
		return "";
	}
}

constexpr float gravity = 0.1f;

inline void generateTrack(std::vector<Obstacle>& out, int atLeast) {
	static sndx::WeightedVector<Obstacle> weights{};

	if (weights.empty()) [[unlikely]] {
		weights.emplace_back(10, Obstacle::None);
		weights.emplace_back(1, Obstacle::Barrier);
	}

	static std::random_device rd;
	static std::mt19937 gen(rd());

	static std::uniform_int_distribution<> distrib(0, weights.size() - 1);

	for (auto i = 0; i < atLeast; ++i) {
		Obstacle* ob = weights[distrib(gen)];

		Obstacle obj = ob ? *ob : Obstacle::None;

		out.emplace_back(obj);
	}
}

struct Gamestate {
	State state = State::Menu;

	glm::vec3 pos{};
	glm::vec3 velocity{ glm::vec3(2.0f, 0.0f, 0.0f)};

	std::vector<Obstacle> track{};
	size_t animFrame{};

	bool edging = true;
	bool blurring = true;
	bool posterize = false;
	bool warping = false;
	bool megaWarping = false;
	bool shake = false;
	bool invert = false;
	bool rotate = true;


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
	}

	void update(double dt) {
		

		if (state == State::Game) {
			timer += dt;

			if (timer >= 0.1) {
				timer = 0.0;
				++animFrame;
			}

			if (floor(pos.x) >= track.size() - 32) {
				generateTrack(track, 32);
			}

			pos += velocity * float(dt);
			velocity.y -= gravity * dt;

			if (pos.y <= 0.0f && velocity.y < 0.0f) {
				velocity.y = 0.0f;
				pos.y = 0.0f;
			}
		}
	}
};