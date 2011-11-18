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
#include <QStringList>
#include <QVector>
class QDataStream;

class Trie
{
public:
	struct Node
	{
		Node()
			: m_letter(QChar()), m_word(false), m_children(0), m_child_count(0)
		{
		}

		bool isEmpty() const
		{
			return !m_child_count;
		}

		bool isWord() const
		{
			return m_word;
		}

		bool operator==(const QChar& c) const
		{
			return c == m_letter;
		}

		QChar m_letter;
		bool m_word;
		quint32 m_children;
		quint8 m_child_count;
	};

public:
	Trie();
	Trie(const QString& word);
	Trie(const QStringList& words);
	Trie(const QVector<Node>& nodes);

	void clear();

	const Node* child() const
	{
		return &m_nodes[0];
	}

	const Node* child(const QChar& letter, const Node* node) const;

	friend QDataStream& operator<<(QDataStream& stream, const Trie& trie);
	friend QDataStream& operator>>(QDataStream& stream, Trie& trie);

private:
	void setNodes(const QVector<Node>& nodes);

private:
	QVector<Node> m_nodes;
};

QDataStream& operator<<(QDataStream& stream, const Trie& trie);
QDataStream& operator>>(QDataStream& stream, Trie& trie);
QDataStream& operator<<(QDataStream& stream, const Trie::Node& node);
QDataStream& operator>>(QDataStream& stream, Trie::Node& node);

#endif
