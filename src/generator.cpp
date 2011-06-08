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

namespace {
	QByteArray gunzip(const QFile& file) {
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
}

//-----------------------------------------------------------------------------

Generator::Generator(QObject* parent)
	: QThread(parent), m_max_score(0) {
}

//-----------------------------------------------------------------------------

void Generator::cancel() {
	blockSignals(true);
	m_cancelled = true;
	wait();
	blockSignals(false);
}

//-----------------------------------------------------------------------------

void Generator::create(bool higher_scores, int size, int timer, const QStringList& letters, unsigned int seed) {
	m_higher_scores = higher_scores;
	m_size = size;
	m_minimum = size - 1;
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

void Generator::run() {
	update();
	if (!m_error.isEmpty()) {
		return;
	}

	bool shuffle = m_letters.isEmpty();
	Random random(m_seed);
	QList<QStringList> dice = this->dice(m_size);
	while (!m_cancelled) {
		if (shuffle) {
			m_letters.clear();
			random.shuffle(dice);
			for (int i = 0; i < dice.count(); ++i) {
				QStringList& die = dice[i];
				random.shuffle(die);
				m_letters += die.at(0);
			}
		}

		Solver solver(m_words, m_letters, m_minimum);
		m_max_score = solver.score(m_max_words);
		if (!m_higher_scores || (m_max_score >= 200)) {
			m_solutions = solver.solutions();
			break;
		}
	}
}

//-----------------------------------------------------------------------------

void Generator::update() {
	m_error.clear();

	LanguageSettings settings;
	m_dictionary_url = settings.dictionary();
	m_dictionary_query.clear();
	int index = m_dictionary_url.indexOf("%s");
	if (index != -1) {
		int start_index = m_dictionary_url.lastIndexOf('&', index);
		if (start_index != -1) {
			int length = index - start_index;
			m_dictionary_query = m_dictionary_url.mid(start_index + 1, length - 2);
			m_dictionary_url.remove(start_index, length + 2);
		}
	}

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

void Generator::setError(const QString& error) {
	m_error = error;
	m_letters.clear();
	int count = m_size * m_size;
	for (int i = 0; i < count; ++i) {
		m_letters.append("?");
	}
}

//-----------------------------------------------------------------------------
