TEMPLATE = aux

LANGUAGES = $$files(../data/*)
updatedice.input = LANGUAGES
updatedice.output = ${QMAKE_FILE_IN}/dice
updatedice.depends = ${QMAKE_FILE_IN}/words
updatedice.commands = $$OUT_PWD/../dice -b -d -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_IN}/words
updatedice.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updatedice
