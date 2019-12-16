#include <iostream>
#include <json11.hpp>
#include <map>
#include <algorithm>
#include <vector>
#include <functional>
#include "../headers/main.hpp"
			void Bot::bot::onMessage(SleepyDiscord::Message message) {
				//===if addresing this bot
				if(message.startsWith("!quran")) {
					//===deliminate by whitespace
					//prediocate
					auto isWhitespace=[](char toTest)->bool {
						return (toTest==' ')||(toTest=='\t');
					};
					auto& str=message.content;
					auto where=str.begin();
					std::vector<std::string> items;
					//loop to get deliminated stuff
					while(true) {
						//find whitespace
						auto whiteSpace=std::find_if(where, str.end(), isWhitespace);
						auto nonWhiteSpace=std::find_if_not(whiteSpace,str.end(),isWhitespace);
						//if didnt find any non-whitepsace,quit
						if(nonWhiteSpace==str.end())
							break;
						//make a slice
						std::string slice=std::string(where,whiteSpace);
						items.push_back(slice);
						//repeat at the current "item"
						where=nonWhiteSpace;
					}
				}
				//got the items so ignore first item("!quran")
				//=== look for command(first item after !quran)
				
			}
	int main() {
		const std::string token( "NjU1MTE1MTA0MTQwMTMyMzg1.Xfag5g.z7ASq6_pCaTinL10dWoYQX3f2Z4");
		
		Bot::bot client(token);
		client.run();
		return 0;
	}
