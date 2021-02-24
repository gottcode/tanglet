/*
	SPDX-FileCopyrightText: 2009-2015 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_BOARD_H
#define TANGLET_BOARD_H

class Clock;
class Generator;
class Letter;
class View;
class WordCounts;
class WordTree;

#include <QHash>
#include <QList>
#include <QPoint>
#include <QWidget>
class QLabel;
class QLineEdit;
class QSettings;
class QTabWidget;
class QToolButton;

/**
 * @brief The Board class controls the play area of the game, including the word lists and score area.
 */
class Board : public QWidget
{
	Q_OBJECT

public:
	/**
	 * Constructs a board instance.
	 * @param parent the QWidget that manages the board
	 */
	explicit Board(QWidget* parent = nullptr);

	/**
	 * Destroys the board.
	 */
	~Board();

	/**
	 * @return whether the current game has finished
	 */
	bool isFinished() const;

	/**
	 * Cancels the current running game.
	 */
	void abort();

	/**
	 * Creates a new game in a thread, replacing the contents of the game board.
	 * @param game the details to use to create a game
	 * @return @c true if the game is valid and will be started
	 */
	bool generate(const QSettings& game);

	/**
	 * Pauses or resumes the game.
	 * @param pause pauses the game if @c true
	 */
	void setPaused(bool pause);

	static QString sizeToString(int size);

public slots:
	/**
	 * Sets the visibility of the missed words tab.
	 * @param show whether to show the missed words
	 */
	void setShowMissedWords(bool show);

	/**
	 * Sets the visibility of the maximum score of the game and each word.
	 * @param show whether to show the maximum score
	 */
	void setShowMaximumScore(QAction* show);

	/**
	 * Sets the visibility of the word counts for each length.
	 * @param show whether to show the word counts
	 */
	void setShowWordCounts(bool show);

	/**
	 * Updates the score display based on if it is a high score and if it is the highest score.
	 */
	void updateScoreColor();

signals:
	/**
	 * Emitted when the game has been generated and is now playable.
	 */
	void started();

	/**
	 * Emitted when the game has finished.
	 * @param score how many points the player earned
	 */
	void finished(int score);

	/**
	 * Emitted when the word list is being optimized for fast access.
	 */
	void optimizingStarted();

	/**
	 * Emitted when the word list has been optimized.
	 */
	void optimizingFinished();

	/**
	 * Emitted when the game starts or finishes.
	 * @param available @c true if the game is running and can be paused
	 */
	void pauseAvailable(bool available);

private slots:
	/**
	 * Fills the contents of the play area (letters, word lists, maximum scores) after the generator
	 * thread finishes. It also updates the current game in the settings so that it will resume on
	 * next launch.
	 */
	void gameStarted();

	/**
	 * Clears the current guess from the board.
	 */
	void clearGuess();

	/**
	 * Processes a guess for a word made by the player. If they are correct, the guess is added to
	 * the found word list and the current guess is cleared.
	 */
	void guess();

	/**
	 * Updates the current guess when the player types in the line edit.
	 */
	void guessChanged();

	/**
	 * Ends the current running game.
	 */
	void finish();

	/**
	 * Shows the word list selection on the game board.
	 */
	void wordSelected();

	/**
	 * Processes the player clicking on a letter on the game board.
	 * @param letter which letter the player clicked on
	 */
	void letterClicked(Letter* letter);

	/**
	 * Displays a dialog listing the 30 words used to determine the maximum score in an Allotment game.
	 */
	void showMaximumWords();

private:
	/**
	 * Highlight a word on the board.
	 * @param positions the locations of the letters to highlight
	 * @param color the color used to highlight the word
	 */
	void highlightWord(const QList<QPoint>& positions, const QColor& color);

	/**
	 * Determine the state of the guess and then calls highlightWord(const QList<QPoint>&, const QColor&)
	 * to actually update the letters on the board.
	 */
	void highlightWord();

	/**
	 * Removes all highlight colors on the board and sets the letters to white if the game is still
	 * running, or gray if the game is over.
	 */
	void clearHighlight();

	/**
	 * Checks if a word has already been found before, and if so it selects it in the found word list.
	 */
	void selectGuess();

	/**
	 * Calculates the current score by adding up the values in the found word list. It updates the
	 * display of the score as well as the display of the maximum scores for each word length.
	 * @return the value of the score
	 */
	int updateScore();

	/**
	 * Processes the board and marks which cells can be clicked on to continue making a guess. If
	 * the game is over, it disables clicking on the letters.
	 */
	void updateClickableStatus();

	/**
	 * Enables or disables the buttons next to the line edit based on the length of the current guess.
	 */
	void updateButtons();

private:
	Clock* m_clock; /**< shows the time remaining in the game */
	View* m_view; /**< game area that shows the letters */
	QLabel* m_score; /**< shows how many points the player has earned */
	QVector<QVector<Letter*>> m_cells; /**< contains the letters that make up the board */
	QLineEdit* m_guess; /**< allows players to type a guess instead of click on it */
	QTabWidget* m_tabs; /**< tabs containing the word lists */
	WordTree* m_found; /**< list of found words */
	WordTree* m_missed; /**< list of missing words */
	WordCounts* m_counts; /**< displays counts of words */
	QToolButton* m_guess_button; /**< allows player to make a guess when typing */
	QToolButton* m_max_score_details; /**< displays the words used to make the maximum score with Allotment timer */

	bool m_paused; /**< is the game currently paused */
	bool m_wrong; /**< is the clicked guess wrong (will color positions on board) */
	bool m_wrong_typed; /**< is the typed guess wrong (no positions on board to color) */
	int m_show_counts; /**< are maximum scores for each word length shown */

	int m_size; /**< how many letters wide is the board */
	int m_minimum; /**< the shortest word allowed */
	int m_maximum; /**< the longest word possible */
	int m_max_score; /**< the maximum score available */
	QStringList m_letters; /**< the board layout */
	QHash<QString, QList<QList<QPoint>>> m_solutions; /**< all available words and their positions on the board */
	QList<QPoint> m_positions; /**< the currently selected positions */

	Generator* m_generator; /**< builds the board layout */
};

#endif // TANGLET_BOARD_H
