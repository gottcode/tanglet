/*
	SPDX-FileCopyrightText: 2009-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
	: QTreeWidget(parent)
	, m_active_item(nullptr)
	, m_hebrew(false)
	, m_trie(nullptr)
{
	setColumnCount(3);
	hideColumn(2);
	header()->setStretchLastSection(false);
	header()->setSectionResizeMode(0, QHeaderView::Stretch);
	header()->setSectionResizeMode(1, QHeaderView::Fixed);
	header()->hide();
	setColumnWidth(1, 22);

	setIconSize(QSize(16, 16));
	setMouseTracking(true);
	setRootIsDecorated(false);
	setSortingEnabled(true);
	sortByColumn(0, Qt::AscendingOrder);
	setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	connect(this, &QTreeWidget::itemClicked, this, &WordTree::onItemClicked);
}

//-----------------------------------------------------------------------------

QTreeWidgetItem* WordTree::addWord(const QString& word)
{
	QTreeWidgetItem* item = new QTreeWidgetItem(this);
	item->setText(2, word);
	if (!m_hebrew) {
		item->setText(0, word);
	} else {
		QString copy = word;
		int end = copy.length() - 1;
		switch (copy.at(end).unicode()) {
		case 0x05db:
			copy[end] = QChar(0x05da);
			break;
		case 0x05de:
			copy[end] = QChar(0x05dd);
			break;
		case 0x05e0:
			copy[end] = QChar(0x05df);
			break;
		case 0x05e4:
			copy[end] = QChar(0x05e3);
			break;
		case 0x05e6:
			copy[end] = QChar(0x05e5);
			break;
		default:
			break;
		}
		item->setText(0, copy);
	}

	QStringList spellings = m_trie->spellings(word, QStringList(item->text(0).toLower()));
	item->setData(1, Qt::UserRole, spellings);

	item->setIcon(1, QIcon(":/empty.png"));
	int score = Solver::score(word);
	item->setData(0, Qt::UserRole, score);

	spellings.append(tr("%n point(s)", "", score));
	item->setToolTip(0, spellings.join("\n"));

	return item;
}

//-----------------------------------------------------------------------------

void WordTree::removeAll()
{
	m_active_item = nullptr;
	clear();
	scrollToTop();
}

//-----------------------------------------------------------------------------

void WordTree::setDictionary(const QString& url)
{
	m_url = QUrl::toPercentEncoding(url, "#$%&+,/:;=?@~");
	m_url.replace("%25", "%"); // work around Qt's decision to force percent encoding of percent symbol (bug 110446)
}

//-----------------------------------------------------------------------------

void WordTree::setHebrew(bool hebrew)
{
	m_hebrew = hebrew;
}

//-----------------------------------------------------------------------------

void WordTree::setTrie(const Trie* trie)
{
	m_trie = trie;
}

//-----------------------------------------------------------------------------

void WordTree::leaveEvent(QEvent* event)
{
	QTreeWidget::leaveEvent(event);
	enterItem(nullptr);
}

//-----------------------------------------------------------------------------

void WordTree::mouseMoveEvent(QMouseEvent* event)
{
	QTreeWidget::mouseMoveEvent(event);
	enterItem(itemAt(event->pos()));
}

//-----------------------------------------------------------------------------

void WordTree::wheelEvent(QWheelEvent* event)
{
	QTreeWidget::wheelEvent(event);
	enterItem(itemAt(event->position().toPoint()));
}

//-----------------------------------------------------------------------------

void WordTree::onItemClicked(QTreeWidgetItem* item, int column)
{
	if (item && column == 1) {
		const QStringList spellings = item->data(1, Qt::UserRole).toStringList();
		QString word = spellings.first();

		if (spellings.count() > 1) {
			QMenu menu;
			for (const QString& spelling : spellings) {
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

void WordTree::enterItem(QTreeWidgetItem* item)
{
	if (m_active_item) {
		m_active_item->setIcon(1, QIcon(":/empty.png"));
	}
	m_active_item = item;
	if (m_active_item) {
		m_active_item->setIcon(1, QIcon::fromTheme("accessories-dictionary", QIcon(":/help-browser.png")));
	}
}

//-----------------------------------------------------------------------------
