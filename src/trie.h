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

#ifndef TRIE_H
#define TRIE_H

#include <QChar>
class QString;

#include <algorithm>
#include <vector>

class Trie {
	public:
		Trie(const QChar& key = QChar())
		: m_key(key), m_word(false) {
		}

		Trie(const QString& word)
		: m_word(false) {
			addWord(word);
		}

		void addWord(const QString& word);
		void clear();

		bool isEmpty() const {
			return m_children.empty();
		}

		bool isWord() const {
			return m_word;
		}

		const Trie* child(const QChar& letter) const {
			std::vector<Trie>::const_iterator i = std::find(m_children.begin(), m_children.end(), letter);
			return (i != m_children.end()) ? &*i : 0;
		}

		operator QChar() const {
			return m_key;
		}

	private:
		Trie* addChild(const QChar& letter);

	private:
		std::vector<Trie> m_children;
		QChar m_key;
		bool m_word;
};

#endif
