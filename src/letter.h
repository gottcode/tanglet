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

#ifndef LETTER_H
#define LETTER_H

class BeveledRect;

#include <QGraphicsPathItem>
#include <QObject>
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;

class Letter : public QObject, public QGraphicsPathItem {
	Q_OBJECT

	public:
		Letter(const QFont& font, int size, const QPoint& position);

		QPoint position() const {
			return m_position;
		}

		QString text() const {
			return m_text;
		}

		void setArrow(qreal angle, int z);
		void setCellColor(const QColor& color);
		void setColor(const QColor& color);
		void setClickable(bool clickable);
		void setText(const QString& text);

	signals:
		void clicked(Letter* letter);

	protected:
		void mousePressEvent(QGraphicsSceneMouseEvent* event);

	private:
		void createCornerArrow();
		void createSideArrow();

	private:
		QGraphicsEllipseItem* m_face;
		QGraphicsSimpleTextItem* m_item;
		QString m_text;
		BeveledRect* m_cell;
		QGraphicsItem* m_arrow;
		bool m_clickable;
		QPoint m_position;
};

#endif
