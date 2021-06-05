/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "trie.h"

#include <QDataStream>
#include <QTextStream>

#include <queue>
#include <utility>

//-----------------------------------------------------------------------------

namespace
{

/**
 * @brief The TrieGenerator class generates the optimized word list for the trie class.
 */
class TrieGenerator
{
public:
	/**
	 * Constructs a generator instance.
	 * @param key the letter contained by the generator instance
	 */
	explicit TrieGenerator(const QChar& key = QChar())
		: m_key(key)
		, m_word(false)
		, m_children(nullptr)
		, m_next(nullptr)
		, m_count(0)
	{
	}

	/**
	 * Constructs a generator instance.
	 * @param word a list of letters to build child instances from
	 */
	explicit TrieGenerator(const QString& word)
		: m_word(false)
		, m_children(nullptr)
		, m_next(nullptr)
		, m_count(0)
	{
		addWord(word, QStringList(word));
	}

	/**
	 * Constructs a generator instance.
	 * @param data contents of a word list file including alternate spellings
	 */
	explicit TrieGenerator(const QByteArray& data);

	/**
	 * Destroys the generator.
	 */
	~TrieGenerator();

	/**
	 * Compiles a compressed list of the trie for faster access.
	 * @param [out] nodes the compressed list of nodes
	 * @param [out] spellings the list of spellings
	 */
	void run(QList<Trie::Node>& nodes, QStringList& spellings) const;

private:
	/**
	 * Fetch a child generator matching a specific letter. Creates one if does not exist yet.
	 * @param letter the letter to add
	 * @return the child generator
	 */
	TrieGenerator* addChild(const QChar& letter);

	/**
	 * Adds a word by walking through the letters and fetching the next child of each letter.
	 * @param word the word to add
	 * @param spellings alternate spellings of the word to store
	 */
	void addWord(const QString& word, const QStringList& spellings);

	// Uncopyable
	TrieGenerator(const TrieGenerator&) = delete;
	TrieGenerator& operator=(const TrieGenerator&) = delete;

private:
	QChar m_key; /**< letter represented by the generator */
	bool m_word; /**< is this node a word */
	TrieGenerator* m_children; /**< first child generator */
	TrieGenerator* m_next; /**< first sibling generator */
	int m_count; /**< how many children the generator has */
	QStringList m_spellings; /**< alternate spellings of word */
};

//-----------------------------------------------------------------------------

TrieGenerator::TrieGenerator(const QByteArray& data)
	: m_word(false)
	, m_children(nullptr)
	, m_next(nullptr)
	, m_count(0)
{
	QTextStream stream(data);
	while (!stream.atEnd()) {
		QStringList spellings = stream.readLine().simplified().split(QChar(' '), Qt::SkipEmptyParts);
		if (spellings.isEmpty()) {
			continue;
		}

		QString word = spellings.first().toUpper();
		if (spellings.count() == 1) {
			spellings[0] = word.toLower();
		} else {
			spellings.removeFirst();
		}

		if (word.length() >= 3 && word.length() <= 25) {
			addWord(word, spellings);
		}
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

void TrieGenerator::run(QList<Trie::Node>& nodes, QStringList& spellings) const
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

Trie::Trie(const QByteArray& data)
{
	TrieGenerator generator(data);
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
	const quint32 count = m_nodes.size();
	for (const Node& node : qAsConst(m_nodes)) {
		const quint32 start = node.m_children;
		const quint32 end = start + node.m_child_count;
		if ((start >= count) || (end > count)) {
			clear();
			break;
		}
	}
}

//-----------------------------------------------------------------------------

const Trie::Node* Trie::child(const QChar& letter, const Node* node) const
{
	const auto end = m_nodes.cbegin() + node->m_children + node->m_child_count;
	for (auto i = m_nodes.cbegin() + node->m_children; i < end; ++i) {
		if (*i == letter) {
			return &*i;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------

QStringList Trie::spellings(const QString& word, const QStringList& default_value) const
{
	const Trie::Node* node = &m_nodes[0];
	const int length = word.length();
	for (int i = 0; i < length; ++i) {
		node = child(word.at(i), node);
		if (!node) {
			return default_value;
		}
	}
	return m_spellings.mid(node->m_word, node->m_word_count);
}

//-----------------------------------------------------------------------------

QDataStream& operator<<(QDataStream& stream, const Trie& trie)
{
	stream << trie.m_nodes << trie.m_spellings.join('\n');
	return stream;
}

//-----------------------------------------------------------------------------

QDataStream& operator>>(QDataStream& stream, Trie& trie)
{
	QString spellings;
	stream >> trie.m_nodes >> spellings;
	trie.checkNodes();
	if (!trie.isEmpty()) {
		trie.m_spellings = spellings.split('\n');
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
