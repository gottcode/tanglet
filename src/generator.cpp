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

#include "generator.h"

#include "clock.h"
#include "language_settings.h"
#include "random.h"
#include "solver.h"

#include <QFile>
#include <QTextStream>

#include <zlib.h>

//-----------------------------------------------------------------------------

namespace
{
	QByteArray gunzip(const QFile& file)
	{
		QByteArray data;

		gzFile gz = gzdopen(file.handle(), "rb");
		if (gz == NULL) {
			return data;
		}

		int read = 0;
		char buffer[1024];
		memset(buffer, 0, 1024);
		do {
			data.append(buffer, read);
			read = qMin(gzread(gz, buffer, 1024), 1024);
		} while (read > 0);
		gzclose(gz);

		return data;
	}

	struct State
	{
		State(const QList<QStringList>& dice, Solver* solver, int target, Random* random)
			: m_dice(dice), m_solver(solver), m_target(target), m_random(random)
		{
		}

		int delta() const
		{
			return m_delta;
		}

		QStringList letters() const
		{
			return m_letters;
		}

		void permute()
		{
			if (m_random->nextInt(2) == 1) {
				int index = m_random->nextInt(m_dice.count());
				m_random->shuffle(m_dice[index]);
				m_letters[index] = m_dice.at(index).first();
			} else {
				int index1 = m_random->nextInt(m_dice.count());
				int index2 = m_random->nextInt(m_dice.count());
				m_dice.swap(index1, index2);
				m_letters.swap(index1, index2);
			}
			solve();
		}

		void roll()
		{
			m_random->shuffle(m_dice);
			m_letters.clear();
			int count = m_dice.count();
			for (int i = 0; i < count; ++i) {
				QStringList& die = m_dice[i];
				m_random->shuffle(die);
				m_letters += die.first();
			}
			solve();
		}

	private:
		void solve()
		{
			m_solver->solve(m_letters);
			int words = m_solver->count();
			m_delta = abs(words - m_target);
		}

	private:
		QList<QStringList> m_dice;
		QStringList m_letters;
		int m_delta;
		Solver* m_solver;
		int m_target;
		Random* m_random;
	};
}

//-----------------------------------------------------------------------------

Generator::Generator(QObject* parent)
	: QThread(parent), m_max_score(0)
{
}

//-----------------------------------------------------------------------------

void Generator::cancel()
{
	blockSignals(true);
	m_cancelled = true;
	wait();
	blockSignals(false);
}

//-----------------------------------------------------------------------------

void Generator::create(int density, int size, int minimum, int timer, const QStringList& letters, unsigned int seed)
{
	m_density = density;
	m_size = size;
	m_minimum = minimum;
	m_timer = timer;
	m_max_words = (m_timer != Clock::Allotment) ? -1 : 30;
	m_letters = letters;
	m_seed = seed;
	m_cancelled = false;
	m_max_score = 0;
	m_solutions.clear();
	start();
}

//-----------------------------------------------------------------------------

void Generator::run()
{
	update();
	if (!m_error.isEmpty()) {
		return;
	}

	// Store solutions for loaded board
	Solver solver(m_words, m_size, m_minimum);
	if (!m_letters.isEmpty()) {
		solver.solve(m_letters);
		m_max_score = solver.score(m_max_words);
		m_solutions = solver.solutions();
		return;
	}

	Random random(m_seed);
	if (m_density == 3) {
		m_density = random.nextInt(3);
	}

	// Find word range
	int offset = ((m_size == 4) ? 6 : 7) - m_minimum;
	int words_target = 0, words_range = 0;
	switch (m_density) {
	case 0:
		words_target = 37;
		words_range = 5;
		break;
	case 1:
		words_target = 150 + (25 * offset);
		words_range = 25;
		break;
	case 2:
		words_target = 250 + (75 * offset);
		words_range = 50;
		break;
	default:
		break;
	}

	// Create board state
	solver.setTrackPositions(false);
	State current(dice(m_size), &solver, words_target, &random);
	current.roll();
	State next = current;

	int max_tries = m_size * m_size * 2;
	int tries = 0;
	int loops = 0;
	do {
		// Change the board
		next = current;
		next.permute();

		// Check if this is a better board
		if (next.delta() < current.delta()) {
			current = next;
			tries = 0;
			loops = 0;
		}

		// Prevent getting stuck at local minimum
		tries++;
		if (tries == max_tries) {
			current = next;
			tries = 0;
			loops++;

			// Restart if still stuck at local minimum
			if (loops == m_size) {
				current.roll();
				loops = 0;
			}
		}
	} while (!m_cancelled && (current.delta() > words_range));

	// Store solutions for generated board
	m_letters = current.letters();
	solver.setTrackPositions(true);
	solver.solve(m_letters);
	m_max_score = solver.score(m_max_words);
	m_solutions = solver.solutions();
}

//-----------------------------------------------------------------------------

void Generator::update()
{
	m_error.clear();

	LanguageSettings settings("Current");
	m_dictionary_url = settings.dictionary();

	// Load dice
	QString dice_path = settings.dice();
	if (dice_path != m_dice_path) {
		m_dice_path.clear();
		m_dice.clear();
		m_dice_large.clear();

		QList<QStringList> dice;
		QFile file(dice_path);
		if (file.open(QFile::ReadOnly | QIODevice::Text)) {
			QTextStream stream(&file);
			stream.setCodec("UTF-8");
			while (!stream.atEnd()) {
				QStringList line = stream.readLine().split(',', QString::SkipEmptyParts);
				if (line.count() == 6) {
					dice.append(line);
				}
			}
			file.close();
		}

		if (dice.count() == 41) {
			m_dice_path = dice_path;
			m_dice = dice.mid(0, 16);
			m_dice_large = dice.mid(16);
		} else {
			m_dice = m_dice_large = QList<QStringList>() << QStringList("?");
			return setError(tr("Unable to read dice from file."));
		}
	}

	// Load words
	QString words_path = settings.words();
	if (words_path != m_words_path) {
		m_words_path.clear();
		m_words.clear();

		int count = 0;
		QFile file(words_path);
		if (file.open(QFile::ReadOnly)) {
			QByteArray data = gunzip(file);
			file.close();

			QTextStream stream(&data);
			stream.setCodec("UTF-8");
			while (!stream.atEnd()) {
				QString line = stream.readLine().toUpper();
				if (line.length() >= 3 && line.length() <= 25) {
					m_words.addWord(line);
					count++;
				}
			}
		}

		if (count > 0) {
			m_words_path = words_path;
		} else {
			return setError(tr("Unable to read word list from file."));
		}
	}
}

//-----------------------------------------------------------------------------

void Generator::setError(const QString& error)
{
	m_error = error;
	m_letters.clear();
	int count = m_size * m_size;
	for (int i = 0; i < count; ++i) {
		m_letters.append("?");
	}
}

//-----------------------------------------------------------------------------
