/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef LETTER_H
#define LETTER_H

class BeveledRect;

#include <QGraphicsPathItem>
#include <QObject>
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;

class Letter : public QObject, public QGraphicsPathItem
{
	Q_OBJECT

public:
	Letter(const QFont& font, int size, const QPoint& position);

	QPoint position() const
	{
		return m_position;
	}

	QString text() const
	{
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
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

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
