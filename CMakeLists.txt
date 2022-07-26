cmake_minimum_required(VERSION 3.15)

option(WITH_HTTP_REQUEST "Should plugin enable http request support?" ON)
option(WITH_OPENSSL "Should plugin enable OpenSSL support?." ON)

project(OtakuMiraiPlugin VERSION 1.0 LANGUAGES CXX)

configure_file(OtakuMiraiPluginConfig.h.in ${PROJECT_SOURCE_DIR}/Source/Public/OtakuMiraiPluginConfig.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(Boost_USE_RELEASE_LIBS       ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME 	OFF)
set(Boost_USE_STATIC_LIBS 		OFF)

find_package(Boost 1.79.0 REQUIRED COMPONENTS random)

file(GLOB_RECURSE SOURCE_FILES ./Source/*.h ./Source/*.hpp ./Source/*.c ./Source/*.cpp)
file(GLOB_RECURSE THIRDPARTY_FILES ./ThirdParty/*.h ./ThirdParty/*.hpp ./ThirdParty/*.c ./ThirdParty/*.cpp)

add_library(${PROJECT_NAME} SHARED  
	${SOURCE_FILES}
	${THIRDPARTY_FILES}
	)

source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${THIRDPARTY_FILES})

# Add boost support
if (Boost_FOUND)
	message("Boost VERSION: ${Boost_VERSION}")
    message("Boost INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message("Boost Boost_LIBRARY_DIRS: ${Boost_LIBRARY_DIRS}")

	target_link_libraries(${PROJECT_NAME} PUBLIC 
		Boost::headers 
		Boost::diagnostic_definitions 
		Boost::dynamic_linking
		Boost::random
		)
endif()

# Add TextTable multibyte support
add_definitions(-DTEXTTABLE_ENCODE_MULTIBYTE_STRINGS)

# Add http request ability support
if (WITH_HTTP_REQUEST)
	include(FetchContent)
	FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
	FetchContent_MakeAvailable(cpr)
	add_definitions(-DWITH_HTTP_REQUEST)
	target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)
endif()

# Add openssl support
if (WITH_OPENSSL)
	find_package(OpenSSL REQUIRED)
	add_definitions(-DWITH_OPENSSL)
	target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
endif()

# Add MySQL-connector-cpp support, we used cxx17, so we should define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING to disable std::iterator warning
add_definitions(-DWITH_MYSQL -D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING -DSTATIC_CONCPP)

if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/ThirdParty/mysql-connector-c++/include")
	target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/ThirdParty/mysql-connector-c++/lib/Win64/mysqlcppconn8.lib")
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_include_directories(${PROJECT_NAME} PUBLIC "/usr/include/mysql-cppconn-8/")
	target_link_libraries(${PROJECT_NAME} PRIVATE "/usr/lib/x86_64-linux-gnu/libmysqlcppconn8.so")
endif()

# Add utf8 charset translation support
add_definitions(-DWITH_UTF8_TRANSLATION)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/ThirdParty/utf8")

# Add json support
add_definitions(-DWITH_JSON)
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/ThirdParty/json")

# Collect source files from project dir.
set(PROJECT_SOURCE_PATHS)
file(GLOB_RECURSE PROJECT_SOURCE_FILES ./Source/*.*)

foreach(CURRENT_PROJECT_FILE ${PROJECT_SOURCE_FILES})
	if (NOT IS_DIRECTORY ${CURRENT_PROJECT_FILE})
		get_filename_component(TEMP_FILE_PATH ${CURRENT_PROJECT_FILE} DIRECTORY)
		list(APPEND PROJECT_SOURCE_PATHS ${TEMP_FILE_PATH})
	endif()
endforeach()

list(REMOVE_DUPLICATES PROJECT_SOURCE_PATHS)

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_PATHS}")

if (MSVC)
target_compile_options(${PROJECT_NAME} PUBLIC /EHs /utf-8)
endif()