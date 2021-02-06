/*
	SPDX-FileCopyrightText: 2010-2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef NEW_GAME_DIALOG_H
#define NEW_GAME_DIALOG_H

#include <QDialog>
class QComboBox;
class QToolButton;

class NewGameDialog : public QDialog
{
	Q_OBJECT

public:
	NewGameDialog(QWidget* parent = nullptr);

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
