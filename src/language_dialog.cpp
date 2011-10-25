/***********************************************************************
 *
 * Copyright (C) 2009, 2010, 2011 Graeme Gott <graeme@gottcode.org>
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

#include "language_dialog.h"

#include <QDialogButtonBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QGridLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QTextStream>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

LanguageDialog::LanguageDialog(QWidget* parent)
: QDialog(parent) {
	setWindowTitle(tr("Board Language"));

	m_language = new QComboBox(this);
	QStringList languages = QDir("tanglet:").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach (const QString& iso_code, languages) {
		QLocale::Language language = QLocale(iso_code).language();
		QString name = QLocale::languageToString(language);
		QFile file("tanglet:/" + iso_code + "/name");
		if (file.open(QFile::ReadOnly | QFile::Text)) {
			QString localized = QString::fromUtf8(file.readAll()).simplified();
			file.close();
			name = !localized.isEmpty() ? localized : name;
		}
		int i;
		for (i = 0; i < m_language->count(); ++i) {
			if (m_language->itemText(i).localeAwareCompare(name) >= 0) {
				break;
			}
		}
		m_language->insertItem(i, name, language);
	}
	m_language->addItem(tr("Custom"), 0);
	m_language->setCurrentIndex(m_language->count() - 1);
	connect(m_language, SIGNAL(currentIndexChanged(int)), this, SLOT(chooseLanguage(int)));

	m_dice = new QLineEdit(this);
	m_dice_path = m_settings.dice();
	m_dice->setText(QDir::toNativeSeparators(QFileInfo(m_dice_path).canonicalFilePath()));
	connect(m_dice, SIGNAL(textEdited(const QString&)), this, SLOT(chooseDice(const QString&)));
	m_choose_dice = new QPushButton(tr("Choose..."), this);
	connect(m_choose_dice, SIGNAL(clicked()), this, SLOT(chooseDice()));

	m_words = new QLineEdit(this);
	m_words_path = m_settings.words();
	m_words->setText(QDir::toNativeSeparators(QFileInfo(m_words_path).canonicalFilePath()));
	connect(m_words, SIGNAL(textEdited(const QString&)), this, SLOT(chooseWords(const QString&)));
	m_choose_words = new QPushButton(tr("Choose..."), this);
	connect(m_choose_words, SIGNAL(clicked()), this, SLOT(chooseWords()));

	m_dictionary = new QLineEdit(this);
	m_dictionary->setText(m_settings.dictionary());

	setLanguage(m_settings.language());

	// Creat warning message
	QLabel* warning = new QLabel(tr("<b>Note:</b> These settings will take effect when you start a new game."), this);

	// Create buttons
	m_buttons = new QDialogButtonBox(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(m_buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
	connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));

	// Lay out window
	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(1, 1);

	layout->addWidget(new QLabel(tr("Language:"), this), 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_language, 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

	layout->addWidget(new QLabel(tr("Dice:"), this), 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_dice, 1, 1);
	layout->addWidget(m_choose_dice, 1, 2);

	layout->addWidget(new QLabel(tr("Word list:"), this), 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_words, 2, 1);
	layout->addWidget(m_choose_words, 2, 2);

	layout->addWidget(new QLabel(tr("Dictionary:"), this), 3, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_dictionary, 3, 1, 1, 2);

	layout->addWidget(warning, 4, 1, 1, 2);

	layout->setRowStretch(5, 1);
	layout->setRowMinimumHeight(5, 24);

	layout->addWidget(m_buttons, 6, 0, 1, 3);
}

//-----------------------------------------------------------------------------

void LanguageDialog::restoreDefaults() {
	QSettings().setValue("Language", -1);
	LanguageDialog dialog;
	dialog.m_buttons->button(QDialogButtonBox::RestoreDefaults)->click();
	dialog.accept();
}

//-----------------------------------------------------------------------------

void LanguageDialog::accept() {
	m_settings.setDice(m_dice_path);
	m_settings.setWords(m_words_path);
	m_settings.setDictionary(m_dictionary->text());
	m_settings.setLanguage(m_language->itemData(m_language->currentIndex()).toInt());
	QDialog::accept();
}

//-----------------------------------------------------------------------------

void LanguageDialog::clicked(QAbstractButton* button) {
	if (m_buttons->buttonRole(button) == QDialogButtonBox::ResetRole) {
		QSettings settings;
		settings.remove("CustomDice");
		settings.remove("CustomWords");
		settings.remove("CustomDictionary");
		setLanguage(QLocale::system().language());
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::chooseLanguage(int index) {
	QSettings settings;

	bool enabled = false;
	int language = m_language->itemData(index).toInt();
	if (language != 0) {
		QString iso_code = QLocale(static_cast<QLocale::Language>(language)).name().left(2);
		m_dice_path = "tanglet:" + iso_code + "/dice";
		m_words_path = "tanglet:" + iso_code + "/words";
		QString dictionary;
		QFile file("tanglet:" + iso_code + "/dictionary");
		if (file.open(QFile::ReadOnly | QFile::Text)) {
			QTextStream stream(&file);
			stream.setCodec("UTF-8");
			dictionary = stream.readLine().simplified();
			file.close();
		}
		m_dictionary->setText(!dictionary.isEmpty() ? dictionary : "http://" + iso_code + ".wiktionary.org/wiki/%s");
	} else {
		m_dice_path = settings.value("CustomDice", m_dice_path).toString();
		m_words_path = settings.value("CustomWords", m_words_path).toString();
		m_dictionary->setText(settings.value("CustomDictionary", m_dictionary->text()).toString());
		enabled = true;
	}

	m_dice->setText(QDir::toNativeSeparators(QFileInfo(m_dice_path).canonicalFilePath()));
	m_dice->setEnabled(enabled);
	m_choose_dice->setEnabled(enabled);

	m_words->setText(QDir::toNativeSeparators(QFileInfo(m_words_path).canonicalFilePath()));
	m_words->setEnabled(enabled);
	m_choose_words->setEnabled(enabled);

	m_dictionary->setEnabled(enabled);
}

//-----------------------------------------------------------------------------

void LanguageDialog::chooseDice(const QString& dice) {
	QString path = !dice.isEmpty() ? dice : QFileDialog::getOpenFileName(this, tr("Choose Dice File"), m_dice->text());
	if (!path.isEmpty()) {
		m_dice_path = QFileInfo(path).canonicalFilePath();
		m_dice->setText(QDir::toNativeSeparators(m_dice_path));
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::chooseWords(const QString& words) {
	QString path = !words.isEmpty() ? words : QFileDialog::getOpenFileName(this, tr("Choose Word List File"), m_words->text());
	if (!path.isEmpty()) {
		m_words_path = QFileInfo(path).canonicalFilePath();
		m_words->setText(QDir::toNativeSeparators(m_words_path));
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::setLanguage(int language) {
	int index = m_language->findData(language);
	if (index == -1) {
		index = m_language->findData(QLocale::English);
	}
	m_language->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
