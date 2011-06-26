/***********************************************************************
 *
 * Copyright (C) 2010, 2011 Graeme Gott <graeme@gottcode.org>
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

#ifndef NEW_GAME_DIALOG_H
#define NEW_GAME_DIALOG_H

#include <QDialog>
class QComboBox;
class QToolButton;

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	NewGameDialog(QWidget* parent = 0);

	static QString densityString(int density);

private slots:
	void lengthChanged(int length);
	void sizeChanged();
	void timerChosen(int timer);

private:
	QToolButton* m_normal_size;
	QToolButton* m_large_size;
	QComboBox* m_density;
	QComboBox* m_length;
	int m_minimum;
};

#endif
