# SPDX-FileCopyrightText: 2021-2022 Graeme Gott <graeme@gottcode.org>
#
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

# Configure project
project(tanglet VERSION 1.6.2 LANGUAGES CXX)

set(project_copyright "2009-2022 Graeme Gott")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools Widgets)
find_package(ZLIB REQUIRED)
include(GNUInstallDirs)

add_compile_definitions(
	QT_NO_KEYWORDS
	$<$<CONFIG:DEBUG>:QT_STRICT_ITERATORS>
	$<$<CONFIG:DEBUG>:QT_NO_NARROWING_CONVERSIONS_IN_CONNECT>
	$<$<CONFIG:DEBUG>:QT_DISABLE_DEPRECATED_BEFORE=0x060300>
)

# Version number
include(cmake/AddVersionCompileDefinition.cmake)
add_version_compile_definition(src/main.cpp VERSIONSTR)

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

# Create word lists
add_subdirectory(tools)

file(GLOB languages wordlists/*)
foreach(lang_file ${languages})
	get_filename_component(lang ${lang_file} NAME)
	set(lang_source ${CMAKE_SOURCE_DIR}/data/${lang})

	# Create wordlist for ${lang}
	set(wordlist_file ${lang_source}/words)
	add_custom_command(
		OUTPUT ${wordlist_file}
		COMMAND "$<SHELL_PATH:$<TARGET_FILE:wordlist>>" -l ${lang} -o "$<SHELL_PATH:${wordlist_file}>" "$<SHELL_PATH:${lang_file}>"
		DEPENDS ${lang_file} ${CMAKE_SOURCE_DIR}/tools/wordlist.cpp
		WORKING_DIRECTORY ${CMAKE_PREFIX_PATH}/bin # needed for Windows to find Qt
		VERBATIM
	)

	# Create dice for ${lang}
	set(dice_file ${lang_source}/dice)
	add_custom_command(
		OUTPUT ${dice_file}
		COMMAND "$<SHELL_PATH:$<TARGET_FILE:dice>>" -b -d -o "$<SHELL_PATH:${dice_file}>" "$<SHELL_PATH:${wordlist_file}>"
		DEPENDS ${wordlist_file} ${CMAKE_SOURCE_DIR}/tools/dice.cpp
		WORKING_DIRECTORY ${CMAKE_PREFIX_PATH}/bin # needed for Windows to find Qt
		VERBATIM
	)

	# Add ${lang} as dependency of target
	set(data_files ${wordlist_file} ${dice_file} ${lang_source}/language.ini)
	if(EXISTS ${lang_source}/README)
		list(APPEND data_files ${lang_source}/README)
	endif()
	target_sources(tanglet PRIVATE ${data_files})

	# Install data for ${lang}
	if(APPLE)
		set_source_files_properties(
			${data_files}
			PROPERTIES
			MACOSX_PACKAGE_LOCATION Resources/data/${lang}
		)
	elseif(NOT WIN32)
		install(FILES ${data_files} DESTINATION ${CMAKE_INSTALL_DATADIR}/tanglet/data/${lang})
	endif()
endforeach()

# Optimize build
option(ENABLE_LINK_TIME_OPTIMIZATION "Enable link time optimization" OFF)
if(ENABLE_LINK_TIME_OPTIMIZATION)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT result)
	if(result)
		set_target_properties(tanglet PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endif()

option(ENABLE_STRIP "Enable automatic stripping of builds" OFF)
if(ENABLE_STRIP)
	add_custom_command(TARGET tanglet
		POST_BUILD
		COMMAND ${CMAKE_STRIP} $<TARGET_FILE:tanglet>
	)
endif()

# Install
if(APPLE)
	set(datadir "../Resources")

	set_target_properties(tanglet PROPERTIES
		OUTPUT_NAME Tanglet
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/mac/Info.plist.in
	)

	include(cmake/BundleResources.cmake)
	bundle_data(tanglet ${CMAKE_SOURCE_DIR}/icons/tanglet.icns Resources)
	bundle_translations(tanglet "${translations_QM}")
elseif(WIN32)
	set(datadir ".")

	# Use Qt6 macro until CMake provides something
	# https://bugreports.qt.io/browse/QTBUG-87618
	set_target_properties(tanglet PROPERTIES
		OUTPUT_NAME Tanglet
		WIN32_EXECUTABLE TRUE
		QT_TARGET_VERSION "${PROJECT_VERSION}"
		QT_TARGET_COMPANY_NAME "Graeme Gott"
		QT_TARGET_DESCRIPTION "Word finding game"
		QT_TARGET_COPYRIGHT "\\xA9 ${project_copyright}"
		QT_TARGET_PRODUCT_NAME "Tanglet"
		QT_TARGET_RC_ICONS ${CMAKE_SOURCE_DIR}/icons/tanglet.ico
	)
	_qt_internal_generate_win32_rc_file(tanglet)
else()
	file(RELATIVE_PATH datadir ${CMAKE_INSTALL_FULL_BINDIR} ${CMAKE_INSTALL_FULL_DATADIR}/tanglet)

	target_sources(tanglet PRIVATE icons/icon.qrc)

	install(TARGETS tanglet RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
	install(FILES ${translations_QM} DESTINATION ${CMAKE_INSTALL_DATADIR}/tanglet/translations)
	install(FILES doc/tanglet.6 DESTINATION ${CMAKE_INSTALL_MANDIR}/man6 COMPONENT doc)
	install(DIRECTORY icons/hicolor DESTINATION ${CMAKE_INSTALL_DATADIR}/icons)
	install(FILES icons/tanglet.appdata.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo)
	install(FILES icons/tanglet.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
	install(FILES icons/tanglet.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/mime/packages)
endif()

set_property(SOURCE src/main.cpp APPEND PROPERTY COMPILE_DEFINITIONS TANGLET_DATADIR="${datadir}")
