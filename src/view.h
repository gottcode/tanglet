/*
	SPDX-FileCopyrightText: 2009-2010 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef VIEW_H
#define VIEW_H

#include <QGraphicsView>

class View : public QGraphicsView
{
	Q_OBJECT

public:
	explicit View(QGraphicsScene* scene, QWidget* parent = nullptr);

signals:
	void mousePressed();

protected:
	void mousePressEvent(QMouseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
};

#endif
