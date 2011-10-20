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

#include "letter.h"

#include "beveled_rect.h"

#include <QBrush>
#include <QCursor>
#include <QGraphicsEllipseItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPainterPath>
#include <QPen>

//-----------------------------------------------------------------------------

Letter::Letter(const QFont& font, int size, const QPoint& position)
: m_arrow(0), m_clickable(true), m_position(position) {
	QPainterPath path;
	path.addRoundedRect(0, 0, size, size, 5, 5);
	setPath(path);
	setPen(Qt::NoPen);
	setBrush(Qt::white);
	setHandlesChildEvents(true);
	setZValue(1);

	m_face = new QGraphicsEllipseItem(0.5, 0.5, size - 1, size - 1, this);
	m_face->setPen(QPen(Qt::white, 1));
	m_face->setBrush(Qt::white);

	m_item = new QGraphicsSimpleTextItem(this);
	m_item->setFont(font);
	m_item->setBrush(Qt::black);
	setText("?");

	m_cell = new BeveledRect(size + 3, this);
	m_cell->setPos(-1.5, -1.5);
	m_cell->setFlag(QGraphicsItem::ItemStacksBehindParent);
	m_cell->setColor("#0057ae");
}

//-----------------------------------------------------------------------------

void Letter::setArrow(qreal angle, int z) {
	delete m_arrow;
	m_arrow = 0;
	if (angle < 0.0) {
		return;
	}

	QRect rect = sceneBoundingRect().toRect();
	switch (qRound(angle / 45.0)) {
		case 0: // right
			createSideArrow();
			m_arrow->moveBy(rect.right() - 8, rect.center().y());
			break;

		case 1: // top right
			createCornerArrow();
			m_arrow->moveBy(rect.right() - 8.5, rect.top() + 8.5);
			m_arrow->rotate(270);
			break;

		case 2: // top
			createSideArrow();
			m_arrow->moveBy(rect.center().x(), rect.top() + 8);
			m_arrow->rotate(270);
			break;

		case 3: // top left
			createCornerArrow();
			m_arrow->moveBy(rect.left() + 8.5, rect.top() + 8.5);
			m_arrow->rotate(180);
			break;

		case 4: // left
			createSideArrow();
			m_arrow->moveBy(rect.left() + 8, rect.center().y());
			m_arrow->rotate(180);
			break;

		case 5: // bottom left
			createCornerArrow();
			m_arrow->moveBy(rect.left() + 8.5, rect.bottom() - 8.5);
			m_arrow->rotate(90);
			break;

		case 6: // bottom
			createSideArrow();
			m_arrow->moveBy(rect.center().x(), rect.bottom() - 8);
			m_arrow->rotate(90);
			break;

		case 7: // bottom right
			createCornerArrow();
			m_arrow->moveBy(rect.right() - 8.5, rect.bottom() - 8.5);
			break;
	};

	m_arrow->setZValue(z + 2);
}

//-----------------------------------------------------------------------------

void Letter::setCellColor(const QColor& color) {
	if (color.isValid()) {
		m_cell->setColor(color, false);
	} else {
		m_cell->setColor("#0057ae");
	}
}

//-----------------------------------------------------------------------------

void Letter::setColor(const QColor& color) {
	QColor darker = color.darker(106);
	QColor darkest = color.darker(160);

	int height = boundingRect().height();

	QLinearGradient sides(0, 0, 0, height);
	sides.setColorAt(0, darker);
	sides.setColorAt(1, darkest);
	setBrush(sides);

	QLinearGradient face_border(0, 0, 0, height);
	face_border.setColorAt(0, color);
	face_border.setColorAt(1, darker);
	m_face->setPen(QPen(face_border, 1));

	int radius = height / 2;
	QRadialGradient face(radius, radius, radius);
	face.setColorAt(0, color);
	face.setColorAt(0.5, color);
	face.setColorAt(1, darker);
	m_face->setBrush(face);
}

//-----------------------------------------------------------------------------

void Letter::setClickable(bool clickable) {
	m_clickable = clickable;
	if (m_clickable) {
		setCursor(Qt::PointingHandCursor);
		setColor(Qt::white);
	} else {
		unsetCursor();
		setColor(QColor("#aaa"));
	}
}

//-----------------------------------------------------------------------------

void Letter::setText(const QString& text) {
	m_text = text;
	m_item->setText(m_text);
	m_item->setPos(boundingRect().center() - m_item->boundingRect().center());
}

//-----------------------------------------------------------------------------

void Letter::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	Q_UNUSED(event);
	if (m_clickable) {
		emit clicked(this);
	}
}

//-----------------------------------------------------------------------------

void Letter::createCornerArrow() {
	QPainterPath path;
	path.moveTo(0, 0);
	path.lineTo(22, 22);
	path.moveTo(22, 14);
	path.lineTo(22, 22);
	path.lineTo(14, 22);

	m_arrow = scene()->addPath(path, QPen(Qt::black, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	QGraphicsItem* fill = scene()->addPath(path, QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	fill->setParentItem(m_arrow);
}

//-----------------------------------------------------------------------------

void Letter::createSideArrow() {
	QPainterPath path;
	path.moveTo(0, 0);
	path.lineTo(22, 0);
	path.moveTo(16, -6);
	path.lineTo(22, 0);
	path.lineTo(16, 6);

	m_arrow = scene()->addPath(path, QPen(Qt::black, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	QGraphicsItem* fill = scene()->addPath(path, QPen(Qt::white, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	fill->setParentItem(m_arrow);
}

//-----------------------------------------------------------------------------
