/*
	SPDX-FileCopyrightText: 2009-2017 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_GENERATOR_H
#define TANGLET_GENERATOR_H

#include "trie.h"

#include <QHash>
#include <QList>
#include <QPoint>
#include <QRandomGenerator>
#include <QStringList>
#include <QThread>

#include <atomic>

/**
 * @brief The Generator class builds and solves a game layout.
 */
class Generator : public QThread
{
	Q_OBJECT

public:
	/**
	 * Constructs a generator instance.
	 * @param parent the QObject that manages the generator
	 */
	explicit Generator(QObject* parent = nullptr);

	/**
	 * Aborts the current game generation.
	 */
	void cancel();

	/**
	 * Creates and solves the game layout.
	 * @param density how many words are in the layout
	 * @param size how big the layout is
	 * @param minimum the shortest word allowed
	 * @param timer the game mode used to determine the maximum amount of words
	 * @param letters an already built layout to solve instead of create
	 */
	void create(int density, int size, int minimum, int timer, const QStringList& letters);

	/**
	 * Fetch the dice for a given board size.
	 * @param size board size
	 * @return list of dice
	 */
	QList<QStringList> dice(int size) const
	{
		return (size == 4) ? m_dice : m_dice_large;
	}

	/**
	 * @return the dictionary site to look up word definitions
	 */
	QString dictionary() const
	{
		return m_dictionary_url;
	}

	/**
	 * @return current error message
	 */
	QString error() const
	{
		return m_error;
	}

	/**
	 * @return layout of the board as a single list of letters
	 */
	QStringList letters() const
	{
		return m_letters;
	}

	/**
	 * @return maximum score possible of current layout
	 */
	int maxScore() const
	{
		return m_max_score;
	}

	/**
	 * @return length of the shortest word allowed
	 */
	int minimum() const
	{
		return m_minimum;
	}

	/**
	 * @return map of the findable words and their locations on the board
	 */
	QHash<QString, QList<QList<QPoint>>> solutions() const
	{
		return m_solutions;
	}

	/**
	 * @return optimized word list
	 */
	const Trie* trie() const
	{
		return &m_words;
	}

	/**
	 * @return size of generated board
	 */
	int size() const
	{
		return m_size;
	}

	/**
	 * @return timer used when generating board
	 */
	int timer() const
	{
		return m_timer;
	}

	/**
	 * Sets where to store cached tries.
	 * @param path location of cache
	 */
	static void setTriePath(const QString& path)
	{
		m_cache_path = path;
	}

Q_SIGNALS:
	/**
	 * Emitted when the word list is being optimized for fast access.
	 */
	void optimizingStarted();

	/**
	 * Emitted when the word list has been optimized.
	 */
	void optimizingFinished();

private:
	/**
	 * Loads the dice and words.
	 */
	void update();

	/**
	 * Sets the current error.
	 * @param error message to inform player something has gone wrong
	 */
	void setError(const QString& error);

protected:
	/**
	 * Builds the game layout in a thread.
	 */
	void run() override;

private:
	static QString m_cache_path; /**< where to find cached tries */

	QString m_dice_path; /**< where to load the dice */
	QString m_words_path; /**< where to load the word list */
	QString m_dictionary_url; /**< where to look up word definitions */
	QList<QStringList> m_dice; /**< the dice used to generate a normal board */
	QList<QStringList> m_dice_large; /**< the dice used to generate a large board */
	QRandomGenerator m_random; /**< the random number generator */
	Trie m_words; /**< the word list optimized for solving */
	QString m_error; /**< error encountered while loading dice and words */

	int m_density; /**< how many words to target */
	int m_size; /**< how many letters wide to make the board */
	int m_minimum; /**< the shortest word allowed */
	int m_timer; /**< the timer mode for the generated board */
	int m_max_words; /**< the maximum number of words used to generate maximum score */
	int m_max_score; /**< the maximum score found on generated board */

	QStringList m_letters; /**< the generated layout */
	QHash<QString, QList<QList<QPoint>>> m_solutions; /**< the available words and their locations */

	std::atomic<bool> m_canceled; /**< if the generation has been aborted by player */
};

#endif // TANGLET_GENERATOR_H
