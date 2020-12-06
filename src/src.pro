TEMPLATE = app
QT += widgets

CONFIG(debug, debug|release) {
	CONFIG += warn_on
	DEFINES += QT_DEPRECATED_WARNINGS
	DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
	DEFINES += QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
}

# Add dependencies
macx {
	LIBS += -lz
} else:win32 {
	LIBS += -lz
} else:unix {
	CONFIG += link_pkgconfig
	PKGCONFIG += zlib
}

# Allow in-tree builds
MOC_DIR = ../build
OBJECTS_DIR = ../build
RCC_DIR = ../build

# Set program version
VERSION = 1.5.6
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# Set program name
unix:!macx {
	TARGET = tanglet
} else {
	TARGET = Tanglet
}
DESTDIR = ..

# Specify program sources
HEADERS += beveled_rect.h \
	board.h \
	clock.h \
	generator.h \
	gzip.h \
	language_dialog.h \
	language_settings.h \
	locale_dialog.h \
	letter.h \
	new_game_dialog.h \
	scores_dialog.h \
	solver.h \
	trie.h \
	view.h \
	window.h \
	word_counts.h \
	word_tree.h

SOURCES += beveled_rect.cpp \
	board.cpp \
	clock.cpp \
	generator.cpp \
	gzip.cpp \
	language_dialog.cpp \
	language_settings.cpp \
	locale_dialog.cpp \
	letter.cpp \
	new_game_dialog.cpp \
	main.cpp \
	scores_dialog.cpp \
	solver.cpp \
	trie.cpp \
	view.cpp \
	window.cpp \
	word_counts.cpp \
	word_tree.cpp

# Generate translations
TRANSLATIONS = $$files(../translations/tanglet_*.ts)
qtPrepareTool(LRELEASE, lrelease)
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Install program data
RESOURCES = ../icons/icons.qrc ../data.qrc

macx {
	ICON = ../icons/tanglet.icns

	GAME_DATA.files = ../data
	GAME_DATA.path = Contents/Resources

	QMAKE_BUNDLE_DATA += GAME_DATA
} else:win32 {
	RC_ICONS = ../icons/tanglet.ico
	QMAKE_TARGET_DESCRIPTION = "Word finding game"
	QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2020 Graeme Gott"
} else:unix {
	RESOURCES += ../icons/icon.qrc

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	data.files = ../data/*
	data.path = $$PREFIX/share/tanglet/data/

	icon.files = ../icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	pixmap.files = ../icons/tanglet.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	mime.files = ../icons/tanglet.xml
	mime.path = $$PREFIX/share/mime/packages/

	desktop.files = ../icons/tanglet.desktop
	desktop.path = $$PREFIX/share/applications/

	appdata.files = ../icons/tanglet.appdata.xml
	appdata.path = $$PREFIX/share/metainfo/

	qm.files = $$replace(TRANSLATIONS, .ts, .qm)
	qm.path = $$PREFIX/share/tanglet/translations
	qm.CONFIG += no_check_exist

	man.files = ../doc/tanglet.6
	man.path = $$PREFIX/share/man/man6

	INSTALLS += target icon pixmap mime desktop appdata data qm man
}
