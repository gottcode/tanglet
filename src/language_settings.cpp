/*
	SPDX-FileCopyrightText: 2010-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "language_settings.h"

#include <QFile>
#include <QLocale>
#include <QSettings>

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings()
	: m_language(QLocale::system().language())
{
	loadDefaults();
}

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings(int language)
	: m_language(language)
{
	loadDefaults();
}

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings(const QSettings& group)
	: m_language(group.value("Language").toInt())
	, m_dice(group.value("Dice").toString())
	, m_words(group.value("Words").toString())
	, m_dictionary(group.value("Dictionary").toString())
{
	loadValues();
	loadDefaults();
}

//-----------------------------------------------------------------------------

void LanguageSettings::loadDefaults()
{
	QString iso_code = QLocale(QLocale::Language(m_language)).name().section(QLatin1Char('_'), 0, 0);
	if (!QFile::exists(QString("tanglet:%1/words").arg(iso_code))) {
		m_language = QLocale::English;
		iso_code = "en";
	}

	if (m_dice.isEmpty()) {
		m_dice = QString("tanglet:%1/dice").arg(iso_code);
	}

	if (m_words.isEmpty()) {
		m_words = QString("tanglet:%1/words").arg(iso_code);
	}

	if (m_dictionary.isEmpty()) {
		QSettings settings(QString("tanglet:%1/language.ini").arg(iso_code), QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		settings.setIniCodec("UTF-8");
#endif
		m_dictionary = settings.value("Language/Dictionary").toString();
	}
	if (m_dictionary.isEmpty()) {
		m_dictionary = QString("https://%1.wiktionary.org/wiki/%s").arg(iso_code);
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::loadValues()
{
	QSettings settings;
	settings.beginGroup("Board");

	if (!m_language) {
		m_language = settings.value("Language").toInt();
	}
	if (!m_language) {
		m_language = QLocale::system().language();
	}

	if (m_dice.isEmpty()) {
		m_dice = settings.value("Dice").toString();
	}

	if (m_words.isEmpty()) {
		m_words = settings.value("Words").toString();
	}

	if (m_dictionary.isEmpty()) {
		m_dictionary = settings.value("Dictionary").toString();
	}
}

//-----------------------------------------------------------------------------
