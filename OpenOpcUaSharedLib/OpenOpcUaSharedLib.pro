TEMPLATE = lib
#CONFIG += staticlib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TARGET = ../lib/linux/OpenOpcUaSharedLib
INCLUDEPATH +=../OpenOpcUaSharedLib/include/ \
                                ../OpenOpcUaStackV1/include/shared/

 DEFINES +=\
       _USE_UNIX98 \
       _USE_POSIX199309 \
       _OPCUA_USE_POSIX \
       _GNUC_ \
       _DEBUG \
       _BIG_ENDIAN

LIBS *= -L/usr/lib \
              -L../lib/linux \
               -lpthread \
               -lcrypto \
               -lssl \
               -lOpenOpcUaStack


SOURCES += \
    ./source/Utils.cpp \
    ./source/SubscriptionDiagnosticsDataType.cpp \
    ./source/Subscription.cpp \
    ./source/stdafx.cpp \
    ./source/SessionSecurityDiagnosticsDataType.cpp \
    ./source/SessionDiagnosticsDataType.cpp \
    ./source/SessionBase.cpp \
    ./source/ServerStatus.cpp \
    ./source/OpenOpcUa.cpp \
    ./source/opcua_certficates.cpp \
    ./source/NumericRange.cpp \
    ./source/MonitoredItemBase.cpp \
    ./source/FileVersionInfo.cpp \
    ./source/ExtensionObject.cpp \
    ./source/EndpointDescription.cpp \
    ./source/DataValue.cpp \
    ./source/CryptoUtils.cpp \
    ./source/Channel.cpp \
    ./source/BuildInfo.cpp \
    ./source/Application.cpp \
    ./source/AggregateCalculator.cpp

HEADERS += \
    ./include/Utils.h \
    ./include/UserTokenPolicy.h \
    ./include/targetver.h \
    ./include/SubscriptionDiagnosticsDataType.h \
    ./include/Subscription.h \
    ./include/stdafx.h \
    ./include/StatusCodeException.h \
    ./include/SessionSecurityDiagnosticsDataType.h \
    ./include/SessionDiagnosticsDataType.h \
    ./include/SessionBase.h \
    ./include/ServerStatus.h \
    ./include/OpenOpcUa.h \
    ./include/opcua_certificates.h \
    ./include/NumericRange.h \
    ./include/NodeIdLookupTable.h \
    ./include/MonitoredItemBase.h \
    ./include/FileVersionInfo.h \
    ./include/ExtensionObject.h \
    ./include/EndpointDescription.h \
    ./include/DataValue.h \
    ./include/cslock.h \
    ./include/CryptoUtils.h \
    ./include/Channel.h \
    ./include/BuildInfo.h \
    ./include/Application.h \
    ./include/AggregateCalculator.h

