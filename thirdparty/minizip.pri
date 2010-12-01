
INCLUDEPATH += $$PWD/minizip/

SOURCES += $$PWD/minizip/ioapi.c \
    $$PWD/minizip/unzip.c \
    $$PWD/minizip/zip.c \
    $$PWD/minizip/zipwriter.cpp

win32:SOURCES += $$PWD/minizip/iowin32.c

HEADERS += $$PWD/minizip/crypt.h \
    $$PWD/minizip/ioapi.h \
    $$PWD/minizip/unzip.h \
    $$PWD/minizip/zip.h \
    $$PWD/minizip/zlib.h \
    $$PWD/minizip/zconf.h \
    $$PWD/minizip/zipwriter.h \
    $$PWD/minizip/minizipglobal.h
