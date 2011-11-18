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

	TrieGenerator(const QStringList& words);

	~TrieGenerator();

	QVector<Trie::Node> run() const;

private:
	TrieGenerator* addChild(const QChar& letter);
	void addWord(const QString& word);

	// Uncopyable
	TrieGenerator(const TrieGenerator&);
	TrieGenerator& operator=(const TrieGenerator&);

private:
	QChar m_key;
	bool m_word;
	TrieGenerator* m_children;
	TrieGenerator* m_next;
	int m_count;
};

//-----------------------------------------------------------------------------

TrieGenerator::TrieGenerator(const QStringList& words)
: m_word(false), m_children(0), m_next(0), m_count(0)
{
	foreach (const QString& word, words) {
		addWord(word);
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

void TrieGenerator::addWord(const QString& word)
{
	TrieGenerator* node = this;
	foreach (const QChar& c, word) {
		node = node->addChild(c);
	}
	node->m_word = true;
}

//-----------------------------------------------------------------------------

QVector<Trie::Node> TrieGenerator::run() const
{
	QVector<Trie::Node> result;

	QList< QPair<const TrieGenerator*, int> > next;
	result.append(Trie::Node());
	next.append(qMakePair(this, 0));

	while (!next.isEmpty()) {
		QPair<const TrieGenerator*, int> item = next.takeFirst();

		const TrieGenerator* trie = item.first;
		Trie::Node& node = result[item.second];
		node.m_letter = trie->m_key;
		node.m_word = trie->m_word;
		node.m_child_count = trie->m_count;

		if (node.m_child_count) {
			node.m_children = result.count();

			trie = trie->m_children;
			int count = node.m_child_count;
			for (int i = 0; i < count; ++i) {
				result.append(Trie::Node());
				next.append(qMakePair(trie, result.count() - 1));
				trie = trie->m_next;
			}
		}
	}

	return result;
}

}

//-----------------------------------------------------------------------------

Trie::Trie()
{
}

//-----------------------------------------------------------------------------

Trie::Trie(const QString& word)
{
	setNodes(TrieGenerator(word).run());
}

//-----------------------------------------------------------------------------

Trie::Trie(const QStringList& words)
{
	setNodes(TrieGenerator(words).run());
}

//-----------------------------------------------------------------------------

Trie::Trie(const QVector<Node>& nodes)
{
	setNodes(nodes);
}

//-----------------------------------------------------------------------------

void Trie::clear()
{
	m_nodes.clear();
}

//-----------------------------------------------------------------------------

void Trie::setNodes(const QVector<Node>& nodes)
{
	m_nodes = nodes;

	// Verify that no nodes reference outside list
	int count = m_nodes.size();
	int start, end;
	for (int i = 0; i < count; ++i) {
		start = m_nodes[i].m_children;
		end = start + m_nodes[i].m_child_count;
		if ((start >= count) || (end > count)) {
			m_nodes.clear();
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

QDataStream& operator<<(QDataStream& stream, const Trie& trie)
{
	stream << trie.m_nodes;
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator>>(QDataStream& stream, Trie& trie)
{
	QVector<Trie::Node> nodes;
	stream >> nodes;
	trie.setNodes(nodes);
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& stream, const Trie::Node& node)
{
	stream << node.m_letter << node.m_word << node.m_children << node.m_child_count;
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator>>(QDataStream& stream, Trie::Node& node)
{
	stream >> node.m_letter >> node.m_word >> node.m_children >> node.m_child_count;
	return stream;
}

//-----------------------------------------------------------------------------
