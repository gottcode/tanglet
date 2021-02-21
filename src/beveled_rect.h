/*
	SPDX-FileCopyrightText: 2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef BEVELED_RECT_H
#define BEVELED_RECT_H

#include <QGraphicsItem>

/**
 * @brief The BeveledRect class displays a rounded square with the appearance of depth.
 */
class BeveledRect: public QGraphicsItem
{
public:
	/**
	 * Constructs a beveled rectangle instance.
	 * @param size how big the rectangle should be
	 * @param parent the item that owns the rectangle
	 */
	explicit BeveledRect(int size, QGraphicsItem* parent = nullptr);

	/**
	 * @return the size of the beveled rectangle
	 */
	QRectF boundingRect() const override;

	/**
	 * Draws the beveled rectangle.
	 * @param painter the painter used to draw the rectangle
	 */
	void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override;

	/**
	 * Sets the color of the beveled rectangle.
	 * @param color what color to draw in the paint event
	 * @param bevel if the rectangle should have the appearance of depth
	 */
	void setColor(const QColor& color, bool bevel = true);

private:
	int m_size; /**< width of the square */
	int m_steps; /**< how many levels deep to draw */
	QRectF m_rects[5]; /**< regions to draw at each depth */
	QColor m_colors[5]; /**< colors to draw at each depth */
};

#endif
