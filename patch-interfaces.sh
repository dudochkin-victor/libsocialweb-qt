#!/bin/bash

cd interfaces;
for PATCH in patches/*; do
	patch -p1 < ${PATCH};
done
cd ..
