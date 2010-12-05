
QT       -= gui

TEMPLATE = lib

TARGET = qtmono

CONFIG += staticlib create_prl

include(../common/project.pri)

SOURCES +=  src/qobjectmonowrapper.cpp

HEADERS +=  src/qtmono_stable.h \
            src/qmonoargumentconverter.h \
			src/qobjectmonowrapper.h

PRECOMPILED_HEADER = src/qtmono_stable.h

include(../thirdparty/mono.pri)
