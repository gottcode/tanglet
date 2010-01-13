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

#ifndef SOLVER_H
#define SOLVER_H

#include <QHash>
#include <QList>
#include <QPoint>
class Trie;

class Solver {
	public:
		Solver(const Trie& words, const QStringList& letters);

		QHash<QString, QList<QPoint> > solutions() const {
			return m_solutions;
		}

	private:
		struct Cell;
		void checkCell(Cell& cell);

	private:
		const Trie* m_words;
		QString m_word;
		QList<QPoint> m_positions;
		QHash<QString, QList<QPoint> > m_solutions;

		struct Cell {
			QString text;
			QList<Cell*> neighbors;
			QPoint position;
			bool checked;
		};
		Cell m_cells[4][4];
};

#endif
