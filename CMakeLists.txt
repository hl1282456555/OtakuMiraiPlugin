cmake_minimum_required(VERSION 3.15)

option(WITH_HTTP_REQUEST "Should plugin enable http request support?" ON)
option(WITH_OPENSSL "Should plugin enable OpenSSL support?." ON)

project(OtakuMiraiPlugin VERSION 1.0 LANGUAGES CXX)

configure_file(OtakuMiraiPluginConfig.h.in ${PROJECT_SOURCE_DIR}/Source/Public/OtakuMiraiPluginConfig.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCE_FILES ./Source/*.h ./Source/*.hpp ./Source/*.c ./Source/*.cpp)
file(GLOB_RECURSE THIRDPARTY_FILES ./ThirdParty/*.h ./ThirdParty/*.hpp ./ThirdParty/*.c ./ThirdParty/*.cpp)

add_library(${PROJECT_NAME} SHARED  
	${SOURCE_FILES}
	${THIRDPARTY_FILES}
	)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${THIRDPARTY_FILES})

if (WITH_HTTP_REQUEST)
	include(FetchContent)
	FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
	FetchContent_MakeAvailable(cpr)
	add_definitions(-DWITH_HTTP_REQUEST)
	target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)
endif()

if (WITH_OPENSSL)
	find_package(OpenSSL REQUIRED)
	add_definitions(-DWITH_OPENSSL)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
endif()

set(PROJECT_PATHS)
file(GLOB_RECURSE PROJECT_ALL_FILES ./Source/*.* ./ThirdParty/*.*)

foreach(CURRENT_PROJECT_FILE ${PROJECT_ALL_FILES})
	if (NOT IS_DIRECTORY ${CURRENT_PROJECT_FILE})
		get_filename_component(TEMP_FILE_PATH ${CURRENT_PROJECT_FILE} DIRECTORY)
		list(APPEND PROJECT_PATHS ${TEMP_FILE_PATH})
	endif()
endforeach()

list(REMOVE_DUPLICATES PROJECT_PATHS)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_PATHS}")

target_compile_options(${PROJECT_NAME} PUBLIC /EHs /utf-8)
