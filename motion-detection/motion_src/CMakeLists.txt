cmake_minimum_required(VERSION 2.4)

PROJECT(MOTION_DETECTION)

# paths 
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(/usr/local/include)

INCLUDE_DIRECTORIES(
	${TINYXML_DIRECTORY} 	/src/motion_detect/tinyxml 
	${RECOGNITION_DIRECTORY}/src/motion_detect/recognition
	${PROTOBUFF_DIRECTORY}  /src/motion_detect/protobuffer
	${STREAMING_DIRECTORY}  /src/motion_detect/streaming	
	${SOCKET_DIRECTORY}  	/src/motion_detect/practical
	${NTP_DIRECTORY}        /src/motion_detect/ntp
	${BASE64_DIRECTORY}     /src/motion_detect/b64
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
        src/motion_detect/streaming/remotecam.hpp
        src/motion_detect/streaming/remotecam.cpp
        src/motion_detect/streaming/send_data.hpp
        src/motion_detect/streaming/send_data.cpp
        src/motion_detect/practical/PracticalSocket.h
	src/motion_detect/practical/PracticalSocket.cpp
	src/motion_detect/practical/sendmessage.h
	src/motion_detect/practical/sendmessage.cpp
	src/motion_detect/recognition/detection.h
        src/motion_detect/recognition/detection.cpp 
	src/motion_detect/tinyxml/tinystr.cpp
	src/motion_detect/tinyxml/tinyxml.cpp
	src/motion_detect/tinyxml/tinyxmlerror.cpp
	src/motion_detect/tinyxml/tinyxmlparser.cpp
        src/motion_detect/ntp/hwclock.c
	src/motion_detect/protobuffer/motion.pb.h
	src/motion_detect/protobuffer/motion.pb.cc
	src/motion_detect/b64/cdecode.c
	src/motion_detect/b64/cdecode.h
	src/motion_detect/b64/cencode.c
	src/motion_detect/b64/cencode.h
	src/motion_detect/b64/decode.h
	src/motion_detect/b64/encode.h
	src/motion_detect/b64/base64.cpp
	src/motion_detect/b64/base64.h
)

TARGET_LINK_LIBRARIES(motion_detect_raspberry ${OpenCV_LIBRARIES} -lpthread -lm ${PROTOBUF_LIBRARIES})

SET(CMAKE_BUILD_TYPE Release)

