TEMPLATE = aux

unix:system(command -v sphinx-build >/dev/null 2>&1) {
	# Update pot files and add the new translations to the po files
	unix:system(command -v sphinx-intl >/dev/null 2>&1) {
		update_pot.commands = \
			sphinx-build -b gettext $$PWD $$PWD/gettext \
			&& sphinx-intl update -p $$PWD/gettext -d $$PWD/locales
		QMAKE_EXTRA_TARGETS += update_pot
	}

	# Rebuild HTML files if necessary
	LANGUAGES = $$files(locales/*)

	qtPrepareTool(QHELPGENERATOR, qhelpgenerator)

	build_help.input = LANGUAGES
	build_help.output = $$PWD/html/${QMAKE_FILE_BASE}/*
	build_help.depends = $$files($$PWD/locales/${QMAKE_FILE_BASE}/LC_MESSAGES/*.po) $$files($$PWD/*.rst) $$PWD/conf.py
	build_help.commands = \
		sphinx-build -D language='${QMAKE_FILE_BASE}' -b html $$PWD/ $$OUT_PWD/_build/${QMAKE_FILE_BASE} \
		&& $(MKDIR) $$PWD/html/${QMAKE_FILE_BASE}/ \
		&& $(COPY_FILE) $$OUT_PWD/_build/${QMAKE_FILE_BASE}/*.html $$PWD/html/${QMAKE_FILE_BASE}/ \
		&& $(DEL_FILE) $$PWD/html/${QMAKE_FILE_BASE}/search.html \
		&& $(SED) -i \'/<script/d\' $$PWD/html/${QMAKE_FILE_BASE}/*.html \
		&& $(SED) -i \'/search.html/d\' $$PWD/html/${QMAKE_FILE_BASE}/*.html
	build_help.CONFIG += no_link target_predeps

	QMAKE_EXTRA_COMPILERS += build_help
}
