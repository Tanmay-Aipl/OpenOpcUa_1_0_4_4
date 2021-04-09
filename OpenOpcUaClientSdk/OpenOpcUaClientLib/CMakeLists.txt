#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: OpenOpcUaClientLib 
#	Auteur	: Michel Condemine  
#	Date	: 01/06/2013
#   Validation et integration : Michel Condemine 11/03/2014
#	Descr.	: Fichier d'autoconfiguration et de génération du
#		  makefile du OpenOpcUaClientLib sous GNU/Linux
#___________________________________________________________________




# Set the minimum cmake version
cmake_minimum_required (VERSION 2.8)

# Set the project name
project(OpenOpcUaClientLib)

SET(CMAKE_C_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon -mfloat-abi=hard -fPIC")
SET(CMAKE_SYSTEM_PROCESSOR armv71)
set(CMAKE_SYSROOT "/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/")

set(CMAKE_BUILD_TYPE Release)
set(LIBRARY_OUTPUT_PATH ../../lib/linux/${CMAKE_BUILD_TYPE})



# Recherche de OpenSSL
#include(FindOpenSSL)
#find_package(
#	OpenSSL
#	0.9
#	REQUIRED *
#)

# Ajout des rÃ©pertoire de la librairie OpenSSL pour la compilation
link_directories(/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib ${CMAKE_CURRENT_SOURCE_DIR}/${LIBRARY_OUTPUT_PATH})
include_directories(/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/include/)
message("-- openssl include dirs : /usr/local/openssl/include/")

# Recherche des librairies utiles pour la gestion des threads
find_package (
	Threads
)

# Platform-dependant optimization and settings
set(
	  CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_    -Wall -g -fPIC -ggdb"
#      message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
  )

set(
	  CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_     -Wall -g -fPIC -ggdb"
 #    message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
)

set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

include_directories(
	 ./include
	 ../include/opcua
	 ../include/OpenOpcUaSharedLib
	 ../include/SaxXml
	 ../include
)


file(
	GLOB_RECURSE
	UAClientLib_SRCS
	ClientApplication.cpp
	ClientApplication.h
	ClientAttribute.h
	ClientSession.h
	LoggerMessage.cpp
	LoggerMessage.h
	MonitoredItemClient.cpp
	MonitoredItemClient.h
	MonitoredItemsNotification.cpp
	MonitoredItemsNotification.h
	OpenOpcUaClientLib.h
	SubscriptionClient.cpp
	SubscriptionClient.h
	UABuiltInType.h
	ClientSession.cpp
	dllmain.cpp
	OpenOpcUaClientLib.cpp
	stdafx.cpp
	stdafx.h
	targetver.h
	MurmurHash3.cpp
	MurmurHash3.h
)

# Compile the objects and link the executable
add_library(OpenOpcUaClientLib SHARED ${UAClientLib_SRCS})
