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

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>
#include <QSpinBox>

//-----------------------------------------------------------------------------

NewGameDialog::NewGameDialog(QWidget* parent)
: QDialog(parent) {
	setWindowTitle(tr("New Game"));

	QSettings settings;

	// Create widgets
	m_size = new QComboBox(this);
	m_size->addItem(Board::sizeString(4), 4);
	m_size->addItem(Board::sizeString(5), 5);
	m_size->setCurrentIndex(m_size->findData(qBound(4, settings.value("Board/Size", 4).toInt(), 5)));

	m_timer = new QComboBox(this);
	m_timer->addItem(Clock::modeString(Clock::BoggleMode), Clock::BoggleMode);
	m_timer->addItem(Clock::modeString(Clock::RefillMode), Clock::RefillMode);
	m_timer->addItem(Clock::modeString(Clock::TangletMode), Clock::TangletMode);
	int index = m_timer->findData(settings.value("Board/TimerMode", Clock::TangletMode).toInt());
	if (index == -1) {
		index = m_timer->findData(Clock::TangletMode);
	}
	m_timer->setCurrentIndex(index);

	m_seed = new QSpinBox(this);
	m_seed->setRange(0, INT_MAX);
	m_seed->setSpecialValueText(tr("Random"));

	m_higher_scores = new QCheckBox(tr("Prevent low scoring boards"), this);
	m_higher_scores->setChecked(settings.value("Board/HigherScores", true).toBool());

	// Create buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	// Lay out window
	QFormLayout* layout = new QFormLayout(this);
	layout->addRow(tr("Size:"), m_size);
	layout->addRow(tr("Timer:"), m_timer);
	layout->addRow(tr("Seed:"), m_seed);
	layout->addRow(" ", m_higher_scores);
	layout->addItem(new QSpacerItem(12, 12, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
	layout->addWidget(buttons);
}

//-----------------------------------------------------------------------------

int NewGameDialog::seed() const {
	return m_seed->value();
}

//-----------------------------------------------------------------------------

void NewGameDialog::accept() {
	QSettings settings;
	settings.setValue("Board/Size", m_size->itemData(m_size->currentIndex()).toInt());
	settings.setValue("Board/TimerMode", m_timer->itemData(m_timer->currentIndex()).toInt());
	settings.setValue("Board/HigherScores", m_higher_scores->isChecked());
	QDialog::accept();
}

//-----------------------------------------------------------------------------
