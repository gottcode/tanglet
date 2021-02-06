/*
	SPDX-FileCopyrightText: 2009-2017 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "solver.h"

#include <QStringList>

#include <algorithm>
#include <functional>

//-----------------------------------------------------------------------------

Solver::Solver(const Trie& words, int size, int minimum)
	: m_words(&words)
	, m_node(words.child())
	, m_size(size)
	, m_minimum(minimum)
	, m_track_positions(true)
	, m_count(0)
{
	// Create neighbors
	QList<QList<QPoint>> neighbors;
	const QPoint deltas[] = {
		QPoint(-1, -1),
		QPoint(0, -1),
		QPoint(1, -1),
		QPoint(-1, 0),
		QPoint(1, 0),
		QPoint(-1, 1),
		QPoint(0, 1),
		QPoint(1, 1)
	};
	for (int r = 0; r < m_size; ++r) {
		for (int c = 0; c < m_size; ++c) {
			QList<QPoint> positions;
			QPoint pos(c, r);
			for (int i = 0; i < 8; ++i) {
				QPoint n = pos + deltas[i];
				if (n.x() > -1 && n.x() < m_size && n.y() > -1 && n.y() < m_size) {
					positions.append(n);
				}
			}
			neighbors.append(positions);
		}
	}

	// Create cells
	m_cells = QVector<QVector<Cell>>(m_size, QVector<Cell>(m_size));
	for (int r = 0; r < m_size; ++r) {
		for (int c = 0; c < m_size; ++c) {
			int index = (r * m_size) + c;
			Cell& cell = m_cells[c][r];
			cell.position = QPoint(c, r);
			cell.checked = false;

			const auto& n = neighbors.at(index);
			for (int i = 0; i < n.count(); ++i) {
				const QPoint& neighbor = n.at(i);
				cell.neighbors.append(&m_cells[neighbor.x()][neighbor.y()]);
			}
		}
	}
}

//-----------------------------------------------------------------------------

void Solver::solve(const QStringList& letters)
{
	m_solutions.clear();
	m_positions.clear();
	m_word.clear();
	m_count = 0;

	// Set cell contents
	for (int r = 0; r < m_size; ++r) {
		for (int c = 0; c < m_size; ++c) {
			m_cells[c][r].text = letters.at((r * m_size) + c).toUpper();
		}
	}

	// Solve board
	for (int r = 0; r < m_size; ++r) {
		for (int c = 0; c < m_size; ++c) {
			checkCell(m_cells[c][r]);
		}
	}
}

//-----------------------------------------------------------------------------

int Solver::score(int max) const
{
	QList<int> scores;
	QHashIterator<QString, QList<QList<QPoint>>> i(m_solutions);
	while (i.hasNext()) {
		scores += score(i.next().key());
	}
	std::sort(scores.begin(), scores.end(), std::greater<int>());

	int result = 0;
	int count = (max == -1) ? scores.count() : max;
	for (int i = 0; i < count; ++i) {
		result += scores.at(i);
	}
	return result;
}

//-----------------------------------------------------------------------------

int Solver::score(const QString& word)
{
	Q_ASSERT(word.length() <= 25);
	static const int scores[24] = {
		 0,  0,  1,  1,  2,
		 3,  5, 11, 11, 11,
		11, 11, 11, 11, 11,
		11, 11, 11, 11, 11,
		11, 11, 11, 11
	};
	return scores[word.length() - 1];
}

//-----------------------------------------------------------------------------

void Solver::setTrackPositions(bool track_positions)
{
	m_track_positions = track_positions;
}

//-----------------------------------------------------------------------------

void Solver::checkCell(Cell& cell)
{
	const Trie::Node* node = m_node;
	int length = cell.text.length();
	for (int i = 0; i < length; ++i) {
		node = m_words->child(cell.text.at(i), node);
		if (!node) {
			return;
		}
	}

	cell.checked = true;
	m_word += cell.text;
	if (m_track_positions) {
		m_positions.append(cell.position);
	}
	qSwap(m_node, node);

	if (m_node->isWord() && (m_word.length() >= m_minimum)) {
		m_count++;
		if (m_track_positions) {
			m_solutions[m_word].append(m_positions);
		}
	}
	if (!m_node->isEmpty()) {
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
	if (m_track_positions) {
		m_positions.removeLast();
	}
	m_node = node;
}

//-----------------------------------------------------------------------------
