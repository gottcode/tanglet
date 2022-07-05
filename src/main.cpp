/*
	SPDX-FileCopyrightText: 2009-2022 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "language_dialog.h"
#include "locale_dialog.h"
#include "scores_dialog.h"
#include "window.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>
#include <QSettings>

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

	const QString appdir = app.applicationDirPath();
	const QStringList datadirs{
#if defined(Q_OS_MAC)
		appdir + "/../Resources"
#elif defined(Q_OS_UNIX)
		DATADIR,
		appdir + "/../share/tanglet"
#else
		appdir
#endif
	};

	QStringList paths;
	for (const QString& datadir : datadirs) {
		paths.append(datadir + "/data/");
	}
	QDir::setSearchPaths("tanglet", paths);

	LocaleDialog::loadTranslator("tanglet_", datadirs);

	LanguageDialog::restoreDefaults();

	ScoresDialog::migrate();

	QCommandLineParser parser;
	parser.setApplicationDescription(QCoreApplication::translate("main", "Word finding game"));
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("file", QCoreApplication::translate("main", "A game file to play."), "[file]");
	parser.process(app);

	QStringList files = parser.positionalArguments();
	Window window(files.isEmpty() ? QString() : files.front());
	window.show();

	return app.exec();
}
