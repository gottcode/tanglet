/***********************************************************************
 *
 * Copyright (C) 2011 Graeme Gott <graeme@gottcode.org>
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

#ifndef WORD_COUNTS_H
#define WORD_COUNTS_H

#include <QScrollArea>
class QLabel;

class WordCounts : public QScrollArea
{
	Q_OBJECT

public:
	WordCounts(QWidget* parent = 0);

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
