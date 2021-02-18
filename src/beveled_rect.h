/*
	SPDX-FileCopyrightText: 2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BEVELED_RECT_H
#define BEVELED_RECT_H

#include <QGraphicsItem>

class BeveledRect: public QGraphicsItem
{
public:
	explicit BeveledRect(int size, QGraphicsItem* parent = nullptr);

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	void setColor(const QColor& color, bool bevel = true);

private:
	int m_size;
	int m_steps;
	QRectF m_rects[5];
	QColor m_colors[5];
};

#endif
