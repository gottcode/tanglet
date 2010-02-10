/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#ifndef BOARD_H
#define BOARD_H

#include "trie.h"

#include <QHash>
#include <QList>
#include <QPoint>
#include <QWidget>
class QLabel;
class QLineEdit;
class QMainWindow;
class QTabWidget;
class QToolButton;
class QTreeWidgetItem;
class Clock;
class Letter;
class View;
class WordTree;

class Board : public QWidget {
	Q_OBJECT

	public:
		Board(QWidget* parent = 0);

		bool isFinished() const;
		void abort();
		void generate();
		void setPaused(bool pause);

		void loadSettings();

	signals:
		void finished(int score);
		void pauseAvailable(bool available);

	private slots:
		void clearGuess();
		void guess();
		void guessChanged();
		void finish();
		void wordSelected();
		void letterClicked(Letter* letter);

	private:
		void highlightWord(const QList<QPoint>& positions, const QColor& color);
		void highlightWord();
		void clearHighlight();
		void selectGuess();
		int updateScore();
		void updateClickableStatus();

	private:
		Clock* m_clock;
		View* m_view;
		QLabel* m_score;
		Letter* m_cells[4][4];
		QLineEdit* m_guess;
		QTabWidget* m_tabs;
		WordTree* m_found;
		WordTree* m_missed;
		QToolButton* m_clear_button;
		QToolButton* m_guess_button;

		bool m_paused;
		bool m_wrong;
		bool m_valid;
		int m_score_type;
		bool m_higher_scores;

		QList<QStringList> m_dice;
		Trie m_words;
		QStringList m_letters;
		QHash<QString, QList<QList<QPoint> > > m_solutions;
		QList<QPoint> m_positions;
};

#endif
