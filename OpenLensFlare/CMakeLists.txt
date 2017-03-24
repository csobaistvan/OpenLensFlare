# Set the minimum version
cmake_minimum_required(VERSION 3.0)

# Declare the project
project(OpenLensFlare LANGUAGES CXX)

# Set the CMake module path for find scripts
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")

# Target properties
set(OLEF_TARGET_NAME "OpenLensFlare")
set(OLEF_VERSION_MAJOR 1)
set(OLEF_VERSION_MINOR 0)
set(OLEF_VERSION_PATCH 0)
set(OLEF_VERSION ${OLEF_VERSION_MAJOR}.${OLEF_VERSION_MINOR}.${OLEF_VERSION_PATCH})

set(OLEF_INSTALL_FOLDER ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Install folder for generated targets")
set(OLEF_HEADER_INSTALL_FOLDER ${OLEF_INSTALL_FOLDER}/include)
set(OLEF_BINARY_INSTALL_FOLDER ${OLEF_INSTALL_FOLDER}/lib)

# Look for the required libraries
find_package(GLM REQUIRED)
find_package(GLEW REQUIRED)

# Add GLM's include directory
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${GLEW_INCLUDE_DIR})

# Enumerate the various source files
file(GLOB_RECURSE OLEF_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE OLEF_SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE OLEF_SHADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.glsl)

# Generate source files out of the shader files
foreach(SHADER_FILE_NAME IN LISTS OLEF_SHADER_FILES)
    # Get the base file name component
    get_filename_component(SHADER_BASE_NAME "${SHADER_FILE_NAME}" NAME)

    # Generate the output file names
    set(GENERATED_SHADER_HEADER "${CMAKE_CURRENT_BINARY_DIR}/Generated/${SHADER_BASE_NAME}.h")
    set(GENERATED_SHADER_SOURCE "${CMAKE_CURRENT_BINARY_DIR}/Generated/${SHADER_BASE_NAME}.cpp")

    # Generate the header file
    add_custom_command(OUTPUT ${GENERATED_SHADER_HEADER} ${GENERATED_SHADER_SOURCE}
        COMMAND ${CMAKE_COMMAND}
            "-DINPUT_FILE_NAME:Path=${SHADER_FILE_NAME}"
            "-DOUTPUT_HEADER_NAME:Path=${GENERATED_SHADER_HEADER}"
            "-DOUTPUT_SOURCE_NAME:Path=${GENERATED_SHADER_SOURCE}" 
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShader.cmake"
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cmake/EmbedShader.cmake ${SHADER_FILE_NAME}
    )

    # Append the generated files to the appropriate source file lists
    list(APPEND OLEF_GENERATED_SHADER_FILES ${GENERATED_SHADER_HEADER} ${GENERATED_SHADER_SOURCE})
endforeach()

# Append the shader output folder as an include dir
include_directories(${CMAKE_CURRENT_BINARY_DIR}/Generated/)

# Declare the library target
add_library(${OLEF_TARGET_NAME} 
    ${OLEF_HEADER_FILES} ${OLEF_SOURCE_FILES}
    ${OLEF_GENERATED_SHADER_FILES}
)

# Link to the required libraries
target_link_libraries(${OLEF_TARGET_NAME} ${GLEW_LIBRARY})

# Configure the installed files
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/
    DESTINATION ${OLEF_HEADER_INSTALL_FOLDER}/OpenLensFlare/
    FILES_MATCHING PATTERN *.h)

install(TARGETS ${OLEF_TARGET_NAME}
    LIBRARY DESTINATION ${OLEF_BINARY_INSTALL_FOLDER}
    ARCHIVE DESTINATION ${OLEF_BINARY_INSTALL_FOLDER}
)