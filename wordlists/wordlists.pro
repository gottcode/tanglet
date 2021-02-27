TEMPLATE = aux

LANGUAGES = ca cs de es en fr he hu nl pl pt pt_BR ru

updatewords.input = LANGUAGES
updatewords.output = $$PWD/../data/${QMAKE_FILE_BASE}/words
updatewords.depends = ${QMAKE_FILE_IN}
updatewords.commands = $$OUT_PWD/../wordlist -l ${QMAKE_FILE_BASE} -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}
updatewords.CONFIG += no_link target_predeps

updatedice.input = LANGUAGES
updatedice.output = $$PWD/../data/${QMAKE_FILE_BASE}/dice
updatedice.depends = $$PWD/../data/${QMAKE_FILE_BASE}/words
updatedice.commands = $$OUT_PWD/../dice -b -d -o ${QMAKE_FILE_OUT} $$PWD/../data/${QMAKE_FILE_BASE}/words
updatedice.CONFIG += no_link target_predeps

QMAKE_EXTRA_COMPILERS += updatewords updatedice
