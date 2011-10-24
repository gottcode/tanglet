/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011 Graeme Gott <graeme@gottcode.org>
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

#ifndef WORD_TREE_H
#define WORD_TREE_H

#include <QTreeWidget>

class WordTree : public QTreeWidget {
	Q_OBJECT

	public:
		WordTree(QWidget* parent = 0);

		QTreeWidgetItem* addWord(const QString& word);
		void removeAll();
		void setDictionary(const QString& url);

	protected:
		virtual void leaveEvent(QEvent* event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* event);

	private slots:
		void onItemClicked(QTreeWidgetItem* item, int column);

	private:
		void enterItem(QTreeWidgetItem* item);

	private:
		QTreeWidgetItem* m_active_item;
		QByteArray m_url;
		bool m_hebrew;
};

#endif
