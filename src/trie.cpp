/***********************************************************************
 *
 * Copyright (C) 2009, 2011 Graeme Gott <graeme@gottcode.org>
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

#include "trie.h"

#include <QString>

//-----------------------------------------------------------------------------

Trie* Trie::addChild(const QChar& letter) {
	std::vector<Trie>::iterator i = std::find(m_children.begin(), m_children.end(), letter);
	if (i != m_children.end()) {
		return &*i;
	} else {
		Trie trie(letter);
		m_children.push_back(trie);
		return &m_children.back();
	}
}

//-----------------------------------------------------------------------------

void Trie::addWord(const QString& word) {
	Trie* node = this;
	foreach (const QChar& c, word) {
		node = node->addChild(c);
	}
	node->m_word = true;
}

//-----------------------------------------------------------------------------

void Trie::clear() {
	m_children.clear();
}

//-----------------------------------------------------------------------------
