/*
	SPDX-FileCopyrightText: 2009-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef WORD_TREE_H
#define WORD_TREE_H

class Trie;

#include <QTreeWidget>

/**
 * @brief The WordTree class contains a list of words to display to the player.
 */
class WordTree : public QTreeWidget
{
	Q_OBJECT

public:
	/**
	 * Constructs a word tree instance.
	 * @param parent the widget that owns the word counts
	 */
	explicit WordTree(QWidget* parent = nullptr);

	/**
	 * Adds a word to the list.
	 * @param word word to add
	 * @return item in list representing @p word
	 */
	QTreeWidgetItem* addWord(const QString& word);

	/**
	 * Removes all words from list.
	 */
	void removeAll();

	/**
	 * Set the location to look up word definitions.
	 * @param url location to look up words
	 */
	void setDictionary(const QString& url);

	/**
	 * Sets if the word list is in Hebrew. This is important because vowels at the end of a word need
	 * to be replaced to make it correct for displaying to the player or looking up in the dictionary.
	 * @param hebrew whether the word list is in Hebrew
	 */
	void setHebrew(bool hebrew);

	/**
	 * Set the optimized word list to look up spellings of the words.
	 * @param trie optimized word list
	 */
	void setTrie(const Trie* trie);

protected:
	/**
	 * Override to detect player mousing off of word tree. This allows the word tree to clear the
	 * icon on the active word.
	 * @param event details of leave event
	 */
	void leaveEvent(QEvent* event) override;

	/**
	 * Override to detect player mousing over word tree. This is to detectwhen the mouse has entered
	 * an item.
	 * @param event details of mouse move event
	 */
	void mouseMoveEvent(QMouseEvent* event) override;

	/**
	 * Override to detect player scrolling word tree. This is to detect if a new item is now active.
	 * @param event details of wheel event
	 */
	void wheelEvent(QWheelEvent* event) override;

private slots:
	/**
	 * Handles player clicking on an item. If they click on the second column, it opens a web
	 * browser at the dictionary definition of the clicked item.
	 * @param item which item was clicked on
	 * @param column which column was clicked on
	 */
	void onItemClicked(QTreeWidgetItem* item, int column);

private:
	/**
	 * Shows an icon on the active item to inform player they can look up a word.
	 * @param item which item is now active, or @c nullptr if none are
	 */
	void enterItem(QTreeWidgetItem* item);

private:
	QTreeWidgetItem* m_active_item; /**< currently hovered item */
	QByteArray m_url; /**< location to look up word definitions */
	bool m_hebrew; /**< is this a Hebrew word list */
	const Trie* m_trie; /**< word list to find all spellings of a word */
};

#endif
