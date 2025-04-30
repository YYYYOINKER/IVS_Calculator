cmake_minimum_required(VERSION 3.10)
project(Kalkulacka)

set(CMAKE_CXX_STANDARD 23)

if(INSTALL_TYPE STREQUAL "app")
	set(PROJECT_NAME "Kalkulacka")
	add_compile_definitions(ASSET_PATH="/usr/share/${PROJECT_NAME}/")

	# Add source files
	file(GLOB_RECURSE SRC_FILES
	    src/main_gui.cpp
    	src/*.h
		src/include/*.cc
		src/include/*.h
		src/src/glad.c
		src/src/TextRenderer.cpp
		src/src/mathlibrary.cpp
	)

	add_executable(${PROJECT_NAME} ${SRC_FILES})

	# After add_executable(${PROJECT_NAME} ${SRC_FILES})
	target_include_directories(${PROJECT_NAME} PRIVATE
	    ${CMAKE_SOURCE_DIR}/src/include
		/usr/include/freetype2
		${CMAKE_SOURCE_DIR}/src/src
	)

	# Link OpenGL and required libraries
	find_package(OpenGL REQUIRED)
	find_package(GLUT REQUIRED)
	find_package(Freetype REQUIRED)


	target_link_libraries(${PROJECT_NAME}
	    ${OPENGL_LIBRARIES}
	    ${GLFW_LIBRARIES}
	    ${FREETYPE_LIBRARIES}
	    glfw
	)

	# Install binary
	install(TARGETS ${PROJECT_NAME} DESTINATION bin)

	install(DIRECTORY src/shaders DESTINATION share/${PROJECT_NAME})
	install(DIRECTORY src/fonts DESTINATION share/${PROJECT_NAME})
	install(DIRECTORY src/objects DESTINATION share/${PROJECT_NAME})
	install(DIRECTORY src/textures DESTINATION share/${PROJECT_NAME})
	install(FILES ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.desktop DESTINATION /usr/share/applications)

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
	        install(FILES "${CMAKE_SOURCE_DIR}/src/${f}" DESTINATION "share/${PROJECT_NAME}/assets/${dir}")
	    endif()
	endforeach()
	
	# Packaging setup
	include(InstallRequiredSystemLibraries)
	set(CPACK_GENERATOR "DEB")
	set(CPACK_PACKAGE_NAME "kalkulacka")
	set(CPACK_PACKAGE_VERSION "1.0.0")
	set(CPACK_PACKAGE_CONTACT "xdurkal00")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1, libglu1-mesa, freeglut3-dev, libglfw3")  # or glfw3, etc.
	set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/postinst;")

	# List of directories to copy
	set(ASSET_DIRECTORIES 
	    "shaders"
	    "fonts"
	    "objects"
	    "textures"
	)

	# Loop through and copy each directory
	foreach(ASSET_DIR ${ASSET_DIRECTORIES})
	    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	        COMMAND ${CMAKE_COMMAND} -E copy_directory
	        ${CMAKE_SOURCE_DIR}/src/${ASSET_DIR}
	        ${CMAKE_BINARY_DIR}/${ASSET_DIR}
	        COMMENT "Copying ${ASSET_DIR} folder from src to build/src"
	    )
	endforeach()
elseif(INSTALL_TYPE STREQUAL "stddev")
	set(PROJECT_NAME "stddev")
	
	# Add source files
	file(GLOB_RECURSE SRC_FILES
		src/profiling.cpp
		src/include/mathlibrary.h
		src/src/mathlibrary.cpp
	)

	add_executable(${PROJECT_NAME} ${SRC_FILES})

	install(TARGETS ${PROJECT_NAME} DESTINATION bin)
	include(InstallRequiredSystemLibraries)
	set(CPACK_GENERATOR "DEB")
	set(CPACK_PACKAGE_NAME "stddev")
	set(CPACK_PACKAGE_VERSION "1.0.0")
	set(CPACK_PACKAGE_CONTACT "xdurkal00")
	set(CPACK_DEBIAN_PACKAGE_DEPENDS "binutils, valgrind")
		
endif()

include(CPack)

