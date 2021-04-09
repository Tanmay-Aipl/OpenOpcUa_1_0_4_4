#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: VPINull 
#	Auteur	: Michel Condemine(4CE Industry)
#	Date	: 28/03/2013
#	Revision : 
#	Description: Fichier d'autoconfiguration et de génération du
#					makefile du VPINull sous GNU/Linux
#___________________________________________________________________


# Set the minimum cmake version
cmake_minimum_required (VERSION 2.8)

# Set the project name
project(VPINull)

set(CMAKE_BUILD_TYPE Debug)
set(LIBRARY_OUTPUT_PATH ../../lib/linux/${CMAKE_BUILD_TYPE})

# Platform-dependant optimization and settings
set(
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D_DEBUG -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -Wall -g -ggdb"
)
set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include_directories(
	include
	../include
)

file(
	GLOB_RECURSE
	VPINull_SRCS
	source/*
)

# Compile the objects and link the executable
add_library(VPINull SHARED ${VPINull_SRCS} )
