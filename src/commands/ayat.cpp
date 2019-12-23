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
				
				auto image=Magick::Image( "480x200","green");
				image.matte(true);
				image.font("DejaVu-Sans-Mono-Bold");
				image.fontPointsize(20);
				image.strokeWidth(1);
				image.backgroundColor("transparent");
				image.strokeColor("green");
				image.fillColor("yellow");
				//make the string
				std::string temp;
				for(auto& item:arabic)
					temp+=item;
				image.read("Pango:"+temp);
				image.trim();
				//image.display();
				image.magick("png");
				image.write("out.png");
				return image;
			}
			//makes the embed
			Bot::commandFunc makeAyatCommand() {
				auto lambda=[](Bot::bot& quranBot,std::vector<std::string>& args, SleepyDiscord::Snowflake<SleepyDiscord::Channel>& channel)->void {
					try{
						//ORDER TO DISPLAY AYATS
						const std::vector<std::string> order={
							"arabic",
							"english",
							"spanish",
							"french",
							"urdu"
						};
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
						if(where==std::string::npos)
							throw std::string("Use like [Surah Number]:[ayat1](,[ayat2]-[ayatN])");
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
						std::cout<<"str is"<<str<<std::endl;
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
					std::string lastAloneNumber;
					const std::string digits="0123456789";
					auto notDigit=[&](char chr)->bool {
						return std::string::npos!=digits.find(chr);
					};
					//list of langauges to display,(will defualt to engilshif none is foumd)
					std::vector<std::string> toShow;
					for(auto& arg:args) {
						//check if found a langauge
						if(order.end()!=std::find_if(order.begin(), order.end(), [&](const std::string& str)->bool {
							return str==arg;
						})) {
							//add the found langauge to list to show
							toShow.push_back(arg);
							continue;
						}
						//if no surah Specified(has a ":"),add to surah Groups
						if(arg.find(':')) {
							//if has text,push it then clear
							if(temp.size()!=0) {
								//works for "5 :" as 5 is the last alone number
								surahGroups.push_back(lastAloneNumber+temp);
								temp.clear();
								lastAloneNumber.clear();
							};
							//check if is a alone number
						} else if(arg.end()== std::find_if(arg.begin(), arg.end(), notDigit)) {
							//pump it into a number
							lastAloneNumber=arg;
						};
						temp+=arg;
					}
					//push remainder if has text
					if(temp!="")
						surahGroups.push_back(temp);
					//===default to english if toShow is empty;
					if(toShow.size()==0)
						toShow.push_back("english");
					//=== get the ayat Ranges in the surah
					std::vector<ayatRange> ranges;
					for(auto& group:surahGroups)
						try {
							ranges.push_back(getAyatRangesInSurah(group));
						} catch(std::string err) {
							std::cout<<(quranBot.sendMessage(channel, err) )<<endl;
						}
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
								//remove {x} "ayat marker" if arabic text \ufd3e
								if(currentLangauge=="arabic") {
									const std::string left="\ufd3e";
									const std::string right="\ufd3f";
									auto side1=retVal.begin()+retVal.find("\ufd3e");
									auto side2=retVal.begin()+retVal.find("\ufd3f");
									//ensure side1 is before side2
									if(side1>side2)
										std::swap(side1,side2);
									//std::swap_ranges(side1, side1+left.size(), side2);
								}
								//cache the result
								memcached_add(cacheServer, key.c_str(), key.size(), retVal.c_str(), retVal.size(), (time_t)0, 0);
							}
						}
						return retVal;
					};
					//go thorugh the ranges
					std::map<std::string,std::vector<std::pair<std::pair<int,int>,std::string>>> langaugeTexts;
					for(auto& group:ranges) {
						//this is the range of the surah to print
						for(auto& range:group.ranges)
							//ensure that the first part of the range is lesser than the last
							//go through the ranges
							for(int i=range.first;i!=range.second;i++) {
								//first do arabic then other
								//langaugeTexts["arabic"].push_back(std::make_pair(std::make_pair(group.surah,i), getAyat(group.surah,i,"arabic")));
								//load the translations
								for(auto& langauge: toShow) {
									langaugeTexts[langauge].push_back(std::make_pair(std::make_pair(group.surah,i), getAyat(group.surah,i,langauge)));
								}
							}
					}
					//===arange in order of arabic,enlisgh,spanish,french,and urdu
					std::string text;
					std::stringstream sstream;
					//=== create the rich embed
					json11::Json::array fields;
					//go through the order of the things
					for(auto& lang:order) {
						auto& text=langaugeTexts[lang];
						//push back entry if text is not empty
						if(text.size()!=0) {
							//go trough the ayats
							for(auto& entry:text) {
								//make the title in form "[Surah]:[Ayat]([langauge])"
								std::stringstream title;
								title<<entry.first.first<<":"<<(entry.first.second)<<"("<<lang<<")";
								fields.push_back(json11::Json::object {{"name",title.str()},{"value",entry.second}});
							}
						}
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
									"color",0x00ff00
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
					std::ofstream poop("output.json");
					poop<<embed.dump();
					poop.close();
					//make the message
					auto message=SleepyDiscord::Embed(&compiledJson);
					//send it out
					quranBot.sendMessage(channel, "", message);
					} catch(Bot::botCommandException x) {
						std::cout<<x.message;
					}
				};
				return lambda;
			}
	}
