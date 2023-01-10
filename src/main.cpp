/*
	SPDX-FileCopyrightText: 2009-2022 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "generator.h"
#include "language_dialog.h"
#include "locale_dialog.h"
#include "scores_dialog.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	app.setApplicationName("Tanglet");
	app.setApplicationVersion(VERSIONSTR);
	app.setApplicationDisplayName(Window::tr("Tanglet"));
	app.setOrganizationDomain("gottcode.org");
	app.setOrganizationName("GottCode");
#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
	app.setWindowIcon(QIcon::fromTheme("tanglet", QIcon(":/tanglet.png")));
	app.setDesktopFileName("tanglet");
#endif

	// Find application data
	const QString appdir = app.applicationDirPath();
	const QString datadir = QDir::cleanPath(appdir + "/" + TANGLET_DATADIR);

	// Handle portability
	QString userdir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#ifdef Q_OS_MAC
	const QFileInfo portable(appdir + "/../../../Data");
#else
	const QFileInfo portable(appdir + "/Data");
#endif
	if (portable.exists() && portable.isWritable()) {
		userdir = portable.absoluteFilePath();
		QSettings::setDefaultFormat(QSettings::IniFormat);
		QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, userdir + "/Settings");
	}

	// Load application language
	LocaleDialog::loadTranslator("tanglet_", datadir);

	// Handle commandline
	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::translate("main", "Word finding game"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("file", QCoreApplication::translate("main", "A game file to play."), "[file]");
	parser.process(app);

	// Find word lists
	QDir::setSearchPaths("tanglet", { datadir + "/gamedata/" });

	// Set where to cache tries
	if (!QFileInfo::exists(userdir + "/Trie") && QFileInfo::exists(userdir + "/cache")) {
		QDir dir(userdir);
		dir.rename("cache", "Trie");
	}
	Generator::setTriePath(userdir + "/Trie");

	// Set where to store imported games
	Window::setDataPath(userdir);

	// Load default board language
	LanguageDialog::restoreDefaults();

	// Convert old scores to new format
	ScoresDialog::migrate();

	// Create main window
	QStringList files = parser.positionalArguments();
	Window window(files.isEmpty() ? QString() : files.front());
	window.show();

	return app.exec();
}
