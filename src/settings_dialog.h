/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#ifndef SETTINGS_DIALOG_H

#include <QDialog>
class QAbstractButton;
class QCheckBox;
class QComboBox;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;
class Settings;

class SettingsDialog : public QDialog {
	Q_OBJECT

	public:
		SettingsDialog(Settings& settings, bool show_warning, QWidget* parent = 0);

		static void restoreDefaults();

	public slots:
		virtual void accept();

	private slots:
		void clicked(QAbstractButton* button);
		void chooseLanguage(int index);
		void chooseDice();
		void chooseWords();

	private:
		void setLanguage(int language);

	private:
		Settings& m_settings;

		QCheckBox* m_show_score;
		QComboBox* m_score_type;
		QCheckBox* m_show_missed;

		QComboBox* m_language;
		QLineEdit* m_dice;
		QPushButton* m_choose_dice;
		QLineEdit* m_words;
		QPushButton* m_choose_words;
		QLineEdit* m_dictionary;

		QDialogButtonBox* m_buttons;
};

#endif
