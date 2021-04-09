#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: OpenOpcUaSharedlib 
#	Auteur	: Michel Condemine 4CE Industry
#	Date	: 17/03/2013
#   	Revision  Jean Bruno Pomares INTESPACE
#   	Date    : 20/03/2013
#   	Validation Jean Bruno Pomares
#	Descr.	: Fichier d'autoconfiguration et de generation du
#		  makefile de l'OpenOpcUaSharedlib sous GNU/Linux
#___________________________________________________________________

# Set the minimum cmake version
cmake_minimum_required (VERSION 2.8)

# Set the project name
project(OpenOpcUaSharedlib)

SET(CMAKE_C_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon -mfloat-abi=hard -fPIC")
#SET(CMAKE_SYSTEM_PROCESSOR armv71)
set(CMAKE_SYSROOT "/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/")

# Type de compilation (Debug/Release)
set(CMAKE_BUILD_TYPE Release)

# Repertoire de sortie de la librairie
set(LIBRARY_OUTPUT_PATH ../lib/linux/${CMAKE_BUILD_TYPE})

#inclusion de open ssl
#include(FindOpenSSL)
#find_package(
#	OpenSSL
#	1.0
#	REQUIRED *
#)

link_directories(/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib)
include_directories(/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/include/)

#message("-- OpenSSL Found : ${OPENSSL_FOUND}")
#message("-- OpenSSL Include Directory : /usr/local/openssl/include/")
#message("-- OpenSSL Librairies : /usr/local/openssl/lib/")

message("-- OpenSSL Found : ${OPENSSL_FOUND}")
message("-- OpenSSL Include Directory : ${OPENSSL_INCLUDE_DIR}")
message("-- OpenSSL Librairies : ${OPENSSL_LIBRARIES}")


# Platform-dependant optimization and settings
set(
CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX  -D_GNUC_  -D_OS_FREE -Wall -g -ggdb"
)


set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


include_directories(
	include
	OpenOpcUaSharedlib_SRCS
	../include/opcua
	../lib/linux/${CMAKE_BUILD_TYPE}
)

# Configure sources variables required to build the project
file(
	GLOB
	OpenOpcUaSharedlib_SRCS
	source/*.cpp
	include
)

link_directories(
	${OpenOpcUaSharedlib_SOURCE_DIR}/../lib/linux/${CMAKE_BUILD_TYPE}
)


# Compile the objects and link the executable
add_library(OpenOpcUaSharedlib SHARED ${OpenOpcUaSharedlib_SRCS})
#
target_link_libraries(OpenOpcUaSharedlib
			 OpenOpcUaStack
			 ssl -L/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib  
			 crypto -L/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib 
			rt -L/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib)

