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

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

NewGameDialog::NewGameDialog(QWidget* parent)
: QDialog(parent) {
	setWindowTitle(tr("New Game"));

	QSettings settings;

	// Create board settings
	QGroupBox* board = new QGroupBox(tr("Board"), this);

	m_seed = new QSpinBox(board);
	m_seed->setRange(0, INT_MAX);
	m_seed->setSpecialValueText(tr("Random"));

	m_higher_scores = new QCheckBox(tr("Prevent low scoring boards"), board);
	m_higher_scores->setChecked(settings.value("Board/HigherScores", true).toBool());

	// Create buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	// Lay out window
	QFormLayout* board_layout = new QFormLayout(board);
	board_layout->addRow(tr("Seed:"), m_seed);
	board_layout->addRow(" ", m_higher_scores);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(board);
	layout->addSpacerItem(new QSpacerItem(12, 12, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
	layout->addWidget(buttons);
}

//-----------------------------------------------------------------------------

int NewGameDialog::seed() const {
	return m_seed->value();
}

//-----------------------------------------------------------------------------

void NewGameDialog::accept() {
	QSettings settings;
	settings.setValue("Board/HigherScores", m_higher_scores->isChecked());

	QDialog::accept();
}

//-----------------------------------------------------------------------------
