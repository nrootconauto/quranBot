#ifndef QURAN_BOT_H
#define QURAN_BOT_H
#include <string>
#include <map>
#include <functional>
#include <vector>
#include <sleepy_discord/websocketpp_websocket.h>
	namespace Bot {
			class bot;
			typedef std::function<void(bot&,std::vector<std::string>&)>  commandFunc;
			
			class bot: public SleepyDiscord::DiscordClient {
					//caommands
					std::map<std::string,commandFunc> commands;

					using SleepyDiscord::DiscordClient::DiscordClient;
					//when gets a message,check for !quran and run command
					void onMessage(SleepyDiscord::Message message) override;
			};
	};
#endif
