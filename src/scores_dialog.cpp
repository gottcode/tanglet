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

#include "scores_dialog.h"

#include "clock.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QVBoxLayout>

#if defined(Q_OS_UNIX)
  #include <pwd.h>
  #include <unistd.h>
#elif defined(Q_OS_WIN32)
  #include <lmcons.h>
  #include <windows.h>
#endif

//-----------------------------------------------------------------------------

int ScoresDialog::m_max = -1;
int ScoresDialog::m_min = 1;

//-----------------------------------------------------------------------------

ScoresDialog::ScoresDialog(QWidget* parent)
: QDialog(parent), m_row(-1) {
	setWindowTitle(tr("High Scores"));

	// Load default name
	m_default_name = QSettings().value("Scores/DefaultName").toString();
	if (m_default_name.isEmpty()) {
#if defined(Q_OS_UNIX)
		passwd* pws = getpwuid(geteuid());
		if (pws) {
			m_default_name = pws->pw_gecos;
			if (m_default_name.isEmpty()) {
				m_default_name = pws->pw_name;
			}
		}
#elif defined(Q_OS_WIN32)
		WCHAR buffer[UNLEN + 1];
		DWORD count = sizeof(buffer);
		if (GetUserName(buffer, &count)) {
			m_default_name = QString::fromStdWString(buffer);
		}
#endif
	}

	// Create score widgets
	m_scores_layout = new QGridLayout;
	m_scores_layout->setSpacing(12);
	m_scores_layout->setColumnStretch(1, 1);
	m_scores_layout->addWidget(new QLabel(tr("<b>Name</b>"), this), 0, 1, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel(tr("<b>Score</b>"), this), 0, 2, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel(tr("<b>Date</b>"), this), 0, 3, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel(tr("<b>Timer</b>"), this), 0, 4, Qt::AlignCenter);

	Qt::Alignment alignments[4] = { Qt::AlignLeft, Qt::AlignRight, Qt::AlignRight, Qt::AlignHCenter };
	for (int r = 0; r < 10; ++r) {
		m_score_labels[r][0] = new QLabel(tr("#%1").arg(r + 1), this);
		m_scores_layout->addWidget(m_score_labels[r][0], r + 1, 0, Qt::AlignRight | Qt::AlignVCenter);
		for (int c = 1; c < 5; ++c) {
			m_score_labels[r][c] = new QLabel("-", this);
			m_scores_layout->addWidget(m_score_labels[r][c], r + 1, c, alignments[c - 1] | Qt::AlignVCenter);
		}
	}

	load();

	m_username = new QLineEdit(this);
	m_username->hide();
	connect(m_username, SIGNAL(editingFinished()), this, SLOT(editingFinished()));

	// Lay out dialog
	m_buttons = new QDialogButtonBox(QDialogButtonBox::Reset | QDialogButtonBox::Close, Qt::Horizontal, this);
	m_buttons->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons));
	m_buttons->button(QDialogButtonBox::Close)->setFocus();
	connect(m_buttons, SIGNAL(rejected()), this, SLOT(reject()));
	connect(m_buttons, SIGNAL(clicked(QAbstractButton*)), this, SLOT(resetClicked(QAbstractButton*)));

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addLayout(m_scores_layout);
	layout->addWidget(m_buttons);
}

//-----------------------------------------------------------------------------

bool ScoresDialog::addScore(int score) {
	// Add score
	m_row = addScore(m_default_name, score, QDateTime::currentDateTime(), QSettings().value("Current/TimerMode", Clock::Tanglet).toInt());
	if (m_row == -1) {
		return false;
	}
	for (int c = 0; c < 5; ++c) {
		QFont f = m_score_labels[m_row][c]->font();
		f.setWeight(QFont::Bold);
		m_score_labels[m_row][c]->setFont(f);
	}
	updateItems();

	// Show lineedit
	m_scores_layout->addWidget(m_username, m_row + 1, 1);
	m_score_labels[m_row][1]->hide();
	m_username->setText(m_default_name);
	m_username->show();
	m_username->setFocus();

	return true;
}

//-----------------------------------------------------------------------------

int ScoresDialog::isHighScore(int score) {
	if (m_max == -1) {
		m_max = 1;
		ScoresDialog();
	}

	if (score >= m_max) {
		return 2;
	} else if (score >= m_min) {
		return 1;
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------

void ScoresDialog::hideEvent(QHideEvent* event) {
	if (m_username->isVisible()) {
		editingFinished();
	}
	QDialog::hideEvent(event);
}

//-----------------------------------------------------------------------------

void ScoresDialog::editingFinished() {
	// Hide lineedit
	m_username->hide();
	m_scores_layout->removeWidget(m_username);
	m_scores[m_row].name = m_username->text();
	m_score_labels[m_row][1]->show();
	updateItems();

	// Save scores
	QStringList values;
	foreach (const Score& s, m_scores) {
		values += QString("%1:%2:%3:%4") .arg(s.name) .arg(s.score) .arg(s.date.toString("yyyy.MM.dd-hh.mm.ss")) .arg(s.timer);
	}
	QSettings settings;
	settings.setValue("Scores/DefaultName", m_username->text());
	settings.setValue("Scores/Values", values);
}

//-----------------------------------------------------------------------------

void ScoresDialog::resetClicked(QAbstractButton* button) {
	if (m_buttons->buttonRole(button) == QDialogButtonBox::ResetRole) {
		if (QMessageBox::question(this, tr("Question"), tr("Clear high scores?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
			if (m_username->isVisible()) {
				editingFinished();
			}
			m_scores.clear();
			if (m_row > -1) {
				for (int c = 0; c < 5; ++c) {
					QFont f = m_score_labels[m_row][c]->font();
					f.setWeight(QFont::Normal);
					m_score_labels[m_row][c]->setFont(f);
				}
			}
			updateItems();
			QSettings().setValue("Scores/Values", QStringList());
		}
	}
}

//-----------------------------------------------------------------------------

int ScoresDialog::addScore(const QString& name, int score, const QDateTime& date, int timer) {
	if (score == 0) {
		return -1;
	}

	int row = 0;
	foreach (const Score& s, m_scores) {
		if (score >= s.score && date >= s.date) {
			break;
		}
		row++;
	}
	if (row == 10) {
		return -1;
	}

	Score s = { name, score, date, timer };
	m_scores.insert(row, s);
	if (m_scores.count() == 11) {
		m_scores.removeLast();
	}

	m_max = m_scores.first().score;
	m_min = (m_scores.count() == 10) ? m_scores.last().score : 1;

	return row;
}

//-----------------------------------------------------------------------------

void ScoresDialog::load() {
	QStringList data = QSettings().value("Scores/Values").toStringList();
	foreach (const QString& s, data) {
		QStringList values = s.split(':');
		if (values.size() == 3 || values.size() == 4) {
			QString name = values[0];
			int score = values[1].toInt();
			QDateTime date = QDateTime::fromString(values[2], "yyyy.MM.dd-hh.mm.ss");
			int timer = values.value(3, QString::number(Clock::Tanglet)).toInt();
			addScore(name, score, date, timer);
		}
	}
	updateItems();
}

//-----------------------------------------------------------------------------

void ScoresDialog::updateItems() {
	int count = m_scores.count();
	for (int r = 0; r < count; ++r) {
		const Score& score = m_scores.at(r);
		m_score_labels[r][1]->setText(score.name);
		m_score_labels[r][2]->setNum(score.score);
		m_score_labels[r][3]->setText(score.date.toString(Qt::DefaultLocaleShortDate));
		m_score_labels[r][4]->setText(Clock::timerToString(score.timer));
	}
	for (int r = count; r < 10; ++r) {
		for (int c = 1; c < 5; ++c) {
			m_score_labels[r][c]->setText("-");
		}
	}
}

//-----------------------------------------------------------------------------
