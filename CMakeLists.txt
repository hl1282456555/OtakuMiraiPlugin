cmake_minimum_required(VERSION 3.15)

project(OtakuMiraiPlugin VERSION 1.0 LANGUAGES CXX)

configure_file(OtakuMiraiPluginConfig.h.in ./Public/OtakuMiraiPluginConfig.h)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCE_PUBLIC
	"${PROJECT_SOURCE_DIR}/Source/Public/*.h"
	"${PROJECT_SOURCE_DIR}/Source/Public/*.hpp"
	)

file(GLOB_RECURSE SOURCE_PRIVATE
	"${PROJECT_SOURCE_DIR}/Source/Private/*.c"
	"${PROJECT_SOURCE_DIR}/Source/Private/*.cpp"
	)

add_library(${PROJECT_NAME} SHARED 
	${SOURCE_PUBLIC} 
	${SOURCE_PRIVATE} 
	)

source_group("/Source/Public" FILES ${SOURCE_PUBLIC})
source_group("/Source/Private" FILES ${SOURCE_PRIVATE})

target_include_directories(${PROJECT_NAME} PUBLIC 
	"${PROJECT_BINARY_DIR}" 
	"${PROJECT_SOURCE_DIR}/Source/Public"
	"${PROJECT_SOURCE_DIR}/Source/Private"
	"${PROJECT_SOURCE_DIR}/ThirdParty/utf8" 
	"${PROJECT_SOURCE_DIR}/ThirdParty/json" 
	"${PROJECT_SOURCE_DIR}/ThirdParty/cpp-httplib"
	)

target_compile_options(${PROJECT_NAME} PUBLIC /EHs /utf-8)