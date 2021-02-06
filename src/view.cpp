/*
	SPDX-FileCopyrightText: 2009-2010 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
