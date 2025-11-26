/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "board.h"

#include "beveled_rect.h"
#include "clock.h"
#include "generator.h"
#include "language_settings.h"
#include "letter.h"
#include "scores_dialog.h"
#include "solver.h"
#include "view.h"
#include "word_counts.h"
#include "word_tree.h"

#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGraphicsScene>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QMessageBox>
#include <QSettings>
#include <QStyle>
#include <QTabWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>

//-----------------------------------------------------------------------------

Board::Board(QWidget* parent)
	: QWidget(parent)
	, m_paused(false)
	, m_wrong(false)
	, m_wrong_typed(false)
	, m_show_counts(1)
	, m_size(0)
	, m_minimum(0)
	, m_maximum(0)
	, m_max_score(0)
	, m_generator(nullptr)
{
	m_generator = new Generator(this);
	connect(m_generator, &Generator::finished, this, &Board::gameStarted);
	connect(m_generator, &Generator::optimizingStarted, this, &Board::optimizingStarted);
	connect(m_generator, &Generator::optimizingFinished, this, &Board::optimizingFinished);

	m_view = new View(nullptr, this);

	// Create clock and score widgets
	m_clock = new Clock(this);
	connect(m_clock, &Clock::finished, this, &Board::finish);

	m_score = new QLabel(this);

	m_max_score_details = new QToolButton(this);
	m_max_score_details->setAutoRaise(true);
	m_max_score_details->setIconSize(QSize(16, 16));
	m_max_score_details->setIcon(QIcon::fromTheme("dialog-information", QIcon(":/dialog-information.png")));
	m_max_score_details->setToolTip(tr("Details"));
	connect(m_max_score_details, &QToolButton::clicked, this, &Board::showMaximumWords);

	QHBoxLayout* score_layout = new QHBoxLayout;
	score_layout->setContentsMargins(0, 0, 0, 0);
	score_layout->addWidget(m_score);
	score_layout->addWidget(m_max_score_details);

	// Create guess widgets
	m_guess = new QLineEdit(this);
	m_guess->setDisabled(true);
	m_guess->setMaxLength(25);
	m_guess->installEventFilter(this);
	m_guess->setClearButtonEnabled(true);
	connect(m_guess, &QLineEdit::textEdited, this, &Board::guessChanged);
	connect(m_guess, &QLineEdit::returnPressed, this, &Board::guess);
	connect(m_view, &View::mousePressed, m_guess, qOverload<>(&QLineEdit::setFocus));

	int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);

	m_guess_button = new QToolButton(this);
	m_guess_button->setAutoRaise(true);
	m_guess_button->setIconSize(QSize(size, size));
	QIcon guess_fallback(":/tango/64x64/actions/list-add.png");
	guess_fallback.addFile(":/tango/48x48/actions/list-add.png");
	guess_fallback.addFile(":/tango/32x32/actions/list-add.png");
	guess_fallback.addFile(":/tango/24x24/actions/list-add.png");
	guess_fallback.addFile(":/tango/22x22/actions/list-add.png");
	guess_fallback.addFile(":/tango/16x16/actions/list-add.png");
	m_guess_button->setIcon(QIcon::fromTheme("list-add", guess_fallback));
	m_guess_button->setToolTip(tr("Guess"));
	m_guess_button->setEnabled(false);
	connect(m_guess_button, &QToolButton::clicked, this, &Board::guess);

	QHBoxLayout* guess_layout = new QHBoxLayout;
	guess_layout->setSpacing(0);
	guess_layout->addWidget(m_guess);
	guess_layout->addWidget(m_guess_button);

	// Create word lists
	m_found = new WordTree(this);
	m_found->setFocusPolicy(Qt::TabFocus);
	connect(m_found, &WordTree::itemSelectionChanged, this, &Board::wordSelected);

	m_missed = new WordTree(this);
	m_missed->setFocusPolicy(Qt::TabFocus);
	m_missed->hide();
	connect(m_missed, &WordTree::itemSelectionChanged, this, &Board::wordSelected);

	QWidget* found_tab = new QWidget(this);
	QVBoxLayout* found_layout = new QVBoxLayout(found_tab);
	found_layout->setSpacing(0);
	found_layout->setContentsMargins(0, 0, 0, 0);
	found_layout->addLayout(guess_layout);
	found_layout->addWidget(m_found);

	m_tabs = new QTabWidget(this);
	m_tabs->setUsesScrollButtons(false);
	m_tabs->addTab(found_tab, tr("Found"));
	connect(m_tabs, &QTabWidget::currentChanged, this, &Board::clearGuess);

	m_tabs->addTab(m_missed, tr("Missed"));
	const int width = std::max(guess_layout->sizeHint().width(), m_tabs->minimumSizeHint().width());
	m_tabs->removeTab(1);
	m_tabs->setFixedWidth(width);

	m_counts = new WordCounts(this);
	m_counts->setMinimumWidth(width);

	// Lay out board
	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(1, 1);
	layout->setRowStretch(1, 1);
	layout->addWidget(m_tabs, 0, 0, 3, 1);
	layout->addWidget(m_clock, 0, 1, Qt::AlignCenter);
	layout->addWidget(m_view, 1, 1);
	layout->addLayout(score_layout, 2, 1, Qt::AlignCenter);
	layout->addWidget(m_counts, 3, 0, 1, 2);
}

//-----------------------------------------------------------------------------

Board::~Board()
{
	m_generator->cancel();

	QSettings game;
	if (isFinished()) {
		// Clear current game
		game.remove("Current");
	} else {
		// Save current game
		game.beginGroup("Current");

		QStringList found;
		for (int i = 0; i < m_found->topLevelItemCount(); ++i) {
			found += m_found->topLevelItem(i)->text(2);
		}
		game.setValue("Found", found);

		QVariantList positions;
		QString word;
		for (const QPoint& position : std::as_const(m_positions)) {
			positions.append(position);
			word.append(m_cells[position.x()][position.y()]->text().toUpper());
		}
		if (!m_wrong && (m_guess->text() == word)) {
			game.setValue("Guess", m_guess->text());
			game.setValue("GuessPositions", positions);
		} else {
			game.remove("Guess");
			game.remove("GuessPositions");
		}

		m_clock->save(game);
	}
}

//-----------------------------------------------------------------------------

bool Board::isFinished() const
{
	return m_clock->isFinished();
}

//-----------------------------------------------------------------------------

void Board::abort()
{
	m_generator->cancel();
	m_clock->stop();
}

//-----------------------------------------------------------------------------

bool Board::generate(const QSettings& game)
{
	constexpr unsigned int TANGLET_FILE_VERSION = 3;

	// Verify version
	if (game.value("Version").toUInt() > TANGLET_FILE_VERSION) {
		return false;
	}

	// Find values
	int size = qBound(4, game.value("Size").toInt(), 5);
	int density = qBound(0, game.value("Density").toInt(), 3);
	int minimum = game.value("Minimum").toInt();
	if (size == 4) {
		minimum = qBound(3, minimum, 6);
	} else {
		minimum = qBound(4, minimum, 7);
	}
	int timer = qBound(0, game.value("TimerMode").toInt(), Clock::TotalTimers - 1);
	QStringList letters = game.value("Letters").toStringList();

	// Verify board size
	if (game.contains("Version") && ((size * size) != letters.size())) {
		return false;
	}

	const LanguageSettings language(game);
	const bool is_hebrew = (QLocale(language.language()).language() == QLocale::Hebrew);
	m_found->setHebrew(is_hebrew);
	m_missed->setHebrew(is_hebrew);

	// Store values
	{
		QSettings settings;
		settings.beginGroup("Current");
		settings.setValue("Version", TANGLET_FILE_VERSION);
		settings.setValue("Size", size);
		settings.setValue("Density", density);
		settings.setValue("Minimum", minimum);
		settings.setValue("TimerMode", timer);
		settings.setValue("Locale", language.language());
		settings.setValue("Dice", language.dice());
		settings.setValue("Words", language.words());
		settings.setValue("Dictionary", language.dictionary());
		if (!letters.isEmpty()) {
			settings.setValue("Letters", letters);
		}
	}

	// Create new game
	m_generator->cancel();
	m_generator->create(density, size, minimum, timer, letters);

	return true;
}

//-----------------------------------------------------------------------------

void Board::setPaused(bool pause)
{
	if (isFinished()) {
		return;
	}

	m_paused = pause;
	m_guess->setDisabled(m_paused);
	m_clock->setPaused(m_paused);

	if (!m_paused) {
		m_guess->setFocus();
	}
}

//-----------------------------------------------------------------------------

QString Board::sizeToString(int size)
{
	return (size == 4) ? tr("Normal") : tr("Large");
}

//-----------------------------------------------------------------------------

void Board::setShowMaximumScore(QAction* show)
{
	int score_type = show->data().toInt();
	QSettings().setValue("ShowMaximumScore", score_type);
	m_show_counts = score_type;
	m_max_score_details->setVisible(isFinished() && m_show_counts && m_clock->timer() == Clock::Allotment);
	updateScore();
}

//-----------------------------------------------------------------------------

void Board::setShowMissedWords(bool show)
{
	QSettings().setValue("ShowMissed", show);
	if (show) {
		if (m_tabs->count() == 1) {
			m_tabs->addTab(m_missed, tr("Missed"));
			m_tabs->setTabEnabled(1, isFinished());
		}
	} else {
		if (m_tabs->count() == 2) {
			m_tabs->removeTab(1);
			m_missed->hide();
		}
	}
}

//-----------------------------------------------------------------------------

void Board::setShowWordCounts(bool show)
{
	QSettings().setValue("ShowWordCounts", show);
	m_counts->setVisible(show);
}

//-----------------------------------------------------------------------------

void Board::gameStarted()
{
	// Load settings
	QSettings settings;
	settings.beginGroup("Current");

	m_clock->setTimer(m_generator->timer());
	if (m_generator->size() != m_size) {
		m_size = m_generator->size();
		m_cells = QList<QList<Letter*>>(m_size, QList<Letter*>(m_size));
		m_maximum = m_size * m_size;
		m_guess->setMaxLength(m_maximum);
	}
	m_minimum = m_generator->minimum();
	m_max_score = m_generator->maxScore();
	m_max_score_details->hide();
	m_letters = m_generator->letters();
	m_solutions = m_generator->solutions();
	m_counts->setWords(m_solutions.keys());
	m_found->setDictionary(m_generator->dictionary());
	m_found->setTrie(m_generator->trie());
	m_missed->setDictionary(m_generator->dictionary());
	m_missed->setTrie(m_generator->trie());
	settings.setValue("Letters", m_letters);

	// Create board
	QFont f = font();
	f.setBold(true);
	f.setPointSize(20);
	QFontMetrics metrics(f);
	int letter_size = 0;
	const auto dice = m_generator->dice(m_size);
	for (const QStringList& die : dice) {
		for (const QString& side : die) {
			letter_size = std::max(letter_size, metrics.boundingRect(side).width());
		}
	}
	int cell_size = std::max(metrics.height(), letter_size) + 10;
	int cell_padding_size = cell_size + 4;
	int board_size = (m_size * cell_padding_size) + 8;

	delete m_view->scene();
	QGraphicsScene* scene = new QGraphicsScene(0, 0, board_size, board_size, this);
	m_view->setScene(scene);
	m_view->setMinimumSize(board_size + 4, board_size + 4);
	m_view->fitInView(m_view->sceneRect(), Qt::KeepAspectRatio);

	BeveledRect* rect = new BeveledRect(board_size);
	rect->setColor(QColor(0, 0x57, 0xae));
	scene->addItem(rect);

	// Create cells
	for (int r = 0; r < m_size; ++r) {
		for (int c = 0; c < m_size; ++c) {
			Letter* cell = new Letter(f, cell_size, QPoint(c, r));
			cell->setText(m_letters.at((r * m_size) + c));
			cell->moveBy((c * cell_padding_size) + 6, (r * cell_padding_size) + 6);
			scene->addItem(cell);
			m_cells[c][r] = cell;
			connect(cell, &Letter::clicked, this, &Board::letterClicked);
		}
	}

	// Switch to found tab
	m_tabs->setCurrentWidget(m_found);
	m_tabs->setTabEnabled(1, false);

	// Clear previous words
	m_paused = false;
	Q_EMIT pauseAvailable(true);
	m_guess_button->setEnabled(true);
	m_positions.clear();
	m_found->removeAll();
	m_found->setColumnHidden(1, true);
	m_missed->removeAll();
	m_guess->setEnabled(true);
	m_guess->clear();
	m_guess->setEchoMode(QLineEdit::Normal);
	m_guess->setFocus();
	clearHighlight();

	// Add solutions
	for (auto i = m_solutions.cbegin(), end = m_solutions.cend(); i != end; ++i) {
		m_missed->addWord(i.key());
	}

	// Add found words
	const QStringList found = settings.value("Found").toStringList();
	for (const QString& text : found) {
		QTreeWidgetItem* item = m_found->findItems(text, Qt::MatchExactly, 2).value(0);
		if (m_missed->findItems(text, Qt::MatchExactly, 2).value(0) && !item) {
			m_found->addWord(text);
			delete m_missed->findItems(text, Qt::MatchExactly, 2).constFirst();
			m_counts->findWord(text);
		}
	}

	// Add guess
	m_guess->setText(settings.value("Guess").toString());
	const QVariantList positions = settings.value("GuessPositions").toList();
	for (const QVariant& position : positions) {
		m_positions.append(position.toPoint());
	}

	// Show errors
	QString error = m_generator->error();
	if (!error.isEmpty()) {
		abort();
		QMessageBox::warning(this, tr("Error"), error);
		return;
	}

	// Start game
	Q_EMIT started();
	if (m_missed->topLevelItemCount() > 0) {
		m_clock->start();
		if (settings.contains("TimerDetails/Time")) {
			m_clock->load(settings);
		}
		updateScore();
		updateClickableStatus();
		updateButtons();
		highlightWord();
	} else {
		m_clock->stop();
	}
}

//-----------------------------------------------------------------------------

void Board::clearGuess()
{
	m_wrong_typed = false;
	m_wrong = false;
	m_positions.clear();
	clearHighlight();
	updateClickableStatus();
	m_guess->clear();
	m_found->setCurrentItem(nullptr);
	m_missed->setCurrentItem(nullptr);
	m_guess->setFocus();
	updateButtons();
}

//-----------------------------------------------------------------------------

void Board::guess()
{
	if (isFinished() || m_paused || m_positions.isEmpty() || m_wrong_typed || m_wrong) {
		return;
	}

	const QString text = m_guess->text().trimmed().toUpper();
	if (text.isEmpty() || (text.length() < m_minimum) || (text.length() > m_maximum)) {
		return;
	}
	if (!m_solutions.contains(text)) {
		m_wrong = true;
		highlightWord();
		updateButtons();
		m_clock->addIncorrectWord(Solver::score(text));
		return;
	}

	// Create found item
	QTreeWidgetItem* item = m_found->findItems(text, Qt::MatchExactly, 2).value(0);
	if (!item) {
		item = m_found->addWord(text);
		delete m_missed->findItems(item->text(2), Qt::MatchExactly, 2).constFirst();

		m_clock->addWord(item->data(0, Qt::UserRole).toInt());
		updateScore();

		QList<QList<QPoint>>& solutions = m_solutions[text];
		const int index = solutions.indexOf(m_positions);
		if (index != -1) {
			solutions.move(index, 0);
		} else {
			solutions.prepend(m_positions);
		}

		m_counts->findWord(text);
	}
	m_found->scrollToItem(item);
	m_found->setCurrentItem(nullptr);

	// Clear guess
	clearGuess();

	// Handle finding all of the words
	if (m_missed->topLevelItemCount() == 0) {
		// Increase score
		for (int i = 0; i < m_found->topLevelItemCount(); ++i) {
			QTreeWidgetItem* item = m_found->topLevelItem(i);
			item->setData(0, Qt::UserRole, item->data(0, Qt::UserRole).toInt() + 1);
		}

		// Stop the game
		m_clock->stop();
	}
}

//-----------------------------------------------------------------------------

void Board::guessChanged()
{
	m_wrong_typed = false;
	m_wrong = false;
	clearHighlight();
	m_found->setCurrentItem(nullptr);

	QString word = m_guess->text().trimmed().toUpper();
	if (!word.isEmpty()) {
		int pos = m_guess->cursorPosition();
		m_guess->setText(word);
		m_guess->setCursorPosition(pos);
		QTreeWidgetItem* item = m_found->findItems(word, Qt::MatchStartsWith, 2).value(0);
		m_found->scrollToItem(item, QAbstractItemView::PositionAtTop);

		Trie trie(word);
		Solver solver(trie, m_size, 0);
		solver.solve(m_letters);
		QList<QList<QPoint>> solutions = m_solutions.value(word, solver.solutions().value(word));
		m_wrong_typed = solutions.isEmpty();
		if (!m_wrong_typed) {
			int index = 0;
			int matched = -1;
			int difference = INT_MAX;

			int count = solutions.count();
			for (int i = 0; i < count; i++) {
				bool order = true;
				int match = 0;
				int prev_pos = -1;
				int deltas = INT_MAX;

				// Find how many cells match and are in order between solution and m_positions
				const QList<QPoint>& solution = solutions.at(i);
				for (const QPoint& cell : solution) {
					int pos = m_positions.indexOf(cell);
					if (pos != -1) {
						match++;
						if (prev_pos != -1) {
							int delta = pos - prev_pos;
							if (delta < 0) {
								order = false;
								break;
							} else {
								deltas += delta;
							}
						} else {
							deltas = pos;
						}
						prev_pos = pos;
					}
				}

				// Figure out if current solution best matches m_positions
				if (order == true && (match > matched || (match == matched && deltas < difference))) {
					matched = match;
					difference = deltas;
					index = i;
				}
			}
			m_positions = solutions.at(index);
		}
		updateClickableStatus();
		highlightWord();

		selectGuess();
	} else {
		m_positions.clear();
		updateClickableStatus();
	}

	updateButtons();
}

//-----------------------------------------------------------------------------

void Board::finish()
{
	m_clock->setText((m_missed->topLevelItemCount() == 0 && m_found->topLevelItemCount() > 0) ? tr("Success") : tr("Game Over"));

	clearGuess();
	m_found->setColumnHidden(1, false);
	m_guess->setDisabled(true);
	m_guess_button->setDisabled(true);
	m_guess->setEchoMode(QLineEdit::NoEcho);
	m_guess->releaseKeyboard();
	m_tabs->setTabEnabled(1, true);
	m_max_score_details->setVisible(m_show_counts && m_clock->timer() == Clock::Allotment);
	Q_EMIT pauseAvailable(false);

	int score = updateScore();
	Q_EMIT finished(score, m_max_score);
}

//-----------------------------------------------------------------------------

void Board::wordSelected()
{
	QList<QTreeWidgetItem*> items;
	if (m_tabs->currentWidget() == m_missed) {
		items = m_missed->selectedItems();
	} else {
		items = m_found->selectedItems();
	}
	if (items.isEmpty()) {
		return;
	}

	QString word = items.first()->text(2);
	if (!word.isEmpty() && word != m_guess->text()) {
		m_guess->setText(word);
		m_positions = m_solutions.value(word).value(0);
		clearHighlight();
		updateClickableStatus();
		highlightWord();
	}

	updateButtons();
}

//-----------------------------------------------------------------------------

void Board::letterClicked(Letter* letter)
{
	// Handle adding a letter to the guess
	if (!m_positions.contains(letter->position())) {
		QString word = m_guess->text().trimmed().toUpper();
		word.append(letter->text().toUpper());
		m_guess->setText(word);
		QTreeWidgetItem* item = m_found->findItems(word, Qt::MatchStartsWith, 2).value(0);
		m_found->scrollToItem(item, QAbstractItemView::PositionAtTop);

		m_wrong = false;
		m_positions.append(letter->position());
		clearHighlight();
		updateClickableStatus();
		highlightWord();

		selectGuess();
	// Handle making or clearing a guess
	} else if (letter->position() == m_positions.last()) {
		if (m_positions.count() == 1) {
			clearGuess();
		} else {
			guess();
		}
	// Handle backing up in a guess
	} else {
		m_wrong = false;
		m_guess->clear();
		m_positions = m_positions.mid(0, m_positions.indexOf(letter->position()) + 1);
		clearHighlight();
		updateClickableStatus();

		QString word;
		for (const QPoint& position : std::as_const(m_positions)) {
			word.append(m_cells[position.x()][position.y()]->text().toUpper());
		}
		m_guess->setText(word);
		highlightWord();

		selectGuess();
	}

	updateButtons();
}

//-----------------------------------------------------------------------------

void Board::highlightWord(const QList<QPoint>& positions, const QColor& color)
{
	Q_ASSERT(!positions.isEmpty());

	m_cells[positions.at(0).x()][positions.at(0).y()]->setColor(color);
	for (int i = 1; i < positions.count(); ++i) {
		const QPoint& pos1 = positions.at(i);
		m_cells[pos1.x()][pos1.y()]->setColor(color);

		const QPoint& pos0 = m_positions.at(i - 1);
		QLineF line(pos0, pos1);
		m_cells[pos0.x()][pos0.y()]->setArrow(line.angle(), i - 1);
	}

	int alpha = 192 / positions.count();
	QColor border = Qt::white;
	for (int i = positions.count() - 1; i >= 0; --i) {
		const QPoint& position = positions.at(i);
		m_cells[position.x()][position.y()]->setCellColor(border);
		border.setAlpha(border.alpha() - alpha);
	}
}

//-----------------------------------------------------------------------------

void Board::highlightWord()
{
	QString guess = m_guess->text();
	if (guess.isEmpty()) {
		return;
	}

	QPalette p = palette();
	if (!m_wrong) {
		if (m_wrong_typed) {
			p.setColor(m_guess->foregroundRole(), Qt::white);
			p.setColor(m_guess->backgroundRole(), Qt::red);
		} else if (!m_found->findItems(guess, Qt::MatchExactly).isEmpty()) {
			p.setColor(m_guess->foregroundRole(), Qt::white);
			p.setColor(m_guess->backgroundRole(), QColor(0xff, 0xaa, 0));
			highlightWord(m_positions, QColor(0xff, 0xaa, 0));
		} else if (m_positions.count() < m_minimum) {
			highlightWord(m_positions, QColor(0xbf, 0xd9, 0xff));
		} else {
			highlightWord(m_positions, QColor(0x80, 0xb3, 0xff));
		}
	} else {
		p.setColor(m_guess->foregroundRole(), Qt::white);
		p.setColor(m_guess->backgroundRole(), Qt::red);
		highlightWord(m_positions, Qt::red);
	}
	if (m_guess->isEnabled()) {
		m_guess->setPalette(p);
	}
}

//-----------------------------------------------------------------------------

void Board::clearHighlight()
{
	QColor color = !isFinished() ? Qt::white : QColor(0xaa, 0xaa, 0xaa);
	for (int c = 0; c < m_size; ++c) {
		for (int r = 0; r < m_size; ++r) {
			m_cells[c][r]->setColor(color);
			m_cells[c][r]->setCellColor(QColor());
			m_cells[c][r]->setArrow(-1, 0);
		}
	}
	m_guess->setPalette(palette());
}

//-----------------------------------------------------------------------------

void Board::selectGuess()
{
	QTreeWidgetItem* item = m_found->findItems(m_guess->text(), Qt::MatchExactly, 0).value(0);
	if (item) {
		m_found->setCurrentItem(item);
		m_found->scrollToItem(item);
	} else {
		m_found->setCurrentItem(nullptr);
	}
}

//-----------------------------------------------------------------------------

int Board::updateScore()
{
	int score = 0;
	for (int i = 0; i < m_found->topLevelItemCount(); ++i) {
		score += m_found->topLevelItem(i)->data(0, Qt::UserRole).toInt();
	}

	if (m_show_counts == 2 || (m_show_counts == 1 && isFinished())) {
		if (score > 3) {
			m_score->setText(tr("%1 of %n point(s)", "", m_max_score).arg(score));
		} else if (score == 3) {
			m_score->setText(tr("3 of %n point(s)", "", m_max_score));
		} else if (score == 2) {
			m_score->setText(tr("2 of %n point(s)", "", m_max_score));
		} else if (score == 1) {
			m_score->setText(tr("1 of %n point(s)", "", m_max_score));
		} else {
			m_score->setText(tr("0 of %n point(s)", "", m_max_score));
		}
		m_counts->setMaximumsVisible(true);
	} else {
		m_score->setText(tr("%n point(s)", "", score));
		m_counts->setMaximumsVisible(false);
	}

	QFont f = font();
	QPalette p = palette();
	switch (ScoresDialog::isHighScore(score, m_clock->timer())) {
	case 2:
		f.setBold(true);
		p.setColor(m_score->foregroundRole(), Qt::blue);
		break;
	case 1:
		f.setBold(true);
		break;
	default:
		break;
	}
	m_score->setFont(f);
	m_score->setPalette(p);

	return score;
}

//-----------------------------------------------------------------------------

void Board::updateClickableStatus()
{
	bool finished = isFinished();
	bool has_word = !m_positions.isEmpty() && !finished;
	bool clickable = !has_word && !finished;

	for (int y = 0; y < m_size; ++y) {
		for (int x = 0; x < m_size; ++x) {
			m_cells[x][y]->setClickable(clickable);
		}
	}

	if (has_word && !m_wrong_typed) {
		const QPoint& position = m_positions.last();
		int min_x = std::max(position.x() - 1, 0);
		int max_x = std::min(position.x() + 2, m_size);
		int min_y = std::max(position.y() - 1, 0);
		int max_y = std::min(position.y() + 2, m_size);
		for (int y = min_y; y < max_y; ++y) {
			for (int x = min_x; x < max_x; ++x) {
				m_cells[x][y]->setClickable(true);
			}
		}

		for (const QPoint& position : std::as_const(m_positions)) {
			m_cells[position.x()][position.y()]->setClickable(true);
		}
	}
}

//-----------------------------------------------------------------------------

void Board::updateButtons()
{
	QString text = m_guess->text();
	bool has_guess = !text.isEmpty();
	m_guess_button->setEnabled(has_guess
			&& (text.length() >= m_minimum)
			&& (text.length() <= m_maximum)
			&& !m_positions.isEmpty()
			&& !m_wrong_typed
			&& !m_wrong);
}

//-----------------------------------------------------------------------------

void Board::showMaximumWords()
{
	QDialog dialog(window(), Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	dialog.setWindowTitle(tr("Details"));

	QList<int> scores;
	for (auto i = m_solutions.cbegin(), end = m_solutions.cend(); i != end; ++i) {
		scores.append(Solver::score(i.key()));
	}
	std::sort(scores.begin(), scores.end(), std::greater<int>());
	scores = scores.mid(0, 30);

	QLabel* message = new QLabel(tr("The maximum score was calculated from the following thirty words:"), this);
	message->setWordWrap(true);

	WordTree* words = new WordTree(this);
	words->setTrie(m_generator->trie());
	words->setDictionary(m_generator->dictionary());
	const QList<QTreeWidget*> trees{ m_found, m_missed };
	for (QTreeWidget* tree : trees) {
		for (int i = 0; i < tree->topLevelItemCount(); ++i) {
			QTreeWidgetItem* item = tree->topLevelItem(i);
			int index = scores.indexOf(item->data(0, Qt::UserRole).toInt());
			if (index != -1) {
				words->addWord(item->text(0));
				scores.removeAt(index);
			}
		}
	}

	QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
	buttons->setCenterButtons(style()->styleHint(QStyle::SH_MessageBox_CenterButtons));
	connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

	QVBoxLayout* layout = new QVBoxLayout(&dialog);
	layout->addWidget(message);
	layout->addWidget(words);
	layout->addWidget(buttons);

	dialog.exec();
}

//-----------------------------------------------------------------------------
