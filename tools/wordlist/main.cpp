/*
	SPDX-FileCopyrightText: 2013-2020 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QBuffer>
#include <QByteArray>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QTextStream>

#include <zlib.h>

#include <algorithm>
#include <functional>
#include <map>
#include <iostream>
#include <stdexcept>
#include <vector>

//-----------------------------------------------------------------------------

class Exception : public std::runtime_error
{
public:
	explicit Exception(const QString& what_arg) : runtime_error(what_arg.toStdString()) { }
	explicit Exception(const std::string& what_arg) : runtime_error(what_arg) { }
	explicit Exception(const char* what_arg) : runtime_error(what_arg) { }
};

//-----------------------------------------------------------------------------

QString generateSolutionDe(QString string)
{
	string.replace(u'Ä', "AE");
	string.replace(u'Ö', "OE");
	string.replace(u'Ü', "UE");
	string.replace(u'Œ', "OE");
	string.replace(u'À', 'A');
	string.replace(u'Á', 'A');
	string.replace(u'Â', 'A');
	string.replace(u'Å', 'A');
	string.replace(u'Ç', 'C');
	string.replace(u'Č', 'C');
	string.replace(u'É', 'E');
	string.replace(u'È', 'E');
	string.replace(u'Ê', 'E');
	string.replace(u'Ē', 'E');
	string.replace(u'Ë', 'E');
	string.replace(u'Ī', 'I');
	string.replace(u'Í', 'I');
	string.replace(u'Ï', 'I');
	string.replace(u'Î', 'I');
	string.replace(u'Ł', 'L');
	string.replace(u'Ñ', 'N');
	string.replace(u'Ō', 'O');
	string.replace(u'Ó', 'O');
	string.replace(u'Õ', 'O');
	string.replace(u'Š', 'S');
	string.replace(u'Ū', 'U');
	string.replace(u'Û', 'U');
	string.replace(u'Ú', 'U');
	return string;
}

QString generateSolutionEn(QString string)
{
	string.replace(u'À', 'A');
	string.replace(u'Á', 'A');
	string.replace(u'Â', 'A');
	string.replace(u'Ä', 'A');
	string.replace(u'Å', 'A');
	string.replace(u'Ç', 'C');
	string.replace(u'È', 'E');
	string.replace(u'É', 'E');
	string.replace(u'Ê', 'E');
	string.replace(u'Ï', 'I');
	string.replace(u'Ñ', 'N');
	string.replace(u'Ó', 'O');
	string.replace(u'Ö', 'O');
	string.replace(u'Ø', 'O');
	string.replace(u'Û', 'U');
	string.replace(u'Ü', 'U');
	return string;
}

QString generateSolutionFr(QString string)
{
	string.replace(u'À', 'A');
	string.replace(u'Â', 'A');
	string.replace(u'Æ', 'A');
	string.replace(u'É', 'E');
	string.replace(u'È', 'E');
	string.replace(u'Ê', 'E');
	string.replace(u'Ë', 'E');
	string.replace(u'Î', 'I');
	string.replace(u'Ï', 'I');
	string.replace(u'Ô', 'O');
	string.replace(u'Œ', 'O');
	string.replace(u'Ù', 'U');
	string.replace(u'Û', 'U');
	string.replace(u'Ü', 'U');
	string.replace(u'Ÿ', 'Y');
	string.replace(u'Ç', 'C');
	return string;
}

QString generateSolutionHe(QString string)
{
	string.replace(u'ך', u'כ');
	string.replace(u'ם', u'מ');
	string.replace(u'ן', u'נ');
	string.replace(u'ף', u'פ');
	string.replace(u'ץ', u'צ');
	return string;
}

QString generateSolutionNl(QString string)
{
	string.replace(u'À', 'A');
	string.replace(u'Ä', 'A');
	string.replace(u'È', 'E');
	string.replace(u'É', 'E');
	string.replace(u'Ê', 'E');
	string.replace(u'Ë', 'E');
	string.replace(u'Î', 'I');
	string.replace(u'Ï', 'I');
	string.replace(u'Ô', 'O');
	string.replace(u'Ö', 'O');
	string.replace(u'Û', 'U');
	string.replace(u'Ü', 'U');
	string.replace(u'Ç', 'C');
	string.replace(u'Ñ', 'N');
	return string;
}

QString generateSolutionNone(QString string)
{
	return string;
}

std::function<QString(QString)> fetchGenerateSolution(const QString& language)
{
	if (language == "de") {
		return generateSolutionDe;
	} else if (language == "en") {
		return generateSolutionEn;
	} else if (language == "fr") {
		return generateSolutionFr;
	} else if (language == "he") {
		return generateSolutionHe;
	} else if (language == "nl") {
		return generateSolutionNl;
	} else {
		return generateSolutionNone;
	}
}

//-----------------------------------------------------------------------------

std::map<QString, std::vector<QString>> readWords(const QString& filename, const QString& language)
{
	QFile in(filename);
	if (!in.open(QIODevice::ReadOnly | QIODevice::Text)) {
		throw Exception("Unable to open file '" + filename + "' for reading.");
	}

	const auto generateSolution = fetchGenerateSolution(language);

	QTextStream stream(&in);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	stream.setCodec("UTF-8");
#endif

	std::map<QString, std::vector<QString>> words;
	while (!stream.atEnd()) {
		const QString word = stream.readLine().trimmed();

		// Replace letters
		const QString solution = generateSolution(word.toUpper());

		// Discard words that are too short or too long
		if ((solution.length() < 3) || (solution.length() > 25)) {
			continue;
		}

		// Store unique spellings
		auto& spellings = words[solution];
		if (std::find(spellings.cbegin(), spellings.cend(), word) == spellings.cend()) {
			spellings.push_back(word);
		}
	}

	in.close();

	return words;
}

//-----------------------------------------------------------------------------

QByteArray joinWordsIntoLines(const std::map<QString, std::vector<QString>>& words)
{
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly | QIODevice::Text);

	QTextStream stream(&buffer);
#if (QT_VERSION < QT_VERSION_CHECK(6,0,0))
	stream.setCodec("UTF-8");
#endif

	for (const auto& i : words) {
		const auto& solution = i.first;
		const auto& spellings = i.second;
		if ((spellings.size() == 1) && (spellings[0] == solution.toLower())) {
			// Doesn't need to store proper spellings in this case;
			// Tanglet will fall back to all lowercase of solution.
			stream << solution << '\n';
		} else {
			// Store solution and proper spellings.
			stream << solution;
			for (const auto& spelling : spellings) {
				stream << ' ' << spelling;
			}
			stream << '\n';
		}
	}

	buffer.close();

	return buffer.data();
}

//-----------------------------------------------------------------------------

void writeLines(const QString& filename, const QByteArray& lines)
{
	QFile out(filename);
	if (!out.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		throw Exception("Unable to open file '" + filename + "' for writing.");
	}

	gzFile gz = gzdopen(out.handle(), "wb9");
	if (!gz) {
		throw Exception("Unable to open file '" + filename + "' for compression.");
	}

	gzwrite(gz, lines.constData(), lines.size());
	gzclose(gz);

	out.close();
}

//-----------------------------------------------------------------------------

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);

	try {
		QCommandLineParser parser;
		parser.setApplicationDescription("Create Tanglet word list from a list of words.");
		parser.addHelpOption();
		parser.addOption(QCommandLineOption({"l", "language"}, "Specify <language>.", "language"));
		parser.addOption(QCommandLineOption({"o", "output"}, "Place words in <output> instead of default file.", "output"));
		parser.addPositionalArgument("file", "The <file> containing list of words.");
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

		QString language = filename.section('/', -1);
		if (parser.isSet("language")) {
			language = parser.value("language");
		}

		QString outfilename = "words";
		if (parser.isSet("output")) {
			outfilename = parser.value("output");
		}

		const auto words = readWords(filename, language);
		const auto lines = joinWordsIntoLines(words);
		writeLines(outfilename, lines);
	} catch (const std::exception& err) {
		std::cerr << err.what() << std::endl;
		return -1;
	}
}

//-----------------------------------------------------------------------------
