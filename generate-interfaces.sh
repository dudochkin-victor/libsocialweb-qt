#!/bin/bash
#qmake
#make $(grep "interface.h: interfaces" Makefile  | grep -v qdbus | awk ' { print $1 } ' | sed 's/\:$//')
for XML in interfaces/*.xml; do
	NAME=$(basename ${XML} .xml);
	qdbusxml2cpp -N -i swclientdbustypes.h -p interfaces/${NAME}_interface.h:interfaces/${NAME}_interface.cpp ${XML};
	if [[ $? -ne 0 ]]; then
		echo "Error in ${XML}"
	fi
done
#Fix the #ifndef/#define tokens that qdbusxml2cpp generates - if the source XML filename contains a -, then
#the token will contain a -, which is invalid in a defined token
sed -i '/ifndef/,+1s/-/_/g' interfaces/*_interface.h

for FILE in $(grep -B 1 In[01] interfaces/*.xml | grep signal | awk ' BEGIN { FS="- " }; {print $1}' | sort | uniq); do
	HFILE="interfaces/$(basename ${FILE} .xml)_interface.h";
	echo "Patching 'In' signal signature to 'Out' in ${HFILE}:";
	for SIGNAL in $(grep -B 1 In[01] ${FILE} | grep signal | awk ' BEGIN{ FS="\"" }; { print $2 } '); do
		echo "  Patching signal ${SIGNAL}";
		sed -i "/void ${SIGNAL}/{s/(/(const /;s/ \([a-zA-Z0-9_]*\));$/ \&\1);/}" ${HFILE};
	done;
done
