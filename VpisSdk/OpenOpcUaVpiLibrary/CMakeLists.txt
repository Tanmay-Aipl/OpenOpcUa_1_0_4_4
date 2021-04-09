#___________________________________________________________________
#
#	Fichier    : CMakeLists.txt
#	Projet 	   : OpenOpcUaVpiLibrary  
#	Auteur	   : Michel Condemine (4CE Industry)
#	Date	   : 20/03/2014
#   Revision   : 
#   Date       : 
#   Validation 
#	Descr.	   : Makefile  file generation and autoconfiguration
#					for OpenOpcUaVpiLibrary for Linux Debian
#___________________________________________________________________



# Set the minimum cmake version
cmake_minimum_required (VERSION 2.8)

# Set the project name
project(OpenOpcUaVpiLibrary)

set(CMAKE_BUILD_TYPE Debug)
IF(NOT DEFINED ARCH_DIR)
  set(LIBRARY_OUTPUT_PATH ../../lib/linux/${CMAKE_BUILD_TYPE})
ENDIF(NOT DEFINED ARCH_DIR)
# Recherche de OpenSSL
include(FindOpenSSL)
find_package(
	OpenSSL
	0.9
	REQUIRED *
)

# Ajout des repertoire de la librairie OpenSSL pour la compilation
link_directories(${OPENSSL_LIBRARY_DIRS})
include_directories(${OPENSSL_INCLUDE_DIRS})

message("-- openssl include dirs : ${OPENSSL_INCLUDE_DIRS}")
# Add link folder
set (LINK_PATH ${OpenOpcUaVpiLibrary_SOURCE_DIR}/../lib/linux/${CMAKE_BUILD_TYPE})
link_directories(${LINK_PATH})
message("-- library path : ${LINK_PATH}")

# Platform-dependant optimization and settings
set(
	CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -Wall -g -ggdb"
)

set(
	CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -Wall -g -ggdb"
)

set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

include_directories(
	include
	../include
)

file(
	GLOB_RECURSE
	OpenOpcUaVpiLibrary_SRCS
	source/*.cpp
)

# Compile the objects and link the executable
add_library(OpenOpcUaVpiLibrary SHARED ${OpenOpcUaVpiLibrary_SRCS})
