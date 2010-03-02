/***********************************************************************
 *
 * Copyright (C) 2010 Graeme Gott <graeme@gottcode.org>
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

#ifndef RANDOM_H
#define RANDOM_H

#include <QList>

class Random {
public:
	Random(quint64 seed = 1);

	double nextDouble();
	unsigned int nextInt(unsigned int max = maximum);
	void setSeed(quint64 seed);

	template <typename T> void shuffle(QList<T>& list) {
		for (int i = list.length(); i > 1; --i) {
			list.swap(nextInt(i), i - 1);
		}
	}

	static const unsigned int maximum;

private:
	quint64 m_seed;
};

#endif
