cmake_minimum_required(VERSION 3.16.0)

# Configure project
project(tanglet VERSION 1.6.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools Widgets)
find_package(ZLIB REQUIRED)
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)
endif()

add_compile_definitions(
	QT_STRICT_ITERATORS
	QT_NO_NARROWING_CONVERSIONS_IN_CONNECT
	QT_DISABLE_DEPRECATED_BEFORE=0x060000
)

# Create translations
file(GLOB translations_SRCS translations/*.ts)
qt6_add_translation(translations_QM ${translations_SRCS})

add_custom_target(update_translations
	COMMAND Qt6::lupdate -no-obsolete -locations none src -ts ${translations_SRCS}
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	VERBATIM
)

# Create program
qt6_add_executable(tanglet
	src/beveled_rect.cpp
	src/board.cpp
	src/clock.cpp
	src/generator.cpp
	src/gzip.cpp
	src/language_dialog.cpp
	src/language_settings.cpp
	src/locale_dialog.cpp
	src/letter.cpp
	src/new_game_dialog.cpp
	src/main.cpp
	src/scores_dialog.cpp
	src/solver.cpp
	src/trie.cpp
	src/view.cpp
	src/window.cpp
	src/word_counts.cpp
	src/word_tree.cpp
	icons/icons.qrc
	${translations_QM}
)

target_link_libraries(tanglet PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Widgets
	ZLIB::ZLIB
)

target_compile_definitions(tanglet PRIVATE VERSIONSTR="${PROJECT_VERSION}")

# Create word lists
add_subdirectory(wordlists)

# Install
if(UNIX AND NOT APPLE)
	target_sources(tanglet PRIVATE icons/icon.qrc)

	install(TARGETS tanglet RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/tanglet/translations)
	install(FILES doc/tanglet.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
	install(FILES icons/tanglet.xpm DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps)
	install(FILES icons/tanglet.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	install(FILES icons/tanglet.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
	install(FILES icons/tanglet.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
elseif(WIN32)
	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(tanglet PROPERTIES
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Word finding game"
		QT_TARGET_COPYRIGHT "Copyright (C) 2021 Graeme Gott"
		QT_TARGET_PRODUCT_NAME "Tanglet"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/tanglet.ico
	)
	_qt_internal_generate_win32_rc_file(tanglet)
endif()
