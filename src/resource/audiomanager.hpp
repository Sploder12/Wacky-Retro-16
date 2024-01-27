#pragma once

#include <sndx/util/resourcemanager.hpp>


#include <sndx/util/stringmanip.hpp>
#include <sndx/data.hpp>

#include <sndx/audio.hpp>

class AudioManager : public sndx::ResourceManager<sndx::ABO> {
public:
	sndx::ALContext<> context{};

	sndx::ALSource bgm{};
	sndx::ALSource carNoises{};

	AudioManager() {
		context.bind();

		bgm = context.createSource("bgm");
		carNoises = context.createSource("car");

		bgm.setPos(glm::vec3(0.0f));
		carNoises.setPos(glm::vec3(0.0f));

		context.setVolume(0.3);

		bgm.setParam(AL_LOOPING, AL_TRUE);
	}

	void setSpeed(float speed) {
		bgm.setSpeed(speed);
		carNoises.setSpeed(speed);
	}

	void setBGM(const std::string& id) {
		bgm.setBuffer(this->at(id)).play();
	}

	void bgmSkip(float t) {
		auto time = bgm.tell() + std::chrono::duration<float>(t);

		bgm.seekSec(time);
	}
};

inline bool loadAudioResource(const std::filesystem::path& path, AudioManager& manager) {
	auto data = sndx::decodeData<sndx::JSONdecoder>(path);
	if (!data.has_value()) {
		return false;
	}

	if (!data->holdsAlternative<sndx::DataDict>()) {
		return false;
	}

	const sndx::DataDict& dict = std::get<sndx::DataDict>(data->data);

	// LOL stbi isn't thread safe!

	for (const auto& [id, obj] : dict) {
		if (obj.holdsAlternative<sndx::Primitive>()) {
			const auto& prim = *obj.get<sndx::Primitive>();

			if (std::holds_alternative<std::string>(prim.data)) {
				auto ret = sndx::loadAudioFile("resources/sounds/" + std::get<std::string>(prim.data));

				if (ret.has_value()) {
					sndx::ABO abo{};

					abo.setData(ALenum(ret->format), std::span<short>{ret->buffer.begin(), ret->buffer.size()}, ret->freq);

					manager[id] = std::move(abo);
				}
			}
		}
	}

	return true;
}