
INCLUDEPATH += $${PWD}
CONFIG(debug, debug|release) {
    LIBS += -L$${PWD}/../bin
    LIBS += -ltroikaformats_d
} else {
    LIBS += -L$${PWD}/../bin
    LIBS += -ltroikaformats
}
