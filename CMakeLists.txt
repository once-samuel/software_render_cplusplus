cmake_minimum_required(VERSION 3.8)

project("render_cplusplus")

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5Gui_INCLUDE_DIRS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

include_directories("./window")
include_directories("./core")
include_directories("./core/common")
include_directories("./core/mathematics/geometry")
include_directories("./core/mathematics/linear")
include_directories("./core/pipeline/font")
include_directories("./core/pipeline/light")
include_directories("./core/pipeline/mesh")
include_directories("./core/pipeline/texture")

file(
	GLOB_RECURSE
	cpp_file
	./core/*.cpp
	./enter/*.cpp
	./window/*.cpp)

add_executable(
   ${PROJECT_NAME}
   ${cpp_file})

target_link_libraries(
	${PROJECT_NAME}
	${Qt5Core_LIBRARIES}
	${Qt5Gui_LIBRARIES}
	${Qt5Widgets_LIBRARIES})
	
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(CMAKE_C_FLAGS "/utf-8 ${CMAKE_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}")
endif ()
