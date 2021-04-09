#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: OpenOpcUaCoreServer 
#	Auteur	: Michel Condemine 4CE Industry
#	Date	: 17/03/2013
#   	Revision  Jean Bruno Pomares INTESPACE
#   	Date    : 20/03/2013
#   	Validation 
#	Descr.	: Fichier d'autoconfiguration et de generation du
#		  makefile de l'OpenOpcUaCoreServer sous GNU/Linux
#___________________________________________________________________

# Version minimum de cmake requise : 2.8
cmake_minimum_required (VERSION 2.8)

# Nom du projet
project(OpenOpcUaCoreServer)
SET(CMAKE_C_COMPILER /opt/DATCON/sysroots/x86_64-fslcsdk-linux/usr/bin/arm-fslc-linux-gnueabi/arm-fslc-linux-gnueabi-gcc)
SET(CMAKE_CXX_COMPILER /opt/DATCON/sysroots/x86_64-fslcsdk-linux/usr/bin/arm-fslc-linux-gnueabi/arm-fslc-linux-gnueabi-g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon -mfloat-abi=hard -fPIC")
SET(CMAKE_SYSTEM_PROCESSOR armv71)
set(CMAKE_SYSROOT "/opt/DATCON/sysroots/armv7at2hf-neon-fslc-linux-gnueabi/")

# Type de compilation (Debug/Release)
set(CMAKE_BUILD_TYPE Debug)

#RÃ©pertoire de sortie de l'exe
set(EXECUTABLE_OUTPUT_PATH ../bin/linux/${CMAKE_BUILD_TYPE})



#inclusion de open ssl
#include(FindOpenSSL)
#find_package(
#	OpenSSL
#	1.0
#	REQUIRED *
#)

link_directories(/opt/DATCON/sysroots/armv7at2hf-neon-fslc-linux-gnueabi/usr/lib)
include_directories(/opt/DATCON/sysroots/armv7at2hf-neon-fslc-linux-gnueabi/usr/include/)

message("-- OpenSSL Found: ${OPENSSL_FOUND}")
message("-- OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
message("-- OpenSSL librairie: ${OPENSSL_LIBRARIES}")

# Platform-dependant optimization and settings
set(
      CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -D___cplusplus -D_DEBUG -D_OS_FREE -Wall -g -ggdb -ldl -rdynamic"
)

set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# Configure the headers search path for inclusion 
include_directories(
	include
	ServerShared
	../include/opcua
	../include/OpenOpcUaSharedLib
	../include/SaxXml
	../include/lua
	../lib/linux/${CMAKE_BUILD_TYPE}
)

link_directories(${OpenOpcUaCoreServer_SOURCE_DIR}/../lib/linux/${CMAKE_BUILD_TYPE})

message("-- src path : ${CMAKE_CURRENT_SOURCE_DIR}")

# Configure sources variables required to build the project
file(
	GLOB_RECURSE
	OpenOpcUaCoreServer_SRCS
	source/*.cpp
)
message("-- OpenOpcUaCoreServer_SOURCE_DIR is: ${OpenOpcUaCoreServer_SOURCE_DIR}")

list (REMOVE_ITEM OpenOpcUaCoreServer_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/NodeId.cpp")

list (REMOVE_ITEM OpenOpcUaCoreServer_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/CompilerIdCXX/CMakeCXXCompilerId.cpp")

# Compile the objects and link the executable
add_executable(	OpenOpcUaCoreServer ${OpenOpcUaCoreServer_SRCS})



#Configuration de l'Ã©dition de liens
target_link_libraries( OpenOpcUaCoreServer
 XMLSAXParser
 OpenOpcUaSharedlib
 OpenOpcUaStack
 libLuaLib.a
 pthread
 ssl
 crypto
 rt
 dl)

