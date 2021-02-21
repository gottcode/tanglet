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

class Window : public QMainWindow
{
	Q_OBJECT

public:
	explicit Window(const QString& file = QString());

	bool eventFilter(QObject* watched, QEvent* event) override;

protected:
	void closeEvent(QCloseEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dropEvent(QDropEvent* event) override;
	bool event(QEvent* event) override;

private slots:
	void about();
	void aboutHspell();
	void aboutScowl();
	void newRoll();
	void newGame();
	void chooseGame();
	void shareGame();
	bool endGame();
	void autoPause();
	void autoResume();
	void setPaused(bool paused);
	void showDetails();
	void showScores();
	void showLanguage();
	void showLocale();
	void showControls();
	void optimizingStarted();
	void optimizingFinished();
	void gameStarted();
	void gameFinished(int score);

private:
	void startGame(const QString& filename = QString());
	void monitorVisibility(QMenu* menu);

private:
	Board* m_board;
	QStackedWidget* m_contents;
	QAction* m_details_action;
	QAction* m_pause_action;
	QLabel* m_pause_screen;

	class State;
	friend class State;
	class NewGameState;
	class OpenGameState;
	class OptimizingState;
	class PlayState;
	class AutoPauseState;
	class PauseState;
	class FinishState;
	State* m_state;
	State* m_previous_state;
	QHash<QString, State*> m_states;
};

#endif
