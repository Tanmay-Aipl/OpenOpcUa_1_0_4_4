#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: LuaLib 
#	Auteur	: Michel Condemine 4CE Industry
#	Date	: 24/11/2015
#   	Revision  
#   	Date    : 
#   	Validation 
#	Descr.	: Fichier d'autoconfiguration et de generation du
#		  makefile de LuaLib sous GNU/Linux
#___________________________________________________________________

# Set the minimum cmake version
cmake_minimum_required (VERSION 2.8)

# Set the project name
project(LuaLib)

# Type de compilation (Debug/Release)
set(CMAKE_BUILD_TYPE Debug)

# Repertoire de sortie de la librairie
set(LIBRARY_OUTPUT_PATH ../lib/linux/${CMAKE_BUILD_TYPE})

#inclusion de open ssl
include(FindOpenSSL)
find_package(
	OpenSSL
	1.0
	REQUIRED *
)

link_directories(${OPENSSL_LIBRARY_DIRS})
include_directories(${OPENSSL_INCLUDE_DIRS})

message("-- OpenSSL Found : ${OPENSSL_FOUND}")
message("-- OpenSSL Include Directory : ${OPENSSL_INCLUDE_DIR}")
message("-- OpenSSL Librairies : ${OPENSSL_LIBRARIES}")

# Platform-dependant optimization and settings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -D_DEBUG -Wall -g -ggdb")


set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")


include_directories(
	include
	LuaLib_SRCS
	../include/lua
	../lib/linux/${CMAKE_BUILD_TYPE}
)

# Configure sources variables required to build the project
file(
	GLOB_RECURSE
	LuaLib_SRCS
	source/*.c
	include
)

link_directories(
	${LuaLib_SOURCE_DIR}/../lib/linux/${CMAKE_BUILD_TYPE}
)


# Compile the objects and link the executable
add_library(LuaLib STATIC ${LuaLib_SRCS})
#
target_link_libraries(LuaLib rt)

