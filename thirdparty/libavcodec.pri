win32:INCLUDEPATH += $${PWD}/libavcodec-minimal/include
win32:LIBS += -L$${PWD}/libavcodec-minimal

win32-msvc2008:INCLUDEPATH += $${PWD}/libavcodec-minimal/msvc
win32-msvc2010:INCLUDEPATH += $${PWD}/libavcodec-minimal/msvc

unix {
	INCLUDEPATH += $${PWD}/ffmpeg/include/
	LIBS += -L$${PWD}/ffmpeg/lib/ -lavcodec -lavformat -lavutil -lswscale
}

win32 {
	LIBS += -lavcodec-52 -lavformat-52 -lavutil-50 -lswscale-0
}
