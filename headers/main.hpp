#ifndef QURAN_BOT_H
#define QURAN_BOT_H
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <string>
#include "../ext/sleepy-discord/include/sleepy_discord/websocketpp_websocket.h"
	namespace Bot {
			class bot;
			typedef std::function<void(bot&,std::vector<std::string>&,SleepyDiscord::Snowflake<SleepyDiscord::Channel>& channel)>  commandFunc;
			class botCommandException {
				public:
					std::string message;
					botCommandException(std::string msg="DefineMe"):message(msg) {};
			};
			class bot: public SleepyDiscord::DiscordClient {
				public:
					//caommands
					std::map<std::string,commandFunc> commands;

					//options for showing stuff
					std::vector<std::string> currentLanguagesToShow;
					bool showImage;
					
					using SleepyDiscord::DiscordClient::DiscordClient;
					//when gets a message,check for !quran and run command
					void onMessage(SleepyDiscord::Message message) override;
			};
	};
#endif
