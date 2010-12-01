
#
# Include this file to use the engine library in your project.
#

INCLUDEPATH += $$PWD/include/
LIBS += -lengine

CONFIG(debug, debug|release) {
    LIBS += -L$$PWD/../out/debug/bin/
} else {
    LIBS += -L$$PWD/../out/release/bin/
}
