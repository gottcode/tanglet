/***********************************************************************
 *
 * Copyright (C) 2009 Graeme Gott <graeme@gottcode.org>
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

Solver::Solver(const Trie& words, const QStringList& letters)
: m_words(&words) {
	if (letters.count() != 16) {
		return;
	}

	// Create neighbors
	static QList<QList<QPoint> > neighbors;
	if (neighbors.isEmpty()) {
		QPoint deltas[] = {
			QPoint(-1,-1),
			QPoint(0,-1),
			QPoint(1,-1),
			QPoint(-1,0),
			QPoint(1,0),
			QPoint(-1,1),
			QPoint(0,1),
			QPoint(1,1)
		};
		for (int r = 0; r < 4; ++r) {
			for (int c = 0; c < 4; ++c) {
				QList<QPoint> positions;
				QPoint pos(c, r);
				for (int i = 0; i < 8; ++i) {
					QPoint n = pos + deltas[i];
					if (n.x() > -1 && n.x() < 4 && n.y() > -1 && n.y() < 4) {
						positions.append(n);
					}
				}
				neighbors.append(positions);
			}
		}
	}

	// Create cells
	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			int index = (r * 4) + c;
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
	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			checkCell(m_cells[c][r]);
		}
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

	if (m_words->isWord()) {
		m_solutions[m_word] = m_positions;
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
