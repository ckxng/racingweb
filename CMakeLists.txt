cmake_minimum_required(VERSION 3.16)
project(racingweb)

# Wt is less common and may not be available on all systems.
# How would we like to link it?
set(UNCOMMON_LINK_TYPE SHARED)
#set(UNCOMMON_LINK_TYPE STATIC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_library(Wt_location NAMES libwt.so)
add_library(Wt ${UNCOMMON_LINK_TYPE} IMPORTED)
set_target_properties(Wt PROPERTIES IMPORTED_LOCATION ${Wt_location})

find_library(WtHttp_location NAMES libwthttp.so)
add_library(WtHttp ${UNCOMMON_LINK_TYPE} IMPORTED)
set_target_properties(WtHttp PROPERTIES IMPORTED_LOCATION ${WtHttp_location})

include_directories(${CMAKE_SOURCE_DIR})

add_executable(racingweb src/main.cc src/RacingWebApplication.cc src/raceutil.cc src/pregen.cc src/RacingWebApplication_ui.cc)
target_link_libraries(racingweb Wt WtHttp)
