lessThan(QT_VERSION, 5.2) {
	error("Tanglet requires Qt 5.2 or greater")
}

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = tools src
