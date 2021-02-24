/*
	SPDX-FileCopyrightText: 2009-2013 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_LETTER_H
#define TANGLET_LETTER_H

class BeveledRect;

#include <QGraphicsPathItem>
#include <QObject>
class QGraphicsEllipseItem;
class QGraphicsSimpleTextItem;

/**
 * @brief The Letter class displays a letter with background.
 */
class Letter : public QObject, public QGraphicsPathItem
{
	Q_OBJECT

public:
	/**
	 * Constructs a letter instance.
	 * @param font the font used for the text of the letter
	 * @param size how big to make the letter
	 * @param position the column and row location on the board
	 */
	Letter(const QFont& font, int size, const QPoint& position);

	/**
	 * @return the column and row location on the board
	 */
	QPoint position() const
	{
		return m_position;
	}

	/**
	 * @return the text displayed by the letter
	 */
	QString text() const
	{
		return m_text;
	}

	/**
	 * Add an arrow connecting to another letter. This will replace any previous arrow as a letter
	 * can only have one connection at a time.
	 * @param angle the direction to the next letter or 0 to remove arrow
	 * @param z the depth to position the arrow
	 */
	void setArrow(qreal angle, int z);

	/**
	 * Add a highlight to the letter.
	 * @param color the color of the highlight or empty to clear
	 */
	void setCellColor(const QColor& color);

	/**
	 * Sets the color of the letter.
	 * @param color the color of the letter
	 */
	void setColor(const QColor& color);

	/**
	 * Set whether player can interact with the letter.
	 * @param clickable
	 */
	void setClickable(bool clickable);

	/**
	 * Set the text displayed by the letter.
	 * @param text what to display
	 */
	void setText(const QString& text);

signals:
	/**
	 * Emitted when player clicks on letter.
	 * @param letter the letter instance
	 */
	void clicked(Letter* letter);

protected:
	/**
	 * Override parent mousePressEvent to process player clicks.
	 * @param event details of mouse press event
	 */
	void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

	/**
	 * Override parent mousePressEvent to prevent double clicks.
	 * @param event details of double press event
	 */
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
	/**
	 * Creates an arrow pointing at a 45 degree angle to the next letter.
	 */
	void createCornerArrow();

	/**
	 * Creates an arrow pointing horizontally or vertically to the next letter.
	 */
	void createSideArrow();

private:
	QGraphicsEllipseItem* m_face; /**< the dice of the letter */
	QGraphicsSimpleTextItem* m_item; /**< displays letter text */
	QString m_text; /**< what text to display */
	BeveledRect* m_cell; /**< the background of the letter */
	QGraphicsItem* m_arrow; /**< points to the next arrow in the word if it exists */
	bool m_clickable; /**< is the letter currently clickable */
	QPoint m_position; /**< where the letter is located in the board layout */
};

#endif // TANGLET_LETTER_H
