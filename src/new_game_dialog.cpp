/***********************************************************************
 *
 * Copyright (C) 2010, 2011, 2012, 2014, 2015, 2017 Graeme Gott <graeme@gottcode.org>
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

namespace
{
	class TimerDescription
	{
	public:
		TimerDescription(int id)
			: m_name(Clock::timerToString(id)), m_description(Clock::timerDescription(id)), m_id(id)
		{
		}

		QString name() const
		{
			return m_name;
		}

		QString description() const
		{
			return m_description;
		}

		int id() const
		{
			return m_id;
		}

		bool operator<(const TimerDescription& timer) const
		{
			return m_name.localeAwareCompare(timer.m_name) < 0;
		}

	private:
		QString m_name;
		QString m_description;
		int m_id;
	};
}

//-----------------------------------------------------------------------------

NewGameDialog::NewGameDialog(QWidget* parent)
	: QDialog(parent, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint), m_minimum(3)
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
	connect(m_length, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &NewGameDialog::lengthChanged);
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
	QScrollArea* area = new QScrollArea(this);
	layout->addWidget(area);

	QWidget* timers_widget = new QWidget(area);
	QVBoxLayout* timers_layout = new QVBoxLayout(timers_widget);
	area->setWidget(timers_widget);
	area->setWidgetResizable(true);

	QCommandLinkButton* active_timer = 0;
	QList<TimerDescription> timers;
	for (int i = Clock::Tanglet; i < Clock::TotalTimers; ++i) {
		timers.append(i);
	}
	std::sort(timers.begin(), timers.end());
	for (const TimerDescription& timer : timers) {
		QCommandLinkButton* button = new QCommandLinkButton(timer.name(), timer.description(), timers_widget);
		button->setMinimumWidth(500);
		connect(button, &QCommandLinkButton::clicked, [=] { timerChosen(timer.id()); });
		timers_layout->addWidget(button);

		if (timer.id() == previous_timer) {
			button->setDefault(true);
			button->setFocus();
			active_timer = button;
		}
	}

	// Create cancel button
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, &QDialogButtonBox::rejected, this, &NewGameDialog::reject);
	layout->addSpacing(6);
	layout->addWidget(buttons);

	// Show contents
	QSize size = sizeHint() + QSize(area->verticalScrollBar()->sizeHint().width(), area->frameWidth() * 2);
	resize(QSettings().value("NewGameDialog/Size", size).toSize());
	show();
	if (active_timer) {
		area->ensureWidgetVisible(active_timer);
	}
}

//-----------------------------------------------------------------------------

QString NewGameDialog::densityString(int density) {
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
