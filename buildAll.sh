echo Global OpenOpcUaServer and dependencies generation
echo XMLSaxParser generation
cd XMLSaxParser
cmake CMakeLists.txt
make
cd ..
echo LuaLib generation
cd LuaLib
cmake ./
make
cd ..
echo OpenOpcUaStack generation
cd OpenOpcUaStackV1
cmake CMakeLists.txt
make
cd ..
echo OpenOpcUaSharedLib generation
cd OpenOpcUaSharedLib
cmake CMakeLists.txt
make
cd ..
echo OpenOpcUaCoreServer generation
cd OpenOpcUaCoreServer
cmake CMakeLists.txt
make
cd ..
echo all file was generated
dir ./bin/linux/Debug
dir ./lib/linux/Debug
