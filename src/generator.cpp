/*
	SPDX-FileCopyrightText: 2009-2021 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "generator.h"

#include "clock.h"
#include "gzip.h"
#include "language_settings.h"
#include "solver.h"
#include "trie.h"

#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>

//-----------------------------------------------------------------------------

namespace
{

/**
 * @brief The State class represents a current game being built.
 */
struct State
{
	/**
	 * Constructs a state instance.
	 * @param dice the dice used to fill the board
	 * @param solver the solve used to check the board for solutions
	 * @param target the target amount of words
	 * @param random the random number generator
	 */
	State(const QList<QStringList>& dice, Solver* solver, int target, QRandomGenerator* random)
		: m_dice(dice)
		, m_solver(solver)
		, m_target(target)
		, m_random(random)
	{
	}

	/**
	 * @return how far off the amount of words on the board is from the target amount of words
	 */
	int delta() const
	{
		return m_delta;
	}

	/**
	 * @return the board layout
	 */
	QStringList letters() const
	{
		return m_letters;
	}

	/**
	 * Flips individual dice to tweak the board and searches it for solutions.
	 */
	void permute()
	{
		if (m_random->bounded(2)) {
			const int index = m_random->bounded(m_dice.count());
			QStringList& die = m_dice[index];
			std::shuffle(die.begin(), die.end(), *m_random);
			m_letters[index] = m_dice.at(index).first();
		} else {
			const int index1 = m_random->bounded(m_dice.count());
			const int index2 = m_random->bounded(m_dice.count());
#if (QT_VERSION >= QT_VERSION_CHECK(5,13,0))
			m_dice.swapItemsAt(index1, index2);
			m_letters.swapItemsAt(index1, index2);
#else
			m_dice.swap(index1, index2);
			m_letters.swap(index1, index2);
#endif
		}
		solve();
	}

	/**
	 * Rolls the dice to generate a new board and searches it for solutions.
	 */
	void roll()
	{
		std::shuffle(m_dice.begin(), m_dice.end(), *m_random);
		m_letters.clear();
		int count = m_dice.count();
		for (int i = 0; i < count; ++i) {
			QStringList& die = m_dice[i];
			std::shuffle(die.begin(), die.end(), *m_random);
			m_letters += die.first();
		}
		solve();
	}

private:
	/**
	 * Searches the board for solutions and determines how far off the amount of words is from the
	 * target amount of words.
	 */
	void solve()
	{
		m_solver->solve(m_letters);
		int words = m_solver->count();
		m_delta = abs(words - m_target);
	}

private:
	QList<QStringList> m_dice; /**< the dice used to generate a layout */
	QStringList m_letters; /**< the generated layout */
	Solver* m_solver; /**< solves the generated layout */
	int m_target; /**< the target number of words on generated board */
	int m_delta; /**< how far the actual number of words is from the target */
	QRandomGenerator* m_random; /**< random number generator */
};

}

//-----------------------------------------------------------------------------

Generator::Generator(QObject* parent)
	: QThread(parent)
	, m_random(QRandomGenerator::securelySeeded())
	, m_max_score(0)
	, m_canceled(false)
{
}

//-----------------------------------------------------------------------------

void Generator::cancel()
{
	blockSignals(true);
	m_canceled.store(true, std::memory_order_relaxed);
	wait();
	blockSignals(false);
}

//-----------------------------------------------------------------------------

void Generator::create(int density, int size, int minimum, int timer, const QStringList& letters)
{
	m_density = density;
	m_size = size;
	m_minimum = minimum;
	m_timer = timer;
	m_max_words = (m_timer != Clock::Allotment) ? -1 : 30;
	m_letters = letters;
	m_canceled.store(false, std::memory_order_relaxed);
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

	if (m_density == 3) {
		m_density = m_random.bounded(0, 3);
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
	State current(dice(m_size), &solver, words_target, &m_random);
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
	} while (!m_canceled.load(std::memory_order_relaxed) && (current.delta() > words_range));

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

	QSettings config;
	config.beginGroup("Current");
	LanguageSettings settings(config);
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
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
			stream.setCodec("UTF-8");
#endif
			while (!stream.atEnd()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
				const QStringList line = stream.readLine().split(',', Qt::SkipEmptyParts);
#else
				const QStringList line = stream.readLine().split(',', QString::SkipEmptyParts);
#endif
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

		// Load cached words
		constexpr quint32 TANGLET_CACHE_MAGICNUMBER = 0x54524945;
		constexpr quint32 TANGLET_CACHE_VERSION = 2;
		QString cache_dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/cache";
		QString cache_file = QCryptographicHash::hash(words_path.toUtf8(), QCryptographicHash::Sha1).toHex();
		QFileInfo cache_info(cache_dir + "/" + cache_file);
		if (cache_info.exists() && (cache_info.lastModified() > QFileInfo(words_path).lastModified())) {
			QFile file(cache_info.absoluteFilePath());
			if (file.open(QFile::ReadOnly)) {
				QDataStream stream(&file);
				quint32 magic, version;
				stream >> magic >> version;
				if ((magic == TANGLET_CACHE_MAGICNUMBER) && (version == TANGLET_CACHE_VERSION)) {
					stream.setVersion(QDataStream::Qt_5_9);
					stream >> m_words;
				}
				file.close();
			}
		}

		// Load uncached words
		if (m_words.isEmpty()) {
			emit optimizingStarted();

			m_words = Trie(gunzip(words_path));

			// Cache words
			if (!m_words.isEmpty()) {
				QDir::home().mkpath(cache_dir);
				QFile file(cache_info.absoluteFilePath());
				if (file.open(QFile::WriteOnly)) {
					QDataStream stream(&file);
					stream << TANGLET_CACHE_MAGICNUMBER;
					stream << TANGLET_CACHE_VERSION;
					stream.setVersion(QDataStream::Qt_5_9);
					stream << m_words;
					file.close();
				}
			}

			emit optimizingFinished();
		}

		if (!m_words.isEmpty()) {
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
