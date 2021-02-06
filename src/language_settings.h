/*
	SPDX-FileCopyrightText: 2010-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef LANGUAGE_SETTINGS_H
#define LANGUAGE_SETTINGS_H

#include <QString>

class LanguageSettings
{
public:
	LanguageSettings(const QString& group = QString());
	LanguageSettings(int language);
	~LanguageSettings();

	bool isChanged() const;
	int language() const;
	QString dice() const;
	QString words() const;
	QString dictionary() const;
	void setLanguage(int language);
	void setDice(const QString& dice);
	void setWords(const QString& words);
	void setDictionary(const QString& dictionary);

private:
	void loadDefault();

private:
	int m_language;
	QString m_dice;
	QString m_words;
	QString m_dictionary;
	bool m_changed;
};

#endif
