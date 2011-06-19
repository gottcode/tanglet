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

#include "word_counts.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>

//-----------------------------------------------------------------------------

WordCounts::WordCounts(QWidget* parent)
	: QScrollArea(parent), m_show_max(false)
{
	setBackgroundRole(QPalette::Mid);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QWidget* contents = new QWidget(this);
	setWidget(contents);
	setWidgetResizable(true);

	QLabel* label = new QLabel(QString("<small><b>%1<br>%2</b></small>").arg(tr("Letters:"), tr("Found:")), contents);
	label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	QHBoxLayout* layout = new QHBoxLayout(contents);
	layout->setSpacing(12);
	layout->addWidget(label);

	for (int i = 0; i < 26; ++i) {
		Group group;
		group.length = i;
		group.count = 0;
		group.max = 0;
		group.label = new QLabel(contents);
		group.label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
		group.label->setVisible(false);
		layout->addWidget(group.label);
		m_groups.append(group);
	}
	layout->addStretch();

	updateString();
	setMinimumHeight(contents->sizeHint().height() + horizontalScrollBar()->sizeHint().height());
}

//-----------------------------------------------------------------------------

void WordCounts::findWord(const QString& word)
{
	Q_ASSERT(word.length() < 26);
	Group& group = m_groups[word.length()];
	group.count++;
	updateString();
}

//-----------------------------------------------------------------------------

void WordCounts::setMaximumsVisible(bool visible)
{
	m_show_max = visible;
	updateString();
}

//-----------------------------------------------------------------------------

void WordCounts::setWords(const QStringList& words)
{
	for (int i = 0; i < 26; ++i) {
		Group& group = m_groups[i];
		group.count = 0;
		group.max = 0;
	}

	foreach (const QString& word, words) {
		Q_ASSERT(word.length() < 26);
		Group& group = m_groups[word.length()];
		group.max++;
	}

	updateString();
}

//-----------------------------------------------------------------------------

void WordCounts::updateString()
{
	// Set text in columns
	int max_count = 0;
	int max = 0;
	for (int i = 0; i < 26; ++i) {
		Group& group = m_groups[i];
		max = qMax(max, group.max);
		max_count = qMax(max_count, group.count);

		QString text;
		if (!m_show_max) {
			if (group.count > 0) {
				text = tr("%1<br><b>%2</b>").arg(group.length).arg(group.count);
			}
		} else {
			if (group.max > 0) {
				if (group.count > 0) {
					text = tr("%1<br><b>%2/%3</b>").arg(group.length).arg(group.count).arg(group.max);
				} else {
					text = tr("%1<br>%2/%3").arg(group.length).arg(group.count).arg(group.max);
				}
			}
		}
		group.label->setText("<small>" + text + "</small>");
		group.label->setVisible(!text.isEmpty());
	}
	int width = fontMetrics().width(m_show_max ? QString("%1/%2").arg(max_count).arg(max) : QString::number(max));

	// Resize columns
	for (int i = 0; i < 26; ++i) {
		m_groups[i].label->setMinimumWidth(width);
	}
}

//-----------------------------------------------------------------------------
