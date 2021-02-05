/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#include "view.h"

//-----------------------------------------------------------------------------

View::View(QGraphicsScene* scene, QWidget* parent)
	: QGraphicsView(scene, parent)
{
	setFrameStyle(QFrame::NoFrame);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

	setBackgroundRole(QPalette::Window);
	QPalette p = palette();
	p.setColor(QPalette::Window, Qt::transparent);
	setPalette(p);
}

//-----------------------------------------------------------------------------

void View::mousePressEvent(QMouseEvent* event)
{
	QGraphicsView::mousePressEvent(event);
	emit mousePressed();
}

//-----------------------------------------------------------------------------

void View::resizeEvent(QResizeEvent* event)
{
	QGraphicsView::resizeEvent(event);
	fitInView(sceneRect(), Qt::KeepAspectRatio);
}

//-----------------------------------------------------------------------------
