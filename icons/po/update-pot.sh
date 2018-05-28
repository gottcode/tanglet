#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f tanglet.desktop.in
cp tanglet.desktop tanglet.desktop.in
sed -e '/^Name\[/ d' \
	-e '/^GenericName\[/ d' \
	-e '/^Comment\[/ d' \
	-e '/^Icon/ d' \
	-e '/^Keywords/ d' \
	-i tanglet.desktop.in

rm -f tanglet.appdata.xml.in
cp tanglet.appdata.xml tanglet.appdata.xml.in
sed -e '/p xml:lang/ d' \
	-e '/summary xml:lang/ d' \
	-e '/name xml:lang/ d' \
	-e '/<developer_name>/ d' \
	-i tanglet.appdata.xml.in

rm -f tanglet.xml.in.h
rm -f tanglet.xml.in
cp tanglet.xml tanglet.xml.in
sed -e '/comment xml:lang/ d' \
	-e 's/<comment>/<_comment>/' \
	-e 's/<\/comment>/<\/_comment>/' \
	-i tanglet.xml.in
intltool-extract --quiet --type=gettext/xml tanglet.xml.in
rm -f tanglet.xml.in

cd po
echo ' DONE'


echo -n 'Extracting messages...'
xgettext --from-code=UTF-8 --output=description.pot \
	--package-name='Tanglet' --copyright-holder='Graeme Gott' \
	../*.in
xgettext --from-code=UTF-8 -j --keyword=N_:1 --output=description.pot \
	--package-name='Tanglet' --copyright-holder='Graeme Gott' \
	../*.h
sed 's/CHARSET/UTF-8/' -i description.pot
echo ' DONE'


echo -n 'Cleaning up...'
cd ..

rm -f tanglet.desktop.in
rm -f tanglet.appdata.xml.in
rm -f tanglet.xml.in.h

echo ' DONE'
