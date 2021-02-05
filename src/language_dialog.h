/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2014 Graeme Gott <graeme@gottcode.org>
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

#ifndef LANGUAGE_DIALOG_H
#define LANGUAGE_DIALOG_H

#include <QDialog>
class QAbstractButton;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;

class LanguageDialog : public QDialog
{
	Q_OBJECT

public:
	LanguageDialog(QWidget* parent = 0);

	static void restoreDefaults();

public slots:
	virtual void accept();

private slots:
	void clicked(QAbstractButton* button);
	void chooseLanguage(int index);
	void browseDice();
	void chooseDice(const QString& dice);
	void browseWords();
	void chooseWords(const QString& words);

private:
	void setLanguage(int language);

private:
	QComboBox* m_language;
	QString m_dice_path;
	QLineEdit* m_dice;
	QPushButton* m_choose_dice;
	QString m_words_path;
	QLineEdit* m_words;
	QPushButton* m_choose_words;
	QLineEdit* m_dictionary;

	QDialogButtonBox* m_buttons;
};

#endif
