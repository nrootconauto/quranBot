#include "../../headers/main.hpp"
#include <json11.hpp>
#include <fstream>
#include <string.h>
#include <vector>
#include <utility>
#include <list>
#include <sstream>
#include <Magick++.h>
#include <libmemcached/memcached.hpp>
#include "./ayat.hpp"
	//global reference to memcached inst,port 11211
	namespace ayatCommand {
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
			//makes the image
			Magick::Image makeRenderOfAyats(std::vector<std::string>& arabic) {
				auto image=Magick::Image( "200x200","white");
				image.backgroundColor("green");
				image.font("DejaVu-Sans-Mono-Bold");
				image.fontPointsize(20);
				image.strokeWidth(1);
				image.strokeColor("green");
				image.fillColor("yellow");
				image.read("Pango:Hello World");
				image.display();
				return image;
			}
			//makes the embed
			Bot::commandFunc makeAyatCommand() {
				auto lambda=[](Bot::bot& quranBot,std::vector<std::string>& args)->void {
					try{
					json11::Json current;
					auto loadSurahAyats=[&](int surah)->void {
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
						std::string line;
						//rtead line by line
						while(std::getline(readFile,line))
							contents+=line+"\n";
						std::string err;
						current=current.parse(contents, err);
						std::cout<<err<<std::endl;
					};
					//
					typedef struct {
							int surah;
							std::vector<std::pair<int,int>> ranges;
					} ayatRange;
					//Renders the iamges
					//Get gets the stuff
					auto getAyatRangesInSurah=[&](std::string str)->ayatRange {
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
						std::cout<<"str is:"<<str<<std::endl;
						while(iter!=str.end()) {
							auto oldIter=iter;
							//skip whitespace
							iter=std::find_first_of(iter, str.end(),  delim.begin(), delim.end());
							//check for "-" for range of ayats
							std::string cut=trimString(std::string(oldIter, iter));
							std::cout<<"cut:"<<cut<<std::endl;
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
							if(iter!=str.end())
								iter++;
						}
						return retVal;
					};
					//===groups "things" by Surah Number
					std::vector<std::string> surahGroups;
					std::string temp;
					for(auto& arg:args) {
						//if no surah Specified(has a ":"),add to surah Groups
						if(arg.find(':')) {
							//if has text,push it then clear
							if(temp.size()!=0) {
								surahGroups.push_back(temp);
								temp.clear();
							};
						}
						temp+=arg;
					}
					//push remainder if has text
					if(temp!="")
						surahGroups.push_back(temp);
					//=== get the ayat Ranges in the surah
					std::vector<ayatRange> ranges;
					for(auto& group:surahGroups)
						ranges.push_back(getAyatRangesInSurah(group));
					//=== lamvda to fetch ayat
					auto getAyat=[&](int surah,int ayat,std::string currentLangauge)->std::string {
						std::string retVal;
						//==! key is form "[SurahNumber]:[ayatNumber]:[langauge]" for text
						//==go through the langauges(start with Arabic)
						if(true) {
							//make key
							std::stringstream stream;
							stream<<surah<<":"<<ayat<<":"<<currentLangauge;
							std::string key=stream.str();
							stream.clear();
							//check if is cached
							if(MEMCACHED_SUCCESS==memcached_exist(cacheServer, key.c_str(), key.size())) {
								//in cache so retive
								size_t howBig;
								char* textbin=memcached_get(cacheServer, key.c_str(), key.size(),&howBig , NULL, NULL);
								std::string text(textbin,textbin+howBig);
								//free the lump
								free(textbin);
							} else {
								//retrive from json
								//if doenst have current surah,load the current ayat
								if(current["Surah"].number_value()!=surah) {
									loadSurahAyats(surah);
								}
								//make sure the ayat is above 0
								if(ayat<=0)
									throw(Bot::botCommandException("Make sure the Ayat is above 0"));
								//get the ayat
								std::cout<<"Current Lang:"<<currentLangauge<<std::endl;
								auto temp=current["Ayats"];
								//make sure ayat is in surah
								if(ayat+1>=temp.array_items().size()) {
									std::stringstream ss;
									ss<<"This Surah only has "<<temp.array_items().size()<<" ayats!"<<std::endl;
									throw(Bot::botCommandException(ss.str()));
								}
								auto temp2=temp[ayat-1];
								retVal=temp2[currentLangauge].string_value();
								//cache the result
								memcached_add(cacheServer, key.c_str(), key.size(), retVal.c_str(), retVal.size(), (time_t)0, 0);
							}
						}
						return retVal;
					};
					//go thorugh the ranges
					std::map<std::string,std::string > langaugeTexts;
					for(auto& group:ranges) {
						//this is the range of the surah to print
						for(auto& range:group.ranges)
							for(int i=range.first;i!=range.second+1;i++) {
								//first do arabic then other
								langaugeTexts["arabic"]+=getAyat(group.surah,i,"arabic");
								//load the translations
								for(auto& langauge: quranBot.currentLanguagesToShow) {
									langaugeTexts[langauge]=getAyat(group.surah,i,langauge);
								}
							}
					}
					//===arange in order of arabic,enlisgh,spanish,french,and urdu
					std::string text;
					std::stringstream sstream;
					//ORDER TO DISPLAY AYATS
					const std::vector<std::string> order={
						"arabic",
						"english",
						"spanish",
						"french",
						"urdu"
					};
					//=== create the rich embed
					json11::Json::array fields;
					//go through the order of the things
					for(auto& lang:order) {
						auto& text=langaugeTexts[lang];
						//push back entry if text is not empty
						if(text.size()!=0)
							fields.push_back(json11::Json::object {{"title",lang},{"Value",text}});
					}
					//jsonify that stuff
					json11::Json embed =json11::Json::object {
						{"embed", json11::Json::object {
								{"author" , json11::Json::object {
										{"name", "QuranBot" },
										{"icon_url","stuff here"},
										{"url","INSERT TEXT HERE"},
									},
								},
								{
									"image","RENDERED TEXT HERE"
								},
								{
									"color","#00ff00"
								},
								//insert the Sauce
								{
									"fields",fields
								},
							},
						},
					};
					std::string compiledJson=embed.dump();
					std::cout<<"JSON:"<<embed.dump();
					//make the message
					//auto message=SleepyDiscord::Message(&compiledJson);
					//send it out
					//message.send( dynamic_cast<SleepyDiscord::BaseDiscordClient*>(&quranBot));
					} catch(Bot::botCommandException x) {
						std::cout<<x.message;
					}
				};
				return lambda;
			}
	}
