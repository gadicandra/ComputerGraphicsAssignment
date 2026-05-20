cmake_minimum_required(VERSION 3.10)
project(TVG_OpenGL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- GLAD ---
add_library(glad STATIC glad/src/glad.c)
target_include_directories(glad PUBLIC glad/include)

# --- Dependencies ---
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

# --- Executable ---
add_executable(app
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/stb_image_impl.cpp
)
target_include_directories(app PRIVATE include src)
target_link_libraries(app PRIVATE glad glfw dl glm::glm assimp)