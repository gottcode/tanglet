/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef SOLVER_H
#define SOLVER_H

#include "trie.h"

#include <QHash>
#include <QPoint>
#include <QVector>

class Solver
{
public:
	Solver(const Trie& words, int size, int minimum);

	void solve(const QStringList& letters);

	int count() const
	{
		return m_count;
	}

	QHash<QString, QList<QList<QPoint>>> solutions() const
	{
		return m_solutions;
	}

	int score(int max = -1) const;
	static int score(const QString& word);

	void setTrackPositions(bool track_positions);

private:
	struct Cell;
	void checkCell(Cell& cell);

private:
	const Trie* m_words;
	const Trie::Node* m_node;
	int m_size;
	int m_minimum;
	bool m_track_positions;
	QString m_word;
	QList<QPoint> m_positions;
	QHash<QString, QList<QList<QPoint>>> m_solutions;
	int m_count;

	struct Cell
	{
		QString text;
		QVector<Cell*> neighbors;
		QPoint position;
		bool checked;
	};
	QVector<QVector<Cell>> m_cells;
};

#endif
