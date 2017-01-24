TEMPLATE = aux

LANGUAGES = cs de en fr he hu nl
updatewords.input = LANGUAGES
updatewords.output = $$PWD/../data/${QMAKE_FILE_BASE}/words
updatewords.depends = ${QMAKE_FILE_IN}/words
updatewords.commands = $$OUT_PWD/../wordlist -l ${QMAKE_FILE_BASE} -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}/words
updatewords.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updatewords
