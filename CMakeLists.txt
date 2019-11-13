cmake_minimum_required(VERSION 3.0)

IF(arch STREQUAL "arm")
include(armHeader.txt)
message("using arm926 compiler")
else()
include(x86Header.txt)
message("using x86 compiler")
ENDIF()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

PROJECT(PROJECT_ONE)

include_directories(${PROJECT_SOURCE_DIR})


#主目录编译文件
add_executable(serial2tun main.cpp serial.cpp tundev.cpp serial_protol.cpp)


target_link_libraries(serial2tun pthread)

#imageProcess
#add_subdirectory(imageProcess)
#target_link_libraries(main imageProcess)
#errHandle
#add_subdirectory(errHandle)
#target_link_libraries(main errHandle)
