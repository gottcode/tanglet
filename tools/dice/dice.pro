TEMPLATE = app
QT += core
CONFIG += c++11

CONFIG(debug, debug|release) {
	CONFIG += warn_on
	DEFINES += QT_DEPRECATED_WARNINGS
	DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051400
	DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
}

DESTDIR = ../..

HEADERS =

SOURCES = main.cpp

macx {
	CONFIG -= app_bundle
	LIBS += -lz
} else:win32 {
	LIBS += -lz
} else:unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += zlib
}
