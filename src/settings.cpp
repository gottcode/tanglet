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

#include "settings.h"

#include <QLocale>
#include <QSettings>

//-----------------------------------------------------------------------------

Settings::Settings()
: m_changed(false), m_new(false) {
	QSettings settings;

	m_score_type = settings.value("Gameplay/ScoreType", 1).toInt();
	m_show_missed = settings.value("Gameplay/ShowMissed", true).toBool();

	m_language = settings.value("Language", QLocale::system().language()).toInt();
	m_dice = settings.value("Dice").toString();
	m_words = settings.value("Words").toString();
	m_dictionary = settings.value("Dictionary").toString();
}

//-----------------------------------------------------------------------------

Settings::~Settings() {
	if (isChanged()) {
		QSettings settings;

		settings.setValue("Gameplay/ScoreType", m_score_type);
		settings.setValue("Gameplay/ShowMissed", m_show_missed);

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

int Settings::scoreType() const {
	return m_score_type;
}

//-----------------------------------------------------------------------------

bool Settings::showMissed() const {
	return m_show_missed;
}

//-----------------------------------------------------------------------------

void Settings::setScoreType(int type) {
	if (m_score_type != type) {
		m_score_type = type;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------

void Settings::setShowMissed(bool show) {
	if (m_show_missed != show) {
		m_show_missed = show;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------

int Settings::language() const {
	return m_language;
}

//-----------------------------------------------------------------------------

QString Settings::dice() const {
	return m_dice;
}

//-----------------------------------------------------------------------------

QString Settings::words() const {
	return m_words;
}

//-----------------------------------------------------------------------------

QString Settings::dictionary() const {
	return m_dictionary;
}

//-----------------------------------------------------------------------------

void Settings::setLanguage(int language) {
	if (m_language != language) {
		m_language = language;
		m_changed = true;
		m_new = true;
	}
}

//-----------------------------------------------------------------------------

void Settings::setDice(const QString& dice) {
	if (m_dice != dice) {
		m_dice = dice;
		m_changed = true;
		m_new = true;
	}
}

//-----------------------------------------------------------------------------

void Settings::setWords(const QString& words) {
	if (m_words != words) {
		m_words = words;
		m_changed = true;
		m_new = true;
	}
}

//-----------------------------------------------------------------------------

void Settings::setDictionary(const QString& dictionary) {
	if (m_dictionary != dictionary) {
		m_dictionary = dictionary;
		m_changed = true;
	}
}

//-----------------------------------------------------------------------------
