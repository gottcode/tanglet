/*
	SPDX-FileCopyrightText: 2009-2014 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_LANGUAGE_DIALOG_H
#define TANGLET_LANGUAGE_DIALOG_H

#include <QDialog>
class QAbstractButton;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;

/**
 * @brief The LanguageDialog class allows the player to choose the board language.
 */
class LanguageDialog : public QDialog
{
	Q_OBJECT

public:
	/**
	 * Constructs a language dialog.
	 * @param parent the QWidget that manages the dialog
	 */
	LanguageDialog(QWidget* parent = nullptr);

	/**
	 * Migrates old board language settings to the new location. It also removes the settings if
	 * they are merely the defaults.
	 */
	static void restoreDefaults();

public Q_SLOTS:
	/**
	 * Saves the new language settings.
	 */
	void accept() override;

private Q_SLOTS:
	/**
	 * Checks if the player has activated the clear button and resets if they have.
	 * @param button which dialog button the player activated
	 */
	void clicked(QAbstractButton* button);

	/**
	 * Sets the default placeholder text of the dice, words, and dictionary to that of the language.
	 * @param index which language settings to load
	 */
	void chooseLanguage(int index);

	/**
	 * Pops up a dialog for the player to browse the filesystem for a dice file.
	 */
	void browseDice();

	/**
	 * Sets the value of the custom dice file, or resets if they have chosen the default value for
	 * the dice by blanking the field.
	 * @param dice the file for the custom dice.
	 */
	void chooseDice(const QString& dice);

	/**
	 * Pops up a dialog for the player to browse the filesystem for a word list.
	 */
	void browseWords();

	/**
	 * Sets the value of the custom word list, or resets if they have chosen the default value for
	 * the word list by blanking the field.
	 * @param words the file for the custom word list.
	 */
	void chooseWords(const QString& words);

private:
	/**
	 * Sets the placeholder language by language ID instead of index in combobox.
	 * @param language the language to load the settings from
	 */
	void setLanguage(const QString& language);

private:
	QComboBox* m_language; /**< the language selector */
	QString m_dice_path; /**< location of custom dice */
	QLineEdit* m_dice; /**< entry for custom dice */
	QString m_words_path; /**< location of custom word list */
	QLineEdit* m_words; /**< entry for custom word list */
	QLineEdit* m_dictionary; /**< entry for custom dictionary */

	QDialogButtonBox* m_buttons; /**< buttons to control dialog */
};

#endif // TANGLET_LANGUAGE_DIALOG_H
