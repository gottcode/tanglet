/*
	SPDX-FileCopyrightText: 2010-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_LANGUAGE_SETTINGS_H
#define TANGLET_LANGUAGE_SETTINGS_H

class QSettings;

#include <QString>

/**
 * @brief The LanguageSettings class defines the set of language settings for game play.
 */
class LanguageSettings
{
public:
	/**
	 * Constructs a language instance. Uses system locale to load defaults.
	 */
	explicit LanguageSettings();

	/**
	 * Constructs a language instance.
	 * @param language what language to load defaults for
	 */
	explicit LanguageSettings(int language);

	/**
	 * Constructs a language instance.
	 * @param group where to load the language settings
	 */
	explicit LanguageSettings(const QSettings& group);

	/**
	 * @return QLocale::Language value to specify a language
	 */
	int language() const
	{
		return m_language;
	}

	/**
	 * @return path to load dice
	 */
	QString dice() const
	{
		return m_dice;
	}

	/**
	 * @return path to load words
	 */
	QString words() const
	{
		return m_words;
	}

	/**
	 * @return URL to define words
	 */
	QString dictionary() const
	{
		return m_dictionary;
	}

private:
	/**
	 * Load the default values based on m_language as fallbacks.
	 */
	void loadDefaults();

	/**
	 * Load the values from the Board group of the settings as fallbacks.
	 */
	void loadValues();

private:
	int m_language; /**< QLocale::Language value to specify a language */
	QString m_dice; /**< where to load dice */
	QString m_words; /**< where to load words */
	QString m_dictionary; /**< where to define words */
};

#endif // TANGLET_LANGUAGE_SETTINGS_H
