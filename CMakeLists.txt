cmake_minimum_required(VERSION 3.0.0)
project(2DArena VERSION 0.1.0)

file(COPY myRecources DESTINATION .)

file(GLOB allFiles
     "src/*.hpp"
     "src/*.cpp"
     "src/*.h"
     "src/*.c"
)

add_executable(2DArena ${allFiles})
target_include_directories(2DArena PUBLIC ${PROJECT_SOURCE_DIR}/Textures ${PROJECT_SOURCE_DIR}/2DLib/include)

find_package(SFML 2 REQUIRED graphics window system)

include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(2DArena sfml-graphics sfml-window sfml-system)

target_link_libraries(2DArena -pthread)
