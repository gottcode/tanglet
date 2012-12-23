TEMPLATE = app
greaterThan(QT_MAJOR_VERSION, 4) {
	QT += widgets
}
CONFIG += warn_on
macx {
	CONFIG += x86_64
}

!win32 {
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
	src/random.h \
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
	src/random.cpp \
	src/scores_dialog.cpp \
	src/solver.cpp \
	src/trie.cpp \
	src/view.cpp \
	src/window.cpp \
	src/word_counts.cpp \
	src/word_tree.cpp

TRANSLATIONS = translations/tanglet_cs.ts \
	translations/tanglet_de.ts \
	translations/tanglet_en.ts \
	translations/tanglet_es.ts \
	translations/tanglet_es_CL.ts \
	translations/tanglet_fr.ts \
	translations/tanglet_he.ts \
	translations/tanglet_nl.ts \
	translations/tanglet_uk.ts

RESOURCES = icons/icons.qrc data.qrc
macx:ICON = icons/tanglet.icns
win32:RC_FILE = icons/icon.rc

macx {
	GAME_DATA.files = data
	GAME_DATA.path = Contents/Resources

	QMAKE_BUNDLE_DATA += GAME_DATA
}

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

	icon.files = icons/hicolor/*
	icon.path = $$PREFIX/share/icons/hicolor/

	pixmap.files = icons/tanglet_32.xpm
	pixmap.path = $$PREFIX/share/pixmaps/

	desktop.files = icons/tanglet.desktop
	desktop.path = $$PREFIX/share/applications/

	qm.files = translations/*.qm
	qm.path = $$PREFIX/share/tanglet/translations

	INSTALLS += target icon pixmap desktop data qm
}
