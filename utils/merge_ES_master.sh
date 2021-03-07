#!/bin/bash

# Helper script to support merging code from ES master into ES Stories.
# This script automatically re-applies some modifications from ES Stories that need to be done on every merge from ES master.
ES_MASTER=$1
TMP_FILE="./see.txt"
FILENAME_PATTERN_GREP="[./a-zA-Z0-9 ]+"
FILENAME_PATTERN_SED="[./a-zA-Z0-9 ]\+"

if [ "$1" == "" ]
then
	echo "Need to specifiy the ES master branch as parameter!"
	exit 1
fi

echo "Enable auto-merger ours driver for use with data-files, image-files and sound-files"
git config --local merge.ours.driver true

echo "Performing basic merge (from ES master in branch \"${ES_MASTER}\" to ESCW in current/active branch)"
git merge ${ES_MASTER} > "${TMP_FILE}"
echo "Re-Applying ES-Stories deletions."
IFS="
"
cat "${TMP_FILE}" |\
	grep -E \
		-e "^CONFLICT \(modify/delete\): ${FILENAME_PATTERN_GREP} deleted in HEAD and modified in .*$" \
		-e "^CONFLICT \(rename/delete\): ${FILENAME_PATTERN_GREP} deleted in HEAD and renamed to ${FILENAME_PATTERN_GREP} in .*$" |\
	grep -E -e "^.*Version ${ES_MASTER} of ${FILENAME_PATTERN_GREP} left in tree.$" |\
	sed "s,^.*Version ${ES_MASTER} of \(${FILENAME_PATTERN_SED}\) left in tree.$,\1," |\
while read -r fileToDelete
do
	echo "(Re)removing ${fileToDelete}"
	rm "$fileToDelete"
	git add "$fileToDelete"
	sed -i "\,${fileToDelete},d" "${TMP_FILE}"
done

echo ""
echo "Resolve images and sounds using ours"
echo "^CONFLICT (content): Merge conflict in ${FILENAME_PATTERN_GREP}.png$"
cat "${TMP_FILE}" |\
	grep -E \
		-e "^CONFLICT \(content\): Merge conflict in ${FILENAME_PATTERN_GREP}.png$" |\
	sed "s,^CONFLICT (content): Merge conflict in \(${FILENAME_PATTERN_SED}\)$,\1," |\
while read -r fileToUse
do
	echo "Using ES-Stories artwork for ${fileToUse}"
	git add "${fileToUse}"
	sed -i "\,${fileToUse},d" "${TMP_FILE}"
done

echo ""
echo "The following is still left for you to handle:"
cat "${TMP_FILE}"
