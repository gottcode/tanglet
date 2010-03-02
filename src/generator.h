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

#ifndef GENERATOR_H
#define GENERATOR_H

#include "trie.h"

#include <QHash>
#include <QList>
#include <QPoint>
#include <QStringList>
#include <QThread>

class Generator : public QThread {
	Q_OBJECT

public:
	Generator(QObject* parent = 0);

	void cancel();
	void create(bool higher_scores, int size, int timer, unsigned int seed);

	QList<QStringList> dice(int size) const {
		return (size == 4) ? m_dice : m_dice_large;
	}

	QString dictionary() const {
		return m_dictionary_url;
	}

	QString error() const {
		return m_error;
	}

	QStringList letters() const {
		return m_letters;
	}

	int maxScore() const {
		return m_max_score;
	}

	QHash<QString, QList<QList<QPoint> > > solutions() const {
		return m_solutions;
	}

	int size() const {
		return m_size;
	}

	int timer() const {
		return m_timer;
	}

signals:
	void warning(const QString& warning);

private:
	void update();

protected:
	void run();

private:
	QString m_dice_path;
	QString m_words_path;
	QString m_dictionary_url;
	QList<QStringList> m_dice;
	QList<QStringList> m_dice_large;
	Trie m_words;
	QString m_error;

	bool m_higher_scores;
	int m_size;
	int m_minimum;
	int m_timer;
	int m_max_words;
	unsigned int m_seed;
	int m_max_score;

	QStringList m_letters;
	QHash<QString, QList<QList<QPoint> > > m_solutions;

	QAtomicInt m_cancelled;
};

#endif
