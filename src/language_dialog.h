/*
	SPDX-FileCopyrightText: 2009-2014 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

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
	LanguageDialog(QWidget* parent = nullptr);

	static void restoreDefaults();

public slots:
	void accept() override;

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
