
TEMPLATE = app
TARGET = launcher

QT += opengl declarative

# Enable a console for debug builds
CONFIG(debug, debug|release) {
	CONFIG += console
}

SOURCES += launcher.cpp mainwindow.cpp datafileengine.cpp

HEADERS += launcher_stable.h mainwindow.h datafileengine.h

include(../common/project.pri)
include(../thirdparty/mono.pri)
include(../thirdparty/minizip.pri)
include(../engine/engine.pri)
include(../thirdparty/glew.pri)
include(../qtmono/qtmono.pri)
include(../thirdparty/game-math/game-math.pri)

PRECOMPILED_HEADER = launcher_stable.h
