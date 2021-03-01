/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "language_dialog.h"

#include "language_settings.h"

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
	: QDialog(parent)
{
	setWindowTitle(tr("Board Language"));

	m_language = new QComboBox(this);
	const QStringList languages = QDir("tanglet:").entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	for (const QString& language : languages) {
		QSettings settings(QString("tanglet:%1/language.ini").arg(language), QSettings::IniFormat);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
		settings.setIniCodec("UTF-8");
#endif
		QString name = settings.value("Language/Name").toString();
		if (name.isEmpty()) {
			name = QLocale(language).nativeLanguageName();
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
	connect(m_language, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LanguageDialog::chooseLanguage);

	QSettings settings;
	settings.beginGroup("Board");

	m_dice = new QLineEdit(this);
	m_dice_path = settings.value("Dice").toString();
	m_dice->setText(QDir::toNativeSeparators(QFileInfo(m_dice_path).canonicalFilePath()));
	connect(m_dice, &QLineEdit::textEdited, this, &LanguageDialog::chooseDice);
	QPushButton* choose_dice = new QPushButton(tr("Choose..."), this);
	connect(choose_dice, &QPushButton::clicked, this, &LanguageDialog::browseDice);

	m_words = new QLineEdit(this);
	m_words_path = settings.value("Words").toString();
	m_words->setText(QDir::toNativeSeparators(QFileInfo(m_words_path).canonicalFilePath()));
	connect(m_words, &QLineEdit::textEdited, this, &LanguageDialog::chooseWords);
	QPushButton* choose_words = new QPushButton(tr("Choose..."), this);
	connect(choose_words, &QPushButton::clicked, this, &LanguageDialog::browseWords);

	m_dictionary = new QLineEdit(this);
	m_dictionary->setText(settings.value("Dictionary").toString());

	setLanguage(LanguageSettings(settings).language());

	// Creat warning message
	QLabel* warning = new QLabel(tr("<b>Note:</b> These settings will take effect when you start a new game."), this);

	// Create buttons
	m_buttons = new QDialogButtonBox(QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(m_buttons, &QDialogButtonBox::accepted, this, &LanguageDialog::accept);
	connect(m_buttons, &QDialogButtonBox::rejected, this, &LanguageDialog::reject);
	connect(m_buttons, &QDialogButtonBox::clicked, this, &LanguageDialog::clicked);

	// Lay out window
	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(1, 1);

	layout->addWidget(new QLabel(tr("Language:"), this), 0, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_language, 0, 1, 1, 2, Qt::AlignLeft | Qt::AlignVCenter);

	layout->addWidget(new QLabel(tr("Dice:"), this), 1, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_dice, 1, 1);
	layout->addWidget(choose_dice, 1, 2);

	layout->addWidget(new QLabel(tr("Word list:"), this), 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_words, 2, 1);
	layout->addWidget(choose_words, 2, 2);

	layout->addWidget(new QLabel(tr("Dictionary:"), this), 3, 0, Qt::AlignRight | Qt::AlignVCenter);
	layout->addWidget(m_dictionary, 3, 1, 1, 2);

	layout->addWidget(warning, 4, 1, 1, 2);

	layout->setRowStretch(5, 1);
	layout->setRowMinimumHeight(5, 24);

	layout->addWidget(m_buttons, 6, 0, 1, 3);
}

//-----------------------------------------------------------------------------

void LanguageDialog::restoreDefaults()
{
	QSettings settings;
	const LanguageSettings language;

	if (settings.contains("Language")) {
		const int lang = settings.value("Language").toInt();
		if (lang) {
			const QString name = QLocale(QLocale::Language(lang)).name().section('_', 0, 0);
			if (name != language.language()) {
				settings.setValue("Board/Locale", name);
			}
		}
		settings.remove("Language");
	}

	if (settings.contains("CustomDice")) {
		settings.setValue("Board/Dice", settings.value("CustomDice"));
		settings.remove("CustomDice");
		settings.remove("Dice");
	} else if (settings.contains("Dice")) {
		const QString dice = settings.value("Dice").toString();
		if (dice != language.dice()) {
			settings.setValue("Board/Dice", dice);
		}
		settings.remove("Dice");
	}

	if (settings.contains("CustomWords")) {
		settings.setValue("Board/Words", settings.value("CustomWords"));
		settings.remove("CustomWords");
		settings.remove("Words");
	} else if (settings.contains("Words")) {
		const QString words = settings.value("Words").toString();
		if (words != language.words()) {
			settings.setValue("Board/Words", words);
		}
		settings.remove("Words");
	}

	if (settings.contains("CustomDictionary")) {
		settings.setValue("Board/Dictionary", settings.value("CustomDictionary"));
		settings.remove("CustomDictionary");
		settings.remove("Dictionary");
	} else if (settings.contains("Dictionary")) {
		const QString dictionary = settings.value("Dictionary").toString();
		if (dictionary != language.dictionary()) {
			settings.setValue("Board/Dictionary", dictionary);
		}
		settings.remove("Dictionary");
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::accept()
{
	bool changed = false;

	QSettings settings;
	settings.beginGroup("Board");

	const QString language = m_language->itemData(m_language->currentIndex()).toString();
	const QString default_language = LanguageSettings().language();
	if (settings.value("Locale", default_language).toString() != language) {
		if (language != default_language) {
			settings.setValue("Locale", language);
		} else {
			settings.remove("Locale");
		}
		changed = true;
	}

	if (settings.value("Dice").toString() != m_dice_path) {
		if (!m_dice_path.isEmpty()) {
			settings.setValue("Dice", m_dice_path);
		} else {
			settings.remove("Dice");
		}
		changed = true;
	}

	if (settings.value("Words").toString() != m_words_path) {
		if (!m_words_path.isEmpty()) {
			settings.setValue("Words", m_words_path);
		} else {
			settings.remove("Words");
		}
		changed = true;
	}

	const QString dictionary = m_dictionary->text();
	if (settings.value("Dictionary").toString() != dictionary) {
		if (!dictionary.isEmpty()) {
			settings.setValue("Dictionary", dictionary);
		} else {
			settings.remove("Dictionary");
		}
		changed = true;
	}

	if (changed) {
		QDialog::accept();
	} else {
		QDialog::reject();
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::clicked(QAbstractButton* button)
{
	if (m_buttons->buttonRole(button) == QDialogButtonBox::ResetRole) {
		m_dice->clear();
		m_words->clear();
		m_dictionary->clear();
		setLanguage(QLocale::system().name());
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::chooseLanguage(int index)
{
	LanguageSettings settings(m_language->itemData(index).toString());
	m_dice->setPlaceholderText(QDir::toNativeSeparators(QFileInfo(settings.dice()).canonicalFilePath()));
	m_words->setPlaceholderText(QDir::toNativeSeparators(QFileInfo(settings.words()).canonicalFilePath()));
	m_dictionary->setPlaceholderText(settings.dictionary());
}

//-----------------------------------------------------------------------------

void LanguageDialog::browseDice()
{
	QString path = m_dice->text();
	if (path.isEmpty()) {
		path = m_dice->placeholderText();
	}
	path = QFileDialog::getOpenFileName(this, tr("Choose Dice File"), path);
	chooseDice(path);
}

//-----------------------------------------------------------------------------

void LanguageDialog::chooseDice(const QString& path)
{
	if (!path.isEmpty()) {
		m_dice_path = QFileInfo(path).canonicalFilePath();
		m_dice->setText(QDir::toNativeSeparators(m_dice_path));
	} else {
		m_dice_path.clear();
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::browseWords()
{
	QString path = m_words->text();
	if (path.isEmpty()) {
		path = m_words->placeholderText();
	}
	path = QFileDialog::getOpenFileName(this, tr("Choose Word List File"), path);
	chooseWords(path);
}

//-----------------------------------------------------------------------------

void LanguageDialog::chooseWords(const QString& path)
{
	if (!path.isEmpty()) {
		m_words_path = QFileInfo(path).canonicalFilePath();
		m_words->setText(QDir::toNativeSeparators(m_words_path));
	} else {
		m_words_path.clear();
	}
}

//-----------------------------------------------------------------------------

void LanguageDialog::setLanguage(const QString& language)
{
	int index = m_language->findData(language);
	if (index == -1) {
		index = m_language->findData("en");
	}
	m_language->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
