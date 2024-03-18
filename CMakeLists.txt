cmake_minimum_required(VERSION 3.26.2)
project(etudegl) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2") 

find_package(OpenGL REQUIRED)

add_subdirectory(third_party)

file(GLOB_RECURSE SOURCES 
   ${PROJECT_SOURCE_DIR}/src/*.cpp 
   ${PROJECT_SOURCE_DIR}/src/*.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(etudegl PRIVATE glfw glm imgui fmt dl pthread X11 Xxf86vm Xrandr Xi glad ${CMAKE_DL_LIBS} ${OPENGL_LIBRARIES})
