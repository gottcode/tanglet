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

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

//-----------------------------------------------------------------------------

NewGameDialog::NewGameDialog(QWidget* parent)
: QDialog(parent) {
	setWindowTitle(tr("New Game"));

	// Create seed
	m_seed = new QSpinBox(this);
	m_seed->setRange(0, INT_MAX);
	m_seed->setSpecialValueText(tr("Random"));

	// Create buttons
	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttons, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), this, SLOT(reject()));

	// Lay out window
	QFormLayout* layout = new QFormLayout(this);
	layout->addRow(tr("Seed:"), m_seed);
	layout->addRow(buttons);
}

//-----------------------------------------------------------------------------

int NewGameDialog::seed() const {
	return m_seed->value();
}

//-----------------------------------------------------------------------------
