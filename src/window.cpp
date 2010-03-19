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
#include "language_dialog.h"
#include "new_game_dialog.h"
#include "random.h"
#include "scores_dialog.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSettings>
#include <QStackedWidget>
#include <QStyle>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

#include <ctime>

//-----------------------------------------------------------------------------

class Window::State {
public:
	State(Window* window)
		: m_window(window) { }

	virtual ~State() { }

	virtual void enter() { }
	virtual void start() { setState("Start"); }
	virtual void play() { }
	virtual void autoPause() { }
	virtual void autoResume() { }
	virtual void pause() { }
	virtual void resume() { }
	virtual void finish() { setState("Finish"); }

protected:
	void setPaused(bool paused) {
		m_window->m_board->setPaused(paused);
		m_window->m_pause_action->setChecked(paused);
	}

	void setContentsIndex(int index) {
		m_window->m_contents->setCurrentIndex(index);
	}

	void setState(const QString& state) {
		m_window->m_state = m_window->m_states.value(state);
		m_window->m_state->enter();
	}

private:
	Window* m_window;
};

//-----------------------------------------------------------------------------

class Window::StartState : public Window::State {
public:
	StartState(Window* window)
		: State(window) { }

	void enter() {
		m_next_state = "Play";
		setPaused(true);
		setContentsIndex(2);
	}

	void play() { setState(m_next_state); }
	void autoPause() { m_next_state = "AutoPause"; }
	void autoResume() { m_next_state = "Play"; }

private:
	QString m_next_state;
};

//-----------------------------------------------------------------------------

class Window::PlayState : public Window::State {
public:
	PlayState(Window* window)
		: State(window) { }

	void enter() {
		setPaused(false);
		setContentsIndex(0);
	}

	void autoPause() { setState("AutoPause"); }
	void pause() { setState("Pause"); }
};

//-----------------------------------------------------------------------------

class Window::AutoPauseState : public Window::State {
public:
	AutoPauseState(Window* window)
		: State(window), m_count(0) { }

	void enter() {
		setPaused(true);
		setContentsIndex(1);
		m_count++;
	}

	void autoPause() {
		m_count++;
	}

	void autoResume() {
		m_count--;
		if (m_count < 1) {
			m_count = 0;
			setState("Play");
		}
	}

	void start() { m_count = 0; setState("Start"); }
	void pause() { m_count = 0; setState("Pause"); }
	void resume() { m_count = 0; setState("Play"); }
	void finish() { m_count = 0; setState("Finish"); }

private:
	int m_count;
};

//-----------------------------------------------------------------------------

class Window::PauseState : public Window::State {
public:
	PauseState(Window* window)
		: State(window) { }

	void enter() {
		setPaused(true);
		setContentsIndex(1);
	}

	void resume() { setState("Play"); }
};

//-----------------------------------------------------------------------------

class Window::FinishState : public Window::State {
public:
	FinishState(Window* window)
		: State(window) { }

	void enter() {
		setPaused(false);
		setContentsIndex(0);
	}
};

//-----------------------------------------------------------------------------

Window::Window()
: m_pause_action(0) {
	setWindowTitle(tr("Tanglet"));
	setWindowIcon(QIcon(":/tanglet.png"));

	// Create states
	m_states.insert("Start", new StartState(this));
	m_states.insert("Play", new PlayState(this));
	m_states.insert("AutoPause", new AutoPauseState(this));
	m_states.insert("Pause", new PauseState(this));
	m_states.insert("Finish", new FinishState(this));
	m_state = m_states.value("Start");

	// Create widgets
	m_contents = new QStackedWidget(this);
	setCentralWidget(m_contents);

	m_board = new Board(this);
	m_contents->addWidget(m_board);
	connect(m_board, SIGNAL(started()), this, SLOT(gameStarted()));
	connect(m_board, SIGNAL(finished(int)), this, SLOT(gameFinished(int)));

	// Create pause screen
	m_pause_screen = new QLabel(tr("<p><b><big>Paused</big></b><br>Click to resume playing.</p>"), this);
	m_pause_screen->setAlignment(Qt::AlignCenter);
	m_pause_screen->installEventFilter(this);
	m_contents->addWidget(m_pause_screen);

	// Create load screen
	QLabel* load_screen = new QLabel(tr("<p><b><big>Please wait</big></b><br>Generating a new board...</p>"), this);
	load_screen->setAlignment(Qt::AlignCenter);
	m_contents->addWidget(load_screen);

	// Create start screen
	QLabel* start_screen = new QLabel(tr("Click to start a new game."), this);
	start_screen->setAlignment(Qt::AlignCenter);
	start_screen->installEventFilter(this);
	m_contents->addWidget(start_screen);

	// Create game menu
	QMenu* menu = menuBar()->addMenu(tr("&Game"));
	menu->addAction(tr("&New"), this, SLOT(newGame()), tr("Ctrl+N"));
	menu->addAction(tr("&Choose..."), this, SLOT(chooseGame()));
	menu->addSeparator();
	QAction* end_action = menu->addAction(tr("&End"), this, SLOT(endGame()));
	end_action->setEnabled(false);
	connect(m_board, SIGNAL(pauseAvailable(bool)), end_action, SLOT(setEnabled(bool)));
	menu->addSeparator();
	m_pause_action = menu->addAction(tr("&Pause"));
	m_pause_action->setCheckable(true);
	m_pause_action->setShortcut(tr("Ctrl+P"));
	m_pause_action->setEnabled(false);
	connect(m_pause_action, SIGNAL(triggered(bool)), this, SLOT(setPaused(bool)));
	connect(m_board, SIGNAL(pauseAvailable(bool)), m_pause_action, SLOT(setEnabled(bool)));
	menu->addSeparator();
	menu->addAction(tr("&Details"), this, SLOT(showDetails()));
	menu->addAction(tr("&High Scores"), this, SLOT(showScores()));
	menu->addSeparator();
	menu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));
	monitorVisibility(menu);

	// Create settings menu
	menu = menuBar()->addMenu(tr("&Settings"));
	QMenu* submenu = menu->addMenu(tr("Show &Maximum Score"));
	QAction* score_actions[3];
	score_actions[0] = submenu->addAction(tr("&Never"));
	score_actions[1] = submenu->addAction(tr("&End Of Game"));
	score_actions[2]  = submenu->addAction(tr("&Always"));
	QActionGroup* group = new QActionGroup(this);
	for (int i = 0; i < 3; ++i) {
		score_actions[i]->setData(i);
		score_actions[i]->setCheckable(true);
		group->addAction(score_actions[i]);
	}
	connect(group, SIGNAL(triggered(QAction*)), m_board, SLOT(setShowMaximumScore(QAction*)));
	QAction* missed_action = menu->addAction(tr("Show Missed &Words"));
	missed_action->setCheckable(true);
	connect(missed_action, SIGNAL(toggled(bool)), m_board, SLOT(setShowMissedWords(bool)));
	menu->addSeparator();
	QAction* higher_action = menu->addAction(tr("&Higher Scoring Boards"));
	higher_action->setCheckable(true);
	connect(higher_action, SIGNAL(toggled(bool)), m_board, SLOT(setHigherScoringBoards(bool)));
	menu->addAction(tr("&Board Language..."), this, SLOT(showLanguage()));
	monitorVisibility(menu);

	// Create help menu
	menu = menuBar()->addMenu(tr("&Help"));
	menu->addAction(tr("&Controls"), this, SLOT(showControls()));
	menu->addSeparator();
	menu->addAction(tr("&About"), this, SLOT(about()));
	menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
	menu->addAction(tr("About &SCOWL"), this, SLOT(aboutScowl()));
	monitorVisibility(menu);

	// Load settings
	QSettings settings;
	QAction* score_action = score_actions[qBound(0, settings.value("ShowMaximumScore", 1).toInt(), 2)];
	score_action->setChecked(true);
	m_board->setShowMaximumScore(score_action);
	missed_action->setChecked(settings.value("ShowMissed", true).toBool());
	higher_action->setChecked(settings.value("Board/HigherScores", true).toBool());
	restoreGeometry(settings.value("Geometry").toByteArray());

	// Start a new game
	m_state->finish();
	m_contents->setCurrentIndex(3);
	newGame();
}

//-----------------------------------------------------------------------------

bool Window::eventFilter(QObject* watched, QEvent* event) {
	if (watched == m_pause_screen) {
		if (event->type() == QEvent::MouseButtonPress) {
			m_state->resume();
			return true;
		} else {
			return false;
		}
	} else if (watched == m_contents->widget(3)) {
		if (event->type() == QEvent::MouseButtonPress) {
			newGame();
			return true;
		} else {
			return false;
		}
	} else {
		return QMainWindow::eventFilter(watched, event);
	}
}

//-----------------------------------------------------------------------------

void Window::closeEvent(QCloseEvent* event) {
	QSettings().setValue("Geometry", saveGeometry());
	if (!endGame()) {
		event->ignore();
	}
}

//-----------------------------------------------------------------------------

bool Window::event(QEvent* event) {
	if (m_pause_action && m_pause_action->isEnabled()) {
		switch (event->type()) {
		case QEvent::WindowDeactivate:
			m_state->autoPause();
			break;
		case QEvent::WindowUnblocked:
			m_state->autoResume();
			break;
		default:
			break;
		}
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

		QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
		buttons->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons));
		connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));

		QVBoxLayout* layout = new QVBoxLayout(&dialog);
		layout->addWidget(text);
		layout->addWidget(buttons);

		QTextStream stream(&file);
		text->setHtml("<pre>" + stream.readAll() + "</pre>");
		file.close();

		dialog.resize(700, 500);
		dialog.exec();
	}
}

//-----------------------------------------------------------------------------

void Window::newGame() {
	if (endGame()) {
		NewGameDialog dialog(this);
		if (dialog.exec() == QDialog::Accepted) {
			startGame();
		}
	}
}

//-----------------------------------------------------------------------------

void Window::chooseGame() {
	if (endGame()) {
		QDialog dialog(this, Qt::MSWindowsFixedSizeDialogHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
		dialog.setWindowTitle(tr("Choose Game"));

		QLineEdit* number = new QLineEdit(&dialog);
		number->setInputMask("99999000000000");

		QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
		connect(buttons, SIGNAL(accepted()), &dialog, SLOT(accept()));
		connect(buttons, SIGNAL(rejected()), &dialog, SLOT(reject()));

		QFormLayout* layout = new QFormLayout(&dialog);
		layout->addRow(tr("Game Number:"), number);
		layout->addRow(buttons);

		dialog.setFixedSize(dialog.sizeHint());

		if (dialog.exec() == QDialog::Accepted) {
			startGame(!number->text().isEmpty() ? number->text() : "0");
		}
	}
}

//-----------------------------------------------------------------------------

bool Window::endGame() {
	if (!m_board->isFinished()) {
		if (QMessageBox::question(this, tr("Question"), tr("End the current game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
			m_board->abort();
		} else {
			return false;
		}
	}
	return true;
}

//-----------------------------------------------------------------------------

void Window::autoPause() {
	m_state->autoPause();
}

//-----------------------------------------------------------------------------

void Window::autoResume() {
	m_state->autoResume();
}

//-----------------------------------------------------------------------------

void Window::setPaused(bool paused) {
	if (paused) {
		m_state->pause();
	} else {
		m_state->resume();
	}
}

//-----------------------------------------------------------------------------

void Window::showDetails() {
	QString details = QSettings().value("Current").toString();
	int size = qBound(4, details.mid(2,1).toInt(), 5);
	QString board = (size == 4) ? tr("Normal") : tr("Large");
	QString length = tr("%1 or more letters").arg(size - 1);
	QString mode = Clock::timerToString(qBound(0, details.mid(3,1).toInt(), Clock::TotalTimers - 1));
	QMessageBox::information(this, tr("Details"), tr("<p><b>Board Size:</b> %1<br><b>Word Length:</b> %2<br><b>Timer Mode:</b> %3<br><b>Game Number:</b> %4</p>").arg(board).arg(length).arg(mode).arg(details));
}

//-----------------------------------------------------------------------------

void Window::showScores() {
	ScoresDialog scores(this);
	scores.exec();
}

//-----------------------------------------------------------------------------

void Window::showLanguage() {
	LanguageDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		newGame();
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

void Window::gameStarted() {
	m_state->play();
}

//-----------------------------------------------------------------------------

void Window::gameFinished(int score) {
	m_state->finish();
	ScoresDialog scores(this);
	if (scores.addScore(score)) {
		scores.exec();
	}
}

//-----------------------------------------------------------------------------

void Window::monitorVisibility(QMenu* menu) {
#ifndef Q_WS_MAC
	connect(menu, SIGNAL(aboutToShow()), this, SLOT(autoPause()));
	connect(menu, SIGNAL(aboutToHide()), this, SLOT(autoResume()));
#endif
}

//-----------------------------------------------------------------------------

void Window::startGame(const QString& details) {
	QSettings settings;
	bool higher_scores = false;
	int size = 0;
	int timer = 0;
	unsigned int seed = 0;
	if (details.isEmpty()) {
		higher_scores = settings.value("Board/HigherScores", true).toBool();
		size = qBound(4, settings.value("Board/Size", 4).toInt(), 5);
		timer = settings.value("Board/TimerMode", Clock::Tanglet).toInt();
		seed = Random(time(0)).nextInt();
	} else if ((details.length() >= 5) && (details.at(0) == '1')) {
		higher_scores = details.mid(1,1).toInt();
		size = qBound(4, details.mid(2,1).toInt(), 5);
		timer = qBound(0, details.mid(3,1).toInt(), Clock::TotalTimers - 1);
		seed = details.mid(4).toUInt();
	} else {
		QMessageBox::warning(this, tr("Error"), tr("Unable to start requested game."));
		return;
	}

	settings.setValue("Current", QString("1%1%2%3%4").arg(higher_scores).arg(size).arg(timer).arg(seed));
	m_state->start();
	m_board->generate(higher_scores, size, timer, seed);
}

//-----------------------------------------------------------------------------
