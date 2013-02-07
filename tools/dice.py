#!/usr/bin/env python3
#
# Copyright (C) 2012 Graeme Gott <graeme@gottcode.org>
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

"""dice.py: Create Tanglet dice from a Tanglet word list."""


import argparse
import codecs
from collections import defaultdict
import gzip
import random
import sys


def find_letter_frequency(words,
        use_bigrams = False,
        discard_infrequent = False):
    """Scan a list of words and return a dict of each letter's frequency."""
    total = 0
    letters = defaultdict(int)
    words = [word.split(' ',1)[0].strip().upper() for word in words]

    # Find counts of letters
    for word in words:
        for c in word:
            total += 1
            letters[c] += 1

    # Discard letters that occur less than 0.001%
    if discard_infrequent:
        for k in list(letters):
            probability = float(letters[k] * 100) / float(total)
            if round(probability, 3) < 0.001:
                total -= letters[k]
                del letters[k]
                print("Discarded '{0}', probability: {1:f}%"
                        .format(k, probability))

    if use_bigrams:
        # Find counts of bigrams
        bigrams = defaultdict(int)
        b = ''
        for word in words:
            for c in word:
                b += c
                bigrams[b] += 1
                b = c
            b = ''

        # Find letters where bigrams are 99% of occurrences
        replace = dict()
        confidences = dict()
        for k in letters.keys():
            letter_count = 0
            letter_bigrams = []
            for b in bigrams:
                if len(b) > len(k) and b.startswith(k):
                    letter_bigrams.append(b)
                    letter_count += bigrams[b]

            for b in letter_bigrams:
                confidence = float(bigrams[b] * 100) / float(letter_count)
                if confidence >= 99:
                    replace[k] = b
                    confidences[k] = round(confidence)
                    break;

        # Replace letter with bigram
        for k, b in replace.items():
            letters[k + b[1:].lower()] = bigrams[b]
            letters[b[1:]] -= bigrams[b]
            total -= letters[k]
            del letters[k]
            print("Replaced '{0}' with '{1}', confidence: {2}%"
                    .format(k, b[0:1] + b[1:].lower(), confidences[k]))

    # Adjust letter frequencies to be in the range 0-1
    for k, v in letters.items():
        letters[k] = float(v) / float(total)

    return letters


def round_letters(letters, count):
    """Return a copy of a letter frequency dict scaled by dice count."""
    sides = count * 6

    # Scale letters by dice sides
    result = dict()
    frequent = []
    for k, v in letters.items():
        value = v * sides
        if value < 1:
            value = 1
        result[k] = value

        # Track letter frequency
        for i, l in enumerate(frequent):
            if value > result[l]:
                frequent.insert(i, k)
                break
        else:
            frequent.append(k)

    # Round letters
    rounded = 0
    for k in frequent:
        v = int(round(result[k]))
        result[k] = v
        rounded += v
    if rounded < sides:
        sys.exit('Error: Rounded frequencies are less than dice sides')

    # Reduce most frequent letters so that the letter count matches sides
    for i in range(int(rounded) - sides):
        k = frequent[i]
        result[k] -= 1
        if result[k] == 0:
            sys.exit("Error: '{0}' has frequency of 0".format(k))

    return result


def generate_dice(letters, count):
    """Create and return a list of strings representing dice."""
    dice = []
    for i in range(count):
        dice.append([])

    # Find groups of letters
    scaled_letters = round_letters(letters, count)
    single_letters = []
    multi_letters = []
    for k, v in scaled_letters.items():
        if v == 1:
            single_letters.append(k)
        else:
            for i in range(v):
                multi_letters.append(k)

    # Add single-occurrence letters to dice first
    random.shuffle(single_letters)
    dicei = 0
    for k in single_letters:
        dice[dicei].append(k)
        dicei += 1
        if dicei >= count:
            dicei = 0

    # Add multiple-occurrence letters to dice
    random.shuffle(multi_letters)
    pos = 0
    for d in dice:
        size = 6 - len(d)
        if size > 0:
            d += multi_letters[pos:pos+size]
            pos += size

    # Alphabetize dice
    result = []
    for d in dice:
        result.append(','.join(sorted(d)))
    result.sort()

    return result


def main():
    parser = argparse.ArgumentParser(
            description='Create Tanglet dice from a Tanglet word list')
    parser.add_argument('FILE',
            help='file to analyze for letter frequency')
    parser.add_argument('-b', '--bigrams',
            help='automatically detect bigrams', action='store_true')
    parser.add_argument('-d', '--discard',
            help='discard infreqeunt letters', action='store_true')
    parser.add_argument('-o', '--out', type=str,
            help='place dice in file OUT instead of default file')
    parser.add_argument('-s', '--seed', type=int,
            help='use specified random seed')
    args = parser.parse_args()

    # Seed random number generator
    if args.seed:
        print('Using {0} as random seed'.format(args.seed))
        random.seed(args.seed)
    else:
        random.seed()

    # Read lines from file
    words = []
    try:
        reader = codecs.getreader('utf-8')
        with gzip.open(args.FILE, 'rb') as f:
            words = reader(f, errors='replace').readlines()
    except:
        with open(args.FILE, encoding='utf-8', mode='r') as f:
            words = f.readlines()

    # Create dice
    letters = find_letter_frequency(words, args.bigrams, args.discard)
    small = generate_dice(letters, 16)
    large = generate_dice(letters, 25)

    # Save dice to disk
    outf = 'dice'
    if args.out:
        outf = args.out
    with open(outf, encoding='utf-8', mode='w') as f:
        for d in small:
            f.write(d + '\n')
        f.write('\n')
        for d in large:
            f.write(d + '\n')
    print("Dice generated and placed in file '{0}'".format(outf))

if __name__ == '__main__':
    main()
