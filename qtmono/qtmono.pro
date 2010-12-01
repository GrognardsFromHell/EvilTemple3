
QT       -= gui

TEMPLATE = lib

TARGET = qtmono

CONFIG += staticlib create_prl

include(../common/project.pri)

SOURCES +=  \

HEADERS +=  src/qtmono_stable.h \
            src/qmonoargumentconverter.h

PRECOMPILED_HEADER = src/qtmono_stable.h

include(../thirdparty/mono.pri)
