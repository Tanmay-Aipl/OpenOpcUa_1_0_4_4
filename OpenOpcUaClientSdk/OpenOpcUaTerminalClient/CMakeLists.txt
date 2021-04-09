#___________________________________________________________________

#

#	Fichier : CMakeLists.txt

#	Projet 	: OpenOpcUaTerminalClient 

#	Auteur	: Michel Condemine  

#	Date	: 02/03/2015

#   Validation et integration : Michel Condemine 03/03/2015

#	Descr.	: Fichier d'autoconfiguration et de génération du

#		  makefile du OpenOpcUaClientLib sous GNU/Linux

#___________________________________________________________________



# Set the minimum cmake version

cmake_minimum_required (VERSION 2.8)



# Set the project name

project(OpenOpcUaTerminalClient)



set(CMAKE_BUILD_TYPE Debug)



set(EXECUTABLE_OUTPUT_PATH ../bin/linux/${CMAKE_BUILD_TYPE})


# Recherche des librairies utiles pour la gestion des threads

find_package (

	Threads

)



# Platform-dependant optimization and settings

set(

	  CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_    -Wall -g -ggdb"

#      message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

  )



set(

	  CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_     -Wall -g -ggdb"

 #    message("-- CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

)



set(BIN_SUFFIX "")

execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)



include_directories(

	 ./include
	 ../../include/OpenOpcUaClientSdk
	 ../../shared/

	 ../../include/opcua

	 ../../include/SaxXml

	 ../../include

)



file(

	GLOB_RECURSE

	OpenOpcUaTerminalClient_SRCS

	source/main.cpp

)



link_directories(${OpenOpcUaTerminalClient_SOURCE_DIR}/../../../lib/linux/Debug/)

add_executable( OpenOpcUaTerminalClient ${OpenOpcUaTerminalClient_SRCS})

target_link_libraries( OpenOpcUaTerminalClient OpenOpcUaClientLib ${OpenSSL_LIBRAIRIES} pthread ssl crypto rt OpenOpcUaSharedlib OpenOpcUaStack XMLSAXParser )



