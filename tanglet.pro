lessThan(QT_MAJOR_VERSION, 5) {
	error("Tanglet requires Qt 5.2 or greater")
}
equals(QT_MAJOR_VERSION, 5):lessThan(QT_MINOR_VERSION, 2) {
	error("Tanglet requires Qt 5.2 or greater")
}

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = tools resources/words.pro resources/dice.pro src
