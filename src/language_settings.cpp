/***********************************************************************
 *
 * Copyright (C) 2010 Graeme Gott <graeme@gottcode.org>
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

#include <QLocale>
#include <QSettings>

//-----------------------------------------------------------------------------

LanguageSettings::LanguageSettings()
: m_changed(false), m_new(false) {
	QSettings settings;

	m_language = settings.value("Language", QLocale::system().language()).toInt();
	m_dice = settings.value("Dice").toString();
	m_words = settings.value("Words").toString();
	m_dictionary = settings.value("Dictionary").toString();
}

//-----------------------------------------------------------------------------

LanguageSettings::~LanguageSettings() {
	if (isChanged()) {
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
		m_new = true;
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::setDice(const QString& dice) {
	if (m_dice != dice) {
		m_dice = dice;
		m_changed = true;
		m_new = true;
	}
}

//-----------------------------------------------------------------------------

void LanguageSettings::setWords(const QString& words) {
	if (m_words != words) {
		m_words = words;
		m_changed = true;
		m_new = true;
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
