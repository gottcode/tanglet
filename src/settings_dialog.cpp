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

#include "settings_dialog.h"

#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QGridLayout>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

SettingsDialog::SettingsDialog(bool show_warning, QWidget* parent)
: QDialog(parent), m_new(false), m_changed(false) {
	setWindowTitle(tr("Settings"));

	QSettings settings;

	m_language = new QComboBox(this);
	m_language->addItem(tr("English"), QLocale::English);
	m_language->addItem(tr("French"), QLocale::French);
	m_language->addItem(tr("Custom"), 0);
	m_language->setCurrentIndex(m_language->count() - 1);
	connect(m_language, SIGNAL(currentIndexChanged(int)), this, SLOT(chooseLanguage(int)));

	m_dice = new QLineEdit(this);
	m_dice->setText(settings.value("Dice").toString());
	m_choose_dice = new QPushButton(tr("Choose..."), this);
	connect(m_choose_dice, SIGNAL(clicked()), this, SLOT(chooseDice()));

	m_words = new QLineEdit(this);
	m_words->setText(settings.value("Words").toString());
	m_choose_words = new QPushButton(tr("Choose..."), this);
	connect(m_choose_words, SIGNAL(clicked()), this, SLOT(chooseWords()));

	m_dictionary = new QLineEdit(this);
	m_dictionary->setText(settings.value("Dictionary").toString());

	setLanguage(settings.value("Language", QLocale::system().language()).toInt());

	// Creat warning message
	QLabel* warning_img = new QLabel(this);
	warning_img->setPixmap(QString(":/dialog-warning"));
	QLabel* warning_text = new QLabel(tr("<b>Note:</b> Changing these settings will start a new game."), this);

	QHBoxLayout* warning = new QHBoxLayout;
	warning->addWidget(warning_img);
	warning->addWidget(warning_text);
	warning->addStretch();

	// Create score option
	m_show_score = new QCheckBox(tr("Show maximum score:"), this);

	m_score_type = new QComboBox(this);
	m_score_type->addItem(tr("At end of game"));
	m_score_type->addItem(tr("During game"));
	m_score_type->setEnabled(false);
	connect(m_show_score, SIGNAL(toggled(bool)), m_score_type, SLOT(setEnabled(bool)));

	int score_type = settings.value("Gameplay/ScoreType", 1).toInt();
	if (score_type == 2) {
		m_score_type->setCurrentIndex(1);
	}
	m_show_score->setChecked(score_type);

	QHBoxLayout* score_layout = new QHBoxLayout;
	score_layout->setMargin(0);
	score_layout->addWidget(m_show_score);
	score_layout->addWidget(m_score_type);
	score_layout->addStretch();

	// Create buttons
	m_buttons = new QDialogButtonBox(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
	connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));

	// Lay out window
	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(1, 1);

	layout->addLayout(score_layout, 0, 1, 1, 2);

	layout->setRowMinimumHeight(1, 18);

	layout->addWidget(new QLabel(tr("Language:"), this), 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_language, 2, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

	layout->addWidget(new QLabel(tr("Dice:"), this), 3, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_dice, 3, 1);
	layout->addWidget(m_choose_dice, 3, 2);

	layout->addWidget(new QLabel(tr("Word list:"), this), 4, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_words, 4, 1);
	layout->addWidget(m_choose_words, 4, 2);

	layout->addWidget(new QLabel(tr("Dictionary:"), this), 5, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_dictionary, 5, 1, 1, 2);

	layout->addLayout(warning, 6, 1);
	if (!show_warning) {
		warning_img->hide();
		warning_text->hide();
	}
	layout->setColumnMinimumWidth(1, warning->sizeHint().width());

	layout->setRowStretch(7, 1);
	layout->setRowMinimumHeight(7, 24);

	layout->addWidget(m_buttons, 8, 0, 1, 3);
}

//-----------------------------------------------------------------------------

void SettingsDialog::restoreDefaults() {
	SettingsDialog dialog(false);
	dialog.m_buttons->button(QDialogButtonBox::RestoreDefaults)->click();
	dialog.accept();
}

//-----------------------------------------------------------------------------

void SettingsDialog::accept() {
	QSettings settings;

	int score_type = m_show_score->isChecked() ? (m_score_type->currentIndex() + 1) : 0;
	if (settings.value("Gameplay/ScoreType", 1).toInt() != score_type) {
		settings.setValue("Gameplay/ScoreType", score_type);
		m_changed = true;
	}

	int lang = m_language->itemData(m_language->currentIndex()).toInt();
	if (settings.value("Language").toInt() != lang) {
		settings.setValue("Language", lang);
		m_new = true;
		m_changed = true;
		if (lang == 0) {
			settings.setValue("CustomDice", m_dice->text());
			settings.setValue("CustomWords", m_words->text());
			settings.setValue("CustomDictionary", m_dictionary->text());
		}
	}
	if (settings.value("Dice").toString() != m_dice->text()) {
		settings.setValue("Dice", m_dice->text());
		m_new = true;
		m_changed = true;
	}
	if (settings.value("Words").toString() != m_words->text()) {
		settings.setValue("Words", m_words->text());
		m_new = true;
		m_changed = true;
	}
	if (settings.value("Dictionary") != m_dictionary->text()) {
		settings.setValue("Dictionary", m_dictionary->text());
		m_changed = true;
	}

	QDialog::accept();
}

//-----------------------------------------------------------------------------

void SettingsDialog::clicked(QAbstractButton* button) {
	if (m_buttons->buttonRole(button) == QDialogButtonBox::ResetRole) {
		m_show_score->setChecked(true);
		m_score_type->setCurrentIndex(0);

		QSettings settings;
		settings.remove("CustomDice");
		settings.remove("CustomWords");
		settings.remove("CustomDictionary");
		setLanguage(QLocale::system().language());
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::chooseLanguage(int index) {
	QSettings settings;

	bool enabled = false;
	switch (m_language->itemData(index).toInt()) {
	case QLocale::English:
		m_dice->setText("tanglet:en/dice");
		m_words->setText("tanglet:en/words");
		m_dictionary->setText("http://www.google.com/dictionary?langpair=en|en&q=%s");
		break;
	case QLocale::French:
		m_dice->setText("tanglet:fr/dice");
		m_words->setText("tanglet:fr/words");
		m_dictionary->setText("http://www.google.com/dictionary?langpair=fr|fr&q=%s");
		break;
	case 0:
	default:
		m_dice->setText(settings.value("CustomDice", m_dice->text()).toString());
		m_words->setText(settings.value("CustomWords", m_words->text()).toString());
		m_dictionary->setText(settings.value("CustomDictionary", m_dictionary->text()).toString());
		enabled = true;
		break;
	}

	m_dice->setEnabled(enabled);
	m_choose_dice->setEnabled(enabled);
	m_words->setEnabled(enabled);
	m_choose_words->setEnabled(enabled);
	m_dictionary->setEnabled(enabled);
}

//-----------------------------------------------------------------------------

void SettingsDialog::chooseDice() {
	QString path = QFileDialog::getOpenFileName(this, tr("Choose Dice File"), m_dice->text());
	if (!path.isEmpty()) {
		m_dice->setText(path);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::chooseWords() {
	QString path = QFileDialog::getOpenFileName(this, tr("Choose Word List File"), m_words->text());
	if (!path.isEmpty()) {
		m_words->setText(path);
	}
}

//-----------------------------------------------------------------------------

void SettingsDialog::setLanguage(int language) {
	int index = m_language->findData(language);
	if (index == -1) {
		index = 0;
	}
	m_language->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
