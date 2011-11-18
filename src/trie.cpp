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

#include <QDataStream>
#include <QPair>

//-----------------------------------------------------------------------------

namespace
{

class TrieGenerator
{
public:
	TrieGenerator(const QChar& key = QChar())
	: m_key(key), m_word(false), m_children(0), m_next(0), m_count(0)
	{
	}

	TrieGenerator(const QString& word)
	: m_word(false), m_children(0), m_next(0), m_count(0)
	{
		addWord(word);
	}

	TrieGenerator(const QHash<QString, QStringList>& words);

	~TrieGenerator();

	void run(QVector<Trie::Node>& nodes, QStringList& spellings) const;

private:
	TrieGenerator* addChild(const QChar& letter);
	void addWord(const QString& word, const QStringList& spellings = QStringList());

	// Uncopyable
	TrieGenerator(const TrieGenerator&);
	TrieGenerator& operator=(const TrieGenerator&);

private:
	QChar m_key;
	bool m_word;
	TrieGenerator* m_children;
	TrieGenerator* m_next;
	int m_count;
	QStringList m_spellings;
};

//-----------------------------------------------------------------------------

TrieGenerator::TrieGenerator(const QHash<QString, QStringList>& words)
: m_word(false), m_children(0), m_next(0), m_count(0)
{
	QHashIterator<QString, QStringList> i(words);
	while (i.hasNext()) {
		i.next();
		addWord(i.key(), i.value());
	}
}

//-----------------------------------------------------------------------------

TrieGenerator::~TrieGenerator()
{
	delete m_children;
	m_children = 0;

	delete m_next;
	m_next = 0;
}

//-----------------------------------------------------------------------------

TrieGenerator* TrieGenerator::addChild(const QChar& letter)
{
	if (!m_children) {
		m_children = new TrieGenerator(letter);
		m_count = 1;
		return m_children;
	}

	TrieGenerator* previous = 0;
	TrieGenerator* current = m_children;
	while (current && current->m_key != letter) {
		previous = current;
		current = current->m_next;
	}

	if (!current) {
		previous->m_next = current = new TrieGenerator(letter);
		m_count++;
	}
	return current;
}

//-----------------------------------------------------------------------------

void TrieGenerator::addWord(const QString& word, const QStringList& spellings)
{
	TrieGenerator* node = this;
	foreach (const QChar& c, word) {
		node = node->addChild(c);
	}
	node->m_word = true;
	node->m_spellings = spellings;
}

//-----------------------------------------------------------------------------

void TrieGenerator::run(QVector<Trie::Node>& nodes, QStringList& spellings) const
{
	QList< QPair<const TrieGenerator*, int> > next;
	nodes.append(Trie::Node());
	next.append(qMakePair(this, 0));

	while (!next.isEmpty()) {
		QPair<const TrieGenerator*, int> item = next.takeFirst();

		const TrieGenerator* trie = item.first;
		Trie::Node& node = nodes[item.second];
		node.m_letter = trie->m_key;
		node.m_word = spellings.count();
		spellings += trie->m_spellings;
		node.m_word_count = trie->m_spellings.count();
		node.m_child_count = trie->m_count;

		if (node.m_child_count) {
			node.m_children = nodes.count();

			trie = trie->m_children;
			int count = node.m_child_count;
			for (int i = 0; i < count; ++i) {
				nodes.append(Trie::Node());
				next.append(qMakePair(trie, nodes.count() - 1));
				trie = trie->m_next;
			}
		}
	}
}

}

//-----------------------------------------------------------------------------

Trie::Trie()
{
}

//-----------------------------------------------------------------------------

Trie::Trie(const QString& word)
{
	TrieGenerator generator(word);
	generator.run(m_nodes, m_spellings);
	checkNodes();
}

//-----------------------------------------------------------------------------

Trie::Trie(const QHash<QString, QStringList>& words)
{
	TrieGenerator generator(words);
	generator.run(m_nodes, m_spellings);
	checkNodes();
}

//-----------------------------------------------------------------------------

void Trie::clear()
{
	m_nodes.clear();
}

//-----------------------------------------------------------------------------

void Trie::checkNodes()
{
	// Verify that no nodes reference outside list
	int count = m_nodes.size();
	int start, end;
	for (int i = 0; i < count; ++i) {
		start = m_nodes[i].m_children;
		end = start + m_nodes[i].m_child_count;
		if ((start >= count) || (end > count)) {
			m_nodes.clear();
			m_spellings.clear();
			break;
		}
	}
}

//-----------------------------------------------------------------------------

const Trie::Node* Trie::child(const QChar& letter, const Node* node) const
{
	int end = node->m_children + node->m_child_count;
	for (int i = node->m_children; i < end; ++i) {
		if (m_nodes[i] == letter) {
			return &m_nodes[i];
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------

QStringList Trie::spellings(const Node* node) const
{
	return m_spellings.mid(node->m_word, node->m_word_count);
}

//-----------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& stream, const Trie& trie)
{
	stream << trie.m_nodes;
	stream << trie.m_spellings;
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator>>(QDataStream& stream, Trie& trie)
{
	stream >> trie.m_nodes;
	stream >> trie.m_spellings;
	trie.checkNodes();
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& stream, const Trie::Node& node)
{
	stream << node.m_letter << node.m_word << node.m_children << node.m_word_count << node.m_child_count;
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator>>(QDataStream& stream, Trie::Node& node)
{
	stream >> node.m_letter >> node.m_word >> node.m_children >> node.m_word_count >> node.m_child_count;
	return stream;
}

//-----------------------------------------------------------------------------
