/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_SOLVER_H
#define TANGLET_SOLVER_H

#include "trie.h"

#include <QHash>
#include <QList>
#include <QPoint>

/**
 * @brief The Solver class finds all of the words on a board.
 */
class Solver
{
	/**
	 * @brief The Solver::Cell struct represents a letter on the board.
	 */
	struct Cell
	{
		QString text; /**< text of the letter */
		QList<Cell*> neighbors; /**< which letters are connected to this one */
		QPoint position; /**< location on the board */
		bool checked; /**< has it been used while building a word */
	};
public:
	/**
	 * Constructs a solver instance.
	 * @param words the optimized word list
	 * @param size the size of the board
	 * @param minimum the shortest allowed word
	 */
	Solver(const Trie& words, int size, int minimum);

	/**
	 * Finds the solution to a board.
	 * @param letters the board layout
	 */
	void solve(const QStringList& letters);

	/**
	 * @return how many words were found
	 */
	int count() const
	{
		return m_count;
	}

	/**
	 * @return all of the words and their locations on the board
	 */
	QHash<QString, QList<QList<QPoint>>> solutions() const
	{
		return m_solutions;
	}

	/**
	 * The maximum score available on the board.
	 * @param max how many words to limit the score to
	 * @return the score of the board
	 */
	int score(int max = -1) const;

	/**
	 * The score for a word.
	 * @param word the word to check
	 * @return how many points there word is worth
	 */
	static int score(const QString& word);

	/**
	 * Sets if the solve keeps track of the locations of words as it solves.
	 * @param track_positions whether to track positions
	 */
	void setTrackPositions(bool track_positions);

private:
	/**
	 * Checks if cell is part of or the final cell of a word while solving.
	 * @param cell location to check
	 */
	void checkCell(Cell& cell);

private:
	const Trie* m_words; /**< fast access word list */
	const Trie::Node* m_node; /**< current letter being checked in word list */
	int m_size; /**< how many cells wide is the board */
	int m_minimum; /**< the shortest allowed word */
	bool m_track_positions; /**< remember locations of each word when solving */
	QList<QList<Cell>> m_cells; /**< layout of board */

	QString m_word; /**< word currently being assembled */
	QList<QPoint> m_positions; /**< locations of letters in word being assembled */

	QHash<QString, QList<QList<QPoint>>> m_solutions; /**< words found and their positions on the board */
	int m_count; /**< how many words have been found */
};

#endif // TANGLET_SOLVER_H
