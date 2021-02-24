/*
	SPDX-FileCopyrightText: 2010-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_NEW_GAME_DIALOG_H
#define TANGLET_NEW_GAME_DIALOG_H

#include <QDialog>
class QComboBox;
class QToolButton;

/**
 * @brief The NewGameDialog class allows the player to choose the new game settings.
 */
class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	/**
	 * Constructs a new game dialog.
	 * @param parent the QWidget that manages the dialog
	 */
	NewGameDialog(QWidget* parent = nullptr);

	/**
	 * Fetch the description of an amount of words.
	 * @param density the word range
	 * @return the translated description of a word density range
	 */
	static QString densityString(int density);

private slots:
	/**
	 * Tracks the length of the shortest allowed word.
	 * @param length the player selected word length
	 */
	void lengthChanged(int length);

	/**
	 * Adjusts the range of minimum length words when the board size is changed.
	 */
	void sizeChanged();

	/**
	 * The player has chosen a timer so the settings are stored.
	 * @param timer which timer mode the player has chosen
	 */
	void timerChosen(int timer);

private:
	QToolButton* m_normal_size; /**< option for a normal board */
	QToolButton* m_large_size; /**< option for a large board */
	QComboBox* m_density; /**< option to select the amount of words on the board */
	QComboBox* m_length; /**< option to select minimum word length */
	int m_minimum; /**< length of the shortest word allowed */
};

#endif // TANGLET_NEW_GAME_DIALOG_H
