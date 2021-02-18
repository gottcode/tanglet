/*
	SPDX-FileCopyrightText: 2009-2015 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BOARD_H
#define BOARD_H

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

#include <random>

class Board : public QWidget
{
	Q_OBJECT

public:
	explicit Board(QWidget* parent = nullptr);
	~Board();

	bool isFinished() const;
	void abort();
	bool generate(const QSettings& game);
	void setPaused(bool pause);

	static QString sizeToString(int size);

public slots:
	void setShowMissedWords(bool show);
	void setShowMaximumScore(QAction* show);
	void setShowWordCounts(bool show);
	void updateScoreColor();

signals:
	void started();
	void finished(int score);
	void optimizingStarted();
	void optimizingFinished();
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
	void updateButtons();

private:
	Clock* m_clock;
	View* m_view;
	QLabel* m_score;
	QVector<QVector<Letter*>> m_cells;
	QLineEdit* m_guess;
	QTabWidget* m_tabs;
	WordTree* m_found;
	WordTree* m_missed;
	WordCounts* m_counts;
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
	QHash<QString, QList<QList<QPoint>>> m_solutions;
	QList<QPoint> m_positions;

	std::mt19937 m_seed;
	Generator* m_generator;
};

#endif
