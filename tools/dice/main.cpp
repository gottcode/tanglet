/***********************************************************************
 *
 * Copyright (C) 2012, 2017 Graeme Gott <graeme@gottcode.org>
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

#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QTextStream>

#include <zlib.h>

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <vector>

//-----------------------------------------------------------------------------

class Exception : public std::runtime_error
{
public:
	explicit Exception(const QString& what_arg) : runtime_error(what_arg.toStdString()) { }
	explicit Exception(const std::string& what_arg) : runtime_error(what_arg) { }
	explicit Exception(const char* what_arg) : runtime_error(what_arg) { }
};

namespace std
{
	template <>
	struct hash<QChar>
	{
		size_t operator()(const QChar& k) const
		{
			return qHash(k);
		}
	};

#if (QT_VERSION < QT_VERSION_CHECK(5,14,0))
	template <>
	struct hash<QString>
	{
		size_t operator()(const QString& k) const
		{
			return qHash(k);
		}
	};
#endif
}

//-----------------------------------------------------------------------------

std::vector<QString> readWords(const QString& path)
{
	QByteArray data;

	// Open file
	QFile file(path);
	if (!file.open(QFile::ReadOnly)) {
		throw Exception("Unable to open file '" + path +"' for reading.");
	}
	gzFile gz = gzdopen(file.handle(), "rb");
	if (!gz) {
		throw Exception("Unable to open file '" + path + "' for decompressing.");
	}

	// Decompress file
	QByteArray buffer(0x40000, 0);
	int read = 0;
	do {
		data.append(buffer.constData(), read);
		read = gzread(gz, buffer.data(), buffer.size());
		if (read == -1) {
			throw Exception("Error while reading file '" + path + "'.");
		}
	} while (read > 0);
	gzclose(gz);

	// Find words
	std::vector<QString> words;
	QTextStream stream(&data);
	stream.setCodec("UTF-8");
	while (!stream.atEnd()) {
		words.push_back(stream.readLine().trimmed().split(' ').first().toUpper());
	}

	return words;
}

//-----------------------------------------------------------------------------

void saveDice(const QString& path, const std::vector<QString>& small, const std::vector<QString>& large)
{
	QFile out(path);
	if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
		throw Exception("Unable to open file '" + path + "' for writing.");
	}

	QTextStream stream(&out);
	stream.setCodec("UTF-8");

	for (const QString& line : small) {
		stream << line << '\n';
	}

	stream << '\n';

	for (const QString& line : large) {
		stream << line << '\n';
	}

	out.close();
}

//-----------------------------------------------------------------------------

std::unordered_map<QString, qreal> findLetterFrequency(const std::vector<QString>& words, bool use_bigrams, bool discard_infrequent)
{
	std::unordered_map<QString, int> letters;

	// Find counts of letters
	int total = 0;
	{
		std::unordered_map<QChar, int> chars;
		for (const auto& word : words) {
			for (const auto& c : word) {
				++total;
				++chars[c];
			}
		}
		for (auto i = chars.cbegin(); i != chars.cend(); ++i) {
			letters.emplace(i->first, i->second);
		}
	}

	// Discard letters that occur less than 0.001%
	if (discard_infrequent) {
		for (auto i = letters.begin(); i != letters.end(); ++i) {
			const qreal probability = (i->second * 100.0) / total;
			if (probability < 0.001) {
				total -= i->second;
				std::cout << QString("Discarded '%1', probability: %2%")
					.arg(i->first)
					.arg(probability, 0, 'f')
					.toStdString()
					<< std::endl;
				i = letters.erase(i);
				if (i == letters.end()) {
					break;
				}
			}
		}
	}

	if (use_bigrams) {
		// Find counts of bigrams
		std::unordered_map<QString, int> bigrams;
		QString bigram(2, '\0');
		for (const auto& word : words) {
			bigram[0] = word[0];
			for (int i = 1, end = word.length(); i < end; ++i) {
				const QChar c = word[i];
				bigram[1] = c;
				++bigrams[bigram];
				bigram[0] = c;
			}
		}

		// Find letters where bigrams are 99% of occurrences
		std::unordered_map<QString, QString> replace;
		std::unordered_map<QString, int> confidences;
		for (auto i = letters.cbegin(); i != letters.cend(); ++i) {
			const QString& letter = i->first;

			int letter_count = 0;
			std::vector<QString> letter_bigrams;
			for (auto b = bigrams.cbegin(); b != bigrams.cend(); ++b) {
				const QString& bigram = b->first;
				if (bigram.startsWith(letter)) {
					letter_bigrams.push_back(bigram);
					letter_count += b->second;
				}
			}

			for (const auto& bigram : letter_bigrams) {
				const qreal confidence = qreal(bigrams[bigram] * 100) / qreal(letter_count);
				if (confidence >= 99) {
					replace[letter] = bigram;
					confidences[letter] = std::round(confidence);
					break;
				}
			}
		}

		// Replace letter with bigram
		for (auto i = replace.cbegin(); i != replace.cend(); ++i) {
			const QString& letter = i->first;
			const QString& bigram = i->second;
			const int count = bigrams[bigram];

			letters[letter + bigram[1].toLower()] = count;
			letters[bigram[1]] -= count;

			letters.erase(letter);
			total -= count;

			std::cout << QString("Replaced '%1' with '%2', confidence: %3%")
				.arg(letter)
				.arg(letter + bigram[1].toLower())
				.arg(confidences[letter])
				.toStdString()
				<< std::endl;
		}
	}

	// Adjust letter frequencies to be in the range 0-1
	std::unordered_map<QString, qreal> result;
	const qreal inverse_total = 1.0 / qreal(total);
	for (auto i = letters.cbegin(); i != letters.cend(); ++i) {
		result.emplace(i->first, i->second * inverse_total);
	}
	return result;
}

//-----------------------------------------------------------------------------

std::unordered_map<QString, int> roundLetters(const std::unordered_map<QString, qreal>& letters, int count)
{
	const int sides = count * 6;

	// Scale letters by dice sides
	std::unordered_map<QString, qreal> scaled;
	std::vector<QString> frequent;
	for (auto i = letters.cbegin(); i != letters.cend(); ++i) {
		qreal value = i->second * sides;
		if (value < 1.0) {
			value = 1.0;
		}
		const QString& letter = i->first;
		scaled[letter] = value;

		// Track letter frequency
		auto f = frequent.begin();
		for (; f != frequent.end(); ++f) {
			if (value > scaled[*f]) {
				break;
			}
		}
		frequent.insert(f, letter);
	}

	// Round letters
	std::unordered_map<QString, int> result;
	int rounded = 0;
	for (const auto& letter : frequent) {
		const int value = std::round(scaled[letter]);
		result[letter] = value;
		rounded += value;
	}
	if (rounded < sides) {
		throw Exception("Rounded frequencies are less than dice sides.");
	}

	// Reduce most frequent letters so that the letter count matches sides
	rounded -= sides;
	for (int i = 0; i < rounded; ++i) {
		if (--result[frequent[i]] == 0) {
			throw Exception("'" + frequent[i] + "' has frequency of 0.");
		}
	}

	return result;
}

//-----------------------------------------------------------------------------

std::vector<QString> generateDice(const std::unordered_map<QString, qreal>& letters, int count, std::mt19937& random)
{
	std::vector<std::vector<QString>> dice(count);

	// Find groups of letters
	const auto scaled_letters = roundLetters(letters, count);
	std::vector<QString> single_letters;
	std::vector<QString> multi_letters;
	for (auto i = scaled_letters.cbegin(); i != scaled_letters.cend(); ++i) {
		const QString& letter = i->first;
		const int& value = i->second;
		if (value == 1) {
			single_letters.push_back(letter);
		} else {
			multi_letters.insert(multi_letters.end(), value, letter);
		}
	}

	// Add single-occurrence letters to dice first
	std::shuffle(single_letters.begin(), single_letters.end(), random);
	int dice_i = 0;
	for (const auto& letter : single_letters) {
		dice[dice_i].push_back(letter);
		++dice_i;
		if (dice_i >= count) {
			dice_i = 0;
		}
	}

	// Add multiple-occurrence letters to dice
	std::shuffle(multi_letters.begin(), multi_letters.end(), random);
	int pos = 0;
	for (auto& i : dice) {
		const int size = 6 - i.size();
		if (size > 0) {
			const auto m = multi_letters.cbegin() + pos;
			i.insert(i.end(), m, m + size);
			pos += size;
		}
	}

	// Alphabetize dice
	std::vector<QString> result;
	for (auto& i : dice) {
		std::sort(i.begin(), i.end());
		result.push_back(i[0] + ',' + i[1] + ',' + i[2] + ',' + i[3] + ',' + i[4] + ',' + i[5]);
	}
	std::sort(result.begin(), result.end());

	return result;
}

//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);

	try {
		QCommandLineParser parser;
		parser.setApplicationDescription("Create Tanglet dice from a Tanglet word list.");
		parser.addHelpOption();
		parser.addOption(QCommandLineOption({"b", "bigrams"},
			"Automatically detect bigrams."));
		parser.addOption(QCommandLineOption({"d", "discard"},
			"Discard infreqeunt letters."));
		parser.addOption(QCommandLineOption({"o", "output"},
			"Place dice in <output> instead of default file.", "output"));
		parser.addOption(QCommandLineOption({"s", "seed"},
			"Specify random <seed>.", "seed"));
		parser.addOption(QCommandLineOption({"v", "verbose"},
			"Print status messages."));
		parser.addPositionalArgument("file", "The <file> to analyze for letter frequency.");
		parser.process(app);

		QString filename;
		const QStringList files = parser.positionalArguments();
		if (files.isEmpty()) {
			parser.showHelp();
		}
		if (files.size() > 1) {
			throw Exception("Multiple 'file' arguments specified.");
		}
		filename = files.first();

		const bool use_bigrams = parser.isSet("bigrams");

		const bool discard_infrequent = parser.isSet("discard");

		QString outfilename = "dice";
		if (parser.isSet("output")) {
			outfilename = parser.value("output");
		}

		uint64_t seed = 0x54414e47;
		if (parser.isSet("seed")) {
			seed = parser.value("seed").toULongLong();
		}
		std::mt19937 random(seed);

		if (!parser.isSet("verbose")) {
			std::cout.setstate(std::ios::failbit);
		}

		// Read lines from file
		const auto words = readWords(filename);

		// Create dice
		const auto letters = findLetterFrequency(words, use_bigrams, discard_infrequent);
		const auto small = generateDice(letters, 16, random);
		const auto large = generateDice(letters, 25, random);

		// Save dice to disk
		saveDice(outfilename, small, large);
	} catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		return -1;
	}
}

//-----------------------------------------------------------------------------
