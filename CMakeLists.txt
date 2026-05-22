cmake_minimum_required(VERSION 3.16)

# Numele executabilului si limbajele folosite
project(SimulatorMecanica LANGUAGES CXX C)

# Folosim standardul C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Găsim recursiv toate fișierele sursă pentru executabil
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")

# Colectăm sursele externe (vendor, ImGui, etc.) separat
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS "ImGuiSrc/*.cpp")
file(GLOB_RECURSE VENDOR_SOURCES CONFIGURE_DEPENDS "vendor/*.cpp" "vendor/*.c")

# Ne asigurăm că sursele externe nu sunt compilate de două ori în proiectul principal
if(VENDOR_SOURCES OR IMGUI_SOURCES)
    list(REMOVE_ITEM PROJECT_SOURCES ${VENDOR_SOURCES} ${IMGUI_SOURCES})
endif()

# --- OPTIMIZARE 1: Bibliotecă statică pentru codul extern ---
# ImGui, ImPlot si Glad sunt separate in target-ul "vendor".
# Ele vor fi compilate o singura data, iar apoi refolosite.
add_library(vendor STATIC 
    ${IMGUI_SOURCES}
    ${VENDOR_SOURCES}
)
target_include_directories(vendor PUBLIC 

    ${CMAKE_SOURCE_DIR}/vendor
    ${CMAKE_SOURCE_DIR}/vendor/glad
    ${CMAKE_SOURCE_DIR}/vendor/glfw
    ${CMAKE_SOURCE_DIR}/vendor/imgui
    ${CMAKE_SOURCE_DIR}/vendor/implot
    ${CMAKE_SOURCE_DIR}/vendor/json
    ${CMAKE_SOURCE_DIR}/vendor/KHR
    ${CMAKE_SOURCE_DIR}/vendor/pfd
    ${CMAKE_SOURCE_DIR}/vendor/stb
)

# Cream executabilul final
add_executable(simulator WIN32 ${PROJECT_SOURCES})

# --- OPTIMIZARE 3: Flag-uri de avertizare (Warnings) ---
# Ne ajută să scriem cod mai sigur și previn bug-uri subtile.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simulator PRIVATE ) #-Wall -Wextra
    target_compile_options(vendor PRIVATE -w) # Oprim warning-urile pentru librariile externe

    target_compile_options(simulator PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffast-math>
        $<$<CONFIG:Release>:-march=native>
    ) # <--- Această paranteză lipsea
endif()

# --- OPTIMIZARE 2: Precompiled Headers ---
# Compilatorul analizeaza aceste biblioteci o singura data pentru intreg proiectul
target_precompile_headers(simulator PRIVATE <vector> <cmath> <iostream> <string>)

# Ii spunem compilatorului unde sa caute fisierele header (.h)
target_include_directories(simulator PRIVATE 
    ${CMAKE_SOURCE_DIR}/src 
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/fizica
    ${CMAKE_SOURCE_DIR}/src/render
    ${CMAKE_SOURCE_DIR}/src/mate
)

# Ii spunem compilatorului unde sa caute fisierele de biblioteca precompilate (.a, .lib)
target_link_directories(simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/lib,
    ${CMAKE_SOURCE_DIR}/vendor/glfw
)

# Legam biblioteca de "vendor" impreuna cu bibliotecile de sistem
target_link_libraries(simulator PRIVATE vendor glfw3 opengl32)
