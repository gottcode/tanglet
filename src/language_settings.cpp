/***********************************************************************
 *
 * Copyright (C) 2010, 2011 Graeme Gott <graeme@gottcode.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "language_settings.h"

#include <QFile>
#include <QLocale>
#include <QSettings>
#include <QTextStream>

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings(const QString& group)
: m_changed(false) {
	QSettings settings;

	int language = settings.value("Language", QLocale::system().language()).toInt();
	QString dice = settings.value("Dice").toString();
	QString words = settings.value("Words").toString();
	QString dictionary = settings.value("Dictionary").toString();

	if (!group.isEmpty()) {
		settings.beginGroup(group);

		m_language = settings.value("Language", language).toInt();
		m_dice = settings.value("Dice", dice).toString();
		m_words = settings.value("Words", words).toString();
		m_dictionary = settings.value("Dictionary", dictionary).toString();
	} else {
		m_language = language;
		if (language > 0) {
			loadDefault();
			m_changed = (m_dice != dice) || (m_words != words) || (m_dictionary != dictionary);
		} else {
			m_dice = dice;
			m_words = words;
			m_dictionary = dictionary;
		}
	}
}

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings(int language)
: m_changed(false) {
	m_language = language;
	loadDefault();
}

//-----------------------------------------------------------------------------

LanguageSettings::~LanguageSettings() {
	if (m_changed) {
		QSettings settings;

		settings.setValue("Language", m_language);
		settings.setValue("Dice", m_dice);
		settings.setValue("Words", m_words);
		settings.setValue("Dictionary", m_dictionary);

		if (m_language == 0)  {
			settings.setValue("CustomDice", m_dice);
			settings.setValue("CustomWords", m_words);
			settings.setValue("CustomDictionary", m_dictionary);
		}
	}
}

//-----------------------------------------------------------------------------

bool LanguageSettings::isChanged() const {
	return m_changed;
}

//-----------------------------------------------------------------------------

int LanguageSettings::language() const {
	return m_language;
}

//-----------------------------------------------------------------------------

QString LanguageSettings::dice() const {
	return m_dice;
}

//-----------------------------------------------------------------------------

QString LanguageSettings::words() const {
	return m_words;
}

//-----------------------------------------------------------------------------

QString LanguageSettings::dictionary() const {
	return m_dictionary;
}

//-----------------------------------------------------------------------------

void LanguageSettings::setLanguage(int language) {
	if (m_language != language) {
		m_language = language;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::setDice(const QString& dice) {
	if (m_dice != dice) {
		m_dice = dice;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::setWords(const QString& words) {
	if (m_words != words) {
		m_words = words;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::setDictionary(const QString& dictionary) {
	if (m_dictionary != dictionary) {
		m_dictionary = dictionary;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::loadDefault() {
	QString iso_code = QLocale(static_cast<QLocale::Language>(m_language)).name().left(2);
	m_dice = "tanglet:" + iso_code + "/dice";
	m_words = "tanglet:" + iso_code + "/words";
	QString dictionary;
	QFile file("tanglet:" + iso_code + "/dictionary");
	if (file.open(QFile::ReadOnly | QFile::Text)) {
		QTextStream stream(&file);
		stream.setCodec("UTF-8");
		dictionary = stream.readLine().simplified();
		file.close();
	}
	m_dictionary = !dictionary.isEmpty() ? dictionary : ("http://" + iso_code + ".wiktionary.org/wiki/%s");
}

//-----------------------------------------------------------------------------
