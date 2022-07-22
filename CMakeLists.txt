cmake_minimum_required(VERSION 3.10)

project(OtakuMiraiPlugin VERSION 1.0)

configure_file(./Public/Config.h.in ./Public/Config.h)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(OtakuMiraiPlugin SHARED 
	"${PROJECT_SOURCE_DIR}/Private/main.cpp"
	"${PROJECT_SOURCE_DIR}/ThirdParty/MiraiCP/MiraiCP.cpp"
	)

target_include_directories(OtakuMiraiPlugin PUBLIC 
	"${PROJECT_BINARY_DIR}" 
	"${PROJECT_SOURCE_DIR}/Public"
	"${PROJECT_SOURCE_DIR}/ThirdParty/MiraiCP"
	"${PROJECT_SOURCE_DIR}/ThirdParty/utf8" 
	"${PROJECT_SOURCE_DIR}/ThirdParty/json" 
	"${PROJECT_SOURCE_DIR}/ThirdParty/cpp-httplib"
	)

#target_link_libraries(OtakuMiraiPlugin PUBLIC )