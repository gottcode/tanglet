/*
	SPDX-FileCopyrightText: 2009-2015 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef WINDOW_H
#define WINDOW_H

class Board;

#include <QHash>
#include <QMainWindow>
class QLabel;
class QStackedWidget;

/**
 * @brief The Window class controls creation of the game board and user interface.
 */
class Window : public QMainWindow
{
	Q_OBJECT

	class State;
	friend class State;
	class NewGameState;
	class OpenGameState;
	class OptimizingState;
	class PlayState;
	class AutoPauseState;
	class PauseState;
	class FinishState;

public:
	/**
	 * Constructs a main window instance.
	 * @param file a game file to load instead of the previous game
	 */
	explicit Window(const QString& file = QString());

	/**
	 * Override eventFilter to detect click on pause or new game screens.
	 * @param watched object to check
	 * @param event details of current event
	 * @return @c true if event was handled
	 */
	bool eventFilter(QObject* watched, QEvent* event) override;

protected:
	/**
	 * Override closeEvent to save window size and position.
	 * @param event details of the close event
	 */
	void closeEvent(QCloseEvent* event) override;

	/**
	 * Override dragEnterEvent to handle player dragging game file onto window.
	 * @param event details of drag event
	 */
	void dragEnterEvent(QDragEnterEvent* event) override;

	/**
	 * Override dropEvent to handle player dropping game file or data onto window. This will start
	 * a new game based on the data received.
	 * @param event details of drop event
	 */
	void dropEvent(QDropEvent* event) override;

	/**
	 * Override event to handle pausing when game window loses focus.
	 * @param event details of current event
	 * @return @c true if event was handled
	 */
	bool event(QEvent* event) override;

private slots:
	/**
	 * Shows a dialog with version information and copyright notices.
	 */
	void about();

	/**
	 * Starts a new game with the current settings, using the defaults if none have been chosen yet.
	 */
	void newRoll();

	/**
	 * Starts a new game after prompting the player to choose game settings.
	 */
	void newGame();

	/**
	 * Opens a new game.
	 */
	void chooseGame();

	/**
	 * Prompts player where to save the start details for current game.
	 */
	void shareGame();

	/**
	 * Ends current game and adds to high scores.
	 * @return @c true when game is ended, @c false otherwise
	 */
	bool endGame();

	/**
	 * Automatically pause when window is obscured by menu.
	 */
	void autoPause();

	/**
	 * Ends automatic pause and resumes game.
	 */
	void autoResume();

	/**
	 * Pauses or resumes the game.
	 * @param pause pauses the game if @c true
	 */
	void setPaused(bool paused);

	/**
	 * Shows a small dialog with the current game settings.
	 */
	void showDetails();

	/**
	 * Show the high scores.
	 */
	void showScores();

	/**
	 * Shows a dialog that allows the player to choose the board language, dice, and word list.
	 */
	void showLanguage();

	/**
	 * Shows a dialog that allows the player to choose the interface language.
	 */
	void showLocale();

	/**
	 * Shows a dialog describing the gameplay controls.
	 */
	void showControls();

	/**
	 * Inform player that the word list is being optimized.
	 */
	void optimizingStarted();

	/**
	 * Resume state before informing the player.
	 */
	void optimizingFinished();

	/**
	 * Shows game board.
	 */
	void gameStarted();

	/**
	 * Handles game ending and adds to high score board.
	 * @param score how many points the player earned
	 */
	void gameFinished(int score);

private:
	/**
	 * Actually starts a new game.
	 * @param filename location of the game data
	 */
	void startGame(const QString& filename = QString());

	/**
	 * Tracks if a menu is shown to set the game in the autopause state.
	 * @param menu which menu to track
	 */
	void monitorVisibility(QMenu* menu);

private:
	Board* m_board; /**< the play area */
	QStackedWidget* m_contents; /**< the central widget of the window, shows messages or play area */
	QAction* m_details_action; /**< show the details of the current game */
	QAction* m_pause_action; /**< controls if the game is paused */
	QLabel* m_pause_screen; /**< message screen to show the game is paused */

	State* m_state; /**< current state of the window */
	State* m_previous_state; /**< previous state of the window */
	QHash<QString, State*> m_states; /**< available states of the window */
};

#endif
