cmake_minimum_required(VERSION 3.29)
project(ZX16_System_Simulator)

set(CMAKE_CXX_STANDARD 20)

# SFML setup
#set(SFML_DIR "D:/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/lib/cmake/SFML")
#find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

#include_directories("C:/Users/ASUS Zenbook/OneDrive/Documents/GitHub/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/include")
#link_directories("C:/Users/ASUS Zenbook/OneDrive/Documents/GitHub/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/lib")
include_directories("C:/Users/Beba/Documents/GitHub/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/include")
link_directories("C:/Users/Beba/Documents/GitHub/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/lib")

# Main Executable
add_executable(ZX16_System_Simulator
        main.cpp
        Instruction.cpp Instruction.h
        ZX16_Simulator.cpp ZX16_Simulator.h
        Graphics.cpp Graphics.h
        GraphicsMemory.cpp GraphicsMemory.h
)

target_link_libraries(ZX16_System_Simulator
        sfml-graphics
        sfml-window
        sfml-system
)
