
#
# Include this file to use the QtMono library in your project.
#

INCLUDEPATH += $$PWD/include/
LIBS += -lqtmono

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../out/debug/bin/
} else {
    LIBS += -L$$PWD/../out/release/bin/
}

include($$PWD/../thirdparty/mono.pri)
