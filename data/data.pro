TEMPLATE = aux

LANGUAGES = cs de en fr he hu nl pl ru

updatewords.input = LANGUAGES
updatewords.output = $$PWD/${QMAKE_FILE_BASE}/words
updatewords.depends = ${QMAKE_FILE_IN}/wordlist.txt
updatewords.commands = $$OUT_PWD/../wordlist -l ${QMAKE_FILE_BASE} -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}/wordlist.txt
updatewords.CONFIG += no_link target_predeps

updatedice.input = LANGUAGES
updatedice.output = $$PWD/${QMAKE_FILE_BASE}/dice
updatedice.depends = $$PWD/${QMAKE_FILE_BASE}/words
updatedice.commands = $$OUT_PWD/../dice -b -d -o ${QMAKE_FILE_OUT} $$PWD/${QMAKE_FILE_BASE}/words
updatedice.CONFIG += no_link target_predeps

QMAKE_EXTRA_COMPILERS += updatewords updatedice
