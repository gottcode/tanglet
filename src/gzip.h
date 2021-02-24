/*
	SPDX-FileCopyrightText: 2011 Graeme Gott <graeme@gottcode.org>

	SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef TANGLET_GZIP_H
#define TANGLET_GZIP_H

class QByteArray;
class QString;

/**
 * Compress the file located at path with the Gzip algorithm.
 * @param path file to compress
 */
void gzip(const QString& path);

/**
 * Decompress the file located at path with the Gzip algorithm.
 * @param path file to decompress
 * @return uncompressed file contents
 */
QByteArray gunzip(const QString& path);

#endif // TANGLET_GZIP_H
