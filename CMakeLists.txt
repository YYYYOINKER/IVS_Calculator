cmake_minimum_required(VERSION 3.10)
project(Kalkulacka)

set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(ASSET_PATH="/usr/share/Kalkulacka/")

# Add source files
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*.h
	src/include/*.cc
	src/include/*.h
	src/src/glad.c
	src/src/TextRenderer.cpp
)

add_executable(Kalkulacka ${SRC_FILES})

# After add_executable(Kalkulacka ${SRC_FILES})
target_include_directories(Kalkulacka PRIVATE
    ${CMAKE_SOURCE_DIR}/src/include
	/usr/include/freetype2
	${CMAKE_SOURCE_DIR}/src/src
)

# Link OpenGL and required libraries

# Link OpenGL
find_package(OpenGL REQUIRED)

# Link GLAD (assuming you already added glad.c)

# Link GLFW (if you're using it)

# Link OpenGL and required libraries
find_package(GLUT REQUIRED)  # Or GLFW, etc.


# Link FreeType (if you're using it)
find_package(Freetype REQUIRED)

# Link math library (if needed)


target_link_libraries(Kalkulacka
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    glfw
)
# Install binary
install(TARGETS Kalkulacka DESTINATION bin)

install(DIRECTORY src/shaders DESTINATION share/Kalkulacka)
install(DIRECTORY src/fonts DESTINATION share/Kalkulacka)
install(DIRECTORY src/objects DESTINATION share/Kalkulacka)
install(DIRECTORY src/textures DESTINATION share/Kalkulacka)

# Install assets: filter non-source files in src/
file(GLOB_RECURSE ASSET_FILES
    RELATIVE "${CMAKE_SOURCE_DIR}/src"
    src/*.* 
)

# Filter out .cpp and .h files
set(FILTERED_ASSETS "")
foreach(f ${ASSET_FILES})
    if(NOT f MATCHES "\\.(cpp|h)$"  AND NOT f MATCHES ".*/profiling\\.cpp$")
        list(APPEND FILTERED_ASSETS "${CMAKE_SOURCE_DIR}/src/${f}")
        # Also install each file preserving relative path
        get_filename_component(dir "${f}" DIRECTORY)
        install(FILES "${CMAKE_SOURCE_DIR}/src/${f}" DESTINATION "share/Kalkulacka/assets/${dir}")
    endif()
endforeach()

# Packaging setup
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "kalkulacka")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_CONTACT "me")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1, libglu1-mesa, freeglut3-dev, libglfw3")  # or glfw3, etc.

# List of directories to copy
set(ASSET_DIRECTORIES 
    "shaders"
    "fonts"
    "objects"
    "textures"
)

# Loop through and copy each directory
foreach(ASSET_DIR ${ASSET_DIRECTORIES})
    add_custom_command(TARGET Kalkulacka POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/${ASSET_DIR}
        ${CMAKE_BINARY_DIR}/${ASSET_DIR}
        COMMENT "Copying ${ASSET_DIR} folder from src to build/src"
    )
endforeach()


include(CPack)

