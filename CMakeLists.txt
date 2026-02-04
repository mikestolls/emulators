cmake_minimum_required(VERSION 3.24)

project(emulators
  LANGUAGES CXX
  VERSION 1.0
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../_build/$<CONFIG>)

# Build SFML
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/sfml)

# Build ImGuiFileDialog
set(IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/imgui)
add_subdirectory(lib/imguifiledialog)
target_include_directories(ImGuiFileDialog PRIVATE ${IMGUI_INCLUDE_DIR})

# gorup the external libs together
set_target_properties(ImGuiFileDialog PROPERTIES FOLDER "External Libraries")
set_target_properties(sfml-graphics sfml-window sfml-system sfml-audio sfml-network sfml-main 
    PROPERTIES FOLDER "External Libraries/SFML")

# Include directories
include_directories(include)
include_directories(lib/argparse)
include_directories(lib/imgui)
include_directories(lib/imgui-sfml)

# Collect all files
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")
file(GLOB_RECURSE DATA_FILES "data/*")
file(GLOB_RECURSE MAKE_FILES "make/*")

# Preserve folder structure using TREE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES} ${DATA_FILES} ${MAKE_FILES})

# Create executable
add_executable(emulators ${SOURCES})
target_sources(emulators PRIVATE ${DATA_FILES} ${MAKE_FILES})

# Set as startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT emulators)

# Set debugger working directory
set_target_properties(emulators PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/data"
)

# Add ImGui sources
set(IMGUI_SOURCES
    lib/imgui/imgui.cpp
    lib/imgui/imgui_demo.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui-sfml/imgui-SFML.cpp
)
target_sources(emulators PRIVATE ${IMGUI_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${IMGUI_SOURCES})

# Link libraries
target_link_libraries(emulators
    SFML::Graphics
    SFML::Window
    SFML::System
    ImGuiFileDialog
)

target_include_directories(emulators PRIVATE src)

# Enable Edit and Continue for MSVC in Debug mode
if(MSVC)
    # For Debug configuration
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /ZI")
    
    # Remove incompatible flags
    string(REPLACE "/Zi" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/Zi" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    
    # Set linker flags for Edit and Continue
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /INCREMENTAL")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /INCREMENTAL")
    
    # Disable optimization for Debug
    string(REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/O2" "/Od" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
endif()
