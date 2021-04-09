TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TARGET = ../bin/linux/OpenOpcCoreServer

INCLUDEPATH +=  ../OpenOpcUaCoreServer/include/ \
                                 ../OpenOpcUaCoreServer/ServerShared/ \
                                  ../OpenOpcUaStackV1/include/shared/ \
                                  ../OpenOpcUaSharedLib/include/

DEFINES += \
       _USE_UNIX98 \
       _USE_POSIX199309 \
       _OPCUA_USE_POSIX \
       _GNUC_ \
       __cplusplus \
       _OS_FREE \
       _DEBUG \
       _BIG_ENDIAN

LIBS *= -L/usr/lib \
              -L../lib/linux \
               -lpthread \
               -lcrypto \
               -lssl \
               -lrt \
               -lOpenOpcUaStack \
               -lXMLSaxParser \
               -lOpenOpcUaSharedLib



SOURCES += \
    ./source/VPIScheduler.cpp \
    ./source/UAView.cpp \
    ./source/UAVariableType.cpp \
    ./source/UAVariable.cpp \
    ./source/UAStatusChangeNotification.cpp \
    ./source/UAReferenceType.cpp \
    ./source/UAReference.cpp \
    ./source/UAObjectType.cpp \
    ./source/UAObject.cpp \
    ./source/UAMethod.cpp \
    ./source/UAInformationModel.cpp \
    ./source/UAEventNotificationList.cpp \
    ./source/UADataType.cpp \
    ./source/UADataChangeNotification.cpp \
    ./source/UABinding.cpp \
    ./source/UABase.cpp \
    ./source/SubscriptionServer.cpp \
    ./source/stdafx.cpp \
    ./source/StackCallbacks.cpp \
    ./source/SimulatedNode.cpp \
    ./source/SimulatedGroup.cpp \
    ./source/SessionServer.cpp \
    ./source/ServerApplication.cpp \
    ./source/SecureChannel.cpp \
    ./source/QueueRequest.cpp \
    ./source/QueuedReadRequest.cpp \
    ./source/QueuedPublishRequest.cpp \
    ./source/NamespaceUri.cpp \
    ./source/MonitoredItemServer.cpp \
    ./source/Main.cpp \
    ./source/Field.cpp \
    ./source/Definition.cpp \
    ./source/ContinuationPoint.cpp \
    ./source/Alias.cpp \
    ./source/AcqCtrlSignal.cpp \
    ./source/AcqCtrlDevice.cpp

HEADERS += \
    ./include/VPIScheduler.h \
    ./include/VpiFuncCaller.h \
    ./include/UAView.h \
    ./include/UAVariableType.h \
    ./include/UAVariable.h \
    ./include/UAStatusChangeNotification.h \
    ./include/UASimulation.h \
    ./include/UAReferenceType.h \
    ./include/UAReference.h \
    ./include/UAObjectType.h \
    ./include/UAObject.h \
    ./include/UAMethod.h \
    ./include/UAInformationModel.h \
    ./include/UAEventNotificationList.h \
    ./include/UADataType.h \
    ./include/UADataChangeNotification.h \
    ./include/UABinding.h \
    ./include/UABase.h \
    ./include/targetver.h \
    ./include/SubscriptionServer.h \
    ./include/stdafx.h \
    ./include/StackCallbacks.h \
    ./include/SimulatedNode.h \
    ./include/SimulatedGroup.h \
    ./include/SessionServer.h \
    ./include/ServerApplication.h \
    ./include/SecureChannel.h \
    ./include/resource.h \
    ./include/QueueRequest.h \
    ./include/QueuedReadRequest.h \
    ./include/QueuedPublishRequest.h \
    ./include/NamespaceUri.h \
    ./include/MonitoredItemServer.h \
    ./include/Field.h \
    ./include/Definition.h \
    ./include/ContinuationPoint.h \
    ./include/Alias.h \
    ./include/AcqCtrlSignal.h \
    ./include/AcqCtrlDevice.h \
    ./include/AcqCtrlDef.h
