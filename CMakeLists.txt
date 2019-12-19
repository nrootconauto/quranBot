cmake_minimum_required(VERSION 3.13)
#debug build
set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)

#CMAKE_EXPORT_COMPILE_COMMANDS
project(quranBot)

#pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(memCached REQUIRED libmemcached)

#oh yeah
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#c++14 
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#add files in src and headers
file(GLOB_RECURSE botSources "./src/*.cpp" "./headers/*.hpp")
list(FILTER botSources INCLUDE REGEX "^[^\.].+\.cpp$")
add_executable(quranBot ${botSources})

#add sleepy discord library
add_dependencies(quranBot sleepy-discord)
target_link_libraries(quranBot sleepy-discord)
target_include_directories(quranBot PRIVATE ./ext/sleepy-discord/include)
add_subdirectory(./ext/sleepy-discord/)

#add json11
add_dependencies(quranBot json11)
target_include_directories(quranBot PRIVATE ./ext/json11/)
target_link_libraries(quranBot  json11)
add_subdirectory(./ext/json11/)

#add libmemcached
pkg_check_modules(memcached REQUIRED libmemcached)
target_link_libraries(quranBot ${memcached_LIBRARIES})
target_include_directories(quranBot PUBLIC ${memcached_INCLUDE_DIRS})
target_compile_options(quranBot PUBLIC ${memcahced_CFLAGS_OTHER})

#add imagemagick
pkg_check_modules(magick REQUIRED ImageMagick++)
target_link_libraries(quranBot ${magick_LIBRARIES})
target_include_directories(quranBot PUBLIC ${magick_INCLUDE_DIRS})
target_compile_options(quranBot PUBLIC ${magick_CFLAGS_OTHER})
