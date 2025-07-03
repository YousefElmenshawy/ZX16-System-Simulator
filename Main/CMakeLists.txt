cmake_minimum_required(VERSION 3.29)
project(ZX16_System_Simulator)

set(CMAKE_CXX_STANDARD 20)

#set(SFML_DIR "C:/Users/ASUS Zenbook/Downloads/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/lib/cmake/SFML")

#find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

include_directories("C:/Users/ASUS Zenbook/OneDrive/Documents/GitHub/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/include")
link_directories("C:/Users/ASUS Zenbook/OneDrive/Documents/GitHub/ZX16-System-Simulator/Main/SFML-2.5.1-windows-gcc-7.3.0-mingw-64-bit/SFML-2.5.1/lib")    #change the path according to repo path


add_executable(ZX16_System_Simulator main.cpp
        Instruction.cpp
        Instruction.h
        ZX16_Simulator.cpp
        ZX16_Simulator.h
        Graphics.cpp
        Graphics.h
        GraphicsMemory.cpp
        GraphicsMemory.h
)


target_link_libraries(ZX16_System_Simulator
        sfml-graphics
        sfml-window
        sfml-system
)

# GoogleTest via FetchContent (portable, recommended)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(zx16_gtest
        Instruction.cpp
    ZX16_Simulator.cpp
)
add_dependencies(ZX16_System_Simulator zx16_gtest)

target_link_libraries(zx16_gtest gtest gtest_main pthread)
add_test(NAME zx16_gtest COMMAND zx16_gtest)

add_executable(zx16_disasm_gtest
        Instruction.cpp
    ZX16_Simulator.cpp
)
target_link_libraries(zx16_disasm_gtest gtest gtest_main pthread)
add_test(NAME zx16_disasm_gtest COMMAND zx16_disasm_gtest)

add_executable(zx16_exec_gtest
        Instruction.cpp
    ZX16_Simulator.cpp
)
target_link_libraries(zx16_exec_gtest gtest gtest_main pthread)
add_test(NAME zx16_exec_gtest COMMAND zx16_exec_gtest)

add_executable(disassembly_tests
    TestCases/Disassembly\ Tests.cpp
    Instruction.cpp
    ZX16_Simulator.cpp
        GraphicsMemory.cpp
)
target_link_libraries(disassembly_tests gtest gtest_main pthread)
add_test(NAME disassembly_tests COMMAND disassembly_tests)

add_executable(execution_tests
    TestCases/Execution\ Tests.cpp
    Instruction.cpp
    ZX16_Simulator.cpp
)
target_link_libraries(execution_tests gtest gtest_main pthread)
add_test(NAME execution_tests COMMAND execution_tests)

# Copy test data files to build directory for all test targets
file(GLOB TEST_BIN_FILES "${CMAKE_SOURCE_DIR}/TestCases/*.bin")
file(GLOB TEST_ASM_FILES "${CMAKE_SOURCE_DIR}/TestCases/*.asm" "${CMAKE_SOURCE_DIR}/TestCases/*.s")
foreach(test_file ${TEST_BIN_FILES} ${TEST_ASM_FILES})
    add_custom_command(TARGET zx16_disasm_gtest PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${test_file} $<TARGET_FILE_DIR:zx16_disasm_gtest>)
    add_custom_command(TARGET zx16_exec_gtest PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${test_file} $<TARGET_FILE_DIR:zx16_exec_gtest>)
    add_custom_command(TARGET disassembly_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${test_file} $<TARGET_FILE_DIR:disassembly_tests>)
    add_custom_command(TARGET execution_tests PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${test_file} $<TARGET_FILE_DIR:execution_tests>)
endforeach()
