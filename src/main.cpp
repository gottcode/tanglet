/*
	SPDX-FileCopyrightText: 2009-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "language_dialog.h"
#include "locale_dialog.h"
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

	QSettings settings;
	if (settings.value("Language", -1).toInt() == -1) {
		int default_count = 0;
		if (settings.value("Dice", ":/en_US/dice").toString() == ":/en_US/dice") {
			settings.setValue("Dice", "tanglet:en/dice");
			default_count++;
		}
		if (settings.value("Words", ":/en_US/words").toString() == ":/en_US/words") {
			settings.setValue("Words", "tanglet:en/words");
			default_count++;
		}
		if (settings.value("Dictionary").toString().isEmpty()) {
			settings.setValue("Dictionary", "http://en.wiktionary.org/wiki/%s");
			default_count++;
		}
		if (default_count == 3) {
			LanguageDialog::restoreDefaults();
		} else {
			settings.setValue("Language", 0);
		}
	}

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
