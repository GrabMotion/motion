cmake_minimum_required(VERSION 2.4)

PROJECT(MOTION_DETECTION)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

# paths 
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(/usr/local/include)
INCLUDE_DIRECTORIES(/cedric/include)

INCLUDE_DIRECTORIES(
	${TINYXML_DIRECTORY} 	/src/motion_detect/tinyxml 
	${RECOGNITION_DIRECTORY}/src/motion_detect/recognition
	${PROTOBUFF_DIRECTORY}  /src/motion_detect/protobuffer
	${STREAMING_DIRECTORY}  /src/motion_detect/streaming	
	${PRACTICAL_DIRECTORY}  	/src/motion_detect/practical
	${NTP_DIRECTORY}        /src/motion_detect/ntp
	${BASE64_DIRECTORY}     /src/motion_detect/b64
	${SOCKET_DIRECTORY}  /src/motion_detect/socket
	${DATABASE_DIRECTORY}  /src/motion_detect/database
        ${HTTP_DIRECTORY}  /src/motion_detect/http
        ${UTILS_DIRECTORY}  /src/motion_detect/utils
        ${OPERATIONS_DIRECTORY}  /src/motion_detect/operations
)

FIND_PACKAGE(Protobuf REQUIRED)

set ( OpenCV_DIR   "/home/pi/Downloads/opencv-3.0.0/build/")
find_package( OpenCV 3.0 REQUIRED)

LINK_DIRECTORIES(/usr/local/lib)
LINK_DIRECTORIES(/usr/lib)
LINK_DIRECTORIES(/cedric/lib)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/src/motion_detect)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/src/motion_detect)

SET(CMAKE_CXX_FLAGS "-o3 -w")
SET(CMAKE_CXX_LINK_FLAGS "-pg")
SET(OpenCV_LIBRARIES opencv_core opencv_highgui opencv_imgproc opencv_video)
SET(PROTOBUF_LIBRARIES protobuf)

ADD_EXECUTABLE(motion_detect_raspberry 
	src/motion_detect/main.cpp
        src/motion_detect/socket/streamlistener.cpp
	src/motion_detect/socket/streamlistener.h
	src/motion_detect/practical/PracticalSocket.h
	src/motion_detect/practical/PracticalSocket.cpp
	src/motion_detect/recognition/detection.h
        src/motion_detect/recognition/detection.cpp 
	src/motion_detect/tinyxml/tinystr.cpp
	src/motion_detect/tinyxml/tinyxml.cpp
	src/motion_detect/tinyxml/tinyxmlerror.cpp
	src/motion_detect/tinyxml/tinyxmlparser.cpp
        src/motion_detect/ntp/hwclock.c
	src/motion_detect/protobuffer/motion.pb.h
	src/motion_detect/protobuffer/motion.pb.cc
        #src/motion_detect/protobuffer/server.pb.h
	#src/motion_detect/protobuffer/server.pb.cc
	src/motion_detect/b64/cdecode.c
	src/motion_detect/b64/cdecode.h
	src/motion_detect/b64/cencode.c
	src/motion_detect/b64/cencode.h
	src/motion_detect/b64/decode.h
	src/motion_detect/b64/encode.h
	src/motion_detect/b64/base64.cpp
	src/motion_detect/b64/base64.h
	src/motion_detect/socket/streamlistener.cpp
	src/motion_detect/socket/streamlistener.h
	src/motion_detect/socket/netcvc.cpp
        src/motion_detect/socket/netcvc.h
	src/motion_detect/database/database.h
	src/motion_detect/database/database.cpp
        src/motion_detect/http/httppost.c
        src/motion_detect/http/post.cpp
        src/motion_detect/http/post.h
        src/motion_detect/http/json.cpp
        src/motion_detect/http/json.h         
        src/motion_detect/utils/utils.cpp
        src/motion_detect/utils/utils.h
        src/motion_detect/operations/communications.h
        src/motion_detect/operations/communications.cpp
        src/motion_detect/operations/camera.h
        src/motion_detect/operations/camera.cpp
        src/motion_detect/operations/observer.h
        src/motion_detect/operations/observer.cpp
        src/motion_detect/operations/startup.h
        src/motion_detect/operations/startup.cpp
)

TARGET_LINK_LIBRARIES(motion_detect_raspberry ${OpenCV_LIBRARIES} -lcurl -lpthread -ljson -lm -ldl -lparse ${PROTOBUF_LIBRARIES} -lsqlite3 -lusb -lrt)

SET(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")

SET(CMAKE_BUILD_TYPE Debug)

