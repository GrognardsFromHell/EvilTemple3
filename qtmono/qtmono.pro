
TEMPLATE = lib

TARGET = qtmono

CONFIG += staticlib create_prl

include(../common/project.pri)

SOURCES +=  src/qmonoqobjectwrapper.cpp \
			src/qmonoconnectionmanager.cpp \
			src/qmonoargumentconverter.cpp

HEADERS +=  src/qtmono_stable.h \
            src/qmonoargumentconverter.h \
			src/qmonoqobjectwrapper.h \
			src/monopp.h \
			src/qmonoconnectionmanager.h

PRECOMPILED_HEADER = src/qtmono_stable.h

include(../thirdparty/mono.pri)
