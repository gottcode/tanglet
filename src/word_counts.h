/*
	SPDX-FileCopyrightText: 2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef WORD_COUNTS_H
#define WORD_COUNTS_H

#include <QScrollArea>
class QLabel;

class WordCounts : public QScrollArea
{
	Q_OBJECT

public:
	WordCounts(QWidget* parent = nullptr);

	void findWord(const QString& word);
	void setMaximumsVisible(bool visible);
	void setWords(const QStringList& words);

private:
	struct Group;
	void updateString();

private:
	bool m_show_max;
	struct Group
	{
		int length;
		int count;
		int max;
		QLabel* label;
	};
	QVector<Group> m_groups;
};

#endif
