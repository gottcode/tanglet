#!/usr/bin/env python3
#
# Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

"""words.py: Create Tanglet word list from a list of words."""


import argparse
from collections import defaultdict


def replacement_map(language):
    """Return a dict of replacement strings for language."""
    if language == 'cs':
        return {ord('Á'): 'A',
                ord('É'): 'E',
                ord('Ě'): 'E',
                ord('Í'): 'I',
                ord('Ó'): 'O',
                ord('Ú'): 'U',
                ord('Ů'): 'U',
                ord('Ý'): 'Y',
                ord('Č'): 'C',
                ord('Ď'): 'D',
                ord('Ň'): 'N',
                ord('Ř'): 'R',
                ord('Š'): 'S',
                ord('Ť'): 'T',
                ord('Ž'): 'Z'}
    elif language == 'de':
        return {ord('Ä'): 'AE',
                ord('Â'): 'A',
                ord('É'): 'E',
                ord('Ö'): 'OE',
                ord('Ü'): 'UE',
                ord('Ñ'): 'N',
                ord('ß'): 'SS'}
    elif language == 'en':
        return {ord('À'): 'A',
                ord('Á'): 'A',
                ord('Â'): 'A',
                ord('Ä'): 'A',
                ord('Å'): 'A',
                ord('Ç'): 'C',
                ord('È'): 'E',
                ord('É'): 'E',
                ord('Ê'): 'E',
                ord('Ï'): 'I',
                ord('Ñ'): 'N',
                ord('Ó'): 'O',
                ord('Ö'): 'O',
                ord('Û'): 'U',
                ord('Ü'): 'U'}
    elif language == 'fr':
        return {ord('À'): 'A',
                ord('Â'): 'A',
                ord('Æ'): 'A',
                ord('É'): 'E',
                ord('È'): 'E',
                ord('Ê'): 'E',
                ord('Ë'): 'E',
                ord('Î'): 'I',
                ord('Ï'): 'I',
                ord('Ô'): 'O',
                ord('Œ'): 'O',
                ord('Ù'): 'U',
                ord('Û'): 'U',
                ord('Ü'): 'U',
                ord('Ÿ'): 'Y',
                ord('Ç'): 'C'}
    elif language == 'he':
        return {ord('ך'): 'כ',
                ord('ם'): 'מ',
                ord('ן'): 'נ',
                ord('ף'): 'פ',
                ord('ץ'): 'צ'}
    elif language == 'nl':
        return {ord('À'): 'A',
                ord('Ä'): 'A',
                ord('È'): 'E',
                ord('É'): 'E',
                ord('Ê'): 'E',
                ord('Ë'): 'E',
                ord('Î'): 'I',
                ord('Ï'): 'I',
                ord('Ô'): 'O',
                ord('Ö'): 'O',
                ord('Û'): 'U',
                ord('Ü'): 'U',
                ord('Ç'): 'C',
                ord('Ñ'): 'N'}
    else:
        return {}


def main():
    parser = argparse.ArgumentParser(
            description='Create Tanglet word list from a list of words')
    parser.add_argument('FILE',
            help='file containing list of words')
    parser.add_argument('--language',
            help='specify language', type=str,
            choices=['cs', 'de', 'en', 'fr', 'he', 'nl'])
    parser.add_argument('-o', '--out', type=str,
            help='place words in file OUT instead of default file')
    args = parser.parse_args()

    # Read words and replace letters
    replacements = replacement_map(args.language)
    words = defaultdict(list)
    with open(args.FILE, encoding='utf-8', mode='r') as f:
        for word in f:
            word = word.strip()
            if len(word) < 3:
                continue
            key = word.upper().translate(replacements)
            if not word in words[key]:
                words[key].append(word)

    # Join words into lines
    lines = []
    for solution, spellings in words.items():
        if (len(spellings) == 1) and (spellings[0] == solution.lower()):
            # Doesn't need to store proper spellings in this case;
            # Tanglet will fall back to all lowercase of solution.
            lines.append(solution + '\n')
        else:
            # Store solution and proper spellings.
            lines.append(solution + ' ' + ' '.join(spellings) + '\n')
    lines.sort()

    # Save lines to disk
    outf = 'words'
    if args.out:
        outf = args.out
    with open(outf, encoding='utf-8', mode='w') as f:
        for line in lines:
            f.write(line)
    print("Words generated and placed in file '{0}'".format(outf))


if __name__ == '__main__':
    main()
