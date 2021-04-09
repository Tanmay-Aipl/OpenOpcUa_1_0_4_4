TEMPLATE = lib
#CONFIG += staticlib
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
TARGET = ../lib/linux/XMLSAXParser

INCLUDEPATH +=  include/ \
                                  XmlShared/ \
                                  ../OpenOpcUaStackV1/include/shared/


DEFINES += \
       _USE_UNIX98 \
       _USE_POSIX199309 \
       _OPCUA_USE_POSIX \
       _GNUC_ \
       #__cplusplus \
        #_OS_FREE \
       #_DEBUG \
       _BIG_ENDIAN


SOURCES += \
    ./source/XMLSAXParser.cpp \
    ./source/stdafx.cpp \
    ./source/SAXParser_Helper.cpp \
    ./XmlShared/xmltok_ns.c \
    ./XmlShared/xmltok.c \
    ./XmlShared/xmlrole.c \
    ./XmlShared/xmlparse.c \
    ./XmlShared/xmlop.c

HEADERS += \
    ./include/stdafx.h \
    ./include/SAXParser_Helper.h

OTHER_FILES += \
    ./source/XMLSAXParserd.def \
    ./source/XMLSAXParser.def

