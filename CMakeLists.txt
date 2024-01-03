cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(game-1)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

configure_file(Config.h.in ${CMAKE_BINARY_DIR}/Config.h)

# # check the submodules
# if(NOT EXISTS "${PROJECT_SOURCE_DIR}/extern/sdl_image/CMakeLists.txt")
#     message(FATAL_ERROR "The sdl_image submodule was not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
# endif(

# Create an option to switch between a system sdl library and a vendored sdl library
option(MYGAME_VENDORED "Use vendored libraries" OFF)

if(MYGAME_VENDORED)
    add_subdirectory(vendored/sdl EXCLUDE_FROM_ALL)
else()
    # 1. Look for a SDL2 package, 2. look for the SDL2 component and 3. fail if none can be found
    find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)

    # 1. Look for a SDL2 package, 2. Look for the SDL2maincomponent and 3. DO NOT fail when SDL2main is not available
    find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/utils.cpp
    src/shader.cpp
)

# add_subdirectory(extern/sdl2_gfx)
# add_subdirectory(extern/sdl_image)
add_subdirectory(extern/glad)

target_include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_BINARY_DIR}
        PUBLIC ${PROJECT_SOURCE_DIR}/extern/stb_image
        # ${PROJECT_SOURCE_DIR}/extern/glad/include
)

# SDL2::SDL2main may or may not be available. It is e.g. required by Windows GUI applications
if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2main)
endif()

# Link to the actual SDL2 library. SDL2::SDL2 is the shared SDL library, SDL2::SDL2-static is the static SDL libarary.
target_link_libraries(${PROJECT_NAME} PRIVATE SDL2::SDL2 glad)


set(GCC_COMPILE_OPTIONS "-Wall;-Wextra;-pedantic;")
set(GCC_DEBUG_OPTIONS "${GCC_COMPILE_OPTIONS};-g;-O0")
set(GCC_RELEASE_OPTIONS "${GCC_COMPILE_OPTIONS};-O3;-DNDEBUG")

target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:DEBUG>:${GCC_DEBUG_OPTIONS}>")
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:RELEASE>:${GCC_RELEASE_OPTIONS}>")
