cmake_minimum_required(VERSION 3.16)
project(SimulatorMecanica LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "src/*.cpp" "src/*.c")
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS "ImGuiSrc/*.cpp")
file(GLOB_RECURSE VENDOR_SOURCES CONFIGURE_DEPENDS "vendor/*.cpp" "vendor/*.c")

# Pe Emscripten, excludem Glad complet (WebGL nu are nevoie de el)
if(EMSCRIPTEN)
    list(FILTER VENDOR_SOURCES EXCLUDE REGEX ".*glad.*")
    list(FILTER PROJECT_SOURCES EXCLUDE REGEX ".*glad.*")
endif()

if(VENDOR_SOURCES OR IMGUI_SOURCES)
    list(REMOVE_ITEM PROJECT_SOURCES ${VENDOR_SOURCES} ${IMGUI_SOURCES})
endif()

# Flaguri comune Emscripten - folosite PESTE TOT
if(EMSCRIPTEN)
    set(EM_COMPILE_FLAGS
        "-sUSE_GLFW=3"
        "-sUSE_WEBGL2=1"
    )
    set(EM_LINK_FLAGS
        "-sUSE_GLFW=3"
        "-sUSE_WEBGL2=1"
        "-sFULL_ES3=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sASSERTIONS=2"
        "-sSTACK_SIZE=1mb"
            "SHELL:--shell-file ${CMAKE_SOURCE_DIR}/src/shell.html"  
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/exemple@/exemple"
        "SHELL:--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
    )
endif()

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

target_compile_options(vendor PRIVATE -w)

if(EMSCRIPTEN)
    target_compile_options(vendor PUBLIC ${EM_COMPILE_FLAGS})
    # ACUM dupa add_library — corect
    target_compile_definitions(vendor PUBLIC IMGUI_IMPL_OPENGL_ES3)
endif()

target_compile_options(vendor PRIVATE -w)

if(WIN32 AND NOT EMSCRIPTEN)
    add_executable(simulator WIN32 ${PROJECT_SOURCES})
else()
    add_executable(simulator ${PROJECT_SOURCES})
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(simulator PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffast-math>
    )
    if(NOT EMSCRIPTEN)
        target_compile_options(simulator PRIVATE $<$<CONFIG:Release>:-march=native>)
    endif()
endif()

target_precompile_headers(simulator PRIVATE <vector> <cmath> <iostream> <string>)

target_include_directories(simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/fizica
    ${CMAKE_SOURCE_DIR}/src/render
    ${CMAKE_SOURCE_DIR}/src/mate
)

target_link_directories(simulator PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/vendor/glfw
)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    target_compile_options(simulator PRIVATE ${EM_COMPILE_FLAGS})
    target_link_options(simulator PRIVATE ${EM_LINK_FLAGS})
    target_link_libraries(simulator PRIVATE vendor)
else()
    target_link_libraries(simulator PRIVATE vendor glfw3 opengl32)
endif()