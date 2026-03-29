cmake_minimum_required(VERSION 3.10)

# Numele executabilului si limbajele folosite
project(SimulatorMecanica LANGUAGES CXX C)

# Folosim standardul C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Colectam automat toate fisierele sursa (.cpp si .c)
set(PROJECT_SOURCES
    src/main.cpp
    src/sistem.cpp
    src/fizica.cpp
    src/input.cpp
    src/gui.cpp
    src/grafica.cpp
    src/colliziune.cpp
    src/editor.cpp
    src/scena.cpp
    src/rigid.cpp
    src/arc.cpp
    src/legatura.cpp
    src/matrice.cpp
    src/glad.c
)

file(GLOB IMGUI_SOURCES "ImGuiSrc/*.cpp")

# Cream executabilul final
add_executable(simulator ${PROJECT_SOURCES} ${IMGUI_SOURCES})

# Ii spunem compilatorului unde sa caute fisierele header (.h)
target_include_directories(simulator PRIVATE 
    ${CMAKE_SOURCE_DIR}/lib 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/ImGuiSrc
    ${CMAKE_SOURCE_DIR}/ImGuiLib
    ${CMAKE_SOURCE_DIR}/includes
)

# Ii spunem compilatorului unde sa caute fisierele de biblioteca precompilate (.a, .lib)
target_link_directories(simulator PRIVATE ${CMAKE_SOURCE_DIR}/lib)

# Legam bibliotecile de sistem (GLFW si OpenGL pentru Windows/MinGW)
target_link_libraries(simulator PRIVATE glfw3 opengl32)