# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.6

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib

# Include any dependencies generated for this target.
include CMakeFiles/OpenOpcUaClientLib.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/OpenOpcUaClientLib.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/OpenOpcUaClientLib.dir/flags.make

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o: source/ClientApplication.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/ClientApplication.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/ClientApplication.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/ClientApplication.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o: source/LoggerMessage.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/LoggerMessage.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/LoggerMessage.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/LoggerMessage.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o: source/MonitoredItemClient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MonitoredItemClient.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MonitoredItemClient.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MonitoredItemClient.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o: source/MonitoredItemsNotification.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MonitoredItemsNotification.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MonitoredItemsNotification.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MonitoredItemsNotification.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o: source/SubscriptionClient.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/SubscriptionClient.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/SubscriptionClient.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/SubscriptionClient.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o: source/ClientSession.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/ClientSession.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/ClientSession.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/ClientSession.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o: source/dllmain.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/dllmain.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/dllmain.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/dllmain.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o: source/OpenOpcUaClientLib.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/OpenOpcUaClientLib.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/OpenOpcUaClientLib.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/OpenOpcUaClientLib.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o: source/stdafx.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/stdafx.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/stdafx.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/stdafx.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o


CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o: CMakeFiles/OpenOpcUaClientLib.dir/flags.make
CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o: source/MurmurHash3.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o -c /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MurmurHash3.cpp

CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.i"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MurmurHash3.cpp > CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.i

CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.s"
	/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/bin/arm-linux-gnueabihf-g++  --sysroot=/home/murali/tools/arm-bcm2708/arm-rpi-4.9.3-linux-gnueabihf/arm-linux-gnueabihf/sysroot/ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/source/MurmurHash3.cpp -o CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.s

CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.requires:

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.requires

CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.provides: CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.requires
	$(MAKE) -f CMakeFiles/OpenOpcUaClientLib.dir/build.make CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.provides.build
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.provides

CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.provides.build: CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o


# Object files for target OpenOpcUaClientLib
OpenOpcUaClientLib_OBJECTS = \
"CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o" \
"CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o"

# External object files for target OpenOpcUaClientLib
OpenOpcUaClientLib_EXTERNAL_OBJECTS =

/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/build.make
/home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so: CMakeFiles/OpenOpcUaClientLib.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Linking CXX shared library /home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/OpenOpcUaClientLib.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/OpenOpcUaClientLib.dir/build: /home/murali/OpenOpcUa_1_0_4_4/lib/linux/Release/libOpenOpcUaClientLib.so

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/build

CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientApplication.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/LoggerMessage.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemClient.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/MonitoredItemsNotification.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/SubscriptionClient.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/ClientSession.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/dllmain.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/OpenOpcUaClientLib.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/stdafx.cpp.o.requires
CMakeFiles/OpenOpcUaClientLib.dir/requires: CMakeFiles/OpenOpcUaClientLib.dir/source/MurmurHash3.cpp.o.requires

.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/requires

CMakeFiles/OpenOpcUaClientLib.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/OpenOpcUaClientLib.dir/cmake_clean.cmake
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/clean

CMakeFiles/OpenOpcUaClientLib.dir/depend:
	cd /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib /home/murali/OpenOpcUa_1_0_4_4/OpenOpcUaClientSdk/OpenOpcUaClientLib/CMakeFiles/OpenOpcUaClientLib.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/OpenOpcUaClientLib.dir/depend
