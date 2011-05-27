TEMPLATE = app
CONFIG += warn_on release
macx {
	# Uncomment the following line to compile on PowerPC Macs
	# QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk
	CONFIG += x86 ppc
	LIBS += -lz
}

MOC_DIR = build
OBJECTS_DIR = build
RCC_DIR = build

unix: !macx {
	TARGET = tanglet
} else {
	TARGET = Tanglet
}

HEADERS += src/board.h \
	src/clock.h \
	src/generator.h \
	src/language_dialog.h \
	src/language_settings.h \
	src/locale_dialog.h \
	src/letter.h \
	src/new_game_dialog.h \
	src/random.h \
	src/scores_dialog.h \
	src/solver.h \
	src/trie.h \
	src/view.h \
	src/window.h \
	src/word_tree.h

SOURCES += src/board.cpp \
	src/clock.cpp \
	src/generator.cpp \
	src/language_dialog.cpp \
	src/language_settings.cpp \
	src/locale_dialog.cpp \
	src/letter.cpp \
	src/new_game_dialog.cpp \
	src/main.cpp \
	src/random.cpp \
	src/scores_dialog.cpp \
	src/solver.cpp \
	src/trie.cpp \
	src/view.cpp \
	src/window.cpp \
	src/word_tree.cpp

TRANSLATIONS = translations/cs.ts \
	translations/en.ts \
	translations/fr.ts \
	translations/he.ts

RESOURCES = icons/icons.qrc data.qrc
macx:ICON = icons/tanglet.icns
win32:RC_FILE = icons/icon.rc

unix:!macx {
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	isEmpty(BINDIR) {
		BINDIR = bin
	}

	target.path = $$PREFIX/$$BINDIR/

	data.files = data/*
	data.path = $$PREFIX/share/tanglet/data/

	icon.files = icons/tanglet.png
	icon.path = $$PREFIX/share/icons/hicolor/48x48/apps

	desktop.files = icons/tanglet.desktop
	desktop.path = $$PREFIX/share/applications/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/tanglet/translations

	INSTALLS += target icon desktop data qm
}
