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

#include "solver.h"

#include "trie.h"

#include <QStringList>

//-----------------------------------------------------------------------------

Solver::Solver(const Trie& words, const QStringList& letters, int minimum)
: m_words(&words), m_minimum(minimum) {
	// Find size
	int size = 0;
	switch (letters.count()) {
	case 16:
		size = 4;
		break;
	case 25:
		size = 5;
		break;
	default:
		return;
	}

	// Create neighbors
	QList<QList<QPoint> > neighbors;
	const QPoint deltas[] = {
		QPoint(-1,-1),
		QPoint(0,-1),
		QPoint(1,-1),
		QPoint(-1,0),
		QPoint(1,0),
		QPoint(-1,1),
		QPoint(0,1),
		QPoint(1,1)
	};
	for (int r = 0; r < size; ++r) {
		for (int c = 0; c < size; ++c) {
			QList<QPoint> positions;
			QPoint pos(c, r);
			for (int i = 0; i < 8; ++i) {
				QPoint n = pos + deltas[i];
				if (n.x() > -1 && n.x() < size && n.y() > -1 && n.y() < size) {
					positions.append(n);
				}
			}
			neighbors.append(positions);
		}
	}

	// Create cells
	m_cells = QVector<QVector<Cell> >(size, QVector<Cell>(size));
	for (int r = 0; r < size; ++r) {
		for (int c = 0; c < size; ++c) {
			int index = (r * size) + c;
			Cell& cell = m_cells[c][r];
			cell.text = letters.at(index).toUpper();
			cell.position = QPoint(c,r);
			cell.checked = false;

			QList<QPoint> n = neighbors.at(index);
			for (int i = 0; i < n.count(); ++i) {
				const QPoint& neighbor = n.at(i);
				cell.neighbors.append(&m_cells[neighbor.x()][neighbor.y()]);
			}
		}
	}

	// Solve board
	for (int r = 0; r < size; ++r) {
		for (int c = 0; c < size; ++c) {
			checkCell(m_cells[c][r]);
		}
	}
}

//-----------------------------------------------------------------------------

int Solver::score(int max) const {
	QList<int> scores;
	QHashIterator<QString, QList<QList<QPoint> > > i(m_solutions);
	while (i.hasNext()) {
		i.next();
		scores += score(i.key());
	}
	qSort(scores.begin(), scores.end(), qGreater<int>());

	int result = 0;
	scores = (max == -1) ? scores : scores.mid(0, max);
	foreach (int score, scores) {
		result += score;
	}
	return result;
}

//-----------------------------------------------------------------------------

int Solver::score(const QString& word) {
	int length = word.length();
	Q_ASSERT(length <= 25);
	if (length < 8) {
		const int scores[7] = { 0,0,1,1,2,3,5 };
		return scores[length - 1];
	} else {
		return 11;
	}
}

//-----------------------------------------------------------------------------

void Solver::checkCell(Cell& cell) {
	const Trie* words = m_words;
	int length = cell.text.length();
	for (int i = 0; i < length; ++i) {
		words = words->child(cell.text.at(i));
		if (words == 0) {
			return;
		}
	}

	cell.checked = true;
	m_word += cell.text;
	m_positions.append(cell.position);
	qSwap(m_words, words);

	if (m_words->isWord() && (m_word.length() >= m_minimum)) {
		m_solutions[m_word].append(m_positions);
	}
	if (!m_words->isEmpty()) {
		int count = cell.neighbors.count();
		for (int i = 0; i < count; ++i) {
			Cell& next = *cell.neighbors.at(i);
			if (!next.checked) {
				checkCell(next);
			}
		}
	}

	cell.checked = false;
	m_word.chop(length);
	m_positions.removeLast();
	m_words = words;
}

//-----------------------------------------------------------------------------
