project(showdown-bot)

add_subdirectory(json)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required(VERSION 3.17)

add_executable(bot main.cpp)
target_include_directories(bot PUBLIC include)

target_link_libraries(bot nlohmann_json::nlohmann_json)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system thread)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found, cannot compile websocket")
endif()

add_executable(client websocket_client.cpp)
target_include_directories(client PUBLIC include ${Boost_INCLUDE_DIRS}) 
target_compile_features(client PRIVATE cxx_std_20)
target_link_libraries(client  PUBLIC ${Boost_LIBRARIES})
