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

#include "board.h"

#include "clock.h"
#include "letter.h"
#include "settings.h"
#include "solver.h"
#include "view.h"
#include "word_tree.h"

#include <QEvent>
#include <QFile>
#include <QGridLayout>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QListWidget>
#include <QMessageBox>
#include <QPainterPath>
#include <QTabWidget>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>

#include <algorithm>
#include <ctime>

//-----------------------------------------------------------------------------

Board::Board(QWidget* parent)
: QWidget(parent), m_paused(false), m_wrong(false), m_valid(true), m_score_type(1), m_higher_scores(false) {
	m_view = new View(0, this);

	// Create clock and score widgets
	m_clock = new Clock(this);
	connect(m_clock, SIGNAL(finished()), this, SLOT(finish()));

	m_score = new QLabel(this);

	// Create guess widgets
	m_guess = new QLineEdit(this);
	m_guess->setDisabled(true);
	m_guess->setMaxLength(16);
	m_guess->installEventFilter(this);
	connect(m_guess, SIGNAL(textEdited(const QString&)), this, SLOT(guessChanged()));
	connect(m_guess, SIGNAL(returnPressed()), this, SLOT(guess()));
	connect(m_view, SIGNAL(mousePressed()), m_guess, SLOT(setFocus()));

	m_clear_button = new QToolButton(this);
	m_clear_button->setAutoRaise(true);
	m_clear_button->setIconSize(QSize(24,24));
	m_clear_button->setIcon(QIcon(":/edit-clear.png"));
	m_clear_button->setToolTip(tr("Clear"));
	connect(m_clear_button, SIGNAL(clicked()), this, SLOT(clearGuess()));

	m_guess_button = new QToolButton(this);
	m_guess_button->setAutoRaise(true);
	m_guess_button->setIconSize(QSize(24,24));
	m_guess_button->setIcon(QIcon(":/list-add.png"));
	m_guess_button->setToolTip(tr("Guess"));
	connect(m_guess_button, SIGNAL(clicked()), this, SLOT(guess()));

	QHBoxLayout* guess_layout = new QHBoxLayout;
	guess_layout->setSpacing(0);
	guess_layout->addStretch();
	guess_layout->addWidget(m_guess);
	guess_layout->addWidget(m_clear_button);
	guess_layout->addWidget(m_guess_button);
	guess_layout->addStretch();

	// Create word lists
	m_found = new WordTree(this);
	m_found->setFocusPolicy(Qt::TabFocus);
	connect(m_found, SIGNAL(itemSelectionChanged()), this, SLOT(wordSelected()));

	m_missed = new WordTree(this);
	m_missed->setFocusPolicy(Qt::TabFocus);
	connect(m_missed, SIGNAL(itemSelectionChanged()), this, SLOT(wordSelected()));

	QWidget* found_tab = new QWidget(this);
	QVBoxLayout* found_layout = new QVBoxLayout(found_tab);
	found_layout->setSpacing(0);
	found_layout->setMargin(0);
	found_layout->addLayout(guess_layout);
	found_layout->addWidget(m_found);

	m_tabs = new QTabWidget(this);
	m_tabs->addTab(found_tab, tr("Found"));
	m_tabs->addTab(m_missed, tr("Missed"));
	connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(clearGuess()));

	int width = guess_layout->sizeHint().width();
	m_tabs->setFixedWidth(width);

	// Lay out board
	QGridLayout* layout = new QGridLayout(this);
	layout->setColumnStretch(1, 1);
	layout->setColumnStretch(1, 1);
	layout->setRowStretch(1, 1);
	layout->addWidget(m_tabs, 0, 0, 3, 1);
	layout->addWidget(m_clock, 0, 1, Qt::AlignCenter);
	layout->addWidget(m_view, 1, 1);
	layout->addWidget(m_score, 2, 1, Qt::AlignCenter);
}

//-----------------------------------------------------------------------------

bool Board::isFinished() const {
	return m_clock->isFinished();
}

//-----------------------------------------------------------------------------

void Board::abort() {
	m_clock->stop();
}

//-----------------------------------------------------------------------------

void Board::generate() {
	// Roll dice
	forever {
		m_letters.clear();
		srand(time(0));
		std::random_shuffle(m_dice.begin(), m_dice.end());
		for (int i = 0; i < m_dice.count(); ++i) {
			QStringList& die = m_dice[i];
			std::random_shuffle(die.begin(), die.end());
			m_letters += die.at(0);
		}

		Solver solver(m_words, m_letters);
		if (!m_higher_scores || (solver.score() >= 200)) {
			m_solutions = solver.solutions();
			break;
		}
	}

	// Create cells
	QFont f = font();
	f.setBold(true);
	f.setPointSize(20);
	QFontMetrics metrics(f);
	int letter_size = 0;
	foreach (const QStringList& die, m_dice) {
		foreach (const QString& side, die) {
			letter_size = qMax(letter_size, metrics.width(side));
		}
	}
	int cell_size = qMax(metrics.height(), letter_size) + 10;
	int cell_padding_size = cell_size + 4;
	int board_size = (4 * cell_padding_size) + 8;

	delete m_view->scene();
	QGraphicsScene* scene = new QGraphicsScene(0, 0, board_size, board_size, this);
	m_view->setScene(scene);
	m_view->setMinimumSize(board_size, board_size);
	m_view->fitInView(m_view->sceneRect(), Qt::KeepAspectRatio);

	QPainterPath path;
	path.addRoundedRect(0, 0, board_size, board_size, 5, 5);
	scene->addPath(path, Qt::NoPen, QColor("#0057ae"));

	for (int r = 0; r < 4; ++r) {
		for (int c = 0; c < 4; ++c) {
			Letter* cell = new Letter(f, cell_size, QPoint(c,r));
			cell->setText(m_letters.at((r * 4) + c));
			cell->moveBy((c * cell_padding_size) + 6, (r * cell_padding_size) + 6);
			scene->addItem(cell);
			m_cells[c][r] = cell;
			connect(cell, SIGNAL(clicked(Letter*)), this, SLOT(letterClicked(Letter*)));
		}
	}

	// Switch to found tab
	m_tabs->setCurrentWidget(m_found);
	m_tabs->setTabEnabled(1, false);

	// Clear previous words
	emit pauseAvailable(true);
	m_clear_button->setEnabled(true);
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
	QList<QString> solutions = m_solutions.keys();
	foreach (const QString& solution, solutions) {
		m_missed->addWord(solution);
	}

	// Start game
	if (m_missed->topLevelItemCount() > 0) {
		m_clock->start();
		updateScore();
		updateClickableStatus();
	} else {
		m_clock->stop();
	}
}

//-----------------------------------------------------------------------------

void Board::setPaused(bool pause) {
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

void Board::loadSettings(const Settings& settings) {
	// Load gameplay settings
	m_higher_scores = settings.higherScores();
	m_score_type = settings.scoreType();
	updateScore();

	if (settings.showMissed()) {
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

	// Load dice
	m_dice.clear();
	QFile file(settings.dice());
	if (file.open(QFile::ReadOnly | QIODevice::Text)) {
		QTextStream stream(&file);
		while (!stream.atEnd()) {
			QStringList line = stream.readLine().split(',', QString::SkipEmptyParts);
			if (line.count() == 6) {
				m_dice.append(line);
			}
		}
		file.close();
	}
	if (m_dice.count() != 16) {
		m_dice.clear();
		for (int i = 0; i < 16; ++i) {
			m_dice.append(QString("?,?,?,?,?,?").split(','));
		}
		QMessageBox::warning(this, tr("Error"), tr("Unable to read dice from file."));
	}

	// Load words
	m_words = Trie();
	int count = 0;
	file.setFileName(settings.words());
	if (file.open(QFile::ReadOnly | QIODevice::Text)) {
		QTextStream stream(&file);
		while (!stream.atEnd()) {
			QString line = stream.readLine().toUpper();
			if (line.length() >= 3 && line.length() <= 16) {
				count++;
				m_words.addWord(line);
			}
		}
		file.close();
	}
	if (count == 0) {
		QMessageBox::warning(this, tr("Error"), tr("Unable to read word list from file."));
	}

	// Load dictionary
	QString url = settings.dictionary();
	m_found->setDictionary(url);
	m_missed->setDictionary(url);
}

//-----------------------------------------------------------------------------

void Board::clearGuess() {
	m_valid = true;
	m_wrong = false;
	m_positions.clear();
	clearHighlight();
	updateClickableStatus();
	m_guess->clear();
	m_found->clearSelection();
	m_missed->clearSelection();
	m_guess->setFocus();
}

//-----------------------------------------------------------------------------

void Board::guess() {
	if (!isFinished() && !m_paused) {
		QString text = m_guess->text().trimmed().toUpper();
		if (text.isEmpty() || text.length() < 3 || text.length() > 16 || !m_valid || m_positions.isEmpty()) {
			return;
		}
		if (!m_solutions.contains(text)) {
			m_wrong = true;
			highlightWord();
			return;
		}

		// Create found item
		QTreeWidgetItem* item = m_found->findItems(text, Qt::MatchExactly).value(0);
		if (item == 0) {
			item = m_found->addWord(text);
			delete m_missed->findItems(text, Qt::MatchExactly, 0).first();

			m_clock->addTime(item->data(0, Qt::UserRole).toInt() + 8);
			updateScore();

			QList<QList<QPoint> >& solutions = m_solutions[text];
			int index = solutions.indexOf(m_positions);
			if (index != -1) {
				solutions.move(index, 0);
			} else {
				solutions.prepend(m_positions);
			}
		}
		m_found->scrollToItem(item, QAbstractItemView::PositionAtCenter);
		m_found->clearSelection();

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
}

//-----------------------------------------------------------------------------

void Board::guessChanged() {
	m_valid = true;
	m_wrong = false;
	clearHighlight();
	m_found->clearSelection();

	QString word = m_guess->text().trimmed().toUpper();
	if (!word.isEmpty()) {
		int pos = m_guess->cursorPosition();
		m_guess->setText(word);
		m_guess->setCursorPosition(pos);

		QList<QList<QPoint> > solutions = m_solutions.value(word, Solver(word, m_letters).solutions().value(word));
		m_valid = !solutions.isEmpty();
		if (m_valid) {
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
				foreach (const QPoint& cell, solution) {
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
}

//-----------------------------------------------------------------------------

void Board::finish() {
	m_clock->setText((m_missed->topLevelItemCount() == 0 && m_found->topLevelItemCount() > 0) ? tr("Success") : tr("Game Over"));

	clearGuess();
	m_found->setColumnHidden(1, false);
	m_guess->setDisabled(true);
	m_clear_button->setDisabled(true);
	m_guess_button->setDisabled(true);
	m_guess->setEchoMode(QLineEdit::NoEcho);
	m_guess->releaseKeyboard();
	m_tabs->setTabEnabled(1, true);
	emit pauseAvailable(false);

	int score = updateScore();
	emit finished(score);
}

//-----------------------------------------------------------------------------

void Board::wordSelected() {
	QList<QTreeWidgetItem*> items;
	if (m_tabs->currentWidget() == m_missed) {
		items = m_missed->selectedItems();
	} else {
		items = m_found->selectedItems();
	}
	if (items.isEmpty()) {
		return;
	}

	QString word = items.first()->text(0);
	if (!word.isEmpty() && word != m_guess->text()) {
		m_guess->setText(word);
		m_positions = m_solutions.value(word).value(0);
		clearHighlight();
		updateClickableStatus();
		highlightWord();
	}
}

//-----------------------------------------------------------------------------

void Board::letterClicked(Letter* letter) {
	m_wrong = false;

	// Handle adding a letter to the guess
	if (!m_positions.contains(letter->position())) {
		QString word = m_guess->text().trimmed().toUpper();
		word.append(letter->text().toUpper());
		m_guess->setText(word);

		m_positions.append(letter->position());
		clearHighlight();
		updateClickableStatus();
		highlightWord();

		selectGuess();
	// Handle making or clearing a guess
	} else if (letter->position() == m_positions.last()) {
		if (m_positions.count() >= 3) {
			guess();
		} else if (m_positions.count() == 1) {
			clearGuess();
		}
	// Handle backing up in a guess
	} else {
		m_guess->clear();
		m_positions = m_positions.mid(0, m_positions.indexOf(letter->position()) + 1);
		clearHighlight();
		updateClickableStatus();

		QString word;
		foreach (const QPoint& position, m_positions) {
			word.append(m_cells[position.x()][position.y()]->text().toUpper());
		}
		m_guess->setText(word);
		highlightWord();

		selectGuess();
	}
}

//-----------------------------------------------------------------------------

void Board::highlightWord(const QList<QPoint>& positions, const QColor& color) {
	Q_ASSERT(!positions.isEmpty());

	m_cells[positions.at(0).x()][positions.at(0).y()]->setBrush(color);
	for (int i = 1; i < positions.count(); ++i) {
		const QPoint& pos1 = positions.at(i);
		m_cells[pos1.x()][pos1.y()]->setBrush(color);

		const QPoint& pos0 = m_positions.at(i - 1);
		QLineF line(pos0, pos1);
		m_cells[pos0.x()][pos0.y()]->setArrow(line.angle(), i - 1);
	}

	int alpha = 192 / positions.count();
	QColor border = Qt::white;
	for (int i = positions.count() - 1; i >= 0; --i) {
		const QPoint& position = positions.at(i);
		m_cells[position.x()][position.y()]->setPen(QPen(border, 2));
		border.setAlpha(border.alpha() - alpha);
	}
}

//-----------------------------------------------------------------------------

void Board::highlightWord() {
	QString guess = m_guess->text();
	if (guess.isEmpty()) {
		return;
	}

	QPalette p = palette();
	if (!m_wrong) {
		if (!m_valid) {
			p.setColor(m_guess->foregroundRole(), Qt::white);
			p.setColor(m_guess->backgroundRole(), Qt::red);
		} else if (!m_found->findItems(guess, Qt::MatchExactly).isEmpty()) {
			p.setColor(m_guess->foregroundRole(), Qt::white);
			p.setColor(m_guess->backgroundRole(), "#ffaa00");
			highlightWord(m_positions, "#ffaa00");
		} else {
			highlightWord(m_positions, "#a5c1e4");
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

void Board::clearHighlight() {
	QColor color = !isFinished() ? Qt::white : QColor("#dddddd");
	for (int c = 0; c < 4; ++c) {
		for (int r = 0; r < 4; ++r) {
			m_cells[c][r]->setBrush(color);
			m_cells[c][r]->setPen(Qt::NoPen);
			m_cells[c][r]->setArrow(-1, 0);
		}
	}
	m_guess->setPalette(palette());
}

//-----------------------------------------------------------------------------

void Board::selectGuess() {
	QTreeWidgetItem* item = m_found->findItems(m_guess->text(), Qt::MatchExactly, 0).value(0);
	if (item != 0) {
		m_found->setCurrentItem(item);
		m_found->scrollToItem(item, QAbstractItemView::PositionAtCenter);
	} else {
		m_found->clearSelection();
	}
}

//-----------------------------------------------------------------------------

int Board::updateScore() {
	int score = 0;
	for (int i = 0; i < m_found->topLevelItemCount(); ++i) {
		score += m_found->topLevelItem(i)->data(0, Qt::UserRole).toInt();
	}

	if (m_score_type == 2 || (m_score_type == 1 && isFinished())) {
		int max_score = score;
		for (int i = 0; i < m_missed->topLevelItemCount(); ++i) {
			max_score += m_missed->topLevelItem(i)->data(0, Qt::UserRole).toInt();
		}
		m_score->setText(tr("%1 of %n point(s)", "", max_score).arg(score));
	} else {
		m_score->setText(tr("%n point(s)", "", score));
	}
	return score;
}

//-----------------------------------------------------------------------------

void Board::updateClickableStatus() {
	bool finished = isFinished();
	bool has_word = !m_positions.isEmpty() && !finished;
	bool clickable = !has_word && !finished;

	for (int y = 0; y < 4; ++y) {
		for (int x = 0; x < 4; ++x) {
			m_cells[x][y]->setClickable(clickable);
		}
	}

	if (has_word && m_valid) {
		const QPoint& position = m_positions.last();
		int min_x = qMax(position.x() - 1, 0);
		int max_x = qMin(position.x() + 2, 4);
		int min_y = qMax(position.y() - 1, 0);
		int max_y = qMin(position.y() + 2, 4);
		for (int y = min_y; y < max_y; ++y) {
			for (int x = min_x; x < max_x; ++x) {
				m_cells[x][y]->setClickable(true);
			}
		}

		foreach (const QPoint& position, m_positions) {
			m_cells[position.x()][position.y()]->setClickable(true);
		}
	}
}

//-----------------------------------------------------------------------------
