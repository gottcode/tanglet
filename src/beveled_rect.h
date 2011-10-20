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

#ifndef BEVELED_RECT_H
#define BEVELED_RECT_H

#include <QGraphicsItem>

class BeveledRect: public QGraphicsItem
{
public:
	BeveledRect(int size, QGraphicsItem* parent = 0);

	QRectF boundingRect() const;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

	void setColor(const QColor& color, bool bevel = true);

private:
	int m_size;
	int m_steps;
	QRectF m_rects[5];
	QColor m_colors[5];
};

#endif
