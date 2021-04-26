TEMPLATE = aux

unix:system(command -v sphinx-build >/dev/null 2>&1) {
	# Update pot files and add the new translations to the po files
	unix:system(command -v sphinx-intl >/dev/null 2>&1) {
		update_pot.commands = \
			sphinx-build -b gettext $$PWD/manual $$PWD/manual/gettext \
			&& sphinx-intl update -p $$PWD/manual/gettext -d $$PWD/manual/locale
		QMAKE_EXTRA_TARGETS += update_pot
	}

	# Rebuild QtHelp files if necessary
	LANGUAGES = $$files(manual/locale/*)

	qtPrepareTool(QHELPGENERATOR, qhelpgenerator)

	build_qthelp.input = LANGUAGES
	build_qthelp.output = $$PWD/manual/qthelp/${QMAKE_FILE_BASE}/*
	build_qthelp.depends = $$files($$PWD/manual/locale/${QMAKE_FILE_BASE}/LC_MESSAGES/*.po) $$files($$PWD/manual/*.rst) $$PWD/manual/conf.py
	build_qthelp.commands = \
		sphinx-build -D language='${QMAKE_FILE_BASE}' -b qthelp $$PWD/manual/ $$OUT_PWD/manual/_build/${QMAKE_FILE_BASE} \
		&& $$QHELPGENERATOR $$OUT_PWD/manual/_build/${QMAKE_FILE_BASE}/Tanglet.qhp -o $$PWD/manual/qthelp/${QMAKE_FILE_BASE}/Tanglet.qch \
		&& $$QHELPGENERATOR $$OUT_PWD/manual/_build/${QMAKE_FILE_BASE}/Tanglet.qhcp -o $$PWD/manual/qthelp/${QMAKE_FILE_BASE}/Tanglet.qhc
	build_qthelp.CONFIG += no_link target_predeps

	QMAKE_EXTRA_COMPILERS += build_qthelp
}
