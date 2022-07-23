cmake_minimum_required(VERSION 3.15)

project(OtakuMiraiPlugin VERSION 1.0 LANGUAGES CXX)

configure_file(OtakuMiraiPluginConfig.h.in ${PROJECT_SOURCE_DIR}/Source/Public/OtakuMiraiPluginConfig.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCE_FILES ./Source/*.*)
file(GLOB_RECURSE THIRDPARTY_FILES ./ThirdParty/*.*)

add_library(${PROJECT_NAME} SHARED  
	${SOURCE_FILES}
	${THIRDPARTY_FILES}
	)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${THIRDPARTY_FILES})

set(PROJECT_PATHS)
file(GLOB_RECURSE PROJECT_ALL_FILES ./Source/*.* ./ThirdParty/*.*)

foreach(CURRENT_PROJECT_FILE ${PROJECT_ALL_FILES})
	if (NOT IS_DIRECTORY ${CURRENT_PROJECT_FILE})
		get_filename_component(TEMP_FILE_PATH ${CURRENT_PROJECT_FILE} DIRECTORY)
		list(APPEND PROJECT_PATHS ${TEMP_FILE_PATH})
	endif()
endforeach()

list(REMOVE_DUPLICATES PROJECT_PATHS)

target_include_directories(${PROJECT_NAME} PUBLIC 
	"${PROJECT_PATHS}"
	)

target_compile_options(${PROJECT_NAME} PUBLIC /EHs /utf-8)