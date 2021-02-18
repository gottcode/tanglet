/*
	SPDX-FileCopyrightText: 2009-2014 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "trie.h"

#include <queue>
#include <utility>
#include <QDataStream>

//-----------------------------------------------------------------------------

namespace
{

class TrieGenerator
{
public:
	explicit TrieGenerator(const QChar& key = QChar())
		: m_key(key)
		, m_word(false)
		, m_children(nullptr)
		, m_next(nullptr)
		, m_count(0)
	{
	}

	explicit TrieGenerator(const QString& word)
		: m_word(false)
		, m_children(nullptr)
		, m_next(nullptr)
		, m_count(0)
	{
		addWord(word, QStringList(word));
	}

	explicit TrieGenerator(const QHash<QString, QStringList>& words);

	~TrieGenerator();

	void run(QVector<Trie::Node>& nodes, QStringList& spellings) const;

private:
	TrieGenerator* addChild(const QChar& letter);
	void addWord(const QString& word, const QStringList& spellings);

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
	: m_word(false)
	, m_children(nullptr)
	, m_next(nullptr)
	, m_count(0)
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
	delete m_next;
}

//-----------------------------------------------------------------------------

TrieGenerator* TrieGenerator::addChild(const QChar& letter)
{
	if (!m_children) {
		m_children = new TrieGenerator(letter);
		m_count = 1;
		return m_children;
	}

	TrieGenerator* previous = nullptr;
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
	for (const QChar& c : word) {
		node = node->addChild(c);
	}
	node->m_word = true;
	node->m_spellings = spellings;
}

//-----------------------------------------------------------------------------

void TrieGenerator::run(QVector<Trie::Node>& nodes, QStringList& spellings) const
{
	std::queue<std::pair<const TrieGenerator*, int>> next;
	nodes.append(Trie::Node());
	next.emplace(this, 0);

	while (!next.empty()) {
		auto item = next.front();
		next.pop();

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
				next.emplace(trie, nodes.count() - 1);
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
	m_spellings.clear();
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
			clear();
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
	return nullptr;
}

//-----------------------------------------------------------------------------

QStringList Trie::spellings(const QString& word, const QStringList& default_value) const
{
	const Trie::Node* node = &m_nodes[0];
	int length = word.length();
	for (int i = 0; i < length; ++i) {
		node = child(word.at(i), node);
		if (!node) {
			break;
		}
	}

	if (node) {
		return m_spellings.mid(node->m_word, node->m_word_count);
	} else {
		return default_value;
	}
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
	trie.checkNodes();
	if (!trie.isEmpty()) {
		stream >> trie.m_spellings;
	}
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
