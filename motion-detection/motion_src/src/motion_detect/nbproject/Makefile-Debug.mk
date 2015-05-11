#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/practical/PracticalSocket.o \
	${OBJECTDIR}/recognition/detection.o \
	${OBJECTDIR}/tinyxml/tinystr.o \
	${OBJECTDIR}/tinyxml/tinyxml.o \
	${OBJECTDIR}/tinyxml/tinyxmlerror.o \
	${OBJECTDIR}/tinyxml/tinyxmlparser.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath,/usr/local/include/opencv2 `pkg-config --libs opencv`  

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/motion_detect

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/motion_detect: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/motion_detect ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/practical/PracticalSocket.o: practical/PracticalSocket.cpp 
	${MKDIR} -p ${OBJECTDIR}/practical
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/practical/PracticalSocket.o practical/PracticalSocket.cpp

${OBJECTDIR}/recognition/detection.o: recognition/detection.cpp 
	${MKDIR} -p ${OBJECTDIR}/recognition
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/recognition/detection.o recognition/detection.cpp

${OBJECTDIR}/tinyxml/tinystr.o: tinyxml/tinystr.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinystr.o tinyxml/tinystr.cpp

${OBJECTDIR}/tinyxml/tinyxml.o: tinyxml/tinyxml.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinyxml.o tinyxml/tinyxml.cpp

${OBJECTDIR}/tinyxml/tinyxmlerror.o: tinyxml/tinyxmlerror.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinyxmlerror.o tinyxml/tinyxmlerror.cpp

${OBJECTDIR}/tinyxml/tinyxmlparser.o: tinyxml/tinyxmlparser.cpp 
	${MKDIR} -p ${OBJECTDIR}/tinyxml
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/local/include/opencv2 `pkg-config --cflags opencv`   -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/tinyxml/tinyxmlparser.o tinyxml/tinyxmlparser.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/motion_detect

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
