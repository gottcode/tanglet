/*
	SPDX-FileCopyrightText: 2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_WORD_COUNTS_H
#define TANGLET_WORD_COUNTS_H

#include <QScrollArea>
class QLabel;

/**
 * @brief The WordCounts class displays the lengths of found words.
 */
class WordCounts : public QScrollArea
{
	Q_OBJECT

	/**
	 * @brief The WordCounts::Group struct represents a count of words for a length
	 */
	struct Group
	{
		int length; /**< how long the words are */
		int count; /**< wow many words have been found */
		int max; /**< how many words are available */
		QLabel* label; /**< display count */
	};

public:
	/**
	 * Constructs a word counts instance.
	 * @param parent the widget that owns the word counts
	 */
	explicit WordCounts(QWidget* parent = nullptr);

	/**
	 * Add one to a word length column.
	 * @param word the word whose length is used
	 */
	void findWord(const QString& word);

	/**
	 * Hides or shows the maximum available word lengths.
	 * @param visible whether maximums are shown
	 */
	void setMaximumsVisible(bool visible);

	/**
	 * Determines the maximum available word lengths.
	 * @param words list of words to scan for lengths
	 */
	void setWords(const QStringList& words);

private:
	/**
	 * Updates the display of word counts.
	 */
	void updateString();

private:
	bool m_show_max; /**< whether to show the maximum scores available for each length */
	QVector<Group> m_groups; /**< collection of word lengths */
};

#endif // TANGLET_WORD_COUNTS_H
