cmake_minimum_required (VERSION 3.1)
project(bittool)




add_executable(bittool
    main.cpp
    ini.c
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)
    
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

target_include_directories(bittool PRIVATE "imgui" "imgui/backends" "imgui/misc/freetype")

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
