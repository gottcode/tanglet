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

#ifndef TRIE_H
#define TRIE_H

#include <QChar>
class QString;

class Trie {
	public:
		Trie(const QChar& key = QChar())
		: m_key(key), m_word(false), m_children(0), m_next(0) {
		}

		Trie(const QString& word)
		: m_word(false), m_children(0), m_next(0) {
			addWord(word);
		}

		~Trie() {
			delete m_children;
			delete m_next;
		}

		void addWord(const QString& word);
		void clear();

		bool isEmpty() const {
			return m_children == 0;
		}

		bool isWord() const {
			return m_word;
		}

		const Trie* child(const QChar& letter) const {
			const Trie* result = m_children;
			while (result && result->m_key != letter) {
				result = result->m_next;
			}
			return result;
		}

	private:
		Trie* addChild(const QChar& letter);

	private:
		QChar m_key;
		bool m_word;
		Trie* m_children;
		Trie* m_next;
};

#endif
