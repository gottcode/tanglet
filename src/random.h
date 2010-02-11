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

#include <QtGlobal>

class Random {
public:
	Random(unsigned int seed = 1);

	double nextDouble();
	unsigned int nextInt(unsigned int max = maximum);
	unsigned int operator()(unsigned int max = maximum);

	void setSeed(unsigned int seed);

	static const unsigned int maximum;

private:
	quint64 m_seed;
};

#endif
