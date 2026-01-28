cmake_minimum_required(VERSION 3.20)
project(OpenGLFirstProject LANGUAGES CXX)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

file(GLOB SRC_FILES
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/Libs/*.cpp"
)
add_executable(OpenGLStarter ${SRC_FILES})

target_include_directories(OpenGLStarter PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/Libs
)

target_link_libraries(OpenGLStarter PRIVATE
    OpenGL::GL
    glfw
    GLEW::GLEW
    glm::glm
)

if(APPLE)
    target_compile_definitions(OpenGLStarter PRIVATE GL_SILENCE_DEPRECATION)
    target_link_libraries(OpenGLStarter PRIVATE
        "-framework Cocoa" "-framework IOKit" "-framework CoreVideo"
    )
endif()

# Copy shaders next to the binary
add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Shaders
    $<TARGET_FILE_DIR:OpenGLStarter>/Shaders)
add_dependencies(OpenGLStarter copy_shaders)

add_custom_target(copy_textures ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Textures
    $<TARGET_FILE_DIR:OpenGLStarter>/Textures)
add_dependencies(OpenGLStarter copy_textures)

add_custom_target(copy_models ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Models
    $<TARGET_FILE_DIR:OpenGLStarter>/Models)
add_dependencies(OpenGLStarter copy_models)
