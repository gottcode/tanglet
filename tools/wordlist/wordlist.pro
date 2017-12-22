TEMPLATE = app
QT += core
CONFIG += warn_on c++11

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051000
DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT

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
