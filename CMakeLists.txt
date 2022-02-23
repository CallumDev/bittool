cmake_minimum_required (VERSION 3.1)
project(bittool)

set(OUTPUT_NAME "${CMAKE_BINARY_DIR}/generated/lang.h")
set(INPUT_NAME "${CMAKE_CURRENT_SOURCE_DIR}/lang.csv")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

add_custom_command(
  OUTPUT "${OUTPUT_NAME}"
  DEPENDS "${INPUT_NAME}"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/gen_lang.py"
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/gen_lang.py" "${INPUT_NAME}" "${OUTPUT_NAME}"
)

set_source_files_properties(${OUTPUT_NAME} PROPERTIES
  GENERATED TRUE)
add_custom_target(bittool_lang DEPENDS "${OUTPUT_NAME}")

add_executable(bittool
    main.cpp
    DroidSansFallback.cpp
    ini.c
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

add_dependencies(bittool bittool_lang)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    install(TARGETS bittool DESTINATION bin)
    install(FILES unix/bittool.desktop DESTINATION share/applications)
    install(FILES unix/bittool.svg DESTINATION share/icons/hicolor/scalable/apps)
endif()

find_package(Freetype)

if(Freetype_FOUND)
    target_sources(bittool PRIVATE imgui/misc/freetype/imgui_freetype.cpp)
    target_link_libraries(bittool PRIVATE ${FREETYPE_LIBRARIES})
    target_include_directories(bittool PRIVATE ${FREETYPE_INCLUDE_DIRS})
    target_compile_definitions(bittool PRIVATE -DIMGUI_ENABLE_FREETYPE)
    message(STATUS "bittool: compiling using freetype rasterizer")
else()
    message(STATUS "bittool: compiling using stb_truetype rasterizer")
endif()

target_include_directories(bittool PRIVATE "imgui" "imgui/backends" "imgui/misc/freetype" "${CMAKE_BINARY_DIR}/generated")

# SDL2 Dependency
if (DEFINED SDL2_INCLUDE_DIRS AND DEFINED SDL2_LIBRARIES)
	message(STATUS "bittool: using pre-defined SDL2 variables SDL2_INCLUDE_DIRS and SDL2_LIBRARIES")
	target_include_directories(bittool PRIVATE "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
	target_link_libraries(bittool PRIVATE ${SDL2_LIBRARIES})
else()
	# Only try to autodetect if both SDL2 variables aren't explicitly set
	find_package(SDL2 REQUIRED)
	if (TARGET SDL2::SDL2)
		message(STATUS "bittool: using TARGET SDL2::SDL2")
		target_link_libraries(bittool PRIVATE SDL2::SDL2)
	elseif (TARGET SDL2)
		message(STATUS "bittool: using TARGET SDL2")
		target_link_libraries(bittool PRIVATE SDL2)
	else()
		message(STATUS "bittool: no TARGET SDL2::SDL2, or SDL2, using variables")
		target_include_directories(testapp PRIVATE "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
        target_link_libraries(bittool PRIVATE ${SDL2_LIBRARIES})
	endif()
endif()
