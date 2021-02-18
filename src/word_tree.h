/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef WORD_TREE_H
#define WORD_TREE_H

class Trie;

#include <QTreeWidget>

class WordTree : public QTreeWidget
{
	Q_OBJECT

public:
	explicit WordTree(QWidget* parent = nullptr);

	QTreeWidgetItem* addWord(const QString& word);
	void removeAll();
	void setDictionary(const QString& url);
	void setHebrew(bool hebrew);
	void setTrie(const Trie* trie);

protected:
	void leaveEvent(QEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private slots:
	void onItemClicked(QTreeWidgetItem* item, int column);

private:
	void enterItem(QTreeWidgetItem* item);

private:
	QTreeWidgetItem* m_active_item;
	QByteArray m_url;
	bool m_hebrew;
	const Trie* m_trie;
};

#endif
