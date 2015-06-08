cmake_minimum_required(VERSION 2.4)

PROJECT(MOTION_DETECTION)

# paths 
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(/usr/local/include)

INCLUDE_DIRECTORIES(
	${TINYXML_DIRECTORY} 	/src/motion_detect/tinyxml 
	${RECOGNITION_DIRECTORY}/src/motion_detect/recognition
	${PROTOBUFF_DIRECTORY}  /src/motion_detect/protobuffer
	${NTP_DIRECTORY}        /src/motion_detect/ntp
)

FIND_PACKAGE(Protobuf REQUIRED)

LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/lib)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/src/motion_detect)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/src/motion_detect)

SET(CMAKE_CXX_FLAGS "-o3 -w")
SET(CMAKE_CXX_LINK_FLAGS "-pg")
SET(OpenCV_LIBRARIES opencv_core opencv_highgui opencv_imgproc opencv_video)
SET(PROTOBUF_LIBRARIES protobuf)

ADD_EXECUTABLE(motion_detect_raspberry 
	src/motion_detect/main.cpp
        src/motion_detect/remotecam.hpp
        src/motion_detect/remotecam.cpp
        src/motion_detect/send_data.hpp
        src/motion_detect/send_data.cpp
        src/motion_detect/practical/PracticalSocket.h
	src/motion_detect/practical/PracticalSocket.cpp
	src/motion_detect/recognition/detection.h
        src/motion_detect/recognition/detection.cpp 
	src/motion_detect/tinyxml/tinystr.cpp
	src/motion_detect/tinyxml/tinyxml.cpp
	src/motion_detect/tinyxml/tinyxmlerror.cpp
	src/motion_detect/tinyxml/tinyxmlparser.cpp
        src/motion_detect/ntp/hwclock.c
	src/motion_detect/protobuffer/motion_protocol.pb.h
	src/motion_detect/protobuffer/motion_protocol.pb.cc
	src/motion_detect/protobuffer/motion.pb.h
	src/motion_detect/protobuffer/motion.pb.cc
)

TARGET_LINK_LIBRARIES(motion_detect_raspberry ${OpenCV_LIBRARIES} -lpthread -lm ${PROTOBUF_LIBRARIES})

SET(CMAKE_BUILD_TYPE Release)

