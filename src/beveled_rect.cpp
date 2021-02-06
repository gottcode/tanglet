/*
	SPDX-FileCopyrightText: 2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "beveled_rect.h"

#include <QPainter>

//-----------------------------------------------------------------------------

BeveledRect::BeveledRect(int size, QGraphicsItem* parent)
	: QGraphicsItem(parent)
	, m_size(size)
	, m_steps(5)
{
}

//-----------------------------------------------------------------------------

QRectF BeveledRect::boundingRect() const
{
	return QRectF(0, 0, m_size, m_size);
}

//-----------------------------------------------------------------------------

void BeveledRect::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->save();

	painter->setPen(Qt::NoPen);

	for (int i = 0; i < m_steps; ++i) {
		painter->setBrush(m_colors[i]);
		painter->drawRoundedRect(m_rects[i], 5, 5);
	}

	painter->restore();
}

//-----------------------------------------------------------------------------

void BeveledRect::setColor(const QColor& color, bool bevel)
{
	if (bevel) {
		m_steps = 5;

		m_rects[0] = QRectF(0, 0, m_size, m_size);
		m_colors[0] = color.darker();

		m_rects[1] = QRectF(0.5, 0.5, m_size - 1, m_size - 1);
		m_colors[1] = color.darker(168);

		m_rects[2] = QRectF(1, 1, m_size - 2, m_size - 2);
		m_colors[2] = color.darker(127);

		m_rects[3] = QRectF(1.5, 1.5, m_size - 3, m_size - 3);
		m_colors[3] = color.darker(106);

		m_rects[4] = QRectF(2, 2, m_size - 4, m_size - 4);
		m_colors[4] = color;
	} else {
		m_steps = 1;

		m_rects[0] = QRectF(0, 0, m_size, m_size);
		m_colors[0] = color;
	}

	update();
}

//-----------------------------------------------------------------------------
