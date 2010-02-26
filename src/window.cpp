/***********************************************************************
 *
 * Copyright (C) 2009, 2010 Graeme Gott <graeme@gottcode.org>
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

#include "window.h"

#include "board.h"
#include "clock.h"
#include "new_game_dialog.h"
#include "scores_dialog.h"
#include "settings.h"
#include "settings_dialog.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

//-----------------------------------------------------------------------------

Window::Window()
: m_pause_action(0) {
	setWindowTitle(tr("Tanglet"));
	setWindowIcon(QIcon(":/tanglet.png"));

	// Create menus
	QMenu* menu = menuBar()->addMenu(tr("&Game"));
	menu->addAction(tr("&New"), this, SLOT(newGame()), tr("Ctrl+N"));
	m_pause_action = menu->addAction(tr("&Pause"));
	menu->addSeparator();
	menu->addAction(tr("&Details"), this, SLOT(showDetails()));
	menu->addAction(tr("&High Scores"), this, SLOT(showScores()));
	menu->addAction(tr("&Settings"), this, SLOT(showSettings()));
	menu->addSeparator();
	QAction* abort_action = menu->addAction(tr("&Abort"), this, SLOT(abortGame()));
	menu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));

	menu = menuBar()->addMenu(tr("&Help"));
	menu->addAction(tr("&Controls"), this, SLOT(showControls()));
	menu->addSeparator();
	menu->addAction(tr("&About"), this, SLOT(about()));
	menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
	menu->addAction(tr("About &SCOWL"), this, SLOT(aboutScowl()));

	m_pause_action->setCheckable(true);
	m_pause_action->setShortcut(tr("Ctrl+P"));
	connect(m_pause_action, SIGNAL(toggled(bool)), this, SLOT(setPaused(bool)));

	// Create widgets
	m_contents = new QStackedWidget(this);
	setCentralWidget(m_contents);

	m_board = new Board(this);
	m_contents->addWidget(m_board);
	connect(m_board, SIGNAL(finished(int)), this, SLOT(gameFinished(int)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_pause_action, SLOT(setEnabled(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), abort_action, SLOT(setEnabled(bool)));

	// Create pause screen
	QWidget* pause_screen = new QWidget(this);
	m_contents->addWidget(pause_screen);

	QLabel* pause_label = new QLabel(tr("<b><big>Paused</big></b>"), pause_screen);
	QPushButton* resume_button = new QPushButton(tr("Resume"), pause_screen);
	resume_button->setDefault(true);
	connect(resume_button, SIGNAL(clicked()), m_pause_action, SLOT(toggle()));

	QVBoxLayout* pause_layout = new QVBoxLayout(pause_screen);
	pause_layout->addStretch();
	pause_layout->addWidget(pause_label, 0, Qt::AlignCenter);
	pause_layout->addWidget(resume_button, 0, Qt::AlignCenter);
	pause_layout->addStretch();

	// Load settings
	restoreGeometry(QSettings().value("Geometry").toByteArray());
	m_board->loadSettings(Settings());
	m_board->generate();
}

//-----------------------------------------------------------------------------

void Window::closeEvent(QCloseEvent* event) {
	QSettings().setValue("Geometry", saveGeometry());
	if (!abortGame()) {
		event->ignore();
	}
}

//-----------------------------------------------------------------------------

bool Window::event(QEvent* event) {
	if ((event->type() == QEvent::WindowBlocked || event->type() == QEvent::WindowDeactivate) && m_pause_action && m_pause_action->isEnabled()) {
		m_pause_action->setChecked(true);
	}
	return QMainWindow::event(event);
}

//-----------------------------------------------------------------------------

void Window::about() {
	QMessageBox::about(this, tr("About"),
		QString("<center><p><big><b>%1</b></big><br/>%2<br/><small>%3<br/>%4</small></p><p>%5<br/><small>%6</small></p></center>")
		.arg(tr("Tanglet %1").arg(QCoreApplication::applicationVersion()))
		.arg(tr("A single player variant of <a href=\"http://en.wikipedia.org/wiki/Boggle\">Boggle</a>"))
		.arg(tr("Copyright &copy; 2009, 2010 Graeme Gott"))
		.arg(tr("Released under the <a href=\"http://www.gnu.org/licenses/gpl.html\">GPL 3</a> license"))
		.arg(tr("Includes <a href=\"http://wordlist.sourceforge.net/\">SCOWL</a> for list of words"))
		.arg(tr("Copyright &copy; 2000-2004 Kevin Atkinson")));
}

//-----------------------------------------------------------------------------

void Window::aboutScowl() {
	QFile file(":/scowl-readme");
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QDialog dialog(this);
		dialog.setWindowTitle(tr("About SCOWL"));

		QTextEdit* text = new QTextEdit(&dialog);
		text->setWordWrapMode(QTextOption::NoWrap);
		text->setReadOnly(true);

		QFont f = text->font();
		f.setFamily("Monospace");
		f.setFixedPitch(true);
		text->setCurrentFont(f);

		QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
		connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));

		QVBoxLayout* layout = new QVBoxLayout(&dialog);
		layout->addWidget(text);
		layout->addWidget(buttons);

		QTextStream stream(&file);
		text->setPlainText(stream.readAll());
		file.close();

		dialog.resize(700, 600);
		dialog.exec();
	}
}

//-----------------------------------------------------------------------------

void Window::newGame() {
	NewGameDialog dialog;
	if (dialog.exec() == QDialog::Accepted) {
		m_pause_action->setChecked(false);
		m_board->generate(dialog.seed());
	}
}

//-----------------------------------------------------------------------------

void Window::setPaused(bool paused) {
	m_board->setPaused(paused);
	m_contents->setCurrentIndex(paused);
}

//-----------------------------------------------------------------------------

bool Window::abortGame() {
	if (!m_board->isFinished()) {
		if (QMessageBox::question(this, tr("Question"), tr("Abort game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			m_pause_action->setChecked(false);
			m_board->abort();
		} else {
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

void Window::showDetails() {
	QSettings settings;
	QString size = Board::sizeString(settings.value("Board/Size", 4).toInt());
	QString timer = Clock::timerToString(settings.value("Board/TimerMode", Clock::Tanglet).toInt());
	int seed = settings.value("Board/Seed").toInt();
	QString higher_scores = settings.value("Board/HigherScores", true).toBool() ? tr("<p>Prevent low scoring boards is on.</p>") : "";
	QMessageBox::information(this, tr("Details"), tr(
		"<p><b>Size:</b> %1<br>"
		"<b>Timer:</b> %2<br>"
		"<b>Seed:</b> %L3</p>"
		"%4")
			.arg(size)
			.arg(timer)
			.arg(seed)
			.arg(higher_scores));
}

//-----------------------------------------------------------------------------

void Window::showScores() {
	ScoresDialog scores(this);
	scores.exec();
}

//-----------------------------------------------------------------------------

void Window::showSettings() {
	Settings settings;
	SettingsDialog dialog(settings, !m_board->isFinished(), this);
	if (dialog.exec() == QDialog::Accepted) {
		m_board->loadSettings(settings);
		if (settings.newGameRequired() && !m_board->isFinished()) {
			m_pause_action->setChecked(false);
			m_board->generate();
		}
	}
}

//-----------------------------------------------------------------------------

void Window::showControls() {
	QMessageBox::information(this, tr("Controls"), tr(
		"<p><b><big>Mouse Play:</big></b><br>"
		"<b>Select a word:</b> Click on the letters of a word.<br>"
		"<b>Make a guess:</b> Click on the last selected letter.<br>"
		"<b>Erase letters:</b> Click on an earlier selected letter.<br>"
		"<b>Clear the word:</b> Click twice on the first selected letter.</p>"
		"<p><b><big>Keyboard Play:</big></b><br>"
		"<b>Select a word:</b> Type the letters of a word.<br>"
		"<b>Make a guess:</b> Press Enter.<br>"
		"<b>Clear the word:</b> Press Ctrl+Backspace.</p>"
	));
}

//-----------------------------------------------------------------------------

void Window::gameFinished(int score) {
	ScoresDialog scores(this);
	if (scores.addScore(score)) {
		scores.exec();
	}
}

//-----------------------------------------------------------------------------
