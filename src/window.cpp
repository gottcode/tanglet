/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "window.h"

#include "board.h"
#include "clock.h"
#include "gzip.h"
#include "language_dialog.h"
#include "locale_dialog.h"
#include "new_game_dialog.h"
#include "scores_dialog.h"

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QSettings>
#include <QStackedWidget>
#include <QStandardPaths>
#include <QStyle>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>

#include <ctime>

//-----------------------------------------------------------------------------

class Window::State
{
public:
	State(Window* window)
		: m_window(window)
	{
	}

	virtual ~State() { }

	virtual void enter() { }
	virtual void newGame()
	{
		setState("NewGame");
	}
	virtual void openGame()
	{
		setState("OpenGame");
	}
	virtual void optimizingStarted() { }
	virtual void optimizingFinished() { }
	virtual void play() { }
	virtual void autoPause() { }
	virtual void autoResume() { }
	virtual void pause() { }
	virtual void resume() { }
	virtual void finish()
	{
		setState("Finish");
	}

protected:
	void setPaused(bool paused)
	{
		m_window->m_board->setPaused(paused);
		m_window->m_pause_action->setChecked(paused);
	}

	void setContentsIndex(int index)
	{
		m_window->m_contents->setCurrentIndex(index);
	}

	void setState(const QString& state)
	{
		m_window->m_previous_state = m_window->m_state;
		m_window->m_state = m_window->m_states.value(state);
		m_window->m_state->enter();
	}

	void setPreviousState()
	{
		m_window->m_state = m_window->m_previous_state;
		m_window->m_state->enter();
		m_window->m_previous_state = nullptr;
	}

private:
	Window* m_window;
};

//-----------------------------------------------------------------------------

class Window::NewGameState : public Window::State
{
public:
	NewGameState(Window* window)
		: State(window)
	{
	}

	void enter() override
	{
		m_next_state = "Play";
		setPaused(true);
		setContentsIndex(4);
	}

	void optimizingStarted() override
	{
		setState("Optimizing");
	}

	void play() override
	{
		setState(m_next_state);
	}

	void autoPause() override
	{
		m_next_state = "AutoPause";
	}

	void autoResume() override
	{
		m_next_state = "Play";
	}

private:
	QString m_next_state;
};

//-----------------------------------------------------------------------------

class Window::OpenGameState : public Window::State
{
public:
	OpenGameState(Window* window)
		: State(window)
	{
	}

	void enter() override
	{
		m_next_state = "Play";
		setPaused(true);
		setContentsIndex(2);
	}

	void optimizingStarted() override
	{
		setState("Optimizing");
	}

	void play() override
	{
		setState(m_next_state);
	}

	void autoPause() override
	{
		m_next_state = "AutoPause";
	}

	void autoResume() override
	{
		m_next_state = "Play";
	}

private:
	QString m_next_state;
};

//-----------------------------------------------------------------------------

class Window::OptimizingState : public Window::State
{
public:
	OptimizingState(Window* window)
		: State(window)
	{
	}

	void enter() override
	{
		setContentsIndex(5);
	}

	void optimizingFinished() override
	{
		setPreviousState();
	}
};

//-----------------------------------------------------------------------------

class Window::PlayState : public Window::State
{
public:
	PlayState(Window* window)
		: State(window)
	{
	}

	void enter() override
	{
		setPaused(false);
		setContentsIndex(0);
	}

	void autoPause() override
	{
		setState("AutoPause");
	}

	void pause() override
	{
		setState("Pause");
	}
};

//-----------------------------------------------------------------------------

class Window::AutoPauseState : public Window::State
{
public:
	AutoPauseState(Window* window)
		: State(window)
		, m_count(0)
	{
	}

	void enter() override
	{
		setPaused(true);
		setContentsIndex(1);
		m_count++;
	}

	void autoPause() override
	{
		m_count++;
	}

	void autoResume() override
	{
		m_count--;
		if (m_count < 1) {
			m_count = 0;
			setState("Play");
		}
	}

	void newGame() override
	{
		m_count = 0;
		setState("NewGame");
	}

	void openGame() override
	{
		m_count = 0;
		setState("OpenGame");
	}

	void pause() override
	{
		m_count = 0;
		setState("Pause");
	}

	void resume() override
	{
		m_count = 0;
		setState("Play");
	}

	void finish() override
	{
		m_count = 0;
		setState("Finish");
	}

private:
	int m_count;
};

//-----------------------------------------------------------------------------

class Window::PauseState : public Window::State
{
public:
	PauseState(Window* window)
		: State(window)
	{
	}

	void enter() override
	{
		setPaused(true);
		setContentsIndex(1);
	}

	void resume() override
	{
		setState("Play");
	}
};

//-----------------------------------------------------------------------------

class Window::FinishState : public Window::State
{
public:
	FinishState(Window* window)
		: State(window)
	{
	}

	void enter() override
	{
		setPaused(false);
		setContentsIndex(0);
	}
};

//-----------------------------------------------------------------------------

namespace
{

class AboutDialog : public QDialog
{
public:
	AboutDialog(const QString& title, const QString& filename, QWidget* parent = nullptr);
};

AboutDialog::AboutDialog(const QString& title, const QString& filename, QWidget* parent)
	: QDialog(parent)
{
	setWindowTitle(title);

	QTextEdit* text = new QTextEdit(this);
	text->setWordWrapMode(QTextOption::NoWrap);
	text->setReadOnly(true);

	QFile file(filename);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream stream(&file);
		text->setHtml("<pre>" + stream.readAll() + "</pre>");
		file.close();
	}

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	buttons->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons));
	connect(buttons, &QDialogButtonBox::accepted, this, &AboutDialog::accept);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(text);
	layout->addWidget(buttons);

	resize(700, 500);
}

}

//-----------------------------------------------------------------------------

Window::Window(const QString& file)
	: m_pause_action(nullptr)
	, m_previous_state(nullptr)
{
	setAcceptDrops(true);

	// Create states
	m_states.insert("NewGame", new NewGameState(this));
	m_states.insert("OpenGame", new OpenGameState(this));
	m_states.insert("Optimizing", new OptimizingState(this));
	m_states.insert("Play", new PlayState(this));
	m_states.insert("AutoPause", new AutoPauseState(this));
	m_states.insert("Pause", new PauseState(this));
	m_states.insert("Finish", new FinishState(this));
	m_state = m_states.value("NewGame");

	// Create widgets
	m_contents = new QStackedWidget(this);
	setCentralWidget(m_contents);

	m_board = new Board(this);
	m_contents->addWidget(m_board);
	connect(m_board, &Board::started, this, &Window::gameStarted);
	connect(m_board, &Board::finished, this, &Window::gameFinished);
	connect(m_board, &Board::optimizingStarted, this, &Window::optimizingStarted);
	connect(m_board, &Board::optimizingFinished, this, &Window::optimizingFinished);

	// Create pause screen
	m_pause_screen = new QLabel(tr("<p><b><big>Paused</big></b><br>Click to resume playing.</p>"), this);
	m_pause_screen->setAlignment(Qt::AlignCenter);
	m_pause_screen->installEventFilter(this);
	m_contents->addWidget(m_pause_screen);

	// Create open game screen
	QLabel* open_game_screen = new QLabel(tr("<p><b><big>Please wait</big></b><br>Loading game...</p>"), this);
	open_game_screen->setAlignment(Qt::AlignCenter);
	m_contents->addWidget(open_game_screen);

	// Create start screen
	QLabel* start_screen = new QLabel(tr("Click to start a new game."), this);
	start_screen->setAlignment(Qt::AlignCenter);
	start_screen->installEventFilter(this);
	m_contents->addWidget(start_screen);

	// Create new game screen
	QLabel* new_game_screen = new QLabel(tr("<p><b><big>Please wait</big></b><br>Generating a new board...</p>"), this);
	new_game_screen->setAlignment(Qt::AlignCenter);
	m_contents->addWidget(new_game_screen);

	// Create optimizing screen
	QLabel* optimizing_screen = new QLabel(tr("<p><b><big>Please wait</big></b><br>Optimizing word list...</p>"), this);
	optimizing_screen->setAlignment(Qt::AlignCenter);
	m_contents->addWidget(optimizing_screen);

	// Create game menu
	QMenu* menu = menuBar()->addMenu(tr("&Game"));
	menu->addAction(tr("New &Game..."), this, SLOT(newGame()), tr("Ctrl+Shift+N"));
	menu->addAction(tr("&New Roll"), this, SLOT(newRoll()), QKeySequence::New);
	menu->addAction(tr("&Choose..."), this, SLOT(chooseGame()));
	menu->addAction(tr("&Share..."), this, SLOT(shareGame()));
	menu->addSeparator();
	QAction* end_action = menu->addAction(tr("&End"), this, SLOT(endGame()));
	end_action->setEnabled(false);
	connect(m_board, &Board::pauseAvailable, end_action, &QAction::setEnabled);
	m_pause_action = menu->addAction(tr("&Pause"));
	m_pause_action->setCheckable(true);
	m_pause_action->setShortcut(tr("Ctrl+P"));
	m_pause_action->setEnabled(false);
	connect(m_pause_action, &QAction::triggered, this, &Window::setPaused);
	connect(m_board, &Board::pauseAvailable, m_pause_action, &QAction::setEnabled);
	menu->addSeparator();
	m_details_action = menu->addAction(tr("&Details"), this, SLOT(showDetails()));
	m_details_action->setEnabled(false);
	menu->addAction(tr("&High Scores"), this, SLOT(showScores()));
	menu->addSeparator();
	QAction* action = menu->addAction(tr("&Quit"), this, SLOT(close()), tr("Ctrl+Q"));
	action->setMenuRole(QAction::QuitRole);
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
	connect(group, &QActionGroup::triggered, m_board, &Board::setShowMaximumScore);
	QAction* missed_action = menu->addAction(tr("Show Missed &Words"));
	missed_action->setCheckable(true);
	connect(missed_action, &QAction::toggled, m_board, &Board::setShowMissedWords);
	QAction* counts_action = menu->addAction(tr("Show Word &Counts"));
	counts_action->setCheckable(true);
	counts_action->setChecked(true);
	connect(counts_action, &QAction::toggled, m_board, &Board::setShowWordCounts);
	menu->addAction(tr("&Board Language..."), this, SLOT(showLanguage()));
	menu->addSeparator();
	menu->addAction(tr("Application &Language..."), this, SLOT(showLocale()));
	monitorVisibility(menu);

	// Create help menu
	menu = menuBar()->addMenu(tr("&Help"));
	menu->addAction(tr("&Controls"), this, SLOT(showControls()));
	menu->addSeparator();
	action = menu->addAction(tr("&About"), this, SLOT(about()));
	action->setMenuRole(QAction::AboutRole);
	action = menu->addAction(tr("About &Hspell"), this, SLOT(aboutHspell()));
	action->setMenuRole(QAction::ApplicationSpecificRole);
	action = menu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
	action->setMenuRole(QAction::AboutQtRole);
	action = menu->addAction(tr("About &SCOWL"), this, SLOT(aboutScowl()));
	action->setMenuRole(QAction::ApplicationSpecificRole);
	monitorVisibility(menu);

	// Load settings
	QSettings settings;
	QAction* score_action = score_actions[qBound(0, settings.value("ShowMaximumScore", 1).toInt(), 2)];
	score_action->setChecked(true);
	m_board->setShowMaximumScore(score_action);
	missed_action->setChecked(settings.value("ShowMissed", true).toBool());
	counts_action->setChecked(settings.value("ShowWordCounts", true).toBool());
	restoreGeometry(settings.value("Geometry").toByteArray());

	// Start game
	QString current = file;
	if (settings.contains("Current/Version")) {
		if (current.isEmpty() ||
				QMessageBox::question(this, tr("Question"), tr("End the current game?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
			current = ":saved:";
		}
	}

	m_state->finish();
	m_contents->setCurrentIndex(3);
	if (current.isEmpty()) {
		newGame();
	} else {
		startGame(current);
	}
}

//-----------------------------------------------------------------------------

bool Window::eventFilter(QObject* watched, QEvent* event)
{
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

void Window::closeEvent(QCloseEvent* event)
{
	QSettings().setValue("Geometry", saveGeometry());
	QMainWindow::closeEvent(event);
}

//-----------------------------------------------------------------------------

void Window::dragEnterEvent(QDragEnterEvent* event)
{
	if (!(event->possibleActions() & Qt::CopyAction)) {
		return;
	}

	// Handle dragged file
	if (event->mimeData()->hasUrls()) {
		const QList<QUrl> urls = event->mimeData()->urls();
		if (urls.size() != 1) {
			return;
		}
		const QUrl url = urls.first();
		if (!url.isLocalFile()) {
			return;
		}
		const QString filename = url.toLocalFile();
		if (!filename.endsWith(".tanglet")) {
			return;
		}
	// Handle dragged game data
	} else if (!event->mimeData()->hasFormat("application/x-tanglet")) {
		return;
	}

	event->setDropAction(Qt::CopyAction);
	event->acceptProposedAction();
}

//-----------------------------------------------------------------------------

void Window::dropEvent(QDropEvent* event)
{
	if (!(event->possibleActions() & Qt::CopyAction)) {
		return;
	}

	// Handle dropped file
	if (event->mimeData()->hasUrls()) {
		const QList<QUrl> urls = event->mimeData()->urls();
		if (urls.size() != 1) {
			return;
		}
		const QUrl url = urls.first();
		if (!url.isLocalFile()) {
			return;
		}
		const QString filename = url.toLocalFile();
		if (!filename.endsWith(".tanglet")) {
			return;
		}
		if (endGame()) {
			startGame(filename);
		}
	// Handle dropped game data
	} else if (event->mimeData()->hasFormat("application/x-tanglet")) {
		QTemporaryFile file;
		if (file.open()) {
			file.write(event->mimeData()->data("application/x-tanglet"));
		}
		if (endGame()) {
			startGame(file.fileName());
		}
	}

	event->setDropAction(Qt::CopyAction);
	event->acceptProposedAction();
}

//-----------------------------------------------------------------------------

bool Window::event(QEvent* event)
{
	if (m_pause_action && m_pause_action->isEnabled()) {
		switch (event->type()) {
		case QEvent::WindowDeactivate:
			if (!QApplication::activeWindow() && !QApplication::activePopupWidget() && !QApplication::activeModalWidget()) {
				m_state->pause();
			}
			break;
		case QEvent::WindowBlocked:
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

void Window::about()
{
	QMessageBox::about(this, tr("About"),
		QString("<center><p><big><b>%1</b></big><br/>%2<br/><small>%3<br/>%4</small></p><p>%5</p><p>%6</p></center>")
		.arg(tr("Tanglet %1").arg(QCoreApplication::applicationVersion()),
			tr("A single player variant of <a href=\"http://en.wikipedia.org/wiki/Boggle\">Boggle</a>"),
			tr("Copyright &copy; 2009-%1 Graeme Gott").arg("2021"),
			tr("Released under the <a href=\"http://www.gnu.org/licenses/gpl.html\">GPL 3</a> license"),
			tr("English word list is based on <a href=\"http://wordlist.sourceforge.net/\">SCOWL</a> by Kevin Atkinson"),
			tr("Hebrew word list is based on <a href=\"http://hspell.ivrix.org.il/\">Hspell</a> by Nadav Har'El and Dan Kenigsberg"))
	);
}

//-----------------------------------------------------------------------------

void Window::aboutHspell()
{
	AboutDialog dialog(tr("About Hspell"), ":/hspell-readme", this);
	dialog.exec();
}

//-----------------------------------------------------------------------------

void Window::aboutScowl()
{
	AboutDialog dialog(tr("About SCOWL"), ":/scowl-readme", this);
	dialog.exec();
}

//-----------------------------------------------------------------------------

void Window::newRoll()
{
	if (endGame()) {
		startGame();
	}
}

//-----------------------------------------------------------------------------

void Window::newGame()
{
	if (endGame()) {
		NewGameDialog dialog(this);
		if (dialog.exec() == QDialog::Accepted) {
			startGame();
		}
	}
}


//-----------------------------------------------------------------------------

void Window::chooseGame()
{
	if (!endGame()) {
		return;
	}

	QString filename = QFileDialog::getOpenFileName(window(),
			tr("Import Game"),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			tr("Tanglet Games (*.tanglet)"));
	if (filename.isEmpty()) {
		return;
	}

	startGame(filename);
}

//-----------------------------------------------------------------------------

void Window::shareGame()
{
	QString filename = QFileDialog::getSaveFileName(window(),
			tr("Export Game"),
			QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
			tr("Tanglet Games (*.tanglet)"));

	if (filename.isEmpty()) {
		return;
	}

	if (!filename.endsWith(".tanglet")) {
		filename += ".tanglet";
	}

	// Share game
	{
		QSettings settings;
		settings.beginGroup("Current");

		QSettings game(filename, QSettings::IniFormat);
		game.beginGroup("Game");

		game.setValue("Version", settings.value("Version"));
		game.setValue("Size", settings.value("Size"));
		game.setValue("Density", settings.value("Density"));
		game.setValue("Minimum", settings.value("Minimum"));
		game.setValue("TimerMode", settings.value("TimerMode"));
		game.setValue("Letters", settings.value("Letters"));
		game.setValue("Language", settings.value("Language"));
		game.setValue("Dictionary", settings.value("Dictionary"));

		const QString dice = game.value("Dice").toString();
		if (dice.startsWith("tanglet:")) {
			game.setValue("Dice", dice);
		} else {
			QFile file(dice);
			if (file.open(QFile::ReadOnly)) {
				game.setValue("Dice", file.readAll().toBase64());
			}
		}

		const QString words = game.value("Words").toString();
		if (words.startsWith("tanglet:")) {
			game.setValue("Words", words);
		} else {
			QFile file(words);
			if (file.open(QFile::ReadOnly)) {
				game.setValue("Words", file.readAll().toBase64());
			}
		}
	}
	gzip(filename);
}

//-----------------------------------------------------------------------------

bool Window::endGame()
{
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

void Window::autoPause()
{
	m_state->autoPause();
}

//-----------------------------------------------------------------------------

void Window::autoResume()
{
	m_state->autoResume();
}

//-----------------------------------------------------------------------------

void Window::setPaused(bool paused)
{
	if (paused) {
		m_state->pause();
	} else {
		m_state->resume();
	}
}

//-----------------------------------------------------------------------------

void Window::showDetails()
{
	QSettings settings;
	int size = settings.value("Current/Size").toInt();
	int density = settings.value("Current/Density").toInt();
	int minimum = settings.value("Current/Minimum").toInt();
	int timer = settings.value("Current/TimerMode").toInt();
	QMessageBox::information(this, tr("Details"),
		QString("<p><b>%1</b> %2<br>"
			"<b>%3</b> %4<br>"
			"<b>%5</b> %6<br>"
			"<b>%7</b> %8<br>"
			"<b>%9</b> %10</p>")
		.arg(tr("Board Size:"), (size == 4) ? tr("Normal") : tr("Large"))
		.arg(tr("Word Density:"), NewGameDialog::densityString(density))
		.arg(tr("Word Length:"), tr("%1 or more letters").arg(minimum))
		.arg(tr("Game Type:"), Clock::timerToString(timer))
		.arg(tr("Description:"), Clock::timerDescription(timer)));
}

//-----------------------------------------------------------------------------

void Window::showScores()
{
	ScoresDialog scores(this);
	connect(&scores, &ScoresDialog::scoresReset, m_board, &Board::updateScoreColor);
	scores.exec();
}

//-----------------------------------------------------------------------------

void Window::showLanguage()
{
	LanguageDialog dialog(this);
	if (dialog.exec() == QDialog::Accepted) {
		newGame();
	}
}

//-----------------------------------------------------------------------------

void Window::showLocale()
{
	LocaleDialog dialog(this);
	dialog.exec();
}

//-----------------------------------------------------------------------------

void Window::showControls()
{
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

void Window::optimizingStarted()
{
	m_state->optimizingStarted();
}

//-----------------------------------------------------------------------------

void Window::optimizingFinished()
{
	m_state->optimizingFinished();
}

//-----------------------------------------------------------------------------

void Window::gameStarted()
{
	m_state->play();
	m_details_action->setEnabled(true);
}

//-----------------------------------------------------------------------------

void Window::gameFinished(int score)
{
	m_state->finish();
	ScoresDialog scores(this);
	if (scores.addScore(score)) {
		scores.exec();
	}

	QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	dir.remove("current");
	dir.remove("current-dice");
	dir.remove("current-words");
}

//-----------------------------------------------------------------------------

void Window::monitorVisibility(QMenu* menu)
{
#ifndef Q_OS_MAC
	connect(menu, &QMenu::aboutToShow, this, &Window::autoPause);
	connect(menu, &QMenu::aboutToHide, this, &Window::autoResume);
#endif
}

//-----------------------------------------------------------------------------

void Window::startGame(const QString& filename)
{
	QSettings settings;

	if (filename.isEmpty()) {
		// Start a new game
		settings.remove("Current");
		settings.sync();
		m_state->newGame();
		settings.beginGroup("Board");
		m_board->generate(settings);
	} else if (filename == ":saved:") {
		// Continue previous game
		m_state->openGame();
		settings.beginGroup("Current");
		if (!m_board->generate(settings)) {
			QMessageBox::warning(this, tr("Error"), tr("Unable to start requested game."));
			m_state->finish();
			m_contents->setCurrentIndex(3);
		}
	} else {
		// Uncompress requested game
		QString current = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
		QDir::home().mkpath(current);
		current += "/current";

		QByteArray data = gunzip(filename);
		QFile file(current);
		if (!file.open(QFile::WriteOnly)) {
			QMessageBox::warning(this, tr("Error"), tr("Unable to start requested game."));
			return;
		}
		file.write(data);
		file.close();

		QSettings game(current, QSettings::IniFormat);
		game.beginGroup("Game");

		// Extract dice
		data = game.value("Dice").toByteArray();
		if (data.startsWith("tanglet:")) {
			game.setValue("Dice", data);
		} else {
			const QString dice = QString("%1-dice").arg(current);
			game.setValue("Dice", dice);
			file.setFileName(dice);
			if (!file.open(QFile::WriteOnly)) {
				QMessageBox::warning(this, tr("Error"), tr("Unable to start requested game."));
				return;
			}
			file.write(QByteArray::fromBase64(data));
			file.close();
		}

		// Extract words
		data = game.value("Words").toByteArray();
		if (data.startsWith("tanglet:")) {
			game.setValue("Words", data);
		} else {
			const QString words = QString("%1-words").arg(current);
			game.setValue("Words", words);
			file.setFileName(words);
			if (!file.open(QFile::WriteOnly)) {
				QMessageBox::warning(this, tr("Error"), tr("Unable to start requested game."));
				return;
			}
			file.write(QByteArray::fromBase64(data));
			file.close();
		}
		data.clear();

		// Start requested game
		settings.remove("Current");
		settings.sync();
		m_state->openGame();
		if (!m_board->generate(game)) {
			QMessageBox::warning(this, tr("Error"), tr("Unable to start requested game."));
			m_state->finish();
			m_contents->setCurrentIndex(3);
		}
	}
}

//-----------------------------------------------------------------------------
