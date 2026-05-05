cmake_minimum_required(VERSION 3.19)

project(LevelUtilityTool LANGUAGES CXX)

set(vendor_name Vamps)
set(target_name LevelUtilityTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UNIGINE_DOUBLE "Build the plugin for the double-precision Unigine editor." ON)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/local_paths.cmake)
	include(${CMAKE_CURRENT_SOURCE_DIR}/local_paths.cmake)
else()
	if(NOT DEFINED UNIGINE_SDK_DIR)
		set(UNIGINE_SDK_DIR "C:/Users/snare/AppData/Local/unigine/browser/sdks/sim_windows_2.18.1_bin")
	endif()
	if(NOT DEFINED QT5_DIR)
		set(QT5_DIR "C:/Qt/Qt5.12.3/5.12.3/msvc2017_64")
	endif()
	if(NOT DEFINED DEPLOY_BASE_DIR)
		set(DEPLOY_BASE_DIR "C:/Users/snare/Documents/UNIGINE Projects/unigine_project_3")
	endif()
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

if(NOT DEFINED QT5_DIR AND DEFINED QT_DIR)
	set(QT5_DIR ${QT_DIR})
endif()

string(REPLACE "\\" "/" QT5_DIR "${QT5_DIR}")
set(CMAKE_PREFIX_PATH "${QT5_DIR}")
set(Qt5_DIR "${QT5_DIR}/lib/cmake/Qt5")

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

if(NOT DEFINED DEPLOY_DIR)
	set(DEPLOY_DIR "${DEPLOY_BASE_DIR}/bin/plugins/${vendor_name}/${target_name}")
endif()

set(UNIGINE_INCLUDE_DIR "${UNIGINE_SDK_DIR}/include")
set(UNIGINE_LIB_DIR "${UNIGINE_SDK_DIR}/lib")

if(NOT EXISTS "${UNIGINE_INCLUDE_DIR}")
	message(FATAL_ERROR "Unigine include directory was not found: ${UNIGINE_INCLUDE_DIR}")
endif()

set(binary_name ${target_name}_editorplugin)
if(UNIGINE_DOUBLE)
	string(APPEND binary_name "_double")
endif()
string(APPEND binary_name "_x64")

set(SOURCES
	source/core/LevelUtilityToolEditorPlugin.cpp
	source/processing/NodeGenerationService.cpp
	source/ui/LevelUtilityToolPanel.cpp
)

set(HEADERS
	source/core/LevelUtilityToolEditorPlugin.h
	source/processing/NodeGenerationService.h
	source/ui/LevelUtilityToolPanel.h
)

add_library(${target_name} SHARED ${SOURCES} ${HEADERS})

target_include_directories(${target_name}
	PRIVATE
	${UNIGINE_INCLUDE_DIR}
	${UNIGINE_INCLUDE_DIR}/editor
	${CMAKE_CURRENT_SOURCE_DIR}/source
	${CMAKE_CURRENT_SOURCE_DIR}/source/core
	${CMAKE_CURRENT_SOURCE_DIR}/source/processing
	${CMAKE_CURRENT_SOURCE_DIR}/source/ui
)

target_compile_definitions(${target_name}
	PRIVATE
	QT_NO_DEBUG
	_CRT_SECURE_NO_WARNINGS
	$<$<BOOL:${UNIGINE_DOUBLE}>:UNIGINE_DOUBLE>
)

if(MSVC)
	include(ProcessorCount)
	ProcessorCount(proc_count)
	if(NOT proc_count)
		set(proc_count 4)
	endif()
	target_compile_options(${target_name}
		PRIVATE
		/TP
		/FS
		/fp:fast
		/bigobj
		/EHsc
		/MP${proc_count}
	)
	target_link_options(${target_name} PRIVATE /FIXED:NO)
endif()

set(unigine_precision_suffix "")
if(UNIGINE_DOUBLE)
	set(unigine_precision_suffix "_double")
endif()

target_link_libraries(${target_name}
	Shlwapi.lib
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	"${UNIGINE_LIB_DIR}/EditorCore${unigine_precision_suffix}_x64.lib"
	"${UNIGINE_LIB_DIR}/Unigine${unigine_precision_suffix}_x64.lib"
)

set_target_properties(${target_name} PROPERTIES
	OUTPUT_NAME ${binary_name}$<$<CONFIG:Debug>:d>
	RUNTIME_OUTPUT_DIRECTORY_RELEASE "${DEPLOY_DIR}"
	RUNTIME_OUTPUT_DIRECTORY_DEBUG "${DEPLOY_DIR}"
)

add_custom_command(TARGET ${target_name} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory "${DEPLOY_DIR}"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_CURRENT_SOURCE_DIR}/source/LevelUtilityTool.json"
	"${DEPLOY_DIR}/LevelUtilityTool.json"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_CURRENT_SOURCE_DIR}/plugin.json"
	"${DEPLOY_DIR}/plugin.json"
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${CMAKE_CURRENT_SOURCE_DIR}/LevelUtilityTool.plugin"
	"${DEPLOY_DIR}/LevelUtilityTool.plugin"
)
