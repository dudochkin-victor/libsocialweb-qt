#!/bin/bash
for XML in interfaces/*.xml; do
	NAME=$(basename ${XML});
	diff -Naur interfaces.orig/${NAME} interfaces/${NAME} > interfaces/patches/${NAME}-qt.patch;
done
