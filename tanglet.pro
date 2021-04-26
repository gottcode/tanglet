!versionAtLeast(QT_VERSION, 5.12) {
	error("Tanglet requires Qt 5.12 or greater")
}

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = tools wordlists src doc/help
