cmake_minimum_required(VERSION 3.26.2)

project(etudegl)

find_package(OpenGL REQUIRED)
find_package(SDL2 REQUIRED)

include_directories(${SDL2_REQUIRED_DIRS})

file(GLOB_RECURSE SOURCES 
   ${PROJECT_SOURCE_DIR}/src/*.cpp 
   ${PROJECT_SOURCE_DIR}/third_party/*.c 
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(etudegl ${SDL2_LIBRARIES})