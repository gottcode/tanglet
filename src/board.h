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

#ifndef BOARD_H
#define BOARD_H

#include <QHash>
#include <QList>
#include <QPoint>
#include <QWidget>
class QLabel;
class QLineEdit;
class QTabWidget;
class QToolButton;
class Clock;
class Generator;
class Letter;
class View;
class WordCounts;
class WordTree;

class Board : public QWidget {
	Q_OBJECT

	public:
		Board(QWidget* parent = 0);

		bool isFinished() const;
		void abort();
		void generate(int difficulty, int size, int minimum, int timer, const QStringList& letters, unsigned int seed);
		void setPaused(bool pause);

		static QString sizeToString(int size);

	public slots:
		void setShowMissedWords(bool show);
		void setShowMaximumScore(QAction* show);
		void setShowWordCounts(bool show);

	signals:
		void started();
		void finished(int score);
		void pauseAvailable(bool available);

	private slots:
		void gameStarted();
		void clearGuess();
		void guess();
		void guessChanged();
		void finish();
		void wordSelected();
		void letterClicked(Letter* letter);
		void showMaximumWords();

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
		QVector<QVector<Letter*> > m_cells;
		QLineEdit* m_guess;
		QTabWidget* m_tabs;
		WordTree* m_found;
		WordTree* m_missed;
		WordCounts* m_counts;
		QToolButton* m_clear_button;
		QToolButton* m_guess_button;
		QToolButton* m_max_score_details;

		bool m_paused;
		bool m_wrong;
		bool m_valid;
		int m_score_type;

		int m_size;
		int m_minimum;
		int m_maximum;
		int m_max_score;
		QStringList m_letters;
		QHash<QString, QList<QList<QPoint> > > m_solutions;
		QList<QPoint> m_positions;

		Generator* m_generator;
};

#endif
