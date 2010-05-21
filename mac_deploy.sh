#!/bin/bash

APP='Tanglet'
BUNDLE="$APP.app"
VERSION='1.1.1'

DATA="$BUNDLE/Contents/Resources/"
mkdir -p $DATA
cp -fr data/* $DATA
echo 'Copied dice and word lists'

macdeployqt $BUNDLE -dmg -no-plugins
mv "$APP.dmg" "${APP}_$VERSION.dmg"
