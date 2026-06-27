cmake_minimum_required(VERSION 3.16)
project(FluidSim VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)

# --- Dear ImGui --------------------------------------------------------------
# ImGui ships no CMakeLists of its own, so we compile the core + the two
# backends we need (GLFW for input/windowing, OpenGL3 for drawing) into a
# small static library. PUBLIC include dirs mean any target that links
# `imgui` automatically sees <imgui.h> and the backend headers.
set(IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui")

add_library(imgui STATIC
    "${IMGUI_PATH}/imgui.cpp"
    "${IMGUI_PATH}/imgui_draw.cpp"
    "${IMGUI_PATH}/imgui_tables.cpp"
    "${IMGUI_PATH}/imgui_widgets.cpp"
    "${IMGUI_PATH}/imgui_demo.cpp"
    "${IMGUI_PATH}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui PUBLIC
    "${IMGUI_PATH}"
    "${IMGUI_PATH}/backends"
)

# imgui_impl_glfw.cpp #includes <GLFW/glfw3.h>; linking glfw forwards its
# include path. The OpenGL3 backend uses its own bundled GL loader, so it
# does not need Glad.
target_link_libraries(imgui PUBLIC glfw)


set(GLAD_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/Glad")

set(HEADER_FILES
    src/app.h
    src/renderer.h
    src/particleSystem.h
    src/Shader.h
    src/meshing.h
    src/Camera.h
    src/keyHandler.h
)

set(SOURCE_FILES
    src/main.cpp
    src/app.cpp
    src/renderer.cpp
    src/particleSystem.cpp
    src/meshing.cpp
    "${GLAD_PATH}/src/glad.c"
)

set(SHADER_FILES
    shaders/render.vert
    shaders/render.frag
    shaders/CompShader.comp
    shaders/InitCompShader.comp
)

source_group("Shaders" FILES ${SHADER_FILES})

add_executable(FluidSim ${SOURCE_FILES} ${HEADER_FILES} ${SHADER_FILES})

target_include_directories(FluidSim PRIVATE 
    src
    "${GLAD_PATH}/include"
    "vendor/glm"
)

target_compile_definitions(FluidSim PRIVATE 
    SHADER_DIR="${CMAKE_SOURCE_DIR}/shaders/"
)

target_link_libraries(FluidSim PRIVATE
    OpenGL::GL
    glfw
    imgui
)

# Copy shaders to build directory
add_custom_command(TARGET FluidSim POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:FluidSim>/shaders
    COMMENT "Copying shaders to build directory"
)
