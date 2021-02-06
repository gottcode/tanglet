/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TRIE_H
#define TRIE_H

#include <QChar>
#include <QHash>
#include <QStringList>
#include <QVector>
class QDataStream;

class Trie
{
public:
	struct Node
	{
		Node()
			: m_letter(QChar())
			, m_word(0)
			, m_children(0)
			, m_word_count(0)
			, m_child_count(0)
		{
		}

		bool isEmpty() const
		{
			return !m_child_count;
		}

		bool isWord() const
		{
			return m_word_count;
		}

		bool operator==(const QChar& c) const
		{
			return c == m_letter;
		}

		QChar m_letter;
		quint32 m_word;
		quint32 m_children;
		quint8 m_word_count;
		quint8 m_child_count;
	};

public:
	Trie();
	Trie(const QString& word);
	Trie(const QHash<QString, QStringList>& words);

	void clear();

	const Node* child() const
	{
		return &m_nodes[0];
	}

	const Node* child(const QChar& letter, const Node* node) const;
	QStringList spellings(const QString& word, const QStringList& default_value = QStringList()) const;

	bool isEmpty() const
	{
		return m_nodes.isEmpty();
	}

	friend QDataStream& operator<<(QDataStream& stream, const Trie& trie);
	friend QDataStream& operator>>(QDataStream& stream, Trie& trie);

private:
	void checkNodes();

private:
	QVector<Node> m_nodes;
	QStringList m_spellings;
};

QDataStream& operator<<(QDataStream& stream, const Trie& trie);
QDataStream& operator>>(QDataStream& stream, Trie& trie);
QDataStream& operator<<(QDataStream& stream, const Trie::Node& node);
QDataStream& operator>>(QDataStream& stream, Trie::Node& node);

#endif
