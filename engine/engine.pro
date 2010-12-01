
QT += network \
    opengl \
    xml \
    declarative

TEMPLATE = lib

TARGET = engine

CONFIG += dll

include(../thirdparty/game-math/game-math.pri)
include(../common/project.pri)

DEFINES += ENGINE_LIBRARY

SOURCES += \
    src/mainwindow.cpp \
    src/material.cpp \
    src/renderstates.cpp \
    src/modelfile.cpp \
    src/texture.cpp \
    src/materialstate.cpp \
    src/glslprogram.cpp \
    src/gameview.cpp \
    src/backgroundmap.cpp \
    src/clippinggeometry.cpp \
    src/particlesystem.cpp \
    src/modelinstance.cpp \
    src/scenenode.cpp \
    src/scene.cpp \
    src/renderable.cpp \
    src/boxrenderable.cpp \
    src/texturesource.cpp \
    src/lighting.cpp \
    src/lighting_debug.cpp \
    src/materials.cpp \
    src/translations.cpp \
    src/sectormap.cpp \
    src/navigationmesh.cpp \
    src/models.cpp \
    src/selectioncircle.cpp \
    src/skeleton.cpp \
    src/animation.cpp \
    src/bindingpose.cpp \
    src/animatedmeshstate.cpp \
    src/modelfilereader.cpp \
    src/modelfilechunks.cpp \
    src/fogofwar.cpp \
    src/modelviewer.cpp \
    src/geometryrenderables.cpp \
    src/tileinfo.cpp \
    src/pathfinder.cpp \
    src/modelinstanceattachedscenenode.cpp \
	src/profiler.cpp \
	src/tga.cpp \
	src/paths.cpp \
	src/resourcemanager.cpp
	
HEADERS += \
    include/engine/mainwindow.h \
    src/engine_stable.h \
    include/engine/util.h \
    include/engine/global.h \
    include/engine/material.h \
    include/engine/renderstates.h \
    include/engine/modelfile.h \
    include/engine/texturesource.h \
    include/engine/texture.h \
    include/engine/materialstate.h \
    include/engine/glslprogram.h \
    include/engine/gameview.h \
    include/engine/backgroundmap.h \
    include/engine/clippinggeometry.h \
    include/engine/particlesystem.h \
    include/engine/modelinstance.h \
    include/engine/scenenode.h \
    include/engine/scene.h \
	include/engine/profiler.h \
    include/engine/renderqueue.h \
    include/engine/renderable.h \
    include/engine/boxrenderable.h \
    include/engine/lighting.h \
    include/engine/lighting_debug.h \
    include/engine/drawhelper.h \
    include/engine/materials.h \
    include/engine/translations.h \
    include/engine/sectormap.h \
    include/engine/navigationmesh.h \
    include/engine/vertexbufferobject.h \
    include/engine/models.h \
    include/engine/selectioncircle.h \
    include/engine/skeleton.h \
    include/engine/animation.h \
    include/engine/bindingpose.h \
    include/engine/animatedmeshstate.h \
    include/engine/modelfilereader.h \
    include/engine/modelfilechunks.h \
    include/engine/gamemath_streams.h \
    include/engine/fogofwar.h \
    include/engine/modelviewer.h \
    include/engine/geometryrenderables.h \
    include/engine/tileinfo.h \
    include/engine/pathfinder.h \
    include/engine/modelinstanceattachedscenenode.h \
	include/engine/paths.h \
	include/engine/resourcemanager.h \
	include/engine/iresources.h
	
HEADERS += \
    include/engine/audioengine.h \
    include/engine/isoundsource.h \
    include/engine/soundformat.h \
    include/engine/isound.h \
    include/engine/isoundhandle.h \
    include/engine/mp3reader.h \
    include/engine/wavereader.h \
	include/engine/binkplayer.h

SOURCES += \
    src/audioengine.cpp \
    src/isound.cpp \
    src/isoundsource.cpp \
    src/isoundhandle.cpp \
    src/mp3reader.cpp \
    src/wavereader.cpp \
    src/soundformat.cpp \
	src/binkplayer.cpp

INCLUDEPATH += include

PRECOMPILED_HEADER = src/engine_stable.h

win32:LIBS += -lpsapi

include(../thirdparty/openal.pri)
include(../thirdparty/turbojpeg.pri)
include(../thirdparty/libavcodec.pri)
include(../thirdparty/glew.pri)
include(../thirdparty/minizip.pri)

