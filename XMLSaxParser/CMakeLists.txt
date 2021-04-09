#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: XMLSAXParser 
#	Auteur	: Michel Condemine (4CE Industry)
#   
#	Date	: 01/06/2011
#   Revision  Michel Condemine 4CE Industry
#   Date    : 20/03/2013
#   Validation Jean Bruno Pomares
#	Descr.	: Fichier d'autoconfiguration et de génération du
#             file MAKE du XMLSAXParser sous GNU/Linux
#___________________________________________________________________


# Set the minimum cmake version
cmake_minimum_required (VERSION 2.8)

# Set the project name
project(XMLSAXParser)
SET(CMAKE_C_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon -mfloat-abi=hard -fPIC")
SET(CMAKE_SYSTEM_PROCESSOR armv71)
set(CMAKE_SYSROOT "/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/")

set(CMAKE_BUILD_TYPE Release)
IF(NOT DEFINED ARCH_DIR)
  set(LIBRARY_OUTPUT_PATH ../lib/linux/${CMAKE_BUILD_TYPE})
ENDIF(NOT DEFINED ARCH_DIR)

# Platform-dependant optimization and settings
set(
	CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -Wall -g -ggdb"
)

set(
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -Wall  -g -ggdb"
)

set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include_directories(
	XmlShared
      include
      ../include/opcua
)

file(
	GLOB_RECURSE
	XMLSAXParser_SRCS

	XmlShared/ascii.h
	XmlShared/expat.h

	XmlShared/xmlop.h
	XmlShared/xmlop.c

	XmlShared/xmlrole.h
	XmlShared/xmlrole.c

	XmlShared/xmltok.h
	XmlShared/xmltok.c

	XmlShared/xmltok_impl.h
	XmlShared/xmlparse.c

	XmlShared/xmltok_ns.c
	
	XmlShared/expat_external.h
      include/*

       SAXParser_Helper.cpp
       XMLSAXParser.cpp
)

# Compile the objects and link the executable
add_library(XMLSAXParser SHARED ${XMLSAXParser_SRCS})
