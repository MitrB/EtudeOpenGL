add_library(glm INTERFACE)

add_subdirectory(fmt EXCLUDE_FROM_ALL)
add_subdirectory(glfw EXCLUDE_FROM_ALL)

target_include_directories(glm INTERFACE glm)

add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)
target_sources(imgui PRIVATE 
  imgui/imgui.h
  imgui/imgui.cpp
  imgui/imgui_demo.cpp
  imgui/imgui_draw.cpp
  imgui/imgui_widgets.cpp
  imgui/imgui_tables.cpp
  imgui/imgui_impl_opengl3.cpp
  imgui/imgui_impl_glfw.cpp
)

add_library(glad STATIC glad.c)
target_include_directories(glad PUBLIC glad KHR)
