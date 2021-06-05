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
	: m_language(QLocale::system().name())
{
	loadDefaults();
}

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings(const QString& language)
	: m_language(language)
{
	loadDefaults();
}

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings(const QSettings& group)
	: m_language(group.value("Locale").toString())
	, m_dice(group.value("Dice").toString())
	, m_words(group.value("Words").toString())
	, m_dictionary(group.value("Dictionary").toString())
{
	if (m_language.isEmpty()) {
		const int language = group.value("Language").toInt();
		m_language = QLocale(QLocale::Language(language)).name().section('_', 0, 0);
	}

	loadValues();
	loadDefaults();
}

//-----------------------------------------------------------------------------

void LanguageSettings::loadDefaults()
{
	if (!QFile::exists(QString("tanglet:%1/words").arg(m_language))) {
		m_language = m_language.section('_', 0, 0);
		if (!QFile::exists(QString("tanglet:%1/words").arg(m_language))) {
			m_language = "en";
		}
	}

	if (m_dice.isEmpty()) {
		m_dice = QString("tanglet:%1/dice").arg(m_language);
	}

	if (m_words.isEmpty()) {
		m_words = QString("tanglet:%1/words").arg(m_language);
	}

	if (m_dictionary.isEmpty()) {
		QSettings settings(QString("tanglet:%1/language.ini").arg(m_language), QSettings::IniFormat);
		m_dictionary = settings.value("Language/Dictionary").toString();
	}
	if (m_dictionary.isEmpty()) {
		m_dictionary = QString("https://%1.wiktionary.org/wiki/%s").arg(m_language);
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::loadValues()
{
	QSettings settings;
	settings.beginGroup("Board");

	if (m_language.isEmpty()) {
		m_language = settings.value("Locale").toString();
	}
	if (m_language.isEmpty()) {
		m_language = QLocale::system().name();
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
