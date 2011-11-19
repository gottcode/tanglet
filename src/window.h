/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011 Graeme Gott <graeme@gottcode.org>
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

#ifndef WINDOW_H
#define WINDOW_H

#include <QHash>
#include <QMainWindow>
class QLabel;
class QStackedWidget;
class Board;

class Window : public QMainWindow {
	Q_OBJECT

	public:
		Window();

		virtual bool eventFilter(QObject* watched, QEvent* event);

	protected:
		virtual void closeEvent(QCloseEvent* event);
		virtual bool event(QEvent* event);

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
