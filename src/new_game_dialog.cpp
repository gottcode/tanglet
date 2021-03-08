/*
	SPDX-FileCopyrightText: 2010-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "new_game_dialog.h"

#include "board.h"
#include "clock.h"

#include <QComboBox>
#include <QCommandLinkButton>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <QSettings>
#include <QToolButton>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

NewGameDialog::NewGameDialog(QWidget* parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
	, m_minimum(3)
{
	setWindowTitle(tr("New Game"));

	QVBoxLayout* layout = new QVBoxLayout(this);

	QSettings settings;
	int previous_timer = settings.value("Board/TimerMode", Clock::Tanglet).toInt();

	// Create size buttons
	m_normal_size = new QToolButton(this);
	m_normal_size->setAutoExclusive(true);
	m_normal_size->setAutoRaise(true);
	m_normal_size->setCheckable(true);
	m_normal_size->setIconSize(QSize(64, 64));
	m_normal_size->setIcon(QIcon(":/preview/normal.png"));
	m_normal_size->setText(Board::sizeToString(4));
	m_normal_size->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(m_normal_size, &QToolButton::clicked, this, &NewGameDialog::sizeChanged);

	m_large_size = new QToolButton(this);
	m_large_size->setAutoExclusive(true);
	m_large_size->setAutoRaise(true);
	m_large_size->setCheckable(true);
	m_large_size->setIconSize(QSize(64, 64));
	m_large_size->setIcon(QIcon(":/preview/large.png"));
	m_large_size->setText(Board::sizeToString(5));
	m_large_size->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	connect(m_large_size, &QToolButton::clicked, this, &NewGameDialog::sizeChanged);

	if (settings.value("Board/Size", 4).toInt() == 4) {
		m_normal_size->setChecked(true);
	} else {
		m_large_size->setChecked(true);
	}

	QHBoxLayout* size_buttons = new QHBoxLayout;
	size_buttons->setContentsMargins(0, 0, 0, 0);
	size_buttons->addStretch();
	size_buttons->addWidget(m_normal_size);
	size_buttons->addWidget(m_large_size);
	size_buttons->addStretch();
	layout->addLayout(size_buttons);
	layout->addSpacing(6);

	// Create word options
	m_density = new QComboBox(this);
	for (int i = 0; i < 4; ++i) {
		m_density->addItem(densityString(i));
	}
	m_density->setCurrentIndex(settings.value("Board/Density", 1).toInt());

	m_length = new QComboBox(this);
	for (int i = 0; i < 4; ++i) {
		m_length->addItem("");
	}
	connect(m_length, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewGameDialog::lengthChanged);
	m_minimum = settings.value("Board/Minimum", 3).toInt();
	if (m_large_size->isChecked()) {
		--m_minimum;
	}
	m_length->setCurrentIndex(qBound(0, m_minimum - 3, 4));
	sizeChanged();

	QFormLayout* options_layout = new QFormLayout;
	options_layout->setFormAlignment(Qt::AlignHCenter | Qt::AlignTop);
	options_layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
	options_layout->addRow(tr("Amount of Words:"), m_density);
	options_layout->addRow(tr("Minimum Word Length:"), m_length);
	layout->addLayout(options_layout);
	layout->addSpacing(6);

	// Create timer buttons
	m_timers_area = new QScrollArea(this);
	layout->addWidget(m_timers_area);

	QWidget* timers_widget = new QWidget(m_timers_area);
	QVBoxLayout* timers_layout = new QVBoxLayout(timers_widget);
	m_timers_area->setWidget(timers_widget);
	m_timers_area->setWidgetResizable(true);
	m_timers_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QCommandLinkButton* active_timer = nullptr;
	for (int timer = 0; timer < Clock::TotalTimers; ++timer) {
		const QString name = Clock::timerToString(timer);

		QCommandLinkButton* button = new QCommandLinkButton(name, Clock::timerDescription(timer), timers_widget);
		button->setMinimumWidth(500);
		connect(button, &QCommandLinkButton::clicked, this, [this, timer] { timerChosen(timer); });

		int i;
		for (i = 0; i < m_timers.size(); ++i) {
			if (m_timers[i]->text().localeAwareCompare(name) >= 0) {
				break;
			}
		}
		m_timers.insert(i, button);
		timers_layout->insertWidget(i, button);

		if (timer == previous_timer) {
			button->setDefault(true);
			button->setFocus();
			active_timer = button;
		}
	}

	// Create cancel button
	m_buttons = new QDialogButtonBox(QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults, Qt::Horizontal, this);
	connect(m_buttons, &QDialogButtonBox::rejected, this, &NewGameDialog::reject);
	connect(m_buttons, &QDialogButtonBox::clicked, this, &NewGameDialog::restoreDefaults);
	layout->addSpacing(6);
	layout->addWidget(m_buttons);

	// Show contents
	m_timers_area->setMinimumWidth(timers_widget->sizeHint().width()
			+ m_timers_area->verticalScrollBar()->sizeHint().width()
			+ (m_timers_area->frameWidth() * 2));
	QSize size = sizeHint();
	size.setHeight(size.height() - (m_timers_area->sizeHint().height() / 3));
	resize(QSettings().value("NewGameDialog/Size", size).toSize());
	show();
	if (active_timer) {
		m_timers_area->ensureWidgetVisible(active_timer);
	}
}

//-----------------------------------------------------------------------------

QString NewGameDialog::densityString(int density)
{
	static QStringList densities = QStringList()
			<< tr("Low")
			<< tr("Medium")
			<< tr("High")
			<< tr("Random");
	return densities.at(qBound(0, density, densities.count() - 1));
}

//-----------------------------------------------------------------------------

void NewGameDialog::lengthChanged(int length)
{
	m_minimum = length + 3;
}

//-----------------------------------------------------------------------------

void NewGameDialog::sizeChanged()
{
	int minimum = (m_normal_size->isChecked()) ? 3 : 4;
	for (int i = 0; i < 4; ++i) {
		m_length->setItemText(i, tr("%n letter(s)", "", i + minimum));
	}
}

//-----------------------------------------------------------------------------

void NewGameDialog::timerChosen(int timer)
{
	QSettings settings;
	if (m_normal_size->isChecked()) {
		settings.setValue("Board/Size", 4);
		settings.setValue("Board/Minimum", m_minimum);
	} else {
		settings.setValue("Board/Size", 5);
		settings.setValue("Board/Minimum", m_minimum + 1);
	}
	settings.setValue("Board/Density", m_density->currentIndex());
	settings.setValue("Board/TimerMode", timer);
	settings.setValue("NewGameDialog/Size", size());
	QDialog::accept();
}

//-----------------------------------------------------------------------------

void NewGameDialog::restoreDefaults(QAbstractButton* button)
{
	if (m_buttons->buttonRole(button) != QDialogButtonBox::ResetRole) {
		return;
	}

	m_normal_size->setChecked(true);
	m_density->setCurrentIndex(1);
	sizeChanged();
	m_length->setCurrentIndex(0);

	const QString name = Clock::timerToString(Clock::Tanglet);
	QCommandLinkButton* timer = nullptr;
	for (QCommandLinkButton* b : qAsConst(m_timers)) {
		if (b->text() == name) {
			timer = b;
			break;
		}
	}
	if (timer) {
		timer->setDefault(true);
		timer->setFocus();
		m_timers_area->ensureWidgetVisible(timer);
	}
}

//-----------------------------------------------------------------------------
