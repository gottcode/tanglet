/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "scores_dialog.h"

#include "board.h"
#include "clock.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStyle>
#include <QTabWidget>
#include <QVBoxLayout>

#if defined(Q_OS_UNIX)
#include <pwd.h>
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <lmcons.h>
#include <windows.h>
#endif

//-----------------------------------------------------------------------------

QVector<int> ScoresDialog::m_max(Clock::TotalTimers, -1);
QVector<int> ScoresDialog::m_min(Clock::TotalTimers, -1);

//-----------------------------------------------------------------------------

ScoresDialog::Page::Page(int timer, QSettings& settings, QWidget* parent)
	: QWidget(parent)
	, m_timer(timer)
	, m_row(-1)
{
	setWindowTitle(tr("High Scores"));

	// Create score widgets
	m_scores_layout = new QGridLayout(this);
	m_scores_layout->setHorizontalSpacing(18);
	m_scores_layout->setVerticalSpacing(6);
	m_scores_layout->setColumnStretch(1, 1);
	m_scores_layout->addWidget(new QLabel("<b>" + tr("Rank") + "</b>", this), 1, RankColumn, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel("<b>" + tr("Name") + "</b>", this), 1, NameColumn, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel("<b>" + tr("Score") + "</b>", this), 1, ScoreColumn, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel("<b>" + tr("Maximum") + "</b>", this), 1, MaxScoreColumn, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel("<b>" + tr("Date") + "</b>", this), 1, DateColumn, Qt::AlignCenter);
	m_scores_layout->addWidget(new QLabel("<b>" + tr("Size") + "</b>", this), 1, SizeColumn, Qt::AlignCenter);

	QFrame* divider = new QFrame(this);
	divider->setFrameStyle(QFrame::HLine | QFrame::Sunken);
	m_scores_layout->addWidget(divider, 2, 0, 1, TotalColumns);

	QVector<Qt::Alignment> alignments(TotalColumns, Qt::AlignTrailing);
	alignments[NameColumn] = Qt::AlignLeading;
	alignments[SizeColumn] = Qt::AlignHCenter;
	for (int r = 0; r < 10; ++r) {
		m_score_labels[r][0] = new QLabel(tr("#%1").arg(r + 1), this);
		m_scores_layout->addWidget(m_score_labels[r][0], r + 3, 0, alignments[RankColumn] | Qt::AlignVCenter);
		for (int c = RankColumn + 1; c < TotalColumns; ++c) {
			m_score_labels[r][c] = new QLabel("-", this);
			m_scores_layout->addWidget(m_score_labels[r][c], r + 3, c, alignments[c] | Qt::AlignVCenter);
		}
	}

	// Populate scores widgets with values
	load(settings);

	// Hide maximum scores column if showing maximum scores is set to "Never"
	if (settings.value("ShowMaximumScore").toInt() == 0) {
		m_scores_layout->itemAtPosition(0, MaxScoreColumn)->widget()->hide();
		for (int r = 0; r < 10; ++r) {
			m_score_labels[r][MaxScoreColumn]->hide();
		}
	}
}

//-----------------------------------------------------------------------------

QString ScoresDialog::Page::name() const
{
	return Clock::timerToString(m_timer);
}

//-----------------------------------------------------------------------------

bool ScoresDialog::Page::addScore(const QString& name, int score, int max_score, const QDateTime& date, int size)
{
	m_row = -1;
	if (score == 0) {
		return false;
	}

	m_row = 0;
	for (const Score& s : qAsConst(m_scores)) {
		if (score >= s.score && date >= s.date) {
			break;
		}
		++m_row;
	}
	if (m_row == 10) {
		m_row = -1;
		return false;
	}

	Score s = { name, score, max_score, date, size };
	m_scores.insert(m_row, s);
	if (m_scores.size() == 11) {
		m_scores.removeLast();
	}

	m_max[m_timer] = m_scores.first().score;
	m_min[m_timer] = (m_scores.size() == 10) ? m_scores.last().score : 1;

	return true;
}

//-----------------------------------------------------------------------------

void ScoresDialog::Page::editStart(QLineEdit* playername)
{
	Q_ASSERT(m_row != -1);

	// Inform player of success
	QLabel* label = new QLabel(this);
	label->setAlignment(Qt::AlignCenter);
	if (m_row == 0) {
		label->setText(QString("<big>🎉</big> %1<br>%2").arg(tr("Congratulations!"), tr("You beat your top score!")));
	} else {
		label->setText(QString("<big>🙌</big> %1<br>%2").arg(tr("Well done!"), tr("You have a new high score!")));
	}
	m_scores_layout->addWidget(label, 0, 0, 1, TotalColumns);

	// Add score to display
	updateItems();

	// Show lineedit
	m_scores_layout->addWidget(playername, m_row + 3, 1);
	m_score_labels[m_row][1]->hide();
	playername->setText(m_scores[m_row].name);
	playername->show();
	playername->setFocus();
}

//-----------------------------------------------------------------------------

void ScoresDialog::Page::editFinish(QLineEdit* playername)
{
	Q_ASSERT(m_row != -1);

	// Set player name
	m_scores[m_row].name = playername->text();
	m_score_labels[m_row][1]->setText("<b>" + m_scores[m_row].name + "</b>");

	// Hide lineedit
	playername->hide();
	m_scores_layout->removeWidget(playername);
	m_score_labels[m_row][1]->show();

	// Save scores
	QSettings settings;
	settings.setValue("Scores/DefaultName", playername->text());
	settings.beginWriteArray(Clock::timerScoresGroup(m_timer));
	for (int r = 0, size = m_scores.size(); r < size; ++r) {
		const Score& score = m_scores[r];
		settings.setArrayIndex(r);
		settings.setValue("Name", score.name);
		settings.setValue("Score", score.score);
		settings.setValue("Maximum", score.max_score);
		settings.setValue("Size", score.size);
		settings.setValue("Date", score.date.toString(Qt::ISODate));
	}
	settings.endArray();
}

//-----------------------------------------------------------------------------

void ScoresDialog::Page::load(QSettings& settings)
{
	const int size = std::min(settings.beginReadArray(Clock::timerScoresGroup(m_timer)), 10);
	for (int r = 0; r < size; ++r) {
		settings.setArrayIndex(r);
		const QString name = settings.value("Name").toString();
		const int score = settings.value("Score").toInt();
		const int max_score = settings.value("Maximum", -1).toInt();
		const int size = settings.value("Size", -1).toInt();
		const QDateTime date = settings.value("Date").toDateTime();
		addScore(name, score, max_score, date, size);
	}
	settings.endArray();

	m_row = -1;
	updateItems();
}

//-----------------------------------------------------------------------------

void ScoresDialog::Page::updateItems()
{
	const int size = m_scores.size();

	// Add scores
	for (int r = 0; r < size; ++r) {
		const Score& score = m_scores[r];
		m_score_labels[r][NameColumn]->setText(score.name);
		m_score_labels[r][ScoreColumn]->setNum(score.score);
		if (score.max_score > -1) {
			m_score_labels[r][MaxScoreColumn]->setNum(score.max_score);
		} else {
			m_score_labels[r][MaxScoreColumn]->setText(tr("N/A"));
		}
		m_score_labels[r][DateColumn]->setText(QLocale().toString(score.date, QLocale::ShortFormat));
		if (score.size > -1) {
			m_score_labels[r][SizeColumn]->setText(Board::sizeToString(score.size));
		} else {
			m_score_labels[r][SizeColumn]->setText(tr("N/A"));
		}
	}

	// Fill remainder of scores with dashes
	for (int r = size; r < 10; ++r) {
		for (int c = RankColumn + 1; c < TotalColumns; ++c) {
			m_score_labels[r][c]->setText("-");
		}
	}

	// Use bold text for new score
	if (m_row != -1) {
		for (int c = 0; c < TotalColumns; ++c) {
			m_score_labels[m_row][c]->setText("<b>" + m_score_labels[m_row][c]->text() + "</b>");
		}
	}
}

//-----------------------------------------------------------------------------

ScoresDialog::ScoresDialog(QWidget* parent)
	: QDialog(parent)
	, m_active_page(nullptr)
{
	setWindowTitle(tr("High Scores"));

	QSettings settings;

	// Load default name
	m_default_name = settings.value("Scores/DefaultName").toString();
	if (m_default_name.isEmpty()) {
#if defined(Q_OS_UNIX)
		passwd* pws = getpwuid(geteuid());
		if (pws) {
			m_default_name = QString::fromLocal8Bit(pws->pw_gecos).section(',', 0, 0);
			if (m_default_name.isEmpty()) {
				m_default_name = QString::fromLocal8Bit(pws->pw_name);
			}
		}
#elif defined(Q_OS_WIN)
		TCHAR buffer[UNLEN + 1];
		DWORD count = UNLEN + 1;
		if (GetUserName(buffer, &count)) {
			m_default_name = QString::fromWCharArray(buffer, count);
		}
#endif
	}

	m_username = new QLineEdit(this);
	m_username->hide();
	connect(m_username, &QLineEdit::editingFinished, this, &ScoresDialog::editingFinished);

	m_tabs = new QTabWidget(this);
	m_tabs->setTabPosition(QTabWidget::West);

	for (int timer = 0; timer < Clock::TotalTimers; ++timer) {
		Page* page = new Page(timer, settings, this);
		m_pages.append(page);
		if (!page->isEmpty()) {
			addTab(page);
		}
	}
	if (m_tabs->count() == 0) {
		addTab(m_pages[Clock::Tanglet]);
	}

	// Show scores for current timer mode
	const int index = m_tabs->indexOf(m_pages[settings.value("Board/TimerMode").toInt()]);
	if (index != -1) {
		m_tabs->setCurrentIndex(index);
	}

	// Lay out dialog
	m_buttons = new QDialogButtonBox(QDialogButtonBox::Close, Qt::Horizontal, this);
	m_buttons->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons));
	m_buttons->button(QDialogButtonBox::Close)->setDefault(true);
	m_buttons->button(QDialogButtonBox::Close)->setFocus();
	connect(m_buttons, &QDialogButtonBox::rejected, this, &ScoresDialog::reject);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(m_tabs);
	layout->addWidget(m_buttons);
}

//-----------------------------------------------------------------------------

bool ScoresDialog::addScore(int score, int max_score)
{
	QSettings settings;

	// Fetch scores page
	const int timer = settings.value("Current/TimerMode", Clock::Tanglet).toInt();
	m_active_page = m_pages[timer];

	// Add score
	if (!m_active_page->addScore(m_default_name,
			score,
			max_score,
			QDateTime::currentDateTime(),
			settings.value("Current/Size", 4).toInt())) {
		return false;
	}

	// Show tab
	if (m_tabs->indexOf(m_active_page) == -1) {
		addTab(m_active_page);
	}
	m_tabs->setCurrentWidget(m_active_page);

	Page* tanglet_page = m_pages[Clock::Tanglet];
	const int index = m_tabs->indexOf(tanglet_page);
	if ((index != -1) && (m_active_page != tanglet_page) && (tanglet_page->isEmpty())) {
		m_tabs->removeTab(index);
	}

	// Show lineedit
	m_active_page->editStart(m_username);

	m_buttons->button(QDialogButtonBox::Close)->setDefault(false);

	return true;
}

//-----------------------------------------------------------------------------

int ScoresDialog::isHighScore(int score, int timer)
{
	if (m_max[timer] == -1) {
		m_max[timer] = 1;
		ScoresDialog();
	}

	if (score >= m_max[timer]) {
		return 2;
	} else if (score >= m_min[timer]) {
		return 1;
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------

void ScoresDialog::migrate()
{
	QSettings settings;
	if (!settings.contains("Scores/Values")) {
		return;
	}

	const QStringList data = settings.value("Scores/Values").toStringList();
	settings.remove("Scores/Values");

	QVector<int> indexes(Clock::TotalTimers, 0);

	for (const QString& s : data) {
		const QStringList values = s.split(':');
		if (values.size() < 3 || values.size() > 6) {
			continue;
		}

		const QString name = values[0];
		const int score = values[1].toInt();
		const int max_score = values.value(4, "-1").toInt();
		const QDateTime date = QDateTime::fromString(values[2], "yyyy.MM.dd-hh.mm.ss");
		const int timer = values.value(3, QString::number(Clock::Tanglet)).toInt();
		const int size = values.value(5, "-1").toInt();

		int& index = indexes[timer];
		settings.beginWriteArray(Clock::timerScoresGroup(timer));
		settings.setArrayIndex(index);
		settings.setValue("Name", name);
		settings.setValue("Score", score);
		settings.setValue("Maximum", max_score);
		settings.setValue("Size", size);
		settings.setValue("Date", date.toString(Qt::ISODate));
		settings.endArray();
		++index;
	}
}

//-----------------------------------------------------------------------------

void ScoresDialog::hideEvent(QHideEvent* event)
{
	editingFinished();
	QDialog::hideEvent(event);
}

//-----------------------------------------------------------------------------

void ScoresDialog::keyPressEvent(QKeyEvent* event)
{
	if ((event->key() == Qt::Key_Enter) || (event->key() == Qt::Key_Return)) {
		event->ignore();
		return;
	}
	QDialog::keyPressEvent(event);
}

//-----------------------------------------------------------------------------

void ScoresDialog::editingFinished()
{
	if (m_active_page) {
		m_active_page->editFinish(m_username);
		m_active_page = nullptr;

		m_buttons->button(QDialogButtonBox::Close)->setDefault(true);
		m_buttons->button(QDialogButtonBox::Close)->setFocus();
	}
}

//-----------------------------------------------------------------------------

void ScoresDialog::addTab(Page* page)
{
	const QString name = page->name();
	int index = 0;
	for (index = 0; index < m_tabs->count(); ++index) {
		if (m_tabs->tabText(index).localeAwareCompare(name) >= 0) {
			break;
		}
	}
	m_tabs->insertTab(index, page, name);
}

//-----------------------------------------------------------------------------
