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

#ifndef LANGUAGE_SETTINGS_H
#define LANGUAGE_SETTINGS_H

#include <QString>

class LanguageSettings {
	public:
		LanguageSettings();
		~LanguageSettings();

		int language() const;
		QString dice() const;
		QString words() const;
		QString dictionary() const;
		void setLanguage(int language);
		void setDice(const QString& dice);
		void setWords(const QString& words);
		void setDictionary(const QString& dictionary);

	private:
		int m_language;
		QString m_dice;
		QString m_words;
		QString m_dictionary;
		bool m_changed;
};

#endif
