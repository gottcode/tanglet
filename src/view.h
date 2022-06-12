/*
	SPDX-FileCopyrightText: 2009-2010 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_VIEW_H
#define TANGLET_VIEW_H

#include <QGraphicsView>

/**
 * @brief The View class displays the game area.
 */
class View : public QGraphicsView
{
	Q_OBJECT

public:
	/**
	 * Constructs a view instance.
	 * @param scene the game area to display
	 * @param parent the widget that owns the view
	 */
	explicit View(QGraphicsScene* scene, QWidget* parent = nullptr);

Q_SIGNALS:
	/**
	 * Emitted when the player clicks on the view.
	 */
	void mousePressed();

protected:
	/**
	 * Override to detect player clicking on the view.
	 * @param event the details of the mouse press event
	 */
	void mousePressEvent(QMouseEvent* event) override;

	/**
	 * Override to always zoom and recenter view when resized.
	 * @param event the details of the resize event
	 */
	void resizeEvent(QResizeEvent* event) override;
};

#endif // TANGLET_VIEW_H
