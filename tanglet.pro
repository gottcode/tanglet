lessThan(QT_VERSION, 5.2) {
	error("Tanglet requires Qt 5.2 or greater")
}

TEMPLATE = app
QT += widgets
CONFIG += warn_on c++11

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
MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

# Set program version
VERSION = 1.3.1
DEFINES += VERSIONSTR=\\\"$${VERSION}\\\"

# Set program name
unix:!macx {
	TARGET = tanglet
} else {
	TARGET = Tanglet
}

# Specify program sources
HEADERS += src/beveled_rect.h \
	src/board.h \
	src/clock.h \
	src/generator.h \
	src/gzip.h \
	src/language_dialog.h \
	src/language_settings.h \
	src/locale_dialog.h \
	src/letter.h \
	src/new_game_dialog.h \
	src/scores_dialog.h \
	src/solver.h \
	src/trie.h \
	src/view.h \
	src/window.h \
	src/word_counts.h \
	src/word_tree.h

SOURCES += src/beveled_rect.cpp \
	src/board.cpp \
	src/clock.cpp \
	src/generator.cpp \
	src/gzip.cpp \
	src/language_dialog.cpp \
	src/language_settings.cpp \
	src/locale_dialog.cpp \
	src/letter.cpp \
	src/new_game_dialog.cpp \
	src/main.cpp \
	src/scores_dialog.cpp \
	src/solver.cpp \
	src/trie.cpp \
	src/view.cpp \
	src/window.cpp \
	src/word_counts.cpp \
	src/word_tree.cpp

# Generate translations
TRANSLATIONS = $$files(translations/tanglet_*.ts)
qtPrepareTool(LRELEASE, lrelease)
updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

# Install program data
RESOURCES = icons/icons.qrc data.qrc

macx {
	ICON = icons/tanglet.icns

	GAME_DATA.files = data
	GAME_DATA.path = Contents/Resources

	QMAKE_BUNDLE_DATA += GAME_DATA
} else:win32 {
	RC_FILE = icons/icon.rc
} else:unix {
	RESOURCES += icons/icon.qrc

	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	data.files = data/*
	data.path = $$PREFIX/share/tanglet/data/

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	pixmap.files = icons/tanglet.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	mime.files = icons/tanglet.xml
	mime.path = $$PREFIX/share/mime/packages/

	desktop.files = icons/tanglet.desktop
	desktop.path = $$PREFIX/share/applications/

	appdata.files = icons/tanglet.appdata.xml
	appdata.path = $$PREFIX/share/appdata/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/tanglet/translations
	qm.CONFIG += no_check_exist

	man.files = doc/tanglet.6
	man.path = $$PREFIX/share/man/man6

	INSTALLS += target icon pixmap mime desktop appdata data qm man
}
