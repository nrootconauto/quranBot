#ifndef QURANBOT_AYAT_H
#define QURANBOT_AYAT_H
#include <Magick++.h>
#include "../../headers/main.hpp"
	namespace ayatCommand {
			Bot::commandFunc makeAyatCommand();
			void initAyatCommand();
			void destroyAyatCommand();
			Magick::Image makeRenderOfAyats(std::vector<std::string>& arabic);
			Bot::commandFunc makeAyatCommand();
	}
#endif
