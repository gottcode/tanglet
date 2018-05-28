#!/bin/sh


echo -n 'Preparing files...'
cd ..

rm -f tanglet.desktop.in
cp tanglet.desktop tanglet.desktop.in
sed -e '/^Name\[/ d' \
	-e '/^GenericName\[/ d' \
	-e '/^Comment\[/ d' \
	-e 's/^Name/_Name/' \
	-e 's/^GenericName/_GenericName/' \
	-e 's/^Comment/_Comment/' \
	-i tanglet.desktop.in

rm -f tanglet.appdata.xml.in
cp tanglet.appdata.xml tanglet.appdata.xml.in
sed -e '/p xml:lang/ d' \
	-e '/summary xml:lang/ d' \
	-e '/name xml:lang/ d' \
	-e 's/<p>/<_p>/' \
	-e 's/<\/p>/<\/_p>/' \
	-e 's/<summary>/<_summary>/' \
	-e 's/<\/summary>/<\/_summary>/' \
	-e 's/<name>/<_name>/' \
	-e 's/<\/name>/<\/_name>/' \
	-i tanglet.appdata.xml.in

rm -f tanglet.xml.in
cp tanglet.xml tanglet.xml.in
sed -e '/comment xml:lang/ d' \
	-e 's/<comment>/<_comment>/' \
	-e 's/<\/comment>/<\/_comment>/' \
	-i tanglet.xml.in

cd po
echo ' DONE'


echo -n 'Updating translations...'
for POFILE in *.po;
do
	echo -n " $POFILE"
	msgmerge --quiet --update --backup=none $POFILE description.pot
done
echo ' DONE'


echo -n 'Merging translations...'
cd ..

intltool-merge --quiet --desktop-style po tanglet.desktop.in tanglet.desktop
rm -f tanglet.desktop.in

intltool-merge --quiet --xml-style po tanglet.appdata.xml.in tanglet.appdata.xml
echo >> tanglet.appdata.xml
rm -f tanglet.appdata.xml.in

intltool-merge --quiet --xml-style po tanglet.xml.in tanglet.xml
echo >> tanglet.xml
rm -f tanglet.xml.in

echo ' DONE'
