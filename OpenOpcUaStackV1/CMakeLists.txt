#___________________________________________________________________
#
#	Fichier : CMakeLists.txt
#	Projet 	: OpenOpcUaStackV1
#	Auteur	: Michel Condemine
#	Date	: 07/01/2014
#   Validation  : TBD
#	Descr.	: makefile generation for Linux
#		  
#___________________________________________________________________


# Version minimum de cmake requise : 2.8
cmake_minimum_required (VERSION 2.8)

# Nom du projet
project(OpenOpcUaStack)

SET(CMAKE_C_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER /home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon -mfloat-abi=hard")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mfpu=neon -mfloat-abi=hard -fPIC")
SET(CMAKE_SYSTEM_PROCESSOR armv71)
set(CMAKE_SYSROOT "/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/")

# Type de compilation (Debug/Release)
set(CMAKE_BUILD_TYPE Release)

# Repertoire de sortie de la librairie
set(LIBRARY_OUTPUT_PATH ../lib/linux/${CMAKE_BUILD_TYPE})



# Recherche de OpenSSL
#include(FindOpenSSL)
#find_package(
#	OpenSSL
#	1.0
#	REQUIRED *
#)

# Ajout des repertoire de la librairie OpenSSL pour la compilation
link_directories(/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib)
include_directories(/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/include/, /usr/local/openssl/include/)

message("-- openssl include dirs : /usr/local/openssl/include/")

# Recherche des librairies utiles pour la gestion des threads
find_package (
	Threads
)

# Arguments et instructions utiles pour le compilateur 
#Â Ici, le langage utile est le C donc -> CMAKE_C_FLAGS
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_GNU_SOURCE -D__USE_UNIX98 -D__USE_POSIX199309 -D_OPCUA_USE_POSIX -D_GNUC_ -D_DEBUG -Wall -fPIC -g -ggdb")

set(BIN_SUFFIX "")
execute_process(COMMAND uname -m OUTPUT_VARIABLE ARCH)

# Affichage des instruction pre-processeurs requises
message("-- CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")

# Ajout des reertoire contenant les .h
include_directories(
	include/shared
)

# Ajout de toutes les sources de fichiers utiles Ã  la compilation
file(
	GLOB_RECURSE
	OpenOpcUaStack_SRCS
	source/*
	include/shared/*
)

# On demande la sortie d'une librairie statique "libopcua_stack.a"
add_library(
	OpenOpcUaStack 
	SHARED 
	${OpenOpcUaStack_SRCS}
)

#Configuration de l'edition de liens
target_link_libraries(
	OpenOpcUaStack 
	ssl -L/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib
	crypto -L/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib
	pthread -L/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/usr/lib
)


