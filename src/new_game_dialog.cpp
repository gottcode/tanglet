/***********************************************************************
 *
 * Copyright (C) 2010 Graeme Gott <graeme@gottcode.org>
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

#include <QCommandLinkButton>
#include <QDialogButtonBox>
#include <QSettings>
#include <QSignalMapper>
#include <QToolButton>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

namespace {
	class TimerDescription {
	public:
		TimerDescription(int id)
			: m_name(Clock::timerToString(id)), m_description(Clock::timerDescription(id)), m_id(id) {
		}

		QString name() const {
			return m_name;
		}

		QString description() const {
			return m_description;
		}

		int id() const {
			return m_id;
		}

		bool operator<(const TimerDescription& timer) const {
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
: QDialog(parent, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint) {
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
	m_normal_size->setIcon(QPixmap(":/preview/normal.png"));
	m_normal_size->setText(Board::sizeToString(4));
	m_normal_size->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	m_large_size = new QToolButton(this);
	m_large_size->setAutoExclusive(true);
	m_large_size->setAutoRaise(true);
	m_large_size->setCheckable(true);
	m_large_size->setIconSize(QSize(64, 64));
	m_large_size->setIcon(QPixmap(":/preview/large.png"));
	m_large_size->setText(Board::sizeToString(5));
	m_large_size->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	if (settings.value("Board/Size", 4).toInt() == 4) {
		m_normal_size->setChecked(true);
	} else {
		m_large_size->setChecked(true);
	}

	QHBoxLayout* size_buttons = new QHBoxLayout;
	size_buttons->setMargin(0);
	size_buttons->addStretch();
	size_buttons->addWidget(m_normal_size);
	size_buttons->addWidget(m_large_size);
	size_buttons->addStretch();
	layout->addLayout(size_buttons);
	layout->addSpacing(6);

	// Create timer buttons
	QSignalMapper* mapper = new QSignalMapper(this);
	connect(mapper, SIGNAL(mapped(int)), this, SLOT(timerChosen(int)));

	QList<TimerDescription> timers;
	for (int i = Clock::Tanglet; i < Clock::TotalTimers; ++i) {
		timers.append(i);
	}
	qSort(timers);
	foreach (const TimerDescription& timer, timers) {
		QCommandLinkButton* button = new QCommandLinkButton(timer.name(), timer.description(), this);
		button->setMinimumWidth(500);
		connect(button, SIGNAL(clicked()), mapper, SLOT(map()));
		mapper->setMapping(button, timer.id());
		layout->addWidget(button);

		if (timer.id() == previous_timer) {
			button->setDefault(true);
			button->setFocus();
		}
	}

	// Create cancel button
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));
	layout->addSpacing(6);
	layout->addWidget(buttons);

	setFixedSize(sizeHint());
}

//-----------------------------------------------------------------------------

void NewGameDialog::timerChosen(int timer) {
	QSettings settings;
	settings.setValue("Board/Size", m_normal_size->isChecked() ? 4 : 5);
	settings.setValue("Board/TimerMode", timer);
	QDialog::accept();
}

//-----------------------------------------------------------------------------
