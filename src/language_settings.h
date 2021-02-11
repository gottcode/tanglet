/*
	SPDX-FileCopyrightText: 2010-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef LANGUAGE_SETTINGS_H
#define LANGUAGE_SETTINGS_H

class QSettings;

#include <QString>

class LanguageSettings
{
public:
	explicit LanguageSettings();
	explicit LanguageSettings(int language);
	explicit LanguageSettings(const QSettings& group);
	explicit LanguageSettings(const QString& group);

	int language() const
	{
		return m_language;
	}

	QString dice() const
	{
		return m_dice;
	}

	QString words() const
	{
		return m_words;
	}

	QString dictionary() const
	{
		return m_dictionary;
	}

private:
	void loadDefaults();
	void loadValues();

private:
	int m_language;
	QString m_dice;
	QString m_words;
	QString m_dictionary;
};

#endif
