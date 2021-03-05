/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

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
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
#if !defined(Q_OS_MAC)
	if (!qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO")
			&& !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
			&& !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
			&& !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
		QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	}
#endif
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
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

	QString path = app.applicationDirPath();
	QStringList paths;
	paths.append(path + "/data/");
	paths.append(path + "/../share/tanglet/data/");
	paths.append(path + "/../Resources/data/");
	QDir::setSearchPaths("tanglet", paths);

	LocaleDialog::loadTranslator("tanglet_");

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
