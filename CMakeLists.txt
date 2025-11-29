cmake_minimum_required(VERSION 3.11)
project(HydroSim VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)

# --- 1. Descargar e integrar Raylib automáticamente ---
include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master
)
FetchContent_MakeAvailable(raylib)

# --- 2. Definir archivos fuente ---
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
# Asegúrate de que estos archivos existan en tu carpeta src/
set(SOURCES 
    src/main.c 
    src/solver.c 
    src/renderer.c
    src/analysis.c  
)

# --- 3. Crear ejecutable ---
add_executable(${PROJECT_NAME} ${SOURCES})

# --- 4. Enlazar librerías (Linker) ---
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Ajustes para Windows
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE winmm gdi32)
endif()

# Copiar assets si existieran
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})