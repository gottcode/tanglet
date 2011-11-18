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

#include "word_tree.h"

#include "solver.h"
#include "trie.h"

#include <QDesktopServices>
#include <QHeaderView>
#include <QMenu>
#include <QMouseEvent>
#include <QUrl>

//-----------------------------------------------------------------------------

WordTree::WordTree(QWidget* parent)
: QTreeWidget(parent), m_active_item(0), m_hebrew(false), m_trie(0) {
	setColumnCount(3);
	hideColumn(2);
	header()->setStretchLastSection(false);
	header()->setResizeMode(0, QHeaderView::Stretch);
	header()->setResizeMode(1, QHeaderView::Fixed);
	header()->hide();
	setColumnWidth(1, 22);

	setIconSize(QSize(16,16));
	setMouseTracking(true);
	setRootIsDecorated(false);
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	connect(this, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(onItemClicked(QTreeWidgetItem*, int)));
}

//-----------------------------------------------------------------------------

QTreeWidgetItem* WordTree::addWord(const QString& word) {
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setText(2, word);
	if (!m_hebrew) {
		item->setText(0, word);
	} else {
		QString copy = word;
		int end = copy.length() - 1;
		switch (copy.at(end).unicode()) {
		case 0x05db:
			copy[end] = 0x05da;
			break;
		case 0x05de:
			copy[end] = 0x05dd;
			break;
		case 0x05e0:
			copy[end] = 0x05df;
			break;
		case 0x05e4:
			copy[end] = 0x05e3;
			break;
		case 0x05e6:
			copy[end] = 0x05e5;
			break;
		default:
			break;
		}
		item->setText(0, copy);
	}
	item->setIcon(1, QIcon(":/empty.png"));
	int score = Solver::score(word);
	item->setData(0, Qt::UserRole, score);
	item->setToolTip(0, tr("%n point(s)", "", score));
	return item;
}

//-----------------------------------------------------------------------------

void WordTree::removeAll() {
	m_active_item = 0;
	clear();
}

//-----------------------------------------------------------------------------

void WordTree::setDictionary(const QString& url) {
	m_url = QUrl::toPercentEncoding(url, "#$%&+,/:;=?@~");
}

//-----------------------------------------------------------------------------

void WordTree::setHebrew(bool hebrew) {
	m_hebrew = hebrew;
}

//-----------------------------------------------------------------------------

void WordTree::setTrie(const Trie* trie) {
	m_trie = trie;
}

//-----------------------------------------------------------------------------

void WordTree::leaveEvent(QEvent* event) {
	QTreeWidget::leaveEvent(event);
	enterItem(0);
}

//-----------------------------------------------------------------------------

void WordTree::mouseMoveEvent(QMouseEvent* event) {
	QTreeWidget::mouseMoveEvent(event);
	enterItem(itemAt(event->pos()));
}

//-----------------------------------------------------------------------------

void WordTree::wheelEvent(QWheelEvent* event) {
	QTreeWidget::wheelEvent(event);
	enterItem(itemAt(event->pos()));
}

//-----------------------------------------------------------------------------

void WordTree::onItemClicked(QTreeWidgetItem* item, int column) {
	if (item && column == 1) {
		QStringList spellings = m_trie->spellings(item->text(2), QStringList(item->text(0).toLower()));
		QString word = spellings.first();

		if (spellings.count() > 1) {
			QMenu menu;
			foreach (const QString& spelling, spellings) {
				menu.addAction(spelling);
			}
			QAction* action = menu.exec(QCursor::pos());
			if (action) {
				word = action->text();
			} else {
				return;
			}
		}

		QByteArray url(m_url);
		url.replace("%s", QUrl::toPercentEncoding(word));
		QDesktopServices::openUrl(QUrl::fromEncoded(url));
	}
}

//-----------------------------------------------------------------------------

void WordTree::enterItem(QTreeWidgetItem* item) {
	if (m_active_item) {
		m_active_item->setIcon(1, QIcon(":/empty.png"));
	}
	m_active_item = item;
	if (m_active_item) {
		m_active_item->setIcon(1, QIcon::fromTheme("accessories-dictionary", QIcon(":/help-browser.png")));
	}
}

//-----------------------------------------------------------------------------
