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

#include "trie.h"

#include <QString>

//-----------------------------------------------------------------------------

Trie* Trie::addChild(const QChar& letter) {
	if (!m_children) {
		m_children = new Trie(letter);
		return m_children;
	}

	Trie* previous = 0;
	Trie* current = m_children;
	while (current && current->m_key != letter) {
		previous = current;
		current = current->m_next;
	}

	if (!current) {
		previous->m_next = current = new Trie(letter);
	}
	return current;
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
	delete m_children;
	m_children = 0;

	delete m_next;
	m_next = 0;
}

//-----------------------------------------------------------------------------
