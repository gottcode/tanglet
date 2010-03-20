#!/bin/bash

APP='Tanglet'
BUNDLE="$APP.app"
VERSION='1.1.0'

DATA="$BUNDLE/Contents/Resources/"
mkdir -p $DATA/{en,fr}
cp -f data/en/dice "$DATA/en"
cp -f data/en/words "$DATA/en"
cp -f data/fr/dice "$DATA/fr"
cp -f data/fr/words "$DATA/fr"
echo 'Copied dice and word lists'

macdeployqt $BUNDLE -dmg -no-plugins
mv "$APP.dmg" "${APP}_$VERSION.dmg"
