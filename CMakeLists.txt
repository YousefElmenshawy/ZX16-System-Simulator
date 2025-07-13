cmake_minimum_required(VERSION 3.10)
project(ZX16_System_Simulator)

set(CMAKE_CXX_STANDARD 17)

# Set correct SFML location for macOS + Homebrew
set(SFML_DIR "/opt/homebrew/opt/sfml@2/lib/cmake/SFML")

find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# Add to CMakeLists.txt
configure_file(${CMAKE_SOURCE_DIR}/Main/ballhit.wav ${CMAKE_BINARY_DIR}/ballhit.wav COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/Main/balllose.wav ${CMAKE_BINARY_DIR}/balllose.wav COPYONLY)

include_directories("/opt/homebrew/include")
link_directories("/opt/homebrew/lib")

add_executable(ZX16_System_Simulator
        Main/main.cpp
        Main/Instruction.cpp Main/Instruction.h
        Main/ZX16_Simulator.cpp Main/ZX16_Simulator.h
        Main/Graphics.cpp Main/Graphics.h
        Main/GraphicsMemory.cpp Main/GraphicsMemory.h

)

target_link_libraries(ZX16_System_Simulator
        sfml-graphics
        sfml-window
        sfml-system
        sfml-audio
)
