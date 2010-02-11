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

#include "random.h"

#include <climits>

const unsigned int Random::maximum = UINT_MAX;

//-----------------------------------------------------------------------------

Random::Random(unsigned int seed) {
	setSeed(seed);
}

//-----------------------------------------------------------------------------

double Random::nextDouble() {
	return nextInt() / static_cast<double>(maximum);
}

//-----------------------------------------------------------------------------

unsigned int Random::nextInt(unsigned int max) {
	m_seed ^= (m_seed << 21);
	m_seed ^= (m_seed >> 35);
	m_seed ^= (m_seed << 4);
	return m_seed % qMax(max, 1U);
}

//-----------------------------------------------------------------------------

unsigned int Random::operator()(unsigned int max) {
	return nextInt(max);
}

//-----------------------------------------------------------------------------

void Random::setSeed(unsigned int seed) {
	m_seed = (seed != 0) ? seed : 1;
}

//-----------------------------------------------------------------------------
