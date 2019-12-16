#ifndef QURANBOT_AYAT_H
#define QURANBOT_AYAT_H
#include "../../headers/main.hpp"
#include <json11.hpp>
#include <fstream>
#include <string.h>
#include <vector>
#include <utility>
#include <sstream>
#include <libmemcached/memcached.hpp>
	//global reference to memcached inst,port 11211
	memcached_st* cacheServer;
#define SURAH_JSON_DIR "./data/"
#define MEMCACHED_SERVER_STRING "--SERVER=localhost"
	
	//trims the let and right of a string
	std::string trimString(std::string str) {
		auto isWhiteSpace=[](char input)->bool {
			return (input==' ')||(input=='\t');
		};
		//find start of non whiteSpace
		auto start=std::find_if_not(str.begin(), str.end(), isWhiteSpace);
		auto end = std::find_if_not(str.rbegin(), str.rend(), isWhiteSpace).base();
		return std::string(start,end);
	}
	void initAyatCommand() {
		cacheServer=memcached(MEMCACHED_SERVER_STRING,strlen(MEMCACHED_SERVER_STRING));
	}
	void destroyAyatCommand() {
		memcached_free(cacheServer);
	}
	Bot::commandFunc makeAyatCommand() {
		auto lambda=[](Bot::bot& quranBot,std::vector<std::string>& args)->void {
			json11::Json current;
			auto loadSurahAyats=[&](int surah,int ayatStart,int ayatEnd)->void {
				//get the file name
				std::string name=SURAH_JSON_DIR;
				std::stringstream stream;
				stream<<surah;
				name+="Surah_"+stream.str() +".json";
				std::ifstream readFile;
				//open the file
				readFile.open(name.c_str());
				stream.clear();
				//===read the file into a stream
				std::string contents;
				readFile>>contents;
				current=json11::Json(contents); 
			};
			//
			typedef struct {
					int surah;
					std::vector<std::pair<int,int>> ranges;
			} ayatRange;
			auto getAyatRangesInSurah=[](std::string str)->ayatRange {
				auto where=str.find_first_of(":");
				ayatRange retVal;
				//get Surah number
				std::stringstream stream;
				stream<<std::string(str.begin(),str.begin()+where);
				int surahNum;
				stream>>surahNum;
				retVal.surah=surahNum;
				//
				std::string whiteSpace=" \t";
				std::string delim=",";
				//get ayat ranges
				auto iter=where+str.begin()+1; //go past ":"(does this by adding 1)
				while(iter!=str.end()) {
					auto oldIter=iter;
					//skip whitespace
					iter=std::find_first_of(iter, str.end(),  delim.begin(), delim.end());
					//check for "-" for range of ayats
					std::string cut=trimString(std::string(oldIter, iter));
					auto range=std::find(cut.begin(),cut.end(),'-');
					if(range==cut.end()) {
						//found no range so get single ayat
						stream.clear();
						int ayatNumber;
						stream<<cut;
						stream>>ayatNumber;
						retVal.ranges.push_back(std::make_pair(ayatNumber, ayatNumber+1));
					} else {
						//found range so get start and end
						std::string firstPart=std::string(cut.begin(), range);
						std::string secondPart=std::string(range+1,cut.end());
						//translate these into numbers
						int start;
						stream.clear();
						stream<<firstPart;
						stream>>start;
						//----
						int end;
						stream.clear();
						stream<<secondPart;
						stream>>end;
						//push the range
						retVal.ranges.push_back(std::make_pair(start,end+1));
					}
					//go past comma
					iter++;
				}
			};
			//===groups "things" by Surah Number
			std::vector<std::string> surahGroups;
			std::string temp;
			for(auto& arg:args) {
				//if no surah Specified(has a ":"),add to surah Groups
				if(arg.find(':')) {
					if(temp!="")
						surahGroups.push_back(temp);
				}
				//push last bit if has text push
				if(temp!="")
					surahGroups.push_back(temp);
			}
			//push remainder if has text
			if(temp!="")
				surahGroups.push_back(temp);
			//=== get the ayat Ranges in the surah
			std::vector<ayatRange> ranges;
			for(auto& group:surahGroups)
				ranges.push_back(getAyatRangesInSurah(group));
			//=== lamvda to fetch ayat
			auto getAyat=[](int surah,int ayat)->std::string {
				
			};
		};
	}	
#endif
