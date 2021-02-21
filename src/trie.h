/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TRIE_H
#define TRIE_H

#include <QChar>
#include <QHash>
#include <QStringList>
#include <QVector>
class QDataStream;

/**
 * @brief The Trie class contains a word list for fast lookup.
 */
class Trie
{
public:
	/**
	 * @brief The Node struct contains a letter in the trie.
	 */
	struct Node
	{
		/**
		 * Constructs a node instance.
		 */
		Node()
			: m_letter(QChar())
			, m_word(0)
			, m_children(0)
			, m_word_count(0)
			, m_child_count(0)
		{
		}

		/**
		 * @return whether the node has any child letters
		 */
		bool isEmpty() const
		{
			return !m_child_count;
		}

		/**
		 * @return whether the node points to a complete word
		 */
		bool isWord() const
		{
			return m_word_count;
		}

		/**
		 * Compares node with a letter.
		 * @param c letter to compare
		 * @return whether node is a matching letter
		 */
		bool operator==(const QChar& c) const
		{
			return c == m_letter;
		}

		QChar m_letter; /**< letter represented by the node */
		quint32 m_word; /**< offset into word list if this node is a word */
		quint32 m_children; /**< offset into the node list for children of node */
		quint8 m_word_count; /**< how many spellings of the word the node has */
		quint8 m_child_count; /**< how many children the node has */
	};

public:
	/**
	 * Constructs an empty trie instance.
	 */
	explicit Trie();

	/**
	 * Constructs a trie instance containing a single word.
	 * @param word the word to store
	 */
	explicit Trie(const QString& word);

	/**
	 * Constructs a trie instance from a word list.
	 * @param data contents of a word list file including alternate spellings
	 */
	explicit Trie(const QByteArray& data);

	/**
	 * Removes all nodes and spellings.
	 */
	void clear();

	/**
	 * @return the top-level node of the trie
	 */
	const Node* child() const
	{
		return &m_nodes[0];
	}

	/**
	 * Fetches a child node.
	 * @param letter the letter to check for
	 * @param node the parent node
	 * @return child node or @c nullptr if no child matches @p letter
	 */
	const Node* child(const QChar& letter, const Node* node) const;

	/**
	 * Fetches the spellings of a word.
	 * @param word the word to look up
	 * @param default_value fallback list of spellings
	 * @return spellings of a word
	 */
	QStringList spellings(const QString& word, const QStringList& default_value = QStringList()) const;

	/**
	 * @return whether the trie is empty
	 */
	bool isEmpty() const
	{
		return m_nodes.isEmpty();
	}

	friend QDataStream& operator<<(QDataStream& stream, const Trie& trie);
	friend QDataStream& operator>>(QDataStream& stream, Trie& trie);

private:
	/**
	 * Verify that no nodes reference outside the list of nodes.
	 */
	void checkNodes();

private:
	QVector<Node> m_nodes; /**< list of nodes */
	QStringList m_spellings; /**< list of words */
};

/**
 * Writes a trie to a data stream.
 * @param stream the output stream
 * @param trie the trie to write
 * @return reference to the stream
 */
QDataStream& operator<<(QDataStream& stream, const Trie& trie);

/**
 * Reads a trie from a data stream.
 * @param stream the input stream
 * @param trie the trie to read into
 * @return reference to the stream
 */
QDataStream& operator>>(QDataStream& stream, Trie& trie);

/**
 * Writes a trie node to a data stream.
 * @param stream the output stream
 * @param node the trie node to write
 * @return reference to the stream
 */
QDataStream& operator<<(QDataStream& stream, const Trie::Node& node);

/**
 * Reads a trie node from a data stream.
 * @param stream the input stream
 * @param node the trie node to read into
 * @return reference to the stream
 */
QDataStream& operator>>(QDataStream& stream, Trie::Node& node);

#endif
